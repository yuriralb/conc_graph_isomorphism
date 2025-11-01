#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>
#include "timer.h"

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
int buffer_size; // Tamanho do vetor com as permutações a serem analisadas
int** buffer; // Buffer com as permutações
int number_of_consumer_threads; // Número de Threads consumidoras
int number_of_vertices; // Número de vértices de cada grafo
int in = 0, out = 0;
bool isomorphism_found = 0;
bool finish = 0;
pthread_mutex_t buffer_lock, finish_lock;
sem_t permutations_available;
sem_t buffer_empty_slot;
AdjacencyMatrix graph1;
AdjacencyMatrix graph2;


// Realiza uma Deep Copy do vetor copied para o vetor destiny
void copyVec(int size, int* copied, int* destiny) {
  for (int i = 0; i < size; i++) {
    destiny[i] = copied[i];
  }
}

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
void* producer(void* args) {
  int exchange_index = 1;
  int *permutation_model = malloc(number_of_vertices * sizeof(int));
  int *exchange_counter = calloc(number_of_vertices, sizeof(int));

  if (graph1.size != graph2.size) {
    finish++;
  }

  for (int i = 0; i < number_of_vertices; ++i) {
    permutation_model[i] = i;
  }

  int *new_permutation = malloc(number_of_vertices * sizeof(int));
  copyVec(number_of_vertices, permutation_model, new_permutation);
  sem_wait(&buffer_empty_slot);
  pthread_mutex_lock(&buffer_lock);
  buffer[in] = new_permutation;
  in = (in + 1) % buffer_size;
  pthread_mutex_unlock(&buffer_lock);
  sem_post(&permutations_available);

  // Algoritmo de Heap para gerar permutações
  while(exchange_index < number_of_vertices) {
    sem_wait(&buffer_empty_slot);
    pthread_mutex_lock(&finish_lock);
    if (finish) {
      pthread_mutex_unlock(&finish_lock);
      break;
    }
    pthread_mutex_unlock(&finish_lock);
    int *new_permutation = malloc(number_of_vertices * sizeof(int));

    copyVec(number_of_vertices, permutation_model, new_permutation);

    if (exchange_counter[exchange_index] < exchange_index) {
      if (exchange_index % 2 == 0) {
        swap(&new_permutation[0], &new_permutation[exchange_index]);
      }
      else {
        swap(&new_permutation[exchange_counter[exchange_index]], &new_permutation[exchange_index]);
      }

      pthread_mutex_lock(&buffer_lock);
      buffer[in] = new_permutation;
      in = (in + 1) % buffer_size;
      pthread_mutex_unlock(&buffer_lock);
      sem_post(&permutations_available);
      copyVec(number_of_vertices, new_permutation, permutation_model);

      exchange_counter[exchange_index]++;
      exchange_index = 1;
    }
    else {
      sem_post(&buffer_empty_slot);
      exchange_counter[exchange_index] = 0;
      exchange_index++;
    }
  }

  free(permutation_model);
  free(exchange_counter);
  pthread_mutex_lock(&finish_lock);
  finish++;
  pthread_mutex_unlock(&finish_lock);
  sem_post(&permutations_available);
  pthread_exit(NULL);
}

void printPermutation(int *permutation, int size) {
  printf("graph1 | graph2\n");
  for (int i = 0; i < size; ++i) {
    printf(" [%d]  -->  [%d]   \n", i, permutation[i]);
  }
  printf("\n");
}

// Threads consumidoras que consomem os conteúdos dentro do buffer e testam se a permutação
// consumida é um isomorfismo ou não
void* consumer(void* args) {
  while (1) {
    int* new_permutation;
    sem_wait(&permutations_available);

    pthread_mutex_lock(&finish_lock);
    if (finish) {
      pthread_mutex_unlock(&finish_lock);
      break;
    }
    pthread_mutex_unlock(&finish_lock);

    pthread_mutex_lock(&buffer_lock);
    new_permutation = buffer[out];
    out = (out + 1) % buffer_size;
    pthread_mutex_unlock(&buffer_lock);

    if (verifyPermutation(&graph1, &graph2, new_permutation)) {
      pthread_mutex_lock(&finish_lock);
      isomorphism_found++;
      finish++;
      pthread_mutex_unlock(&finish_lock);
      if (options.show_isomorphism) {
        printPermutation(new_permutation, number_of_vertices);
      }
      break;
    }

    sem_post(&buffer_empty_slot);
    free(new_permutation);
  }

  sem_post(&permutations_available);
  sem_post(&buffer_empty_slot);
  pthread_exit(NULL);
}

// Pega entradas do usuário
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

//entrada do programa ./programa <show_isomorphism> <show_time> <number_of_consumer_threads> <buffer_size>
int main(int argc, char* argv[]) {

  double start, finish, elapsed;

  if (argc < 5) {
    printf("Entrada invalida. Digite 0 ou 1 para os parametros de entrada:\n%s <printIsomorphismo> <show_time> <number_of_consumer_threads> <buffer_size>\n", argv[0]);
    return 1;
  }

  graph1 = readGraph();
  graph2 = readGraph();

  for (int i = 1; i < 3; i++) {
    if (atoi(argv[i]) != 0 && atoi(argv[i]) != 1) { //mudei aqui
      printf("Entrada invalida. O valor de %s deve ser de 0 ou 1.", argv[i]);
      return 1;
    }
  }

  options.show_isomorphism = atoi(argv[1]);
  options.show_time = atoi(argv[2]);
  number_of_consumer_threads = atoi(argv[3]);
  buffer_size = atoi(argv[4]);
  number_of_vertices = graph1.size;
  //sem_init(&isophormismFound, 0, 1);

  GET_TIME(start);
  if (sem_init(&permutations_available, 0, 0)) perror("Criacao do semaforo 1");
  if (sem_init(&buffer_empty_slot, 0, buffer_size)) perror("Criacao do semaforo 2");
  pthread_mutex_init(&buffer_lock, NULL);
  pthread_mutex_init(&finish_lock, NULL);
  pthread_t tids[number_of_consumer_threads+1];
  buffer = (int**) malloc(sizeof(int*)*buffer_size);
  if (!buffer) {
    perror("Criacao do buffer");
  }

  // Produtor sendo criado
  pthread_create(&tids[0], NULL, producer, NULL);

  // Consumidoras sendo criadas
  for (int i = 1; i <= number_of_consumer_threads; i++) {
    pthread_create(&tids[i], NULL, consumer, NULL);
  }

  for (int i = 0; i < number_of_consumer_threads + 1; i++) {
    pthread_join(tids[i], NULL);
  }

  if (isomorphism_found) {
    printf("Os grafos sao isomorficos!\n");
  }
  else {
    printf("Os grafos nao sao isomorficos.\n");
  }
  pthread_mutex_destroy(&buffer_lock);
  pthread_mutex_destroy(&finish_lock);
  sem_destroy(&permutations_available);
  sem_destroy(&buffer_empty_slot);
  GET_TIME(finish);
  elapsed = finish - start;
  if (options.show_time) {
    printf("Tempo Concorrente: %e segundos\n", elapsed);
  }
  for (int i = 0; i < graph1.size; i++) free(graph1.matrix[i]);
  free(graph1.matrix);
  for (int i = 0; i < graph2.size; i++) free(graph2.matrix[i]);
  free(graph2.matrix);
  return 0;
}
