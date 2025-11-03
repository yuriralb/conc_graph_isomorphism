#include "../timer.h"
#include "../util.h"

typedef struct {
  int fixed_index;
} ThreadArgs;

pthread_mutex_t isomorphism_lock;
int* permutation_model;
extern Options options;
extern AdjacencyMatrix graph1, graph2;
extern int number_of_vertices; // Número de vértices de cada grafo
extern int number_of_consumer_threads, buffer_size; // Variáveis não utilizadas em seq, e t_index
extern bool isomorphism_found;

void *threadFunction(void *arg);

//entrada do programa ./<nome do programa> <nome do arquivo de leitura>
int main(int argc, char* argv[]) {
  double start, finish, elapsed;
  if (argc != 2) {
    printf("Entrada invalida. \nUso: %s <nome do arquivo de leitura>.bin\nFormato do arquivo de leitura: <mostrar isomorfismo (0 ou 1)> <mostrar tempo (0 ou 1)> <quantidade de threads consumidoras (int)> <tamanho do buffer (int)>\n<quantidade de vértices do primeiro grafo (int)> <quantidade de arestas do primeiro grafo (int)> <primeira adjacência (int int)> ... <última adjacência (int int)>\n<quantidade de vértices do segundo grafo (int)> <quantidade de arestas do segundo grafo (int)> <primeira adjacência (int int)> ... <última adjacência (int int)>\n", argv[0]);
    return 1;
  }

  readGraphsFromFile(argv[1]);

  GET_TIME(start);
  number_of_vertices = graph1.size;
  pthread_t tids[number_of_vertices];
  pthread_mutex_init(&isomorphism_lock, NULL);

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
  if (options.show_time) {
    printf("Concorrente (conc_t_index) levou: %e seconds\n", elapsed);
  }
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
      pthread_mutex_lock(&isomorphism_lock);
      isomorphism_found = true;
      if (options.show_isomorphism) {
        printIsomorphism(permutation, number_of_vertices);
      }
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
        permutation[i] = permutation_part[index];
        index++;
      }

      //printVec(permutation_part, args->fixed_index, 2);
      if (verifyPermutation(&graph1, &graph2, permutation)) {
        pthread_mutex_lock(&isomorphism_lock);
        isomorphism_found = true;
        if (options.show_isomorphism) {
        printIsomorphism(permutation, number_of_vertices);
        }
        pthread_mutex_unlock(&isomorphism_lock);
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
