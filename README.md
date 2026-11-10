# PMSTR-irace — Calibração Automática de Parâmetros da ILS para o PMSTR

Adaptação do solver **Iterated Local Search (ILS)** para o problema de escalonamento em máquinas paralelas com trocas de ferramentas (*Parallel Machine Scheduling with Tool Replacements* — **PMSTR**), estruturada especificamente para a **calibração automática de hiperparâmetros** utilizando o pacote [**irace**](https://mlopez-ibanez.github.io/irace/) (*Iterated Racing for Automatic Algorithm Configuration*).

> **Repositório Principal (Pai):**  
> Este repositório é um subprojeto voltado à calibração de parâmetros. A implementação principal do solver, os experimentos completos e a documentação detalhada da metaheurística encontram-se no repositório pai:  
> 🔗 **[anaclpsouza/PMSTR](https://github.com/anaclpsouza/PMSTR)**
>
> **Artigo de Referência do Problema:**  
> *"[A matheuristic for parallel machine scheduling with tool replacements](https://doi.org/10.1016/j.ejor.2020.09.050)"* (European Journal of Operational Research, 2021).

---

## Objetivo deste Repositório

A eficiência da metaheurística **ILS com VND (Variable Neighborhood Descent)** aplicada ao PMSTR depende diretamente da escolha de seus componentes de busca local, do critério de parada por estagnação e da intensidade de perturbação.

Este projeto adapta a interface do solver original ([`PMSTR`](https://github.com/anaclpsouza/PMSTR)) para receber configurações de parâmetros via linha de comando e retornar diretamente o custo da solução encontrada na saída padrão (`stdout`), permitindo que o **irace** explore o espaço de configurações e identifique estatisticamente os melhores parâmetros.

---

## O Problema (PMSTR)

O **PMSTR** consiste em alocar e sequenciar um conjunto de $O$ **operações** (organizadas em *jobs* com restrições de precedência entre suas operações) em $M$ **máquinas paralelas idênticas**, respeitando:

- **Release time ($r_j$)** — instante mínimo em que uma operação pode iniciar seu processamento.
- **Precedência** — a operação $k$ de um *job* só pode iniciar após a conclusão da operação $k-1$ do mesmo *job* (mesmo que alocada em outra máquina).
- **Due date ($d_j$)** — prazo de entrega desejado para a operação.
- **Restrição de ferramentas (Magazine de capacidade $C$)** — cada operação exige um conjunto de ferramentas dentre $T$ disponíveis. Quando o conjunto necessário não está carregado na máquina, ocorre uma troca de ferramentas (*setup*) gerenciada pela política **KTNS** (*Keep Tool Needed Soonest*), consumindo um tempo de troca $\tau = 1\text{h}$.

A **função objetivo** minimiza a soma ponderada do **atraso total (*total tardiness*)** e do **custo de trocas de ferramentas (*tool setups*)**:

$$
\min \; w_d \sum \text{Tardiness} + w_s \cdot \tau \cdot \text{Setups}
$$

---

## Espaço de Parâmetros Calibrados (`parameters.txt`)

O arquivo `parameters.txt` define os hiperparâmetros avaliados pelo **irace**:

| Parâmetro | Flag CLI | Tipo | Domínio / Valores | Descrição |
| :--- | :---: | :---: | :---: | :--- |
| `re_insertion` | `--ri` | Inteiro (`i`) | `(0, 1)` | Ativa (`1`) ou desativa (`0`) a busca local **Re-insertion** (realocação intra-máquina) |
| `insertion_im` | `--im` | Inteiro (`i`) | `(0, 1)` | Ativa (`1`) ou desativa (`0`) a busca local **Insertion-IM** (realocação inter-máquinas) |
| `two_swap` | `--ts` | Inteiro (`i`) | `(0, 1)` | Ativa (`1`) ou desativa (`0`) a busca local **Two-Swap** (troca de pares intra-máquina) |
| `iterations` | `--i` | Categórico (`c`) | `(50, 70, 100)` | Número máximo de iterações consecutivas sem melhoria global na ILS |
| `disturbance` | `--p` | Categórico (`c`) | `(0.01, 0.02, 0.03, 0.04, 0.05)` | Percentual do total de operações realocadas na fase de perturbação enviesada |

---

## Estrutura do Projeto

```text
PMSTR-irace/
├── Main.cpp               # Ponto de entrada: lê parâmetros CLI e instância via stdin,
│                          #   gera a solução inicial e imprime o custo final da ILS
├── Buscas.cpp             # Implementação da ILS (VND configurável + perturbação enviesada)
│                          #   e das buscas locais (Re-insertion, Insertion-IM, Two-Swap)
├── Buscas.h               # Declarações da ILS, perturbação e buscas locais
├── ObjectiveFunction.cpp  # Avaliação da função objetivo, precedência, detecção de ciclos e KTNS
├── ObjectiveFunction.h    # Declaração da função objetivo e variáveis globais da instância
├── Operation.h            # Classe que modela uma operação (job, op, tempos e ferramentas)
├── Operation.cpp          # Implementação dos construtores de Operation
├── target-runner.cpp      # Executável ponte invocado pelo irace para chamar ./Main
├── Makefile               # Script de compilação dos binários Main e target-runner
├── parameters.txt         # Definição dos hiperparâmetros e domínios para o irace
├── scenario.txt           # Configuração do cenário de execução do irace (orçamento, paralelismo, etc.)
├── instances/             # Subconjunto de instâncias selecionadas para o treinamento no irace
├── basecases/             # Conjunto geral de instâncias base (2M38, 2M46, 6M140, 6M163)
└── scripts/
    └── move_random_basecases.py  # Script utilitário para amostragem aleatória de instâncias
```

---

## Compilação

O projeto requer um compilador **g++** com suporte ao padrão **C++20**.

```bash
# Compilar os executáveis Main e target-runner
make

# Limpar binários compilados
make clean
```

Os binários são compilados com as flags de otimização `-O3 -march=native -std=c++20`.

---

## Como Executar

### 1. Executar o Calibrador (`irace`)

Certifique-se de ter o **R** e o pacote **irace** instalados, além de ter compilado os binários `Main` e `target-runner` com `make`. Em seguida, na raiz do projeto:

```bash
irace --scenario scenario.txt
```

Ou diretamente de uma sessão do **R**:

```r
library(irace)
scenario <- readScenario(filename = "scenario.txt")
irace(scenario = scenario)
```

Principais configurações definidas em `scenario.txt`:
- **Diretório de instâncias de treino (`trainInstancesDir`):** `./instances`
- **Orçamento máximo (`maxExperiments`):** `5000` execuções do `target-runner`
- **Teste estatístico (`testType`):** `F-test` (confiança de 95%, elitista)
- **Execução paralela (`parallel`):** `10` processos simultâneos
- **Arquivo de log (`logFile`):** `./irace.Rdata`

### 2. Executar o `target-runner` Manualmente (Teste de Integração)

O `target-runner` recebe os argumentos no formato padrão exigido pelo **irace**:

```bash
./target-runner <id_config> <id_instancia> <seed> <caminho_instancia> [bound] <parametros...>
```

**Exemplo:**

```bash
./target-runner 1 1 12345 instances/2M38_95C.csv --ri 1 --im 1 --ts 1 --i 100 --p 0.02
```

### 3. Executar o Solver `./Main` Individualmente

O executável `Main` lê a instância a partir da entrada padrão (`stdin`) e recebe os hiperparâmetros via flags de linha de comando:

```bash
./Main --ri 1 --im 1 --ts 0 --i 100 --p 0.02 < instances/2M38_95C.csv
```

A saída impressa no terminal será um único valor numérico correspondente ao custo da melhor solução encontrada pela ILS.

---

## Seleção de Instâncias (`scripts/move_random_basecases.py`)

Para evitar *overfitting* na calibração e manter o tempo de treinamento viável, um subconjunto representativo das instâncias de `basecases/` é separado no diretório `instances/`. O script `scripts/move_random_basecases.py` auxilia na seleção amostral de arquivos:

```bash
# Visualizar (dry-run) uma amostra de 10% das instâncias com semente fixa
python3 scripts/move_random_basecases.py --dry-run --percent 10 --seed 42
```

---

## Links Relacionados

- **Repositório Pai (Solver completo e análise experimental):** [https://github.com/anaclpsouza/PMSTR](https://github.com/anaclpsouza/PMSTR)
- **Documentação do pacote irace:** [https://mlopez-ibanez.github.io/irace/](https://mlopez-ibanez.github.io/irace/)

