#include "Operation.h"

Operation::Operation() : id(0), idJob(0), idOp(0), toolSetId(0), toolSetSize(0), processingTime(0), dueDate(0), releaseTime(0), isProcessed(0) {}

Operation::Operation(int id, int idJob, int idOp, int toolSetId, int toolSetSize, double processingTime, double dueDate, double releaseTime, bool isProcessed)
	: id(id), idJob(idJob), idOp(idOp), toolSetId(toolSetId), toolSetSize(toolSetSize), processingTime(processingTime), dueDate(dueDate), releaseTime(releaseTime), isProcessed(isProcessed) {}
