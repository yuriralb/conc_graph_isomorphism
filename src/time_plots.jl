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

    # Imprime os tempos médios e outras métricas
    @printf("\n>8----------------------------8<\n")
    @printf("  Tempo médio de %d testes:\n", length(test_i_results.sequential))
    if (test_i_results.isomorphic_or_no == 1)
        @printf("  Grafos isomorfos\n")
    else
        @printf("  Grafos não-isomorfos\n")
    end
    @printf("  Número de Vértices : %d\n", test_i_results.number_of_vertices)
    @printf("  Número de Arestas  : %d\n", test_i_results.number_of_edges)
    @printf("  Tempo do Sequencial: %f\n", mean(test_i_results.sequential))
    @printf("  Tempo do t_index1  : %f\n", mean(test_i_results.t_index1))
    @printf("  Tempo do t_index2  : %f\n", mean(test_i_results.t_index2))
    @printf(">8----------------------------8<\n")
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

print_tests(test_1_results)
print_tests(test_2_results)
print_tests(test_3_results)
print_tests(test_4_results)

# test_02 is missing