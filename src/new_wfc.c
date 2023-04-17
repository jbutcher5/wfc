#include "new_wfc.h"
#include "math.h"
#include <stdio.h>
#include <stdlib.h>

int __next_preallocation(const Vector v) {
  return v.a * log(v.size) + v.preallocated;
}

Vector __new_vector_explict(const int type_size, const int a) {
  void *allocation = calloc(INITIAL_PREALLOCATION, type_size);

  if (!allocation)
    fprintf(stderr, "Failed to allocate space for vector.");

  Vector v = {.allocation = allocation,
              .size = 0,
              .preallocated = INITIAL_PREALLOCATION,
              .type_size = type_size,
              .a = a};

  return v;
}

Vector new_vector(const int type_size) {
  return __new_vector_explict(type_size, 100);
}
