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

inline int is_in_bounds(Vector2 l, Vector2 bounds) {
  return (l.x >= 0) && (l.y >= 0) && (l.x < bounds.x) && (l.y < bounds.y);
}

WaveFunction new_wave_function(void) {
  WaveFunction wave;

  wave.allocated = 8;
  wave.superposition = calloc(wave.allocated, sizeof(int));
  wave.updated = 1;
  wave.comparison = (Vector2){.x = 0, .y = 0};
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
    wave->superposition =
        realloc(wave->superposition, wave->allocated * sizeof(int));
  }

  wave->superposition[wave->size] = n;

  wave->size++;
}

WaveFunction clone_wave(WaveFunction wave) {
  WaveFunction clone = new_wave_function();

  for (int i = 0; i < wave.size; i++)
    append(&clone, wave.superposition[i]);

  return clone;
}

void free_wave(WaveFunction wave) { free(wave.superposition); }

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
            realloc(content->id_locations[id], allocated[id] * sizeof(Vector2));
      }

      content->id_locations[id][content->id_locations_size[id] - 1] =
          (Vector2){.x = i, .y = j};
    }

  free(allocated);

  return rule;
}

Grid new_grid(const int **input, const Vector2 input_size,
              const Vector2 output_size) {
  Grid grid = {.input = input,
               .input_size = input_size,
               .output_size = output_size,
               .is_max_entropy = 1};

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

int update_wave_function(Grid *grid, const Vector2 l) {
  if (!is_in_bounds(l, grid->output_size))
    return 1;

  WaveFunction *a, *b;

  a = get_tile(grid, l);
  b = get_tile(grid, a->comparison);

  switch (grid->rule.type) {
  case 0: {
    AdjacentRule *rule = grid->rule.content;

    const Vector2 delta = {.x = a->comparison.x - l.x,
                           .y = a->comparison.y - l.y};

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
      int is_in_a = 0;

      for (int k = 0; k < old_a.size; k++)
        if (old_a.superposition[k] == state) {
          is_in_old_a = 1;
          break;
        }

      for (int k = 0; k < a->size; k++)
        if (a->superposition[k] == state) {
          is_in_a = 1;
          break;
        }

      if (is_in_old_a && !is_in_a)
        append(a, state);
    }
  }

  return !a->size * 2;
}

Vector2 *populate_queue(Grid *grid, Vector2 epicentre) {
  for (int i = 0; i < grid->output_size.x; i++)
    for (int j = 0; j < grid->output_size.y; j++)
      grid->buffer[i][j].updated = 0;

  Vector2 *update_queue, *current_cells, *recently_added;
  int queue_size, current_size, recent_size;

  update_queue =
      calloc(grid->output_size.x * grid->output_size.y, sizeof(Vector2));
  current_cells =
      calloc(grid->output_size.x * grid->output_size.y, sizeof(Vector2));
  recently_added =
      calloc(grid->output_size.x * grid->output_size.y, sizeof(Vector2));

  queue_size = 0;
  current_size = 1;
  recent_size = 0;

  get_tile(grid, epicentre)->updated = 1;
  current_cells[0] = epicentre;

  while (queue_size != grid->output_size.x * grid->output_size.y) {
    for (int i = 0; i < current_size; i++) {
      Vector2 location = current_cells[i];

      for (int i = 0; i < 4; i++) {
        Vector2 next = location;
        next.x += offsets[i].x;
        next.y += offsets[i].y;

        WaveFunction *wave = get_tile(grid, next);

        if (!is_in_bounds(next, grid->output_size) || wave->updated)
          continue;

        wave->comparison = location;
        wave->updated = 1;

        recently_added[recent_size] = next;
        recent_size++;
      }
    }

    for (int i = 0; i < current_size; i++, queue_size++)
      update_queue[queue_size] = current_cells[i];

    Vector2 *temp = current_cells;

    current_cells = recently_added;
    recently_added = temp;

    // swap(current_cells, recently_added);

    current_size = recent_size;
    recent_size = 0;
  }

  free(current_cells);
  free(recently_added);

  return update_queue;
}

inline void swap(void *x, void *y) {
  x = (void *)((long)x ^ (long)y);
  y = (void *)((long)y ^ (long)x);
  x = (void *)((long)x ^ (long)y);
}

void collapse_wave_function(Grid *grid) {
  Vector2 l;

  const int grid_n = grid->output_size.x * grid->output_size.y;

  if (grid->is_max_entropy) {
    l.x = rand() % grid->output_size.x;
    l.y = rand() % grid->output_size.y;
  } else {
    l = get_min_entropy(grid);
  }

  WaveFunction *wave = get_tile(grid, l);

  WaveFunction old_wave = clone_wave(*wave);

  wave->superposition[0] = wave->superposition[rand() % wave->size];
  wave->size = 1;

  Vector2 *update_queue = populate_queue(grid, l);

  WaveFunction *old_waves = calloc(sizeof(WaveFunction), grid_n - 1);

  int i = 1;
  for (; i < grid_n; i++) {
    WaveFunction *wave = get_tile(grid, update_queue[i]);

    old_waves[i - 1] = clone_wave(*wave);

    int result = update_wave_function(grid, update_queue[i]);

    if (result) {
      for (; i > 1; i--) {
        Vector2 location = update_queue[i];

        free_wave(*get_tile(grid, location));

        *get_tile(grid, location) = clone_wave(old_waves[i - 1]);
      }

      free_wave(*wave);

      *wave = clone_wave(old_wave);

      break;
    }
  }

  for (int i = 0; i < grid_n - 1; i++)
    free_wave(old_waves[i]);

  free(old_waves);
  free_wave(old_wave);
  free(update_queue);
}

int **generate(Grid *grid) {
  int **buffer;

CHECK_COMPLETE:

  for (int i = 0; i < grid->output_size.x; i++)
    for (int j = 0; j < grid->output_size.y; j++)
      if (grid->buffer[i][j].size > 1)
        goto GRID_NOT_COMPLETE;

  goto GRID_COMPLETE;

GRID_NOT_COMPLETE:

  collapse_wave_function(grid);

  goto CHECK_COMPLETE;

GRID_COMPLETE:

  buffer = calloc(grid->output_size.x, sizeof(int));

  for (int i = 0; i < grid->output_size.x; i++) {
    buffer[i] = calloc(grid->output_size.y, sizeof(int));

    for (int j = 0; j < grid->output_size.y; j++) {
      buffer[i][j] = grid->buffer[i][j].superposition[0];
      free(grid->buffer[i][j].superposition);
    }
  }

  return buffer;
}
