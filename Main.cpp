
#include <iostream>
#include <climits>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <map>
#include "Operation.h"
#include "ObjectiveFunction.h"
#include <chrono>
#include <deque>
#include <random>
#include "Buscas.h"

using namespace std;
std::ofstream fileSolution;
int m, o, t, c;
std::vector<std::vector<Operation>> maquinas;
std::map<int, double> tempoMaq;
extern high_resolution_clock::time_point t2;

using namespace std::chrono;


void distInicial(std::map<int, std::deque<Operation>> tarefas)
{
    std::vector<int> tarefasDisponiveis;

    for (auto const &[id, fila] : tarefas)
    {
        tarefasDisponiveis.push_back(id);
    }

    if (tarefasDisponiveis.empty() || m <= 0)
    {
        return;
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(tarefasDisponiveis.begin(), tarefasDisponiveis.end(), g);

    auto cargaTarefa = [&](int idJob)
    {
        double soma = 0.0;
        const auto &ops = tarefas[idJob];

        for (const auto &op : ops)
        {
            soma += op.releaseTime + op.processingTime;
        }

        return soma;
    };

    std::vector<double> completionMaq(m, 0.0);
    size_t idxTarefa = 0;

    size_t tarefasIniciais = std::min((size_t)m, tarefasDisponiveis.size());
    for (size_t maq = 0; maq < tarefasIniciais; ++maq)
    {
        int idJob = tarefasDisponiveis[idxTarefa++];
        auto &ops = tarefas[idJob];

        maquinas[maq].insert(maquinas[maq].end(), ops.begin(), ops.end());
        completionMaq[maq] += cargaTarefa(idJob);
    }

    while (idxTarefa < tarefasDisponiveis.size())
    {
        int melhorMaq = 0;
        for (int maq = 1; maq < m; ++maq)
        {
            if (completionMaq[maq] < completionMaq[melhorMaq])
            {
                melhorMaq = maq;
            }
        }

        int idJob = tarefasDisponiveis[idxTarefa++];
        auto &ops = tarefas[idJob];

        maquinas[melhorMaq].insert(maquinas[melhorMaq].end(), ops.begin(), ops.end());
        completionMaq[melhorMaq] += cargaTarefa(idJob);
    }
}

void enviesadaPorRelease(std::vector<Operation> tarefas)
{
    std::sort(tarefas.begin(), tarefas.end(), [](const Operation &a, const Operation &b)
              {
                  if (a.releaseTime != b.releaseTime)
                      return a.releaseTime < b.releaseTime;
                  if (a.idJob != b.idJob)
                      return a.idJob < b.idJob;
                  return a.idOp < b.idOp; });

    std::vector<double> cargaProcessamento(m, 0.0);
    std::map<int, int> maquinaPorJob;

    auto menorCarga = [&]()
    {
        int melhorMaq = 0;
        for (int maq = 1; maq < m; ++maq)
        {
            if (cargaProcessamento[maq] < cargaProcessamento[melhorMaq])
            {
                melhorMaq = maq;
            }
        }
        return melhorMaq;
    };

    for (const auto &op : tarefas)
    {
        int maquinaEscolhida = -1;

        if (op.idOp > 1)
        {
            auto it = maquinaPorJob.find(op.idJob);
            if (it != maquinaPorJob.end())
            {
                maquinaEscolhida = it->second;
            }
        }

        if (maquinaEscolhida < 0)
        {
            maquinaEscolhida = menorCarga();
        }

        maquinas[maquinaEscolhida].push_back(op);
        cargaProcessamento[maquinaEscolhida] += op.processingTime;
        maquinaPorJob[op.idJob] = maquinaEscolhida;
    }
}

void enviesadaPorDueDate(std::vector<Operation> tarefas)
{
    std::sort(tarefas.begin(), tarefas.end(), [](const Operation &a, const Operation &b)
              { return a.dueDate < b.dueDate; });

    std::vector<double> cargaProcessamento(m, 0.0);
    std::map<int, int> maquinaPorJob;

    auto menorCarga = [&]()
    {
        int melhorMaq = 0;
        for (int maq = 1; maq < m; ++maq)
        {
            if (cargaProcessamento[maq] < cargaProcessamento[melhorMaq])
            {
                melhorMaq = maq;
            }
        }
        return melhorMaq;
    };

    for (const auto &op : tarefas)
    {
        int maquinaEscolhida = -1;

        if (op.idOp > 1)
        {
            auto it = maquinaPorJob.find(op.idJob);
            if (it != maquinaPorJob.end())
            {
                maquinaEscolhida = it->second;
            }
        }

        if (maquinaEscolhida < 0)
        {
            maquinaEscolhida = menorCarga();
        }

        maquinas[maquinaEscolhida].push_back(op);
        cargaProcessamento[maquinaEscolhida] += op.processingTime;
        maquinaPorJob[op.idJob] = maquinaEscolhida;
    }
}

int parseHeaderValue(string line)
{
    replace(line.begin(), line.end(), ',', ' ');
    stringstream ss(line);
    ss.imbue(locale("C"));
    string key;
    int value;
    ss >> key >> value;
    return value;
}

int main(int argc, char *argv[])
{
    vector<string> arguments(argv + 1, argv + argc);
    
    // Valores padrão
    int RI = 1, IM = 1, TS = 1, iterations = 100;
    double disturbance = 0.05;
    string instance_file = "";

    size_t sz;
    for(size_t i = 0; i < arguments.size(); i++) {
        if(arguments[i] == "--ri") RI = stoi(arguments[++i], &sz);
        else if(arguments[i] == "--im") IM = stoi(arguments[++i], &sz);
        else if(arguments[i] == "--ts") TS = stoi(arguments[++i], &sz);
        else if(arguments[i] == "--i") iterations = stoi(arguments[++i], &sz);
        else if(arguments[i] == "--p") disturbance = stod(arguments[++i], &sz);
        else if(arguments[i] == "--inst") instance_file = arguments[++i];
    }

    ifstream arquivo_csv(instance_file);
    ios_base::sync_with_stdio(false);
    cin.imbue(locale("C"));

    string line;

    if (getline(cin, line))
        o = parseHeaderValue(line);
    if (getline(cin, line))
        m = parseHeaderValue(line);
    if (getline(cin, line))
        t = parseHeaderValue(line);
    if (getline(cin, line))
        c = parseHeaderValue(line);

    getline(cin, line);
    getline(cin, line);

    std::vector<Operation> vetOperacao;
    int i = 0;
    std::map<int, std::map<int, int>> controleOp;

    controleOp.clear();

    while (getline(cin, line) && !line.empty())
    {
        replace(line.begin(), line.end(), ',', ' ');
        stringstream ss(line);
        ss.imbue(locale("C"));

        int idJob, idOp, toolSetId, toolSetSize;
        double releaseTime, processingTime, dueDate;

        ss >> idJob >> idOp >> releaseTime >> processingTime >> dueDate >> toolSetId >> toolSetSize;

        if (ss)
        {
            Operation op(i, idJob, idOp, toolSetId - 1, toolSetSize, processingTime, dueDate, releaseTime);
            vetOperacao.push_back(op);
            controleOp[idJob][idOp] = 0;
            i++;
        }
    }

    t1 = high_resolution_clock::now();

    std::map<int, double> tempo_final;
    std::vector<double> tardiness_maq;
    std::map<int, std::deque<Operation>> tarefas;
    for (const auto &op : vetOperacao)
    {
        tarefas[op.idJob].push_back(op);
    }

    double sol_inicial = INT_MAX;
    std::vector<std::vector<Operation>> melhorMaquinas;
    std::vector<double> melhorTardiness;

    auto avaliarSolucaoInicial = [&](auto gerarSolucao, bool repetirAteValida)
    {
        double valor = INT_MAX;
        std::vector<double> tardinessCandidato;

        do
        {
            maquinas.clear();
            maquinas.resize(m);
            gerarSolucao();
            valor = objectiveFunction(maquinas, vetOperacao, controleOp, tardinessCandidato);
        } while (repetirAteValida && valor == INT_MAX);

        if (valor != INT_MAX && valor < sol_inicial)
        {
            sol_inicial = valor;
            melhorMaquinas = maquinas;
            melhorTardiness = tardinessCandidato;
        }
    };

    avaliarSolucaoInicial([&]() { distInicial(tarefas); }, true);
    avaliarSolucaoInicial([&]() { enviesadaPorRelease(vetOperacao); }, false);
    avaliarSolucaoInicial([&]() { enviesadaPorDueDate(vetOperacao); }, false);

    maquinas = melhorMaquinas;
    tardiness_maq = melhorTardiness;


    cout << ILS(RI, IM, TS, iterations, disturbance, maquinas, vetOperacao, controleOp, tardiness_maq, o);

    tempo_execucao = high_resolution_clock::now() - t1;
    

    return 0;
}