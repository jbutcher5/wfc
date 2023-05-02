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
  int updated;
  Vector2 comparison;
} WaveFunction;

typedef struct {
  WaveFunction **buffer;

  const int **input;
  const Vector2 input_size;
  const Vector2 output_size;

  int is_max_entropy;

  int max_id;

  Rule rule;
} Grid;

WaveFunction new_wave_function(void);
WaveFunction iota(int n);
void append(WaveFunction *wave, int n);
WaveFunction clone_wave(WaveFunction wave);
void free_wave(WaveFunction wave);

int get_max_id(const int **input, const Vector2 size);
Rule preprocessor(int **input, const Vector2 size);
Grid new_grid(const int **input, const Vector2 input_size,
              const Vector2 output_size);
Vector2 get_min_entropy(const Grid *grid);
WaveFunction *get_tile(const Grid *grid, const Vector2 l);
int is_complete(const Grid *grid);
Vector2 *populate_queue(Grid *grid, Vector2 epicentre);
int update_wave_function(Grid *grid, const Vector2 l);
void collapse_wave_function(Grid *grid);
int **generate(Grid *grid);
int apply_adj_rule(Grid *grid, AdjacentRule *rule, WaveFunction *a,
                   WaveFunction *b, const Vector2 delta);
int is_in_bounds(Vector2 l, Vector2 bounds);
void swap(void *x, void *y);
