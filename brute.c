#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double calc_time(struct timespec *, struct timespec *);

typedef struct matrix_t {
  int n_rows;
  int n_columns;
  int s_row;
  int s_column;
  float **_mat;
} matrix_t;

matrix_t *matrix_create(int, int);
void matrix_delete(matrix_t **);

void matrix_index_set(matrix_t *, int, int, float);
float matrix_index_get(matrix_t *, int, int);

matrix_t *matrix_mul_std(matrix_t *, matrix_t *);

int main() {
  int n;
  scanf("%d", &n);

  matrix_t *A = matrix_create(n, n);
  matrix_t *B = matrix_create(n, n);

  int r;
  for(int i = 0; i < n; i++) {
    for(int j = 0; j < n; j++) {
      //scanf("%d", &r);
	  r=rand()%1000;
      matrix_index_set(A, i, j, r);
    }
  }

  for(int i = 0; i < n; i++) {
    for(int j = 0; j < n; j++) {
      //scanf("%d", &r);
        r=rand()%1000;
      matrix_index_set(B, i, j, r);
    }
  }

  struct timespec start, stop;

  clock_gettime(CLOCK_REALTIME, &start);
  matrix_t *C = matrix_mul_std(A, B);
  clock_gettime(CLOCK_REALTIME, &stop);

  printf("%lf\n", calc_time(&start, &stop));

  matrix_delete(&A);
  matrix_delete(&B);
  matrix_delete(&C);

  return 0;
}

matrix_t *matrix_create(int n_rows, int n_columns) {
  if(n_rows <= 0 || n_columns <= 0) {
    return NULL;
  }

  matrix_t *self = (matrix_t *) malloc(sizeof(matrix_t));
  if(self == NULL) {
    printf("Out of memory\n");
    return NULL;

  }

  self->n_rows = n_rows;
  self->n_columns = n_columns;
  self->s_row = 0;
  self->s_column = 0;

  self->_mat = (float **) malloc(n_rows * sizeof(float *));
  if(self == NULL) {
    printf("Out of memory\n");
    free(self);
    return NULL;
  }

  for(int i = 0; i < n_rows; i++) {
    self->_mat[i] = (float *) malloc(n_columns * sizeof(float));
    if(self->_mat[i] == NULL) {
      printf("Out of memory\n");
      for(int j = 0; j < i; j++) {
        free(self->_mat[j]);
      }
      free(self->_mat);
      free(self);
    }
  }

  for(int i = 0; i < self->n_rows; i++) {
    for(int j = 0; j < self->n_columns; j++) {
      matrix_index_set(self, i, j, 0.0f);
    }
  }

  return self;
}

void matrix_delete(matrix_t **self_ptr) {
  // do i have to check if self_ptr is NULL
  if(self_ptr == NULL) {
    return;
  }

  matrix_t *self = *self_ptr;
  if(self == NULL) {
    return;
  }

  for(int i = 0; i < self->n_rows; i++) {
    free(self->_mat[i]);
  }

  free(self->_mat);
  free(self);

  *self_ptr = NULL;
}

void matrix_index_set(matrix_t *self, int i, int j, float el) {
  if(self == NULL) {
    return;
  }

  if(i > self->n_rows || j > self->n_columns) {
    return;
  }

  self->_mat[self->s_row + i][self->s_column + j] = el;
}

float matrix_index_get(matrix_t *self, int i, int j) {
  if(self == NULL) {
    return -1;
  }

  if(i > self->n_rows || j > self->n_columns) {
    printf("Invalid dimensions\n");
    return -1;
  }

  return self->_mat[self->s_row + i][self->s_column + j];
}

matrix_t *matrix_mul_std(matrix_t *A, matrix_t *B) {
  if(A == NULL || B == NULL) {
    return NULL;
  }

  if(A->n_columns != B->n_rows) {
    return NULL;
  }

  matrix_t *C = matrix_create(A->n_rows, B->n_columns);
  if(C == NULL) {
    return NULL;
  }

  float curr_val;
  for(int i = 0; i < A->n_rows; i++) {
    for(int j = 0; j < B->n_columns; j++) {
      for(int k = 0; k < A->n_columns; k++) {
        curr_val = matrix_index_get(C, i, j);
        matrix_index_set(C, i, j, curr_val + (matrix_index_get(A, i, k) * matrix_index_get(B, k, j)));
      }
    }
  }

  return C;
}

double calc_time(struct timespec *start, struct timespec *stop) {
  double time = (stop->tv_sec - start->tv_sec) + ((stop->tv_nsec - start->tv_nsec) * 1.0 / 1000000000);
  return time;
}
