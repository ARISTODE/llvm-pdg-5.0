#ifndef DATADEPENDENCIES_H
#define DATADEPENDENCIES_H

//#include "AllPasses.h"
//#include "DependencyGraph.h"
//#include "FunctionWrapper.h"
#include "PDGHelper.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"
#include "llvm/Pass.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/MemoryBuiltins.h"
#include "llvm/Analysis/MemoryDependenceAnalysis.h"
#include "llvm/Support/raw_ostream.h"

//#include "llvm/Analysis/AliasAnalysis.h"
//extern std::map<const Function *, FunctionWrapper *> funcMap;
//extern std::set<InstructionWrapper *> instnodes;
//extern std::set<InstructionWrapper *> globalList;
//extern std::map<const llvm::Instruction *, InstructionWrapper *> instMap;
//extern std::map<const llvm::Function *, std::set<InstructionWrapper *>>
//    funcInstWList;

namespace pdg {
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

        void  initializeMemoryDependencyPasses();

        void  constructFuncMapAndCreateFunctionEntry();

        void collectDefUseInfo(llvm::Instruction *inst);

        void collectCallInstUseInfo(llvm::Instruction *inst);

        void collectDataDependencyInFunc();

        virtual std::vector<Instruction *> getDependencyInFunction(Instruction *pLoadInst);

        void collectRAWDependency(llvm::Instruction *inst);

        void collectNonLocalMemDependency(llvm::Instruction *inst);

        virtual bool runOnFunction(llvm::Function &F);

        virtual void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

        virtual llvm::StringRef getPassName() const {
            return "Data Dependency Graph";
        }

        virtual void print(llvm::raw_ostream &OS, const llvm::Module *M = 0) const;

    private:
        AliasAnalysis *aliasAnalysis;
        llvm::Function *func;
        llvm::MemoryDependenceResults *MD;
    };
}

namespace llvm {
    template <>
    struct GraphTraits<pdg::DataDependencyGraph *> : public GraphTraits<pdg::DepGraph *> {
        static NodeRef getEntryNode(pdg::DataDependencyGraph *DG) {
            return *(DG->DDG->begin_children());
        }

        static nodes_iterator nodes_begin(pdg::DataDependencyGraph *DG) {
            return DG->DDG->begin_children();
        }

        static nodes_iterator nodes_end(pdg::DataDependencyGraph *DG) {
            return DG->DDG->end_children();
        }
    };

} // namespace llvm

#endif