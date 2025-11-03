#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include "../timer.h"

typedef struct {
  int fixed_index;
} ThreadArgs;

typedef struct {
  bool show_isomorphism;
  bool show_time;
} Options;

// Estrutura de matriz de adjacência
typedef struct {
  // Tamanho da matriz (n x n)
  int size;
  // Matriz booleana (i, j) = 0 se não há adjacência de i para j e 1 se há
  bool** matrix;
} AdjacencyMatrix;

Options options;
int number_of_vertices; // Número de vértices de cada grafo
bool isomorphism_found = false;
pthread_mutex_t isomorphism_lock;
AdjacencyMatrix graph1;
AdjacencyMatrix graph2;
int* permutation_model;

// Verifica se a permutação dada é um isomorfismo
bool verifyPermutation(AdjacencyMatrix *graph1, AdjacencyMatrix *graph2, int *permutation) {
  // permutation é um vetor que representa a bijeção entre os vértices de graph1 e graph2
  // permutation[x] == y significa que a bijeção leva o vértice x de graph1 no vértice y de graph2
  int n = graph1->size; // == graph2->size
  for (int i = 0; i < n; ++i) {
    for (int j = 0; j <= i; ++j) {
      if (graph1->matrix[i][j] != graph2->matrix[permutation[i]][permutation[j]]) {
        return false;
      }
    }
  }
  return true;
}

// Troca o conteúdo dos ponteiros a e b
void swap(int *a, int *b) {
  int tmp = *a;
  *a = *b;
  *b = tmp;
}

// Thread produtora que produz as permutações e as guarda dentro do buffer

void printPermutation(int *permutation, int size) {
  printf("graph1 | graph2\n");
  for (int i = 0; i < size; ++i) {
    printf(" [%d]  -->  [%d]   \n", i, permutation[i]);
  }
  printf("\n");
}

void printVec(int* permutation, int fixed_index, int size) {
  printf("fixed_index: %d\n", fixed_index);
  for (int i = 0; i < size; i++) {
    printf("%d ", permutation[i]);
  }
  printf("\n");
}
// Threads consumidoras que consomem os conteúdos dentro do buffer e testam se a permutação
// consumida é um isomorfismo ou não
void *threadFunction(void *arg) {
    ThreadArgs* args = (ThreadArgs*) arg;
    int exchange_index = 1;
    int *exchange_counter = calloc(number_of_vertices-1, sizeof(int));
    int *permutation_part = calloc(number_of_vertices-1,  sizeof(int));
    int *permutation = calloc(number_of_vertices, sizeof(int));

    int index = 0;
    for (int i = 0; i < number_of_vertices; i++) {
        if (i != args->fixed_index) {
            permutation_part[index] = permutation_model[i];
            index++;
        }
    }

    permutation[0] = permutation_model[args->fixed_index];
    index = 0;
    for (int i = 1; i < number_of_vertices; i++) {
      permutation[i] = permutation_part[index];
      index++;
    }

    //printVec(permutation, args->fixed_index, 3);
    if (verifyPermutation(&graph1, &graph2, permutation)) {
      //printPermutation(permutation, number_of_vertices);
      pthread_mutex_lock(&isomorphism_lock);
      isomorphism_found = true;
      pthread_mutex_unlock(&isomorphism_lock);
      free(permutation);
      free(permutation_part);
      free(exchange_counter);
      free(args);
      pthread_exit(NULL);
    }

    while (exchange_index < number_of_vertices-1) {
    pthread_mutex_lock(&isomorphism_lock);
    if (isomorphism_found) {
      pthread_mutex_unlock(&isomorphism_lock);
      break;
    }
    pthread_mutex_unlock(&isomorphism_lock);


    if (exchange_counter[exchange_index] < exchange_index) {
      if (exchange_index % 2 == 0)
        swap(&permutation_part[0], &permutation_part[exchange_index]);
      else
        swap(&permutation_part[exchange_counter[exchange_index]], &permutation_part[exchange_index]);

      permutation[0] = permutation_model[args->fixed_index];
      index = 0;
      for (int i = 1; i < number_of_vertices; i++) {
        permutation[i] = permutation_part[i];
        index++;
      }

      //printVec(permutation_part, args->fixed_index, 2);
      if (verifyPermutation(&graph1, &graph2, permutation)) {
        //printPermutation(permutation, number_of_vertices);
        pthread_mutex_lock(&isomorphism_lock);
        isomorphism_found = true;
        pthread_mutex_lock(&isomorphism_lock);
      }

      exchange_counter[exchange_index]++;
      exchange_index = 1;
    }
    else {
      exchange_counter[exchange_index] = 0;
      exchange_index++;
    }
  }
    free(permutation);
    free(permutation_part);
    free(exchange_counter);
    free(args);
    pthread_exit(NULL);
}

AdjacencyMatrix readGraph() {
  int vertices, edges;
  int source, destination;

  printf("Digite a quantidade de vertices: ");
  scanf("%d", &vertices);
  AdjacencyMatrix g;
  g.size = vertices;
  g.matrix = (bool **) malloc(vertices * sizeof(bool *));
  for (int i = 0; i < vertices; ++i) {
    g.matrix[i] = calloc(vertices, sizeof(bool));
  }

  printf("Digite a quantidade de arestas: ");
  scanf("%d", &edges);
  printf("Digite as arestas no formato: <vertice1 vertice2>\n");
  for (int i = 0; i < edges; ++i) {
    scanf("%d %d", &source, &destination);
    while ((source >= vertices) || (destination >= vertices)) {
      printf("Tamanho invalido de vertice. Insira novamete.\n");
      scanf("%d %d", &source, &destination);
    }
    g.matrix[source][destination] = true;
    g.matrix[destination][source] = true;
  }

  return g;
}

int main() {
  double start, finish, elapsed;
  graph1 = readGraph();
  graph2 = readGraph();
  number_of_vertices = graph1.size;
  pthread_t tids[number_of_vertices];
  pthread_mutex_init(&isomorphism_lock, NULL);

  GET_TIME(start);

  permutation_model = malloc(number_of_vertices*sizeof(int));
  for (int i = 0; i < number_of_vertices; i++) {
    permutation_model[i] = i;
  }


  // Cria uma thread para cada prefixo fixo
  for (int i = 0; i < number_of_vertices; i++) {
    ThreadArgs* args = (ThreadArgs*) malloc(sizeof(ThreadArgs));
      args->fixed_index = i;
      pthread_create(&tids[i], NULL, threadFunction, (void*) args);
  }

  // Espera todas as threads terminarem
  for (int i = 0; i < number_of_vertices; i++) {
      pthread_join(tids[i], NULL);
  }

  if (isomorphism_found) {
    printf("Os grafos sao isomorfos!\n");
  } else {
    printf("Os grafos nao sao isomorfos.\n");
  }

  pthread_mutex_destroy(&isomorphism_lock);
  GET_TIME(finish);
  elapsed = finish - start;
  printf("Concorrente levou: %e seconds\n", elapsed);
  for (int i = 0; i < graph1.size; i++) {
    free(graph1.matrix[i]);
  }
  free(graph1.matrix);
  for (int i = 0; i < graph2.size; i++) {
    free(graph2.matrix[i]);
  }
  free(graph2.matrix);
  free(permutation_model);
  return 0;
}
