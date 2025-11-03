#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../timer.h"

typedef struct {
  int size;
  bool **matrix;
} adjacency_matrix;

adjacency_matrix readGraph();
bool testIsomorphism(adjacency_matrix *g1, adjacency_matrix *g2);
bool verifyPermutation(adjacency_matrix *g1, adjacency_matrix *g2, int *equivalence);
void printPermutation(int *equivalence, int size);
void swap(int *a, int *b);

int main() {
  adjacency_matrix g1 = readGraph();
  adjacency_matrix g2 = readGraph();
  double start, finish, elapsed;

  GET_TIME(start);
  if (!testIsomorphism(&g1, &g2)) {
    printf("The graphs are not isomorphic\n");
  }

  GET_TIME(finish);
  elapsed = finish - start;
  printf("Tempo Sequencial: %e segundos\n", elapsed);
  // libera memória
  for (int i = 0; i < g1.size; i++) {
    free(g1.matrix[i]);
  }
  free(g1.matrix);
  for (int i = 0; i < g2.size; i++) {
    free(g2.matrix[i]);
  }
  free(g2.matrix);
  return 0;
}

bool testIsomorphism(adjacency_matrix *g1, adjacency_matrix *g2) {
  if (g1->size != g2->size) {
    return false;
  }

  int n = g1->size;
  int *equivalence = malloc(n * sizeof(int));
  int *exchange_counter = calloc(n, sizeof(int));
  int exchange_index = 1;
  bool isomorphism_found = false;

  for (int i = 0; i < n; ++i) {
    equivalence[i] = i;
  }

  if (verifyPermutation(g1, g2, equivalence)) {
    // printPermutation(equivalence, n);
    free(equivalence);
    free(exchange_counter);
    return true;
  }

  // Algoritmo de Heap para gerar permutações
  while (exchange_index < n) {
    if (exchange_counter[exchange_index] < exchange_index) {
      if (exchange_index % 2 == 0)
        swap(&equivalence[0], &equivalence[exchange_index]);
      else
        swap(&equivalence[exchange_counter[exchange_index]], &equivalence[exchange_index]);

      if (verifyPermutation(g1, g2, equivalence)) {
        printPermutation(equivalence, n);
        isomorphism_found = true;
        return true;
      }

      exchange_counter[exchange_index]++;
      exchange_index = 1;
    }
    else {
      exchange_counter[exchange_index] = 0;
      exchange_index++;
    }
  }

  free(equivalence);
  free(exchange_counter);
  return isomorphism_found;
}

adjacency_matrix readGraph() {
  int vertices, edges;
  int source, destination;

  scanf("%d", &vertices);
  adjacency_matrix g;
  g.size = vertices;
  g.matrix = malloc(vertices * sizeof(bool *));
  for (int i = 0; i < vertices; ++i) {
    g.matrix[i] = calloc(vertices, sizeof(bool));
  }

  scanf("%d", &edges);
  for (int i = 0; i < edges; ++i) {
    scanf("%d %d", &source, &destination);
    g.matrix[source][destination] = true;
    g.matrix[destination][source] = true;
  }

  return g;
}

bool verifyPermutation(adjacency_matrix *g1, adjacency_matrix *g2, int *equivalence) {
  int n = g1->size;
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j <= i; ++j) {
      if (g1->matrix[i][j] != g2->matrix[equivalence[i]][equivalence[j]]) {
        return false;
      }
    }
  }
  return true;
}

void printPermutation(int *equivalence, int size) {
  for (int i = 0; i < size; ++i) {
    printf("%d ", equivalence[i]);
  }
  printf("\n");
}

void swap(int *a, int *b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}
