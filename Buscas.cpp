#include <ctime>
#include <ratio>
#include <string>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <numeric>
#include <random>
#include <utility>
#include <vector>
#include <iostream>
#include <limits>
#include <time.h>
#include <climits>
#include <deque>

#include "Operation.h"
#include "ObjectiveFunction.h"
#include "Buscas.h"

using namespace std;
std::random_device rd;
std::mt19937 rng(rd());
using namespace std::chrono;
high_resolution_clock::time_point t1;
high_resolution_clock::time_point t2;
std::chrono::high_resolution_clock::duration tempo_execucao;

double ILS(int RI, int IM, int TS, int iterations,
           double disturbance, std::vector<std::vector<Operation>> &maquina,
           std::vector<Operation> &vetOperacoes,
           std::map<int, std::map<int, int>> &controleOp,
           std::vector<double> &tardiness_maq, int totalOperacoes)
{
    // intensificação inicial
    double s = re_insertion(RI, maquina, vetOperacoes, controleOp, tardiness_maq);
    s = insertion_im(IM, maquina, vetOperacoes, controleOp, tardiness_maq);
    s = two_swap(TS,maquina, vetOperacoes, controleOp, tardiness_maq);

    double melhor = s;
    std::vector<std::vector<Operation>> melhor_sol = maquina;
    std::vector<std::vector<Operation>> sol_base = maquina;

    // loop principal da ils, onde eu defino as iterações
    for (size_t i = 0; i < iterations; i++)
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

        // Perturbação
        int num_ops = static_cast<int>(std::max(1, static_cast<int>(std::ceil(totalOperacoes * disturbance))));
        double s_atual = pertubacao_enviesada(maquina, vetOperacoes, controleOp, tardiness_maq, ops_emEspera, num_ops);

        // Busca Local (Intensificação)

        double s_candidato = s_atual;

        int qual = 0;
        while (qual < 3)
        {
            if (qual == 0)
            {
                s_candidato = re_insertion(RI, maquina, vetOperacoes, controleOp, tardiness_maq);
            }
            else if (qual == 1)
            {
                s_candidato = insertion_im(IM, maquina, vetOperacoes, controleOp, tardiness_maq);
            }
            else if (qual == 2)
            {
                s_candidato = two_swap(TS,maquina, vetOperacoes, controleOp, tardiness_maq);
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

        if (s_atual < melhor)
        {
            melhor = s_atual;
            melhor_sol = maquina;
            iterations = 0;
        }

        // Critério de Aceitação
        if (s_candidato <= (melhor * 1.10))
        {
            sol_base = maquina;
            s = s_candidato;
        }
        // Se não entrar no if acima, sol_base continua sendo a anterior
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

    // pegando as operações que causaram espera
    for (const auto &op : ops_emEspera)
    {
        if (selecionadas.size() >= (size_t)num_ops)
            break;
        selecionadas.push_back(op);
    }

    // completando com aleatórias se necessário
    while (selecionadas.size() < (size_t)num_ops)
    {
        int m_rand = std::uniform_int_distribution<>(0, numMaquinas - 1)(rng);
        if (maquina[m_rand].empty())
            continue;
        int pos_rand = std::uniform_int_distribution<>(0, maquina[m_rand].size() - 1)(rng);

        selecionadas.push_back(std::make_pair(m_rand, maquina[m_rand][pos_rand]));
    }

    std::vector<double> tardiness_teste = tardiness_maq;

    // perturbação de fato
    for (const auto &item : selecionadas)
    {
        int maqOrigem = item.first;
        Operation op = item.second;

        // remove a operação da máquina de origem
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

        // escolhe máquina de destino aleatória e DIFERENTE da origem
        int maqDestino = maqOrigem;
        if (numMaquinas > 1)
        {
            while (maqDestino == maqOrigem)
            {
                maqDestino = std::uniform_int_distribution<>(0, numMaquinas - 1)(rng);
            }
        }

        // testa todas as posições possíveis na máquina de destino
        int melhorPosicao = -1;
        double melhorSol = std::numeric_limits<double>::max();

        int numOpsDestino = maquina[maqDestino].size();

        // embaralhando a ordem de teste
        std::vector<int> ordemPosicoes(numOpsDestino + 1); // +1 pq pode inserir no final
        std::iota(ordemPosicoes.begin(), ordemPosicoes.end(), 0);
        std::shuffle(ordemPosicoes.begin(), ordemPosicoes.end(), rng);

        // testando
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

        // insere real
        if (melhorPosicao != -1)
        {
            maquina[maqDestino].insert(maquina[maqDestino].begin() + melhorPosicao, op);
        }
        else
        {
            // caso bizarro onde nada melhora, reverte p essa operação; conversar com o leo dps sobre
            maquina[maqOrigem].insert(maquina[maqOrigem].begin() + posOrigem, op);
        }
    }

    return objectiveFunction(maquina, vetOperacoes, controleOp, tardiness_maq);
}

double pertubacao(std::vector<std::vector<Operation>> &maquina,
                  std::vector<Operation> &vetOperacoes,
                  std::map<int, std::map<int, int>> &controleOp,
                  std::vector<double> &tardiness_maq,
                  int num_ops)
{
    int numTrocas = num_ops;
    int numMaquinas = maquina.size();

    for (int k = 0; k < numTrocas; ++k)
    {
        int m1 = std::uniform_int_distribution<>(0, numMaquinas - 1)(rng);
        int m2 = std::uniform_int_distribution<>(0, numMaquinas - 1)(rng);

        if (m1 == m2 || maquina[m1].empty() || maquina[m2].empty())
        {
            k--; // Tenta novamente
            continue;
        }

        // Escolhe posições aleatórias
        int pos1 = std::uniform_int_distribution<>(0, maquina[m1].size() - 1)(rng);
        int pos2 = std::uniform_int_distribution<>(0, maquina[m2].size() - 1)(rng);

        // Troca as operações entre máquinas (Inter-machine Swap)
        std::swap(maquina[m1][pos1], maquina[m2][pos2]);
    }

    return objectiveFunction(maquina, vetOperacoes, controleOp, tardiness_maq);
}

double re_insertion(int RI, std::vector<std::vector<Operation>> &maquina,
                    std::vector<Operation> &vetOperacoes,
                    std::map<int, std::map<int, int>> &controleOp,
                    std::vector<double> &tardiness_maq)
{
    if (RI == 0)
    {
        return;
    }

    double r0 = objectiveFunction(maquina, vetOperacoes, controleOp, tardiness_maq);
    double resultadoAtual = r0;

    int numMaquinas = maquina.size();

    // Máquina com maior atraso primeiro
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

                // First Improvement: Se melhorou, encerra a busca local e retorna S'
                if (resultadoNovo < resultadoAtual)
                {
                    tardiness_maq = tardiness_teste;
                    return resultadoNovo;
                }

                // se não melhorou reverte
                maquina[m].erase(maquina[m].begin() + j);
                maquina[m].insert(maquina[m].begin() + i, opCopia);
            }
        }
    }
    return r0;
}

