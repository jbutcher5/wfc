#include "wfc.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(void) {

  srand(time(NULL));

  int **input = calloc(3, sizeof(int));

  for (int i = 0; i < 3; i++)
    input[i] = calloc(3, sizeof(int));

  input[0][0] = 0;
  input[0][1] = 1;
  input[0][2] = 0;

  input[1][0] = 0;
  input[1][1] = 1;
  input[1][2] = 0;

  input[2][0] = 0;
  input[2][1] = 1;
  input[2][2] = 0;

  Grid grid =
      new_grid(input, (Vector2){.x = 3, .y = 3}, (Vector2){.x = 3, .y = 3});

  int **output = generate(&grid);

  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      printf("%d ", output[j][i]);
    }
    puts("");
  }

  return 0;
}
