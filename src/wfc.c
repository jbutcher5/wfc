#include "wfc.h"
#include <stdlib.h>

#define ID_LOCATIONS_INITIAL_INCREMENT 8

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

  return rule;
}

Grid new_grid(int **input, Vector2 input_size, Vector2 output_size) {
  Grid grid;

  grid.input = input;
  grid.input_size = input_size;
  grid.output_size = output_size;
  grid.buffer = (WaveFunction **)calloc(output_size.x, sizeof(WaveFunction *));

  for (int i = 0; i < output_size.x; i++)
    grid.buffer[i] =
        (WaveFunction *)calloc(output_size.y, sizeof(WaveFunction));

  grid.max_id = get_max_id(input, input_size);

  return grid;
}
