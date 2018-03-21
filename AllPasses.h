#ifndef ALLPASSES_H
#define ALLPASSES_H

#include "llvm/ADT/StringRef.h"
#include "llvm/Pass.h"

// LLVM passes are classes, but usually they are created using a factory
// function. This allows to hide optimization pass implementation. With analysis
// passes, you have to follow the same idiom. The difference is that instead of
// returning a llvm::Pass * you have to return a pointer to the analysis class.

class DataDependencyGraph;
class ControlDependencyGraph;
class ProgramDependencyGraph;
class SystemDataDependencyGraph;
//class FlowDependenceAnalysis;

//static RegisterPass<FlowDependenceAnalysis> FDA("fda", "FlowDependenceAnalysis calling", false, false);

// Analysis.
DataDependencyGraph *CreateDataDependencyGraphPass();
ControlDependencyGraph *CreateControlDependencyGraphPass();
ProgramDependencyGraph *CreateProgramDependencyGraphPass();
SystemDataDependencyGraph *CreateSystemDataDependencyGraphPass();
//FlowDependenceAnalysis *CreateFlowDataDependenceAnalysisPass();

#endif 