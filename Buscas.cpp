#include "Buscas.h"

#include <algorithm>
#include <chrono>
#include <climits>
#include <cmath>
#include <limits>
#include <numeric>
#include <random>
#include <utility>
#include <vector>
#include "ObjectiveFunction.h"
#include "Operation.h"

using namespace std;
using namespace std::chrono;

std::random_device rd;
std::mt19937 rng(rd());

high_resolution_clock::time_point t1;
high_resolution_clock::time_point t2;
std::chrono::high_resolution_clock::duration tempo_execucao;

double ILS(int RI, int IM, int TS, int iterations,
           double disturbance, std::vector<std::vector<Operation>> &maquina,
           std::vector<Operation> &vetOperacoes,
           std::map<int, std::map<int, int>> &controleOp,
           std::vector<double> &tardiness_maq, int totalOperacoes)
{
    // 1. Intensificação inicial sobre a solução construtiva
    double s = objectiveFunction(maquina, vetOperacoes, controleOp, tardiness_maq);
    if (RI == 1)
        s = re_insertion(maquina, vetOperacoes, controleOp, tardiness_maq);
    if (IM == 1)
        s = insertion_im(maquina, vetOperacoes, controleOp, tardiness_maq);
    if (TS == 1)
        s = two_swap(maquina, vetOperacoes, controleOp, tardiness_maq);

    double melhor = s;
    std::vector<std::vector<Operation>> melhor_sol = maquina;
    std::vector<std::vector<Operation>> sol_base = maquina;

    // 2. Loop principal da ILS (para após 'iterations' iterações sem melhoria ou 2h)
    int iteracoesSemMelhoria = 0;
    while (iteracoesSemMelhoria < iterations)
    {
        t2 = high_resolution_clock::now();
        tempo_execucao = t2 - t1;

        if (tempo_execucao >= std::chrono::hours(2))
        {
            break;
        }

        maquina = sol_base;

        std::vector<std::pair<int, Operation>> ops_emEspera;
        std::vector<double> tardiness_temp;
        objectiveFunction(maquina, vetOperacoes, controleOp, tardiness_temp, nullptr, &ops_emEspera);

        // Perturbação enviesada
        int num_ops = static_cast<int>(std::max(1, static_cast<int>(std::ceil(totalOperacoes * disturbance))));
        double s_atual = pertubacao_enviesada(maquina, vetOperacoes, controleOp, tardiness_maq, ops_emEspera, num_ops);

        // Busca Local (VND)
        double s_candidato = s_atual;
        int qual = 0;
        while (qual < 3)
        {
            if (qual == 0)
            {
                if (RI == 0)
                {
                    qual++;
                    continue;
                }
                s_candidato = re_insertion(maquina, vetOperacoes, controleOp, tardiness_maq);
            }
            else if (qual == 1)
            {
                if (IM == 0)
                {
                    qual++;
                    continue;
                }
                s_candidato = insertion_im(maquina, vetOperacoes, controleOp, tardiness_maq);
            }
            else if (qual == 2)
            {
                if (TS == 0)
                {
                    qual++;
                    continue;
                }
                s_candidato = two_swap(maquina, vetOperacoes, controleOp, tardiness_maq);
            }

            if (s_candidato < s_atual)
            {
                s_atual = s_candidato;
                qual = 0;
            }
            else
            {
                qual++;
            }
        }

        // Atualização da melhor solução global
        if (s_atual < melhor)
        {
            melhor = s_atual;
            melhor_sol = maquina;
            iteracoesSemMelhoria = 0;
        }
        else
        {
            iteracoesSemMelhoria++;
        }

        // Critério de aceitação da nova solução base (até 10% acima da melhor global)
        if (s_candidato <= (melhor * 1.10))
        {
            sol_base = maquina;
        }
    }

    maquina = melhor_sol;
    return melhor;
}

