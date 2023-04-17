#pragma once

#define INITIAL_PREALLOCATION 100

typedef struct {
  void* allocation;
  int size;
  int preallocated;
  const int type_size;
  const int a;
} Vector;

Vector new_vector(const int type_size);
Vector __new_vector_explict(const int type_size, const int a);
void free_vector(Vector v);

typedef struct {
  int *superposition;
} WaveFunction;
