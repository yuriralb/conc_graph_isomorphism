#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct {
    bool showIsomorphism;
    bool showTime;
} Options;

// Estrutura de matriz de adjacência
typedef struct {

    // Tamanho da matriz (n x n)
    int size;

    // Matriz booleana (i, j) = 0 se não há adjacência de i para j e 1 se há
    bool** matrix;
} AdjacencyMatrix;

int bufferSize; // Tamanho do vetor com as permutações a serem analisadas
int** buffer; // Buffer com as permutações
int nConsThreads; // Número de Threads consumidoras
int nVertices; // Número de vértices de cada grafo
int in = 0, out = 0;
bool isomorphismFound = 0;
bool finish = 0;
pthread_mutex_t bufferLock, finishLock;
//sem_t isophormismFound;
sem_t permsAvailable;
sem_t bufferSlots;
AdjacencyMatrix g1;
AdjacencyMatrix g2;

void copyVec(int size, int* copied, int* destiny) {
  for (int i = 0; i < size; i++)
    destiny[i] = copied[i];
}

void printVec(int size, int* vec) {
  for (int i = 0; i < size; i++) {
    printf(" [%d] ", vec[i]);
  }
  printf("\n");
}

bool verifyPermutation(AdjacencyMatrix *g1, AdjacencyMatrix *g2, int *equivalence) {
    // equivalence é um vetor que representa a bijeção entre os vértices de g1 e g2
    // equivalence[x] == y significa que a bijeção leva o vértice x de g1 no vértice y de g2
    int n = g1->size; // == g2->size
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j <= i; ++j) {
            if (g1->matrix[i][j] != g2->matrix[equivalence[i]][equivalence[j]])
                return false;
        }
    }
    return true;
}

void swap(int *a, int *b) {
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

void* producer(void* args) {
  int exchange_index = 1;
  int *equivalence = malloc(nVertices * sizeof(int));
  int *exchange_counter = calloc(nVertices, sizeof(int));
  int local_finish = 0;

  if (g1.size != g2.size) {
    finish++;
  }

  printf("entrei no producer\n");
  for (int i = 0; i < nVertices; ++i)
      equivalence[i] = i;

  int *newEquivalence = malloc(nVertices * sizeof(int));
  copyVec(nVertices, equivalence, newEquivalence);
  sem_wait(&bufferSlots);
  pthread_mutex_lock(&bufferLock);
  buffer[in] = newEquivalence;
  in = (in + 1) % bufferSize;
  pthread_mutex_unlock(&bufferLock);
  sem_post(&permsAvailable);

  while(exchange_index < nVertices) {
    printf("produtor no comeco do while\n");
    sem_wait(&bufferSlots);
    printf("produtor passou do wait\n");
    // pode dar problema de exclusao mutua
    printf("antes do lock produtor\n");
    pthread_mutex_lock(&finishLock);
    local_finish = finish;
    pthread_mutex_unlock(&finishLock);
    printf("depois do lock produtor\n");
    if (local_finish) {
      break;
    }
    int *newEquivalence = malloc(nVertices * sizeof(int));

    copyVec(nVertices, equivalence, newEquivalence);

    printf("antes do if\n");
    printVec(nVertices, equivalence);
    printVec(nVertices, exchange_counter);
    printf("%d\n", exchange_index);
    if (exchange_counter[exchange_index] < exchange_index) {
        if (exchange_index % 2 == 0) {
          swap(&newEquivalence[0], &newEquivalence[exchange_index]);
        }
        else {
          swap(&newEquivalence[exchange_counter[exchange_index]], &newEquivalence[exchange_index]);
        }

        printf ("realizei swap\n");
        pthread_mutex_lock(&bufferLock);
        buffer[in] = newEquivalence;
        in = (in + 1) % bufferSize;
        pthread_mutex_unlock(&bufferLock);
        printf("depois do buffer luck produtor\n");
        sem_post(&permsAvailable);
        copyVec(nVertices, newEquivalence, equivalence);

        exchange_counter[exchange_index]++;
        exchange_index = 1;
    } else {
      printf("else\n");
        sem_post(&bufferSlots);
        exchange_counter[exchange_index] = 0;
        exchange_index++;
    }
  }

  free(equivalence);
  free(exchange_counter);
  pthread_mutex_lock(&finishLock);
  finish++;
  pthread_mutex_unlock(&finishLock);
  sem_post(&permsAvailable);
  printf("Produtora terminou\n");
  pthread_exit(NULL);
}

void* consumer(void* args) {
  int local_finish = 0;
  printf("entrei no consumer\n");
  while (1) {
    int* newEquivalence;
    sem_wait(&permsAvailable);
    printf("consumidor passou do wait\n");

    pthread_mutex_lock(&finishLock);
    local_finish = finish;
    pthread_mutex_unlock(&finishLock);
    printf("depois do lock consumidor\n");
    if (local_finish) {
      break;
    }

    pthread_mutex_lock(&bufferLock);
    newEquivalence = buffer[out];
    out = (out + 1) % bufferSize;
    pthread_mutex_unlock(&bufferLock);

    if (verifyPermutation(&g1, &g2, newEquivalence)) {
      pthread_mutex_lock(&finishLock);
      isomorphismFound++;
      finish++;
      pthread_mutex_unlock(&finishLock);
      break;
    } else {
      printf("nao consegui encontrar\n");
    }

    printf("liberou semaforo produtor\n");
    sem_post(&bufferSlots); //mudei aqui
    free(newEquivalence);
  }

  sem_post(&permsAvailable);
  sem_post(&bufferSlots);
  printf("Consumidora terminou\n");
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
        g.matrix[source][destination] = true;
        g.matrix[destination][source] = true;
    }

    return g;
}

