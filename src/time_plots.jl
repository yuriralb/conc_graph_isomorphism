using Printf
using Statistics

# Estrutura de dados com o resultado dos 4 códigos de um teste específico
mutable struct Test
    number_of_vertices::Int
    number_of_edges::Int
    isomorphic_or_no::Int
    sequential::Vector{Float64}
    t_index1::Vector{Float64}
    t_index2::Vector{Float64}
end

# Imprime os tempos médios de cada implementação para um dado conjunto de testes e outras métricas
function print_tests(test_i_results)
    # Garante que todos os testes tem o mesmo tamanho
    @assert length(test_i_results.sequential) == length(test_i_results.t_index1) == length(test_i_results.t_index2)

    # Calcula os tempos médios
    seq_time = mean(test_i_results.sequential)
    index_1_time = mean(test_i_results.t_index1)
    index_2_time = mean(test_i_results.t_index2)
    acc_index_1 = seq_time / index_1_time
    acc_index_2 = seq_time / index_2_time

    # Imprime os tempos médios e outras métricas
    @printf("\n>8--------------------------------8<\n")
    @printf("  Tempo médio de %d testes:\n", length(test_i_results.sequential))
    if (test_i_results.isomorphic_or_no == 1)
        @printf("  Grafos isomorfos\n")
    else
        @printf("  Grafos não-isomorfos\n")
    end
    @printf("  Número de Vértices : %d\n", test_i_results.number_of_vertices)
    @printf("  Número de Arestas  : %d\n", test_i_results.number_of_edges)
    @printf("  Tempo do Sequencial: %f\n", seq_time)
    @printf("  Tempo do t_index1  : %f\n", index_1_time)
    @printf("  Tempo do t_index2  : %f\n", index_2_time)
    @printf("  Aceleração do t_index1  : %f\n", acc_index_1)
    @printf("  Aceleração do t_index2  : %f\n", acc_index_2)
    @printf("  Eficiência do t_index1  : %f\n", acc_index_1 / test_i_results.number_of_vertices)
    @printf("  Eficiência do t_index2  : %f\n", acc_index_2 / test_i_results.number_of_vertices)
    @printf(">8--------------------------------8<\n")
end

# isomorfo_cheio.txt
test_1_results = Test(11,
                      80,
                      1,
                      [1.221460e+00, 1.213941e+00, 1.214182e+00, 1.219117e+00, 1.215904e+00],
                      [5.164979e+00, 5.069275e+00, 5.153544e+00, 5.140761e+00, 5.093615e+00],
                      [2.811770e-01, 2.808340e-01, 2.805681e-01, 2.844551e-01, 2.858651e-01])

# isomorfo_esparso.txt
test_2_results = Test(11,
                      5,
                      1,
                      [2.656198e-02, 2.730203e-02, 2.507401e-02, 2.568007e-02, 2.614999e-02],
                      [2.400432e-01, 2.615380e-01, 2.470469e-01, 2.441108e-01, 2.575250e-01],
                      [1.568470e-01, 1.553390e-01, 1.610620e-01, 1.583781e-01, 1.614389e-01])

# nao_isomorfo_cheio.txt
test_3_results = Test(11,
                      80,
                      0,
                      [8.375049e-01, 7.995789e-01, 8.297060e-01, 8.040550e-01, 7.970340e-01],
                      [4.697292e+00, 4.870156e+00, 4.900520e+00, 4.892679e+00, 4.878019e+00],
                      [3.610191e-01, 3.614190e-01, 3.563750e-01, 3.548589e-01, 3.561740e-01])

# nao_isomorfo_esparso.txt
test_4_results = Test(11,
                      1,
                      0,
                      [1.387302e+00, 1.407635e+00, 1.397683e+00, 1.406363e+00, 1.388226e+00],
                      [5.563325e+00, 5.570441e+00, 5.567013e+00, 5.591806e+00, 5.565319e+00],
                      [3.136392e-01, 3.073802e-01, 3.067329e-01, 3.078449e-01, 3.072031e-01])

# Prints the results of tests with 11 vertices
print_tests(test_1_results)
print_tests(test_2_results)
print_tests(test_3_results)
print_tests(test_4_results)

# isomorfo_cheio.txt
test_5_results = Test(9,
                      64,
                      1,
                      [1.492691e-02, 1.335311e-02, 1.416993e-02, 1.391602e-02, 1.360893e-02],
                      [3.869796e-02, 3.953815e-02, 4.103804e-02, 3.997493e-02, 3.832388e-02],
                      [4.677057e-03, 3.771067e-03, 4.271030e-03, 3.926992e-03, 3.906965e-03])

# isomorfo_esparso.txt
test_6_results = Test(9,
                      5,
                      1,
                      [1.365209e-02, 1.358604e-02, 1.333213e-02, 1.425719e-02, 1.398301e-02],
                      [4.014707e-02, 4.016900e-02, 4.061508e-02, 3.828406e-02, 3.896499e-02],
                      [3.797054e-03, 4.037142e-03, 4.169941e-03, 3.988981e-03, 4.384041e-03])

# nao_isomorfo_cheio.txt
test_7_results = Test(9,
                      64,
                      0,
                      [1.010799e-02, 1.055789e-02, 1.005101e-02, 1.072717e-02, 9.484053e-03],
                      [3.963900e-02, 4.029298e-02, 3.982401e-02, 3.993797e-02, 4.106498e-02],
                      [5.614042e-03, 6.345987e-03, 6.546021e-03, 6.829023e-03, 6.268024e-03])

# nao_isomorfo_esparso.txt
test_8_results = Test(9,
                      1,
                      0,
                      [1.245594e-02, 1.307607e-02, 1.300311e-02, 1.241994e-02, 1.352692e-02],
                      [4.680896e-02, 4.754400e-02, 4.728603e-02, 4.705882e-02, 4.529905e-02],
                      [4.279137e-03, 3.726959e-03, 4.319191e-03, 4.297972e-03, 4.025936e-03])

# Prints the results of tests with 9 vertices
print_tests(test_5_results)
print_tests(test_6_results)
print_tests(test_7_results)
print_tests(test_8_results)