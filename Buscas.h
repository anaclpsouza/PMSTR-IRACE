#ifndef BUSCAS_H
#define BUSCAS_H

#include <chrono>
#include <map>
#include <vector>
#include "ObjectiveFunction.h"
#include "Operation.h"

using namespace std::chrono;

extern high_resolution_clock::time_point t1;
extern high_resolution_clock::time_point t2;
extern std::chrono::high_resolution_clock::duration tempo_execucao;

double pertubacao_enviesada(std::vector<std::vector<Operation>> &maquina,
                            std::vector<Operation> &vetOperacoes,
                            std::map<int, std::map<int, int>> &controleOp,
                            std::vector<double> &tardiness_maq, 
                            const std::vector<std::pair<int, Operation>>& ops_emEspera,
                            int num_ops);

double pertubacao(std::vector<std::vector<Operation>> &maquina,
                  std::vector<Operation> &vetOperacoes,
                  std::map<int, std::map<int, int>> &controleOp,
                  std::vector<double> &tardiness_maq,
                  int o);

double re_insertion(std::vector<std::vector<Operation>> &maquina,
                    std::vector<Operation> &vetOperacoes,
                    std::map<int, std::map<int, int>> &controleOp,
                    std::vector<double> &tardiness_maq);

double insertion_im(std::vector<std::vector<Operation>> &maquina,
                    std::vector<Operation> &vetOperacoes,
                    std::map<int, std::map<int, int>> &controleOp,
                    std::vector<double> &tardiness_maq);

double two_swap(std::vector<std::vector<Operation>> &maquina,
                std::vector<Operation> &vetOperacoes,
                std::map<int, std::map<int, int>> &controleOp,
                std::vector<double> &tardiness_maq);

double ILS(int RI, int IM, int TS, int iterations,
           double disturbance, std::vector<std::vector<Operation>> &maquina,
           std::vector<Operation> &vetOperacoes,
           std::map<int, std::map<int, int>> &controleOp,
           std::vector<double> &tardiness_maq, int totalOperacoes);

#endif