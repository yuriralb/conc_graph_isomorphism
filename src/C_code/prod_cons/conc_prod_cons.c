#include <semaphore.h>
#include "../timer.h"
#include "../util.h"

int** buffer; // Buffer com as permutações
int in = 0, out = 0;
bool finish = false;
bool producer_finished = false;
pthread_mutex_t buffer_lock, finish_lock;
sem_t permutations_available;
sem_t buffer_empty_slots;
extern Options options;
extern AdjacencyMatrix graph1, graph2;
extern int number_of_vertices; // Número de vértices de cada grafo
extern int number_of_consumer_threads, buffer_size; // Variáveis não utilizadas em seq, e t_index
extern bool isomorphism_found;


// Realiza uma Deep Copy do vetor copied para o vetor destiny
void copyVec(int size, int* copied, int* destiny) {
  for (int i = 0; i < size; i++) {
    destiny[i] = copied[i];
  }
}

// Thread produtora que produz as permutações e as guarda dentro do buffer
void* producer(void* args) {
  int exchange_index = 1;
  int *permutation_model = malloc(number_of_vertices * sizeof(int));
  int *exchange_counter = calloc(number_of_vertices, sizeof(int));

  if (graph1.size != graph2.size) {
    finish = true;
  }

  for (int i = 0; i < number_of_vertices; ++i) {
    permutation_model[i] = i;
  }

  int *new_permutation = malloc(number_of_vertices * sizeof(int));
  copyVec(number_of_vertices, permutation_model, new_permutation);
  sem_wait(&buffer_empty_slots);
  pthread_mutex_lock(&buffer_lock);
  buffer[in] = new_permutation;
  in = (in + 1) % (buffer_size);
  pthread_mutex_unlock(&buffer_lock);
  sem_post(&permutations_available);

  // Algoritmo de Heap para gerar permutações
  while(exchange_index < number_of_vertices) {
    pthread_mutex_lock(&finish_lock);
    if (finish) {
      pthread_mutex_unlock(&finish_lock);
      break;
    }
    pthread_mutex_unlock(&finish_lock);

    if (exchange_counter[exchange_index] < exchange_index) {

      if (exchange_index % 2 == 0) {
        swap(&permutation_model[0], &permutation_model[exchange_index]);
      }
      else {
        swap(&permutation_model[exchange_counter[exchange_index]], &permutation_model[exchange_index]);
      }

      int *new_permutation = malloc(number_of_vertices * sizeof(int));
      copyVec(number_of_vertices, permutation_model, new_permutation); // tira foto
      sem_wait(&buffer_empty_slots);
      pthread_mutex_lock(&buffer_lock);
      buffer[in] = new_permutation;
      in = (in + 1) % (buffer_size);
      pthread_mutex_unlock(&buffer_lock);
      sem_post(&permutations_available);

      exchange_counter[exchange_index]++;
      exchange_index = 1;
    }
    else {
      exchange_counter[exchange_index] = 0;
      exchange_index++;
    }
  }

  sem_wait(&buffer_empty_slots);
  pthread_mutex_lock(&buffer_lock);
  buffer[in] = NULL;
  pthread_mutex_unlock(&buffer_lock);

  free(permutation_model);
  free(exchange_counter);
  pthread_mutex_lock(&finish_lock);
  producer_finished = true;
  pthread_mutex_unlock(&finish_lock);
  sem_post(&permutations_available);
  pthread_exit(NULL);
}

bool bufferIsEmpty(int** buffer) {
  for (int i = 0; i < buffer_size; i++) {
    if (buffer[i] != NULL) {
      return false;
    }
  }
  return true;
}

// Threads consumidoras que consomem os conteúdos dentro do buffer e testam se a permutação
// consumida é um isomorfismo ou não
void* consumer(void* args) {
  while (1) {
    sem_wait(&permutations_available);
    pthread_mutex_lock(&finish_lock);
    if (finish) {
      pthread_mutex_unlock(&finish_lock);
      sem_post(&permutations_available);
      break;
    }
    pthread_mutex_unlock(&finish_lock);

    int* new_permutation;

    pthread_mutex_lock(&buffer_lock);
    new_permutation = buffer[out];
    out = (out + 1) % (buffer_size);
    pthread_mutex_unlock(&buffer_lock);

    if (new_permutation == NULL){
      pthread_mutex_lock(&finish_lock);
      finish = true;
      pthread_mutex_unlock(&finish_lock);
      sem_post(&permutations_available);
      break;
    }

    if (verifyPermutation(&graph1, &graph2, new_permutation)) {
      pthread_mutex_lock(&finish_lock);
      isomorphism_found = true;
      finish = true;
      pthread_mutex_unlock(&finish_lock);
      if (options.show_isomorphism) {
        printIsomorphism(new_permutation, number_of_vertices);
      }
      break;
    }

    sem_post(&buffer_empty_slots);
    free(new_permutation);
  }

  sem_post(&permutations_available);
  sem_post(&buffer_empty_slots);
  pthread_exit(NULL);
}

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
  if (sem_init(&permutations_available, 0, 0)) perror("Criacao do semaforo 1");
  if (sem_init(&buffer_empty_slots, 0, buffer_size)) perror("Criacao do semaforo 2");
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

  // Espera as threads consumidoras retornarem
  for (int i = 0; i < number_of_consumer_threads + 1; i++) {
    pthread_join(tids[i], NULL);
  }

  // Imprime o resultado pro usuário a depender se é isomorfo ou não
  if (isomorphism_found) {
    printf("Os grafos sao isomorficos!\n");
  }
  else {
    printf("Os grafos nao sao isomorficos.\n");
  }

  // Destroi os semáforos e mutexes
  pthread_mutex_destroy(&buffer_lock);
  pthread_mutex_destroy(&finish_lock);
  sem_destroy(&permutations_available);
  sem_destroy(&buffer_empty_slots);

  // Calcula e imprime o tempo que o programa esteve rodando
  GET_TIME(finish);
  elapsed = finish - start;
  if (options.show_time) {
    printf("Tempo Concorrente: %e segundos\n", elapsed);
  }

  // Libera a memória alocada pelos grafos
  free(buffer);
  for (int i = 0; i < graph1.size; i++) free(graph1.matrix[i]);
  free(graph1.matrix);
  for (int i = 0; i < graph2.size; i++) free(graph2.matrix[i]);
  free(graph2.matrix);
  return 0;
}
