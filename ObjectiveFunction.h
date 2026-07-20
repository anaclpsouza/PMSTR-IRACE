#ifndef OBJECTIVEFUNCTION_H
#define OBJECTIVEFUNCTION_H

#include "Operation.h"
#include <vector>
#include <map>
#include <string>    
#include <utility>

extern int m;
extern int o;
extern int t;
extern int c;

double objectiveFunction(const std::vector<std::vector<Operation>> &maquina,
                         const std::vector<Operation> &vetOperacoes,
                         const std::map<int, std::map<int, int>> &controleOp_base,
                         std::vector<double> &tardiness_maq,
                         std::vector<std::string> *details = nullptr,
                         std::vector<std::pair<int, Operation>> *out_espera = nullptr);

#endif