double insertion_im(int IM, std::vector<std::vector<Operation>> &maquina,
                    std::vector<Operation> &vetOperacoes,
                    std::map<int, std::map<int, int>> &controleOp,
                    std::vector<double> &tardiness_maq)
{

    if (IM == 0)
    {
        return;
    }
    
    double r0 = objectiveFunction(maquina, vetOperacoes, controleOp, tardiness_maq);
    double resultadoAtual = r0;

    int numMaquinas = maquina.size();
    std::vector<int> indicesMaquinas(numMaquinas);
    std::iota(indicesMaquinas.begin(), indicesMaquinas.end(), 0);

    std::shuffle(indicesMaquinas.begin(), indicesMaquinas.end(), rng);

    std::stable_sort(indicesMaquinas.begin(), indicesMaquinas.end(), [&](int a, int b)
                     { return tardiness_maq[a] < tardiness_maq[b]; });

    int maq_atrasada = indicesMaquinas.back();
    indicesMaquinas.pop_back(); // Remove a origem da lista de destinos

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

double two_swap(int TS,std::vector<std::vector<Operation>> &maquina,
                std::vector<Operation> &vetOperacoes,
                std::map<int, std::map<int, int>> &controleOp,
                std::vector<double> &tardiness_maq)
{

    if (TS == 0)
    {
        return;
    }
    

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

/* long agrupaOp(std::vector<std::vector<Operation>> &maquina,
              std::vector<Operation> &vetOperacoes,
              std::map<int, std::map<int, int>> &controleOp,
              std::vector<double> &tardiness_maq, std::map<int, std::deque<Operation>> tarefas)
{

    long r0 = objectiveFunction(maquina, vetOperacoes, controleOp, tardiness_maq);
    long resultadoAtual = r0;

    int numMaquinas = maquina.size();

    std::vector<int> indicesMaquinas(numMaquinas);
    std::iota(indicesMaquinas.begin(), indicesMaquinas.end(), 0);
    std::sort(indicesMaquinas.begin(), indicesMaquinas.end(), [&](int a, int b)
              { return tardiness_maq[a] < tardiness_maq[b]; });

    std::vector<double> tardiness_teste = tardiness_maq;

    for (int m : indicesMaquinas)
    {
        size_t n = maquina[m].size();
        if (n < 2)
            continue;

        std::vector<int> sequenciaInicial = {0};

        for (size_t i = 0; i < n; i++)
        {
            int idJob = maquina[m][i].idJob;
            int idOp = maquina[m][i].idOp;

            if (idOp > 1)
            {
                Operation ant = tarefas[idJob][idOp - 2];
                // ... lógica para agrupar ant ...
            }

            if (idOp < tarefas[idJob].size())
            {
                Operation suc = tarefas[idJob][idOp];

                // ... lógica para agrupar suc ...
            }
        }
    }
} */