double pertubacao_enviesada(std::vector<std::vector<Operation>> &maquina,
                            std::vector<Operation> &vetOperacoes,
                            std::map<int, std::map<int, int>> &controleOp,
                            std::vector<double> &tardiness_maq,
                            const std::vector<std::pair<int, Operation>> &ops_emEspera,
                            int num_ops)
{
    int numMaquinas = maquina.size();
    std::vector<std::pair<int, Operation>> selecionadas;

    // Prioriza operações que aguardaram precedência
    for (const auto &op : ops_emEspera)
    {
        if (selecionadas.size() >= (size_t)num_ops)
            break;
        selecionadas.push_back(op);
    }

    // Completa com operações aleatórias caso necessário
    while (selecionadas.size() < (size_t)num_ops)
    {
        int m_rand = std::uniform_int_distribution<>(0, numMaquinas - 1)(rng);
        if (maquina[m_rand].empty())
            continue;
        int pos_rand = std::uniform_int_distribution<>(0, maquina[m_rand].size() - 1)(rng);

        selecionadas.push_back(std::make_pair(m_rand, maquina[m_rand][pos_rand]));
    }

    std::vector<double> tardiness_teste = tardiness_maq;

    for (const auto &item : selecionadas)
    {
        int maqOrigem = item.first;
        Operation op = item.second;

        // Remove a operação da máquina de origem
        bool removido = false;
        int posOrigem = -1;
        for (auto it = maquina[maqOrigem].begin(); it != maquina[maqOrigem].end(); ++it)
        {
            if (it->idJob == op.idJob && it->idOp == op.idOp)
            {
                posOrigem = std::distance(maquina[maqOrigem].begin(), it);
                maquina[maqOrigem].erase(it);
                removido = true;
                break;
            }
        }
        if (!removido)
            continue;

        // Escolhe máquina de destino aleatória diferente da origem
        int maqDestino = maqOrigem;
        if (numMaquinas > 1)
        {
            while (maqDestino == maqOrigem)
            {
                maqDestino = std::uniform_int_distribution<>(0, numMaquinas - 1)(rng);
            }
        }

        // Avalia todas as posições possíveis na máquina de destino em ordem aleatória
        int melhorPosicao = -1;
        double melhorSol = std::numeric_limits<double>::max();
        int numOpsDestino = maquina[maqDestino].size();

        std::vector<int> ordemPosicoes(numOpsDestino + 1);
        std::iota(ordemPosicoes.begin(), ordemPosicoes.end(), 0);
        std::shuffle(ordemPosicoes.begin(), ordemPosicoes.end(), rng);

        for (int pos : ordemPosicoes)
        {
            maquina[maqDestino].insert(maquina[maqDestino].begin() + pos, op);

            double solAtual = objectiveFunction(maquina, vetOperacoes, controleOp, tardiness_teste);

            if (solAtual < melhorSol && solAtual < INT_MAX)
            {
                melhorSol = solAtual;
                melhorPosicao = pos;
            }

            maquina[maqDestino].erase(maquina[maqDestino].begin() + pos);
        }

        // Insere na melhor posição válida ou reverte para a origem
        if (melhorPosicao != -1)
        {
            maquina[maqDestino].insert(maquina[maqDestino].begin() + melhorPosicao, op);
        }
        else
        {
            maquina[maqOrigem].insert(maquina[maqOrigem].begin() + posOrigem, op);
        }
    }

    return objectiveFunction(maquina, vetOperacoes, controleOp, tardiness_maq);
}

double re_insertion(std::vector<std::vector<Operation>> &maquina,
                    std::vector<Operation> &vetOperacoes,
                    std::map<int, std::map<int, int>> &controleOp,
                    std::vector<double> &tardiness_maq)
{
    double r0 = objectiveFunction(maquina, vetOperacoes, controleOp, tardiness_maq);
    double resultadoAtual = r0;

    int numMaquinas = maquina.size();

    // Ordena máquinas da maior para a menor tardiness
    std::vector<int> indicesMaquinas(numMaquinas);
    std::iota(indicesMaquinas.begin(), indicesMaquinas.end(), 0);
    std::sort(indicesMaquinas.begin(), indicesMaquinas.end(), [&](int a, int b)
              { return tardiness_maq[a] > tardiness_maq[b]; });

    std::vector<double> tardiness_teste = tardiness_maq;

    for (int m : indicesMaquinas)
    {
        if (maquina[m].size() < 2)
            continue;

        int numOps = maquina[m].size();
        std::vector<int> ordemPosicoes(numOps);
        std::iota(ordemPosicoes.begin(), ordemPosicoes.end(), 0);
        std::shuffle(ordemPosicoes.begin(), ordemPosicoes.end(), rng);

        for (int i : ordemPosicoes)
        {
            for (int j = 0; j < numOps; ++j)
            {
                if (i == j)
                    continue;

                Operation opCopia = maquina[m][i];

                maquina[m].erase(maquina[m].begin() + i);
                maquina[m].insert(maquina[m].begin() + j, opCopia);

                double resultadoNovo = objectiveFunction(maquina, vetOperacoes, controleOp, tardiness_teste);

                // First Improvement
                if (resultadoNovo < resultadoAtual)
                {
                    tardiness_maq = tardiness_teste;
                    return resultadoNovo;
                }

                // Reverte movimento caso não melhore
                maquina[m].erase(maquina[m].begin() + j);
                maquina[m].insert(maquina[m].begin() + i, opCopia);
            }
        }
    }
    return r0;
}

