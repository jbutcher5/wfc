#pragma once

typedef struct {
  int *buffer;
  int size;
  int type;
} Rule;

typedef struct {
  int *superposition;
  int size;
} WaveFunction;

typedef struct {
  int **buffer;

  int **input;
  int input_width;
  int input_height;

  int output_width;
  int output_height;

  Rule rule;
} Grid;