//entrada do programa ./programa <printIsomorphismo> <showTime> <nConsThreads> <bufferSize>
int main(int argc, char* argv[]) {

  if (argc < 5) {
    printf("Entrada invalida. Digite 0 ou 1 para os parametros de entrada:\n%s <printIsomorphismo> <showTime> <nConsThreads> <bufferSize>\n", argv[0]);
    return 1;
  }

  Options options;
  g1 = readGraph();
  g2 = readGraph();

  // // casos teste
  // g1.size = 3;
  // g1.matrix = (bool**) malloc(3*sizeof(bool*));
  // g1.matrix[0] = (bool*) malloc(3*sizeof(bool));
  // g1.matrix[1] = (bool*) malloc(3*sizeof(bool));
  // g1.matrix[2] = (bool*) malloc(3*sizeof(bool));
  // g1.matrix[0][0] = 0;
  // g1.matrix[0][1] = 1;
  // g1.matrix[0][2] = 0;
  // g1.matrix[1][0] = 0;
  // g1.matrix[1][1] = 0;
  // g1.matrix[1][2] = 1;
  // g1.matrix[2][0] = 1;
  // g1.matrix[2][1] = 0;
  // g1.matrix[2][2] = 0;
  // g2.size = 3;
  // g2.matrix = (bool**) malloc(3*sizeof(bool*));
  // g2.matrix[0] = (bool*) malloc(3*sizeof(bool));
  // g2.matrix[1] = (bool*) malloc(3*sizeof(bool));
  // g2.matrix[2] = (bool*) malloc(3*sizeof(bool));
  // g2.matrix[0][0] = 0;
  // g2.matrix[0][1] = 0;
  // g2.matrix[0][2] = 1;
  // g2.matrix[1][0] = 1;
  // g2.matrix[1][1] = 0;
  // g2.matrix[1][2] = 1;
  // g2.matrix[2][0] = 0;
  // g2.matrix[2][1] = 1;
  // g2.matrix[2][2] = 0;

  for (int i = 1; i < 3; i++) {
    if (atoi(argv[i]) != 0 && atoi(argv[i]) != 1) { //mudei aqui
      printf("Entrada invalida. O valor de %s deve ser de 0 ou 1.", argv[i]);
      return 1;
    }
  }

  options.showIsomorphism = atoi(argv[1]);
  options.showTime = atoi(argv[2]);
  nConsThreads = atoi(argv[3]);
  bufferSize = atoi(argv[4]);
  nVertices = g1.size;
  //sem_init(&isophormismFound, 0, 1);

  if (sem_init(&permsAvailable, 0, 0)) perror("criacao do semaforo 1");
  if (sem_init(&bufferSlots, 0, bufferSize)) perror("criacao do semaforo 2");
  pthread_mutex_init(&bufferLock, NULL);
  pthread_mutex_init(&finishLock, NULL);
  pthread_t tids[nConsThreads+1];
  buffer = (int**) malloc(sizeof(int*)*bufferSize);
  if (!buffer) {
    perror("criacao do buffer");
  }
  //produtor sendo criado
  pthread_create(&tids[0], NULL, producer, NULL);

  for (int i = 1; i <= nConsThreads; i++) {
    pthread_create(&tids[i], NULL, consumer, NULL);
  }

  for (int i = 0; i < nConsThreads + 1; i++) {
    pthread_join(tids[i], NULL);
  }

  if (isomorphismFound) {
    printf("Os grafos sao isomorficos!\n");
  } else {
    printf("Os grafos nao sao isomorficos.\n");
  }
  //sem_destroy(&isophormismFound);
  pthread_mutex_destroy(&bufferLock);
  pthread_mutex_destroy(&finishLock);
  sem_destroy(&permsAvailable);
  sem_destroy(&bufferSlots);
  for (int i = 0; i < g1.size; i++) free(g1.matrix[i]);
  free(g1.matrix);
  for (int i = 0; i < g2.size; i++) free(g2.matrix[i]);
  free(g2.matrix);
  return 0;
}
