#include "wfc.h"
#include <stdlib.h>

#define ID_LOCATIONS_INITIAL_INCREMENT 8

const Vector2 offsets[4] = {
    {.x = 0, .y = 1}, {.x = 1, .y = 0}, {.x = 0, .y = -1}, {.x = -1, .y = 0}};

inline int get_max_id(const int **input, const Vector2 size) {
  int max = 0;

  for (int i = 0; i < size.x; i++)
    for (int j = 0; j < size.y; j++) {
      int current = input[i][j];

      if (current > max)
        max = current;
    }

  return max;
}

WaveFunction new_wave_function() {
  WaveFunction wave;

  wave.allocated = 8;
  wave.superposition = calloc(wave.allocated, sizeof(int));

  wave.size = 0;

  return wave;
}

WaveFunction iota(int n) {
  WaveFunction wave = new_wave_function();

  for (int i = 0; i <= n; i++)
    append(&wave, i);

  return wave;
}

void append(WaveFunction *wave, int n) {
  if (wave->size + 1 >= wave->allocated) {
    wave->allocated += 8;
    wave->superposition = realloc(wave->superposition, wave->allocated);
  }

  wave->superposition[wave->size] = n;

  wave->size++;
}

Rule new_adjacent_rule(const int **input, const Vector2 input_size,
                       const int max_id) {

  Rule rule;
  rule.content = malloc(sizeof(AdjacentRule));
  rule.type = 0;

  AdjacentRule *content = rule.content;
  content->id_locations = calloc(max_id + 1, sizeof(Vector2 *));

  int *allocated = (int *)calloc(max_id + 1, sizeof(int));

  content->id_locations_size = calloc(max_id + 1, sizeof(int));

  int id;
  for (int i = 0; i < input_size.x; i++)
    for (int j = 0; j < input_size.y; j++) {
      id = input[i][j];

      content->id_locations_size[id]++;

      if (allocated[id] <= content->id_locations_size[id]) {
        allocated[id] += ID_LOCATIONS_INITIAL_INCREMENT;
        content->id_locations[id] =
            realloc(content->id_locations[id], allocated[id]);
      }

      content->id_locations[id][content->id_locations_size[id] - 1] =
          (Vector2){.x = i, .y = j};
    }

  free(allocated);

  return rule;
}

Grid new_grid(const int **input, const Vector2 input_size,
              const Vector2 output_size) {
  Grid grid = {
      .input = input, .input_size = input_size, .output_size = output_size};

  grid.buffer = (WaveFunction **)calloc(output_size.x, sizeof(WaveFunction *));

  grid.max_id = get_max_id(input, input_size);

  for (int i = 0; i < output_size.x; i++) {
    grid.buffer[i] =
        (WaveFunction *)calloc(output_size.y, sizeof(WaveFunction));
    for (int j = 0; j < output_size.y; j++)
      grid.buffer[i][j] = iota(grid.max_id);
  }

  grid.rule = new_adjacent_rule(input, input_size, grid.max_id);

  return grid;
}

inline WaveFunction *get_tile(const Grid *grid, const Vector2 l) {
  return grid->buffer[l.x] + l.y;
}

Vector2 get_min_entropy(const Grid *grid) {
  Vector2 current_lowest = {.x = 0, .y = 0};
  int current, lowest_entropy;

  for (int i = 0; i < grid->output_size.x; i++)
    for (int j = 0; j < grid->output_size.y; j++) {
      current = grid->buffer[i][j].size;

      lowest_entropy = get_tile(grid, current_lowest)->size;

      if (current == 2)
        return (Vector2){.x = i, .y = j};

      if ((current < lowest_entropy && current > 1) || lowest_entropy == 1)
        current_lowest = (Vector2){.x = i, .y = j};
    }

  return current_lowest;
}

int is_complete(const Grid *grid) {
  for (int i = 0; i < grid->output_size.x; i++)
    for (int j = 0; j < grid->output_size.y; j++)
      if (grid->buffer[i][j].size > 1)
        return 0;

  return 1;
}

int update_wave_function(Grid *grid, const Vector2 l, const Vector2 delta) {
  if (l.x < 0 || l.x >= grid->output_size.x || l.y < 0 ||
      l.y >= grid->output_size.y)
    return 1;

  WaveFunction *a, *b;

  a = get_tile(grid, l);
  b = get_tile(grid, (Vector2){l.x + delta.x, l.x + delta.y});

  switch (grid->rule.type) {
  case 0: {
    AdjacentRule *rule = grid->rule.content;

    return apply_adj_rule(grid, rule, a, b, delta);
  }
  default:
    return 3;
  }

  return 0;
}

int apply_adj_rule(Grid *grid, AdjacentRule *rule, WaveFunction *a,
                   WaveFunction *b, const Vector2 delta) {

  WaveFunction old_a = *a;
  *a = new_wave_function();

  for (int i = 0; i < b->size; i++) {
    int id = b->superposition[i];

    for (int j = 0; j < rule->id_locations_size[id]; j++) {
      Vector2 adj_location = rule->id_locations[id][j];
      adj_location.x += delta.x;
      adj_location.y += delta.y;

      if (adj_location.x < 0 || adj_location.y < 0 ||
          adj_location.x >= grid->output_size.x ||
          adj_location.y >= grid->output_size.y)
        continue;

      int state = grid->input[adj_location.x][adj_location.y];

      int is_in_old_a = 0;

      for (int k = 0; k < old_a.size; k++)
        if (old_a.superposition[k] == state) {
          is_in_old_a = 1;
          break;
        }

      if (is_in_old_a)
        append(a, state);
    }
  }

  return !a->size * 2;
}
