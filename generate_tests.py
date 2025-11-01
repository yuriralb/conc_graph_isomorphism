import struct

# Cria um arquivo binário no formato específico de entrada para o programa de Isomorfismo
def create_binary(file_name,
                  print_isomorphism, show_time, n_cons_threads, buffer_size,
                  nv1, na1, list1,
                  nv2, na2, list2):
    with open(file_name, "wb") as f:
        # Parâmetros do programa
        f.write(struct.pack("4i", print_isomorphism, show_time, n_cons_threads, buffer_size))

        # Parâmetros sobre o primeiro grafo
        f.write(struct.pack("2i", nv1, na1))
        for x, y in list1[:na1]:
            f.write(struct.pack("2i", x, y))

        # Parâmetros sobre o segundo grafo
        f.write(struct.pack("2i", nv2, na2))
        for x, y in list2[:na2]:
            f.write(struct.pack("2i", x, y))

    print(f"created file {file_name}")

# Teste 1 (triângulo) (isomorfo)
create_binary(
    "file1.bin",
    1, 1, 4, 8,
    3, 3, [[0, 1], [1, 2], [2, 0]],
    3, 3, [[0, 1], [2, 0], [2, 1]]
)

# Teste 2 (Grafo gigantesco esparso com 1 ligação) (isomorfo)
create_binary(
    "file2.bin",
    1, 1, 4, 8,
    1000, 1, [[67, 92]],
    1000, 1, [[0, 1]]
)