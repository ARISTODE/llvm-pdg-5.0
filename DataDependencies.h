#ifndef DATADEPENDENCIES_H
#define DATADEPENDENCIES_H

//#include "AllPasses.h"
#include "DependencyGraph.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Type.h"
#include "llvm/Pass.h"
#include "FunctionWrapper.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Support/raw_ostream.h"

//#include "llvm/Analysis/AliasAnalysis.h"
extern std::map<const Function *, FunctionWrapper *> funcMap;
extern std::set<InstructionWrapper *> instnodes;
extern std::set<InstructionWrapper *> instnodes;
extern std::set<InstructionWrapper *> globalList;
extern std::map<const llvm::Instruction *, InstructionWrapper *> instMap;
extern std::map<const llvm::Function *, std::set<InstructionWrapper *>>
    funcInstWList;

static llvm::ModRefInfo GetLocation(const llvm::Instruction *Inst,
                                    llvm::MemoryLocation &Loc,
                                    llvm::AliasAnalysis *AA);

llvm::ModRefInfo GetLocation(const llvm::Instruction *Inst,
                             llvm::MemoryLocation &Loc,
                             llvm::AliasAnalysis *AA);

typedef DependencyGraph<InstructionWrapper> DataDepGraph;

static std::vector<InstructionWrapper *> gp_list;

class DataDependencyGraph : public llvm::FunctionPass {
public:
  static char ID; // Pass ID, replacement for typeid
  DataDepGraph *DDG;

  DataDependencyGraph() : FunctionPass(ID) { DDG = new DataDepGraph(); }

  ~DataDependencyGraph() {
    releaseMemory();
    delete DDG;
  }

  virtual bool runOnFunction(llvm::Function &F);

  virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

  virtual llvm::StringRef getPassName() const {
    return "Data Dependency Graph";
  }

  virtual void print(llvm::raw_ostream &OS, const llvm::Module *M = 0) const;
};

namespace llvm
{

  template <> struct GraphTraits<DataDependencyGraph *>
      : public GraphTraits<DepGraph*> {
    static NodeRef getEntryNode(DataDependencyGraph *DG) {
      return *(DG->DDG->begin_children());
    }

    static nodes_iterator nodes_begin(DataDependencyGraph *DG) {
      return DG->DDG->begin_children();
    }

    static nodes_iterator nodes_end(DataDependencyGraph *DG) {
      return DG->DDG->end_children();
    }
  };

}


#endif