#include <pthread.h>
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

void matrix_index_set(matrix_t *, int, int, float);
float matrix_index_get(matrix_t *, int, int);

matrix_t *matrix_add(matrix_t *, matrix_t *);
matrix_t *matrix_sub(matrix_t *, matrix_t *);

matrix_t *matrix_mul_std(matrix_t *, matrix_t *);

typedef struct _matrix_pair_t {
  matrix_t *A;
  matrix_t *B;
} _matrix_pair_t;

_matrix_pair_t *_matrix_pair_create(matrix_t *, matrix_t *);

matrix_t *matrix_mul_pstrassen(matrix_t *, matrix_t *);
void *_pstrassen_initial(void *);
void *_pstrassen_later(void *);

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
  matrix_t *C = matrix_mul_pstrassen(A, B);
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
  // do i have to check if self_ptr is NULL
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

_matrix_pair_t *_matrix_pair_create(matrix_t *A, matrix_t *B) {
  _matrix_pair_t *self = malloc(sizeof(_matrix_pair_t));
  self->A = A;
  self->B = B;

  return self;
}

matrix_t *matrix_mul_pstrassen(matrix_t *A, matrix_t *B) {
  _matrix_pair_t *arg = _matrix_pair_create(A, B);
  matrix_t *C = (matrix_t *) _pstrassen_initial((void *) arg);

  free(arg);

  return C;
}

void *_pstrassen_initial(void *arg) {
  _matrix_pair_t *p = (_matrix_pair_t *)arg;

  matrix_t *A = p->A;
  matrix_t *B = p->B;

  if(A->n_rows <= 16) {
    matrix_t *C = matrix_mul_std(A, B);
    return C;
  } else if(A->n_rows <= 1024) {
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

    pthread_t P1_thread, P2_thread, P3_thread, P4_thread, P5_thread, P6_thread;
    _matrix_pair_t *P1_pair, *P2_pair, *P3_pair, *P4_pair, *P5_pair, *P6_pair, *P7_pair;

    P1_pair = _matrix_pair_create(A11, S1);
    P2_pair = _matrix_pair_create(S2, B22);
    P3_pair = _matrix_pair_create(S3, B11);
    P4_pair = _matrix_pair_create(A22, S4);
    P5_pair = _matrix_pair_create(S5, S6);
    P6_pair = _matrix_pair_create(S7, S8);
    P7_pair = _matrix_pair_create(S9, S10);

    pthread_create(&P1_thread, NULL, _pstrassen_later, (void *)P1_pair);
    pthread_create(&P2_thread, NULL, _pstrassen_later, (void *)P2_pair);
    pthread_create(&P3_thread, NULL, _pstrassen_later, (void *)P3_pair);
    pthread_create(&P4_thread, NULL, _pstrassen_later, (void *)P4_pair);
    pthread_create(&P5_thread, NULL, _pstrassen_later, (void *)P5_pair);
    pthread_create(&P6_thread, NULL, _pstrassen_later, (void *)P6_pair);

    matrix_t *P1, *P2, *P3, *P4, *P5,*P6, *P7;

    P7 = _pstrassen_later(P7_pair);
    pthread_join(P1_thread, (void **)&P1);
    pthread_join(P2_thread, (void **)&P2);
    pthread_join(P3_thread, (void **)&P3);
    pthread_join(P4_thread, (void **)&P4);
    pthread_join(P5_thread, (void **)&P5);
    pthread_join(P6_thread, (void **)&P6);

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

    free(P1_pair);
    free(P2_pair);
    free(P3_pair);
    free(P4_pair);
    free(P5_pair);
    free(P6_pair);
    free(P7_pair);

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

    pthread_t P1_thread, P2_thread, P3_thread, P4_thread, P5_thread, P6_thread;
    _matrix_pair_t *P1_pair, *P2_pair, *P3_pair, *P4_pair, *P5_pair, *P6_pair, *P7_pair;

    P1_pair = _matrix_pair_create(A11, S1);
    P2_pair = _matrix_pair_create(S2, B22);
    P3_pair = _matrix_pair_create(S3, B11);
    P4_pair = _matrix_pair_create(A22, S4);
    P5_pair = _matrix_pair_create(S5, S6);
    P6_pair = _matrix_pair_create(S7, S8);
    P7_pair = _matrix_pair_create(S9, S10);

    pthread_create(&P1_thread, NULL, _pstrassen_initial, (void *)P1_pair);
    pthread_create(&P2_thread, NULL, _pstrassen_initial, (void *)P2_pair);
    pthread_create(&P3_thread, NULL, _pstrassen_initial, (void *)P3_pair);
    pthread_create(&P4_thread, NULL, _pstrassen_initial, (void *)P4_pair);
    pthread_create(&P5_thread, NULL, _pstrassen_initial, (void *)P5_pair);
    pthread_create(&P6_thread, NULL, _pstrassen_initial, (void *)P6_pair);

    matrix_t *P1, *P2, *P3, *P4, *P5,*P6, *P7;

    P7 = _pstrassen_later(P7_pair);
    pthread_join(P1_thread, (void **)&P1);
    pthread_join(P2_thread, (void **)&P2);
    pthread_join(P3_thread, (void **)&P3);
    pthread_join(P4_thread, (void **)&P4);
    pthread_join(P5_thread, (void **)&P5);
    pthread_join(P6_thread, (void **)&P6);

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

    free(P1_pair);
    free(P2_pair);
    free(P3_pair);
    free(P4_pair);
    free(P5_pair);
    free(P6_pair);
    free(P7_pair);

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

void *_pstrassen_later(void *arg) {
  _matrix_pair_t *p = (_matrix_pair_t *)arg;

  matrix_t *A = p->A;
  matrix_t *B = p->B;

  if(A->n_rows <= 16) {
    matrix_t *C = matrix_mul_std(A, B);
    return C;
  } else {

    int half = A->n_rows / 2;
    //matrix_print(A);
    matrix_t *A11 = matrix_slice(A, 0, half, 0, half);//matrix_print(A11);
    matrix_t *A12 = matrix_slice(A, 0, half, half, half);//matrix_print(A12);
    matrix_t *A21 = matrix_slice(A, half, half, 0, half);//matrix_print(A21);
    matrix_t *A22 = matrix_slice(A, half, half, half, half);//matrix_print(A22);

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

    _matrix_pair_t *P1_pair, *P2_pair, *P3_pair, *P4_pair, *P5_pair, *P6_pair, *P7_pair;

    P1_pair = _matrix_pair_create(A11, S1);
    P2_pair = _matrix_pair_create(S2, B22);
    P3_pair = _matrix_pair_create(S3, B11);
    P4_pair = _matrix_pair_create(A22, S4);
    P5_pair = _matrix_pair_create(S5, S6);
    P6_pair = _matrix_pair_create(S7, S8);
    P7_pair = _matrix_pair_create(S9, S10);

    matrix_t *P1, *P2, *P3, *P4, *P5,*P6, *P7;

    P1 = _pstrassen_later((void *)P1_pair);
    P2 = _pstrassen_later((void *)P2_pair);
    P3 = _pstrassen_later((void *)P3_pair);
    P4 = _pstrassen_later((void *)P4_pair);
    P5 = _pstrassen_later((void *)P5_pair);
    P6 = _pstrassen_later((void *)P6_pair);
    P7 = _pstrassen_later((void *)P7_pair);

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

    free(P1_pair);
    free(P2_pair);
    free(P3_pair);
    free(P4_pair);
    free(P5_pair);
    free(P6_pair);
    free(P7_pair);

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
