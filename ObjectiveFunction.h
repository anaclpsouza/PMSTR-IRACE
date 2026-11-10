#ifndef OBJECTIVEFUNCTION_H
#define OBJECTIVEFUNCTION_H

#include <map>
#include <string>
#include <utility>
#include <vector>
#include "Operation.h"

// Parâmetros globais da instância:
// m = número de máquinas, o = número de operações,
// t = número de conjuntos de ferramentas, c = capacidade do magazine
extern int m;
extern int o;
extern int t;
extern int c;

/**
 * Calcula o valor da função objetivo (soma ponderada de atraso total e trocas de ferramentas).
 * Retorna INT_MAX caso a solução viole restrições de precedência ou gere deadlock entre máquinas.
 */
double objectiveFunction(const std::vector<std::vector<Operation>> &maquina,
                         const std::vector<Operation> &vetOperacoes,
                         const std::map<int, std::map<int, int>> &controleOp_base,
                         std::vector<double> &tardiness_maq,
                         std::vector<std::string> *details = nullptr,
                         std::vector<std::pair<int, Operation>> *out_espera = nullptr);

#endif // OBJECTIVEFUNCTION_H
