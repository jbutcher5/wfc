#pragma once

typedef struct {
  int x;
  int y;
} Vector2;

typedef struct {
  void *content;
  int type;
} Rule;

typedef struct {
  Vector2 **id_locations;
  int *id_locations_size;
} AdjacentRule;

typedef struct {
  int *superposition;
  int size;
  int allocated;
} WaveFunction;

typedef struct {
  WaveFunction **buffer;

  const int **input;
  const Vector2 input_size;
  const Vector2 output_size;

  int max_id;

  Rule rule;
} Grid;

WaveFunction new_wave_function();
WaveFunction iota(int n);
void append(WaveFunction *wave, int n);

Rule preprocessor(int **input, const Vector2 size);
Grid new_grid(const int **input, const Vector2 input_size,
              const Vector2 output_size);
Vector2 get_min_entropy(const Grid *grid);
WaveFunction *get_tile(const Grid *grid, const Vector2 l);
int is_complete(const Grid *grid);
void populate_queue(Grid *grid);
int update_wave_function(Grid *grid, const Vector2 l, const Vector2 delta);
void collapse_wave_function(Grid *grid);
int **generate(Grid **grid);
int apply_adj_rule(Grid *grid, AdjacentRule *rule, WaveFunction *a,
                   WaveFunction *b, const Vector2 delta);
