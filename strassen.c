#include <stdlib.h>
#include <stdio.h>
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

matrix_t *matrix_slice(matrix_t *, int, int, int, int);
void matrix_slice_delete(matrix_t **);

matrix_t *matrix_rand(int, int);

void matrix_index_set(matrix_t *, int, int, float);
float matrix_index_get(matrix_t *, int, int);

matrix_t *matrix_add(matrix_t *, matrix_t *);
matrix_t *matrix_sub(matrix_t *, matrix_t *);

matrix_t *matrix_mul_std(matrix_t *, matrix_t *);
matrix_t *matrix_mul_strassen(matrix_t *, matrix_t *);

int main() {
  int n;
  scanf("%d", &n);

  matrix_t *A = matrix_create(n, n);
  matrix_t *B = matrix_create(n, n);

  int r;

  for(int i = 0; i < n; i++) {
    for(int j = 0; j < n; j++) {
//      scanf("%d", &r);
        r=rand()%1000;
      matrix_index_set(A, i, j, r);
    }
  }

  for(int i = 0; i < n; i++) {
    for(int j = 0; j < n; j++) {
//      scanf("%d", &r);
        r=rand()%1000;
      matrix_index_set(B, i, j, r);
    }
  }

  struct timespec start, stop;

  clock_gettime(CLOCK_REALTIME, &start);
  matrix_t *C = matrix_mul_strassen(A, B);
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

matrix_t *matrix_slice(matrix_t *self, int row_start, int n_rows, int column_start, int n_columns) {
  if(self == NULL) {
    return NULL;
  }

  matrix_t *slice = (matrix_t *) malloc(sizeof(matrix_t));
  if(slice == NULL) {
    printf("Out of memory\n");
    return NULL;
  }
  slice->_mat = self->_mat;

  slice->n_rows = n_rows;
  slice->n_columns = n_columns;

  slice->s_row = self->s_row + row_start;
  slice->s_column = self->s_column + column_start;

  return slice;
}

void matrix_slice_delete(matrix_t **self_ptr) {
  if(self_ptr == NULL) {
    return;
  }

  matrix_t *self = *self_ptr;
  if(self == NULL) {
    return;
  }
  free(self);

  *self_ptr = NULL;
}

void matrix_delete(matrix_t **self_ptr) {
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

matrix_t *matrix_add(matrix_t *A, matrix_t *B) {
  if(A == NULL || B == NULL) {
    return NULL;
  }

  if(A->n_rows != B->n_rows || A->n_columns != B->n_columns) {
    return NULL;
  }

  matrix_t *C = matrix_create(A->n_rows, A->n_columns);
  if(C == NULL) {
    return NULL;
  }

  for(int i = 0; i < C->n_rows; i++) {
    for(int j = 0; j < C->n_columns; j++) {
      matrix_index_set(C, i, j, matrix_index_get(A, i, j) + matrix_index_get(B, i, j));
    }
  }

  return C;
}

matrix_t *matrix_sub(matrix_t *A, matrix_t *B) {
  if(A == NULL || B == NULL) {
    return NULL;
  }

  if(A->n_rows != B->n_rows || A->n_columns != B->n_columns) {
    return NULL;
  }

  matrix_t *C = matrix_create(A->n_rows, A->n_columns);
  if(C == NULL) {
    return NULL;
  }

  for(int i = 0; i < C->n_rows; i++) {
    for(int j = 0; j < C->n_columns; j++) {
      matrix_index_set(C, i, j, matrix_index_get(A, i, j) - matrix_index_get(B, i, j));
    }
  }

  return C;
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

matrix_t *matrix_mul_strassen(matrix_t *A, matrix_t *B) {
  if(A->n_rows <= 16) {
    matrix_t *C = matrix_mul_std(A, B);
    return C;
  } else {
    int half = A->n_rows / 2;
    matrix_t *A11 = matrix_slice(A, 0, half, 0, half);
    matrix_t *A12 = matrix_slice(A, 0, half, half, half);
    matrix_t *A21 = matrix_slice(A, half, half, 0, half);
    matrix_t *A22 = matrix_slice(A, half, half, half, half);

    matrix_t *B11 = matrix_slice(B, 0, half, 0, half);
    matrix_t *B12 = matrix_slice(B, 0, half, half, half);
    matrix_t *B21 = matrix_slice(B, half, half, 0, half);
    matrix_t *B22 = matrix_slice(B, half, half, half, half);

    matrix_t *S1 = matrix_sub(B12, B22);
    matrix_t *S2 = matrix_add(A11, A12);
    matrix_t *S3 = matrix_add(A21, A22);
    matrix_t *S4 = matrix_sub(B21, B11);
    matrix_t *S5 = matrix_add(A11, A22);
    matrix_t *S6 = matrix_add(B11, B22);
    matrix_t *S7 = matrix_sub(A12, A22);
    matrix_t *S8 = matrix_add(B21, B22);
    matrix_t *S9 = matrix_sub(A11, A21);
    matrix_t *S10 = matrix_add(B11, B12);

    matrix_t *P1 = matrix_mul_strassen(A11, S1);
    matrix_t *P2 = matrix_mul_strassen(S2, B22);
    matrix_t *P3 = matrix_mul_strassen(S3, B11);
    matrix_t *P4 = matrix_mul_strassen(A22, S4);
    matrix_t *P5 = matrix_mul_strassen(S5, S6);
    matrix_t *P6 = matrix_mul_strassen(S7, S8);
    matrix_t *P7 = matrix_mul_strassen(S9, S10);

    matrix_t *C111 = matrix_add(P5, P4);
    matrix_t* C112 = matrix_sub(C111, P2);
    matrix_t *C11 = matrix_add(C112, P6);

    matrix_t *C12 = matrix_add(P1, P2);

    matrix_t *C21 = matrix_add(P3, P4);

    matrix_t *C221 = matrix_add(P5, P1);
    matrix_t *C222 = matrix_sub(C221, P3);
    matrix_t *C22 = matrix_sub(C222, P7);

    matrix_t *C = matrix_create(A->n_rows, B->n_columns);

    for(int i = 0, a = 0; i < half; i++, a++) {
      for(int j = 0, b = 0; j < half; j++, b++) {
        matrix_index_set(C, i, j, matrix_index_get(C11, a, b));
      }
    }

    for(int i = 0, a = 0; i < half; i++, a++) {
      for(int j = half, b = 0; j < C->n_columns; j++, b++) {
        matrix_index_set(C, i, j, matrix_index_get(C12, a, b));
      }
    }

    for(int i = half, a = 0; i < C->n_rows; i++, a++) {
      for(int j = 0, b = 0; j < half; j++, b++) {
        matrix_index_set(C, i, j, matrix_index_get(C21, a, b));
      }
    }

    for(int i = half, a = 0; i < C->n_rows; i++, a++) {
      for(int j = half, b = 0; j < C->n_columns; j++, b++) {
        matrix_index_set(C, i, j, matrix_index_get(C22, a, b));
      }
    }

    matrix_slice_delete(&A11);
    matrix_slice_delete(&A12);
    matrix_slice_delete(&A21);
    matrix_slice_delete(&A22);
    matrix_slice_delete(&B11);
    matrix_slice_delete(&B12);
    matrix_slice_delete(&B21);
    matrix_slice_delete(&B22);

    matrix_delete(&S1);
    matrix_delete(&S2);
    matrix_delete(&S3);
    matrix_delete(&S4);
    matrix_delete(&S5);
    matrix_delete(&S6);
    matrix_delete(&S7);
    matrix_delete(&S8);
    matrix_delete(&S9);
    matrix_delete(&S10);

    matrix_delete(&P1);
    matrix_delete(&P2);
    matrix_delete(&P3);
    matrix_delete(&P4);
    matrix_delete(&P5);
    matrix_delete(&P6);
    matrix_delete(&P7);

    matrix_delete(&C111);
    matrix_delete(&C112);
    matrix_delete(&C11);
    matrix_delete(&C12);
    matrix_delete(&C21);
    matrix_delete(&C221);
    matrix_delete(&C222);
    matrix_delete(&C22);

    return C;
  }
}

double calc_time(struct timespec *start, struct timespec *stop) {
  double time = (stop->tv_sec - start->tv_sec) + ((stop->tv_nsec - start->tv_nsec) * 1.0 / 1000000000);
  return time;
}
