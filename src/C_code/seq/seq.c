#include "../timer.h"
#include "../util.h"


bool testIsomorphism(AdjacencyMatrix *graph1, AdjacencyMatrix *graph2);
extern Options options;
extern AdjacencyMatrix graph1, graph2;
extern int number_of_vertices; // Número de vértices de cada grafo
extern int number_of_consumer_threads, buffer_size; // Variáveis não utilizadas em seq, e t_index
extern bool isomorphism_found;

//entrada do programa ./<nome do programa> <nome do arquivo de leitura>
int main(int argc, char* argv[]) {
  double start, finish, elapsed;

  if (readGraphsFromFile(argv[1])) {
    return 1;
  }

  GET_TIME(start);
  if (!testIsomorphism(&graph1, &graph2)) {
    printf("Os grafos nao sao isomorfos.\n");
  } else {
    printf("Os grafos sao isomorfos!\n");
  }

  GET_TIME(finish);
  elapsed = finish - start;
  if (options.show_time) {
    printf("Tempo Sequencial: %e segundos\n", elapsed);
  }
  // libera memória
  for (int i = 0; i < graph1.size; i++) {
    free(graph1.matrix[i]);
  }
  free(graph1.matrix);
  for (int i = 0; i < graph2.size; i++) {
    free(graph2.matrix[i]);
  }
  free(graph2.matrix);
  return 0;
}

bool testIsomorphism(AdjacencyMatrix *graph1, AdjacencyMatrix *graph2) {
  if (graph1->size != graph2->size) {
    return false;
  }

  int n = graph1->size;
  int *equivalence = malloc(n * sizeof(int));
  int *exchange_counter = calloc(n, sizeof(int));
  int exchange_index = 1;
  bool isomorphism_found = false;

  for (int i = 0; i < n; ++i) {
    equivalence[i] = i;
  }

  if (verifyPermutation(graph1, graph2, equivalence)) {
    if (options.show_isomorphism) {
      printIsomorphism(equivalence, n);
    }
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

      if (verifyPermutation(graph1, graph2, equivalence)) {
        if (options.show_isomorphism) {
          printIsomorphism(equivalence, n);
        }
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