double insertion_im(std::vector<std::vector<Operation>> &maquina,
                    std::vector<Operation> &vetOperacoes,
                    std::map<int, std::map<int, int>> &controleOp,
                    std::vector<double> &tardiness_maq)
{
    double r0 = objectiveFunction(maquina, vetOperacoes, controleOp, tardiness_maq);
    double resultadoAtual = r0;

    int numMaquinas = maquina.size();
    std::vector<int> indicesMaquinas(numMaquinas);
    std::iota(indicesMaquinas.begin(), indicesMaquinas.end(), 0);

    std::shuffle(indicesMaquinas.begin(), indicesMaquinas.end(), rng);
    std::stable_sort(indicesMaquinas.begin(), indicesMaquinas.end(), [&](int a, int b)
                     { return tardiness_maq[a] < tardiness_maq[b]; });

    int maq_atrasada = indicesMaquinas.back();
    indicesMaquinas.pop_back(); // Remove a máquina de origem da lista de destinos

    std::vector<double> tardiness_teste = tardiness_maq;

    for (size_t i = 0; i < maquina[maq_atrasada].size(); i++)
    {
        Operation op = maquina[maq_atrasada][i];
        maquina[maq_atrasada].erase(maquina[maq_atrasada].begin() + i);

        for (int m : indicesMaquinas)
        {
            maquina[m].push_back(op);
            int posAtual = maquina[m].size() - 1;

            int numOpsDestino = maquina[m].size();
            std::vector<int> ordemPosicoes(numOpsDestino);
            std::iota(ordemPosicoes.begin(), ordemPosicoes.end(), 0);
            std::shuffle(ordemPosicoes.begin(), ordemPosicoes.end(), rng);

            for (int j : ordemPosicoes)
            {
                Operation opCopia = maquina[m][posAtual];
                maquina[m].erase(maquina[m].begin() + posAtual);
                maquina[m].insert(maquina[m].begin() + j, opCopia);

                double resultadoNovo = objectiveFunction(maquina, vetOperacoes, controleOp, tardiness_teste);
                if (resultadoNovo < resultadoAtual)
                {
                    tardiness_maq = tardiness_teste;
                    return resultadoNovo;
                }

                maquina[m].erase(maquina[m].begin() + j);
                maquina[m].insert(maquina[m].begin() + posAtual, opCopia);
            }

            maquina[m].pop_back();
        }

        maquina[maq_atrasada].insert(maquina[maq_atrasada].begin() + i, op);
    }

    return r0;
}

double two_swap(std::vector<std::vector<Operation>> &maquina,
                std::vector<Operation> &vetOperacoes,
                std::map<int, std::map<int, int>> &controleOp,
                std::vector<double> &tardiness_maq)
{
    double r0 = objectiveFunction(maquina, vetOperacoes, controleOp, tardiness_maq);
    double resultadoAtual = r0;

    int numMaquinas = maquina.size();

    std::vector<int> indicesMaquinas(numMaquinas);
    std::iota(indicesMaquinas.begin(), indicesMaquinas.end(), 0);
    std::sort(indicesMaquinas.begin(), indicesMaquinas.end(), [&](int a, int b)
              { return tardiness_maq[a] > tardiness_maq[b]; });

    std::vector<double> tardiness_teste = tardiness_maq;

    for (int m : indicesMaquinas)
    {
        size_t n = maquina[m].size();
        if (n < 2)
            continue;

        for (size_t i = 0; i < n - 1; i++)
        {
            for (size_t j = i + 1; j < n; j++)
            {
                std::swap(maquina[m][i], maquina[m][j]);

                double novoResultado = objectiveFunction(maquina, vetOperacoes, controleOp, tardiness_teste);
                if (novoResultado < resultadoAtual)
                {
                    tardiness_maq = tardiness_teste;
                    return novoResultado;
                }

                std::swap(maquina[m][i], maquina[m][j]);
            }
        }
    }

    return r0;
}
