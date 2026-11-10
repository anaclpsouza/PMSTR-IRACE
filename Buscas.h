#ifndef BUSCAS_H
#define BUSCAS_H

#include <chrono>
#include <map>
#include <utility>
#include <vector>
#include "ObjectiveFunction.h"
#include "Operation.h"

using namespace std::chrono;

extern high_resolution_clock::time_point t1;
extern high_resolution_clock::time_point t2;
extern std::chrono::high_resolution_clock::duration tempo_execucao;

/**
 * Perturbação enviesada: prioriza realocar operações que sofreram espera por
 * precedência (complementando com operações aleatórias) em outras máquinas.
 */
double pertubacao_enviesada(std::vector<std::vector<Operation>> &maquina,
                            std::vector<Operation> &vetOperacoes,
                            std::map<int, std::map<int, int>> &controleOp,
                            std::vector<double> &tardiness_maq,
                            const std::vector<std::pair<int, Operation>> &ops_emEspera,
                            int num_ops);

/**
 * Busca local Re-insertion (intra-máquina): remove uma operação e a reinsere
 * em outra posição da mesma máquina (First Improvement).
 */
double re_insertion(std::vector<std::vector<Operation>> &maquina,
                    std::vector<Operation> &vetOperacoes,
                    std::map<int, std::map<int, int>> &controleOp,
                    std::vector<double> &tardiness_maq);

/**
 * Busca local Insertion-IM (inter-máquinas): move uma operação da máquina mais
 * atrasada para outra máquina (First Improvement).
 */
double insertion_im(std::vector<std::vector<Operation>> &maquina,
                    std::vector<Operation> &vetOperacoes,
                    std::map<int, std::map<int, int>> &controleOp,
                    std::vector<double> &tardiness_maq);

/**
 * Busca local Two-Swap (intra-máquina): troca a posição de duas operações
 * dentro da mesma máquina (First Improvement).
 */
double two_swap(std::vector<std::vector<Operation>> &maquina,
                std::vector<Operation> &vetOperacoes,
                std::map<int, std::map<int, int>> &controleOp,
                std::vector<double> &tardiness_maq);

/**
 * Metaheurística Iterated Local Search (ILS) com VND configurável via parâmetros.
 */
double ILS(int RI, int IM, int TS, int iterations,
           double disturbance, std::vector<std::vector<Operation>> &maquina,
           std::vector<Operation> &vetOperacoes,
           std::map<int, std::map<int, int>> &controleOp,
           std::vector<double> &tardiness_maq, int totalOperacoes);

#endif // BUSCAS_H
