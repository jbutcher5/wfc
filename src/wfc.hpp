#include "wfc.hpp"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <vector>

const Location offsets[4] = {Location(0, 1), Location(1, 0), Location(0, -1),
                             Location(-1, 0)};

Location TileGeneration::GetMinEntropy() {
  Location current_lowest = Location();
  int current;

  for (int i = 0; i < output_width; i++) {
    for (int j = 0; j < output_height; j++) {
      int current = buffer[i][j].Entropy();

      int lowest_entropy = GetTile(current_lowest)->Entropy();

      if (current > 1 && drand48() > 0.8)
        return Location(i, j);

      if ((current < lowest_entropy && current > 1) || lowest_entropy == 1)
        current_lowest = Location(i, j);
    }
  }

  return current_lowest;
};

bool TileGeneration::GenerationComplete() {
  for (int i = 0; i < output_width; i++)
    for (int j = 0; j < output_height; j++)
      if (buffer[i][j].Entropy() > 1)
        return false;

  return true;
}

TileGeneration::TileGeneration(int output_width, int output_height, int max_id,
                               int **input, int input_width, int input_height)
    : output_width(output_width), output_height(output_height),
      input_width(input_width), input_height(input_height), input(input),
      max_id(max_id) {
  buffer = (WaveFunction **)calloc(output_width, sizeof(void *));

  for (int i = 0; i < output_width; i++) {
    buffer[i] = (WaveFunction *)calloc(output_height, sizeof(WaveFunction));

    for (int j = 0; j < output_height; j++)
      buffer[i][j] = WaveFunction(max_id);
  }

  id_locations = (std::vector<Location> *)calloc(max_id + 1,
                                                 sizeof(std::vector<Location>));

  for (int i = 0; i < input_width; i++) {
    for (int j = 0; j < input_height; j++) {
      id_locations[input[i][j]].push_back(Location(i, j));
    }
  }
}

void TileGeneration::PopulateQueue(Location initial) {
  for (int i = 0; i < output_width; i++)
    for (int j = 0; j < output_height; j++)
      buffer[i][j].updated = false;

  update_queue = {};
  std::vector<Location> current_cells = {initial};
  std::vector<Location> recently_added = {};

  GetTile(initial)->updated = true;

  while (update_queue.size() != output_height * output_width) {

    for (Location current_cell : current_cells) {
      for (int i = 0; i < 4; i++) {
        Location next_tile = current_cell;
        next_tile.x += offsets[i].x;
        next_tile.y += offsets[i].y;

        if (next_tile.x < 0 || next_tile.x >= output_width || next_tile.y < 0 ||
            next_tile.y >= output_height)
          continue;

        WaveFunction *next = GetTile(next_tile);

        if (!next->updated) {
          recently_added.push_back(next_tile);
          next->comparison = current_cell;
          next->updated = true;
        }
      }
    }

    for (Location l : current_cells)
      update_queue.push_back(l);

    current_cells = {};

    for (Location l : recently_added)
      current_cells.push_back(l);

    recently_added = {};
  }
}

int TileGeneration::UpdateTile(Location l, const Location comparee) {
  if (l.x < 0 || l.x >= output_width || l.y < 0 || l.y >= output_height)
    return 1;

  Location delta = l;
  delta.x -= comparee.x;
  delta.y -= comparee.y;

  WaveFunction *current = GetTile(l);
  WaveFunction *other = GetTile(comparee);

  std::vector<int> new_combination;
  int *contributions = (int *)calloc(max_id, sizeof(int));

  for (int n : other->combinations) {
    for (Location id_location : id_locations[n]) {
      id_location.x += delta.x;
      id_location.y += delta.y;

      if (id_location.x < 0 || id_location.x >= input_width ||
          id_location.y < 0 || id_location.y >= input_height)
        continue;

      int new_state = input[id_location.x][id_location.y];

      bool is_in_new_combinations = false;
      bool is_in_combinations = false;

      // TODO: Replace with sort and binary search

      for (int state : new_combination)
        if (state == new_state) {
          is_in_new_combinations = true;
          break;
        }

      for (int state : current->combinations)
        if (state == new_state) {
          is_in_combinations = true;
          break;
        }

      if (!is_in_new_combinations && is_in_combinations) {
        contributions[n]++;
        new_combination.push_back(new_state);
      }
    }
  }

  /*
  std::vector<int> comparee_id_remove = {};

  for (int i = 0; i < max_id; i++)
    if (!contributions[i])
      comparee_id_remove.push_back(i);

  free(contributions);

  std::vector<int> new_other_combinations = {};

  for (int n : other->combinations) {
    bool found = false;

    for (int i : comparee_id_remove) {
      if (i == n) {
        found = true;
        break;
      }
    }

    if (!found) {
      new_other_combinations.push_back(n);
    }
  }

  */
  // TODO: Handle collisions rather than brute forcing them

  // if (!new_combination.size() || !new_other_combinations.size())
  if (!new_combination.size())
    return 2;

  // other->combinations = new_other_combinations;
  current->combinations = new_combination;

  return 0;
}

void TileGeneration::CollapseTile() {
  int i = 0;

  Location l;

  if (!get_random_tile)
    l = GetMinEntropy();

  else {
    l.x = rand() % output_width;
    l.y = rand() % output_height;

    get_random_tile = false;
  }

  WaveFunction *tile = GetTile(l);
  int n = rand() % tile->Entropy();

  tile->combinations = {tile->combinations[n]};
  tile->updated = true;

RETRY:
  Location next_tile = l;
  next_tile.x += offsets[i].x;
  next_tile.y += offsets[i].y;

  PopulateQueue(l);

  for (Location l : update_queue) {
    int result = UpdateTile(l, GetTile(l)->comparison);

    if (result == 1) {
      i = (i + 1) % 4;
      goto RETRY;
    } else if (result == 2) {
      get_random_tile = true;

      for (int i = 0; i < output_width; i++) {
        for (int j = 0; j < output_height; j++)
          buffer[i][j] = WaveFunction(max_id);
      }

      return;
    }
  }
}

int **TileGeneration::Generate() {
  while (!GenerationComplete())
    CollapseTile();

  int **output = (int **)calloc(output_width, sizeof(int *));

  for (int i = 0; i < output_width; i++)
    output[i] = (int *)calloc(output_height, sizeof(int));

  for (int i = 0; i < output_width; i++)
    for (int j = 0; j < output_height; j++)
      output[i][j] = buffer[i][j].combinations[0];

  return output;
}

TileGeneration::~TileGeneration() {
  for (int i = 0; i < output_width; i++) {
    free(buffer[i]);
  }

  free(buffer);
}
