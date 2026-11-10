#ifndef OPERATION_H
#define OPERATION_H

/**
 * Representa uma operação pertencente a uma tarefa (job) no problema PMSTR.
 */
class Operation
{
public:
    int id;                // Identificador único da operação (PK)
    int idJob;             // Identificador do job ao qual a operação pertence
    int idOp;              // Índice da operação dentro do job (ordem de precedência)
    int toolSetId;         // Identificador do conjunto de ferramentas (0-indexed)
    int toolSetSize;       // Tamanho (slots ocupados no magazine) do conjunto de ferramentas
    double processingTime; // Tempo de processamento da operação
    double dueDate;        // Data de entrega (prazo)
    double releaseTime;    // Data de liberação (instante mínimo de início)
    bool isProcessed;      // Indica se a operação já foi processada

    Operation();
    Operation(int id, int idJob, int idOp, int toolSetId, int toolSetSize,
              double processingTime, double dueDate, double releaseTime,
              bool isProcessed = false);
};

#endif // OPERATION_H
