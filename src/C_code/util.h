#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

// Estrutura de matriz de adjacência
typedef struct {
  // Tamanho da matriz (n x n)
  int size;
  // Matriz booleana (i, j) = 0 se não há adjacência de i para j e 1 se há
  bool** matrix;
} AdjacencyMatrix;

typedef struct {
  bool show_isomorphism;
  bool show_time;
} Options;

Options options;
AdjacencyMatrix graph1, graph2;
int number_of_vertices; // Número de vértices de cada grafo
int number_of_consumer_threads, buffer_size; // Variáveis não utilizadas em seq, e t_index
bool isomorphism_found = false;

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
//
void printVec(int* permutation, int size) {
  for (int i = 0; i < size; i++) {
    printf("%d ", permutation[i]);
  }
  printf("\n");
}

void printIsomorphism(int* isomorphism, int size) {
  printf("graph1 | graph2\n");
  for (int i = 0; i < size; ++i) {
    printf(" [%d]  -->  [%d]   \n", i, isomorphism[i]);
  }
  printf("\n");
}

int readGraphsFromFile(char* file_name) {
  FILE* read_file = fopen(file_name, "rb");
  if (read_file == NULL) {
    printf("Erro durante a abertura do arquivo.\n");
    return 1;
  }

  // show_isomorphism
  if (fread(&options.show_isomorphism, sizeof(int), 1, read_file) != 1) {
    printf("Erro durante a leitura do arquivo.\n");
    return 1;
  }

  // show_time
  if (fread(&options.show_time, sizeof(int), 1, read_file) != 1) {
    printf("Erro durante a leitura do arquivo.\n");
    return 1;
  }

  // number_of_consumer_threads (não utilizada nesse caso)
  if (fread(&number_of_consumer_threads, sizeof(int), 1, read_file) != 1) {
    printf("Erro durante a leitura do arquivo.\n");
    return 1;
  }

  // buffer_size (não utilizada nesse caso)
  if (fread(&buffer_size, sizeof(int), 1, read_file) != 1) {
    printf("Erro durante a leitura do arquivo.\n");
    return 1;
  }

  // Número de vértices do primeiro grafo
  if (fread(&graph1.size, sizeof(int), 1, read_file) != 1) {
    printf("Erro durante a leitura do arquivo.\n");
    return 1;
  }

  int number_of_edges_graph1;
  if (fread(&number_of_edges_graph1, sizeof(int), 1, read_file) != 1) {
    printf("Erro durante a leitura do arquivo.\n");
    return 1;
  }

  // Arestras do primeiro grafo
  graph1.matrix = (bool **) malloc(graph1.size * sizeof(bool *));
  for (int i = 0; i < graph1.size; ++i) {
    graph1.matrix[i] = (bool *) calloc(graph1.size, sizeof(bool));
  }
  int source_destiny[2];
  for (int i = 0; i < number_of_edges_graph1; i++) {
    if (fread(source_destiny, sizeof(int), 2, read_file) != 2) {
      printf("Erro durante a leitura do arquivo.\n");
      return 1;
    }
    if (source_destiny[0] < 0 || source_destiny[1] < 0 || source_destiny[0] >= graph1.size || source_destiny[1] >= graph1.size) {
      printf("Erro. Rótulo de vértice inválido no arquivo.\n");
      return 1;
    }
    graph1.matrix[source_destiny[0]][source_destiny[1]] = true;
    graph1.matrix[source_destiny[1]][source_destiny[0]] = true;
  }

  // Número de vértices do segundo grafo
  if (fread(&graph2.size, sizeof(int), 1, read_file) != 1) {
    printf("Erro durante a leitura do arquivo.\n");
    return 1;
  }

  // Número de arestas do segundo grafo
  int number_of_edges_graph2;
  if (fread(&number_of_edges_graph2, sizeof(int), 1, read_file) != 1) {
    printf("Erro durante a leitura do arquivo.\n");
    return 1;
  }

  // Arestas do segundo grafo
  graph2.matrix = (bool **) malloc(graph2.size * sizeof(bool *));
  for (int i = 0; i < graph2.size; ++i) {
    graph2.matrix[i] = (bool *) calloc(graph2.size, sizeof(bool));
  }
  for (int i = 0; i < number_of_edges_graph2; i++) {
    if (fread(source_destiny, sizeof(int), 2, read_file) != 2) {
      printf("Erro durante a leitura do arquivo.\n");
      return 1;
    }
    if (source_destiny[0] < 0 || source_destiny[1] < 0 || source_destiny[0] >= graph2.size || source_destiny[1] >= graph2.size) {
      printf("Erro. Rótulo de vértice inválido no arquivo.\n");
      return 1;
    }
    graph2.matrix[source_destiny[0]][source_destiny[1]] = true;
    graph2.matrix[source_destiny[1]][source_destiny[0]] = true;
  }

  fclose(read_file);
  return 0;
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
    g.matrix[i] = (bool *) calloc(vertices, sizeof(bool));
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
