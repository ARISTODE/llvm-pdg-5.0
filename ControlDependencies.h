/** ---*- C++ -*--- ControlDependencies.h
 *
 * Copyright (C) 2015 soslab
 *
 */
#ifndef CONTROLDEPENDENCIES_H
#define CONTROLDEPENDENCIES_H

//#include "llvm/Analysis/PostDominators.h"
//#include "FunctionWrapper.h"
#include "PDGHelper.h"
#include "llvm/IR/Dominators.h"
#include "llvm/Analysis/PostDominators.h"
#include "llvm/ADT/DepthFirstIterator.h"
#include "llvm/IR/CFG.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "tree.hh"


//extern std::map<const Function *, FunctionWrapper *> funcMap;
//extern std::set<InstructionWrapper *> instnodes;
//extern std::set<InstructionWrapper *> globalList;
//extern std::map<const llvm::Instruction *, InstructionWrapper *> instMap;
//extern std::map<const llvm::Function *, std::set<InstructionWrapper *>>
//    funcInstWList;
/*!
 * Control Dependency Graph
 */

namespace pdg {
    typedef DependencyGraph<InstructionWrapper> ControlDepGraph;
    struct ControlDependencyGraph : public llvm::FunctionPass {
    public:
        static char ID; // Pass ID, replacement for typeid
        ControlDepGraph *CDG;
        llvm::PostDominatorTree *PDT;

        ControlDependencyGraph() : llvm::FunctionPass(ID) {
            isLibrary = false;
            CDG = new ControlDepGraph();
        }

        ~ControlDependencyGraph() {
            releaseMemory();
            delete CDG;
        }

        InstructionWrapper *getRoot() const { return root; }

        int getDependenceType(const llvm::BasicBlock *AW,
                              const llvm::BasicBlock *BW) const;

        bool isLibraryCall() { return isLibrary; }

        /// Zhiyuan: this is for library call
        void mockLibraryCall(llvm::Function &F);

        void computeDependencies(llvm::Function &F, llvm::PostDominatorTree *PDT);

        void createFunctionEntryNode();

        void findAllEdges();

        void computeControlDependencyFromEdgeSet();

        bool runOnFunction(llvm::Function &F);

        void getAnalysisUsage(llvm::AnalysisUsage &AU) const;

        llvm::StringRef getPassName() const;

        void print(llvm::raw_ostream &OS, const llvm::Module *M = 0) const;

    private:
        /// added by Zhiyuan: Mar 4, 2015. transfer basic blocks to instructions
        void addDependency(InstructionWrapper *from, llvm::BasicBlock *to, int type);

        void addDependency(llvm::BasicBlock *from, llvm::BasicBlock *to, int type);

        /// added by Zhiyuan: Feb 19, 2015.
        llvm::Function *func;
        InstructionWrapper *root;
        std::vector<std::pair<BasicBlock *, BasicBlock *>> EdgeSet;
        bool isLibrary;
        static int entry_id;
    };
}
namespace llvm
{
    template <> struct GraphTraits<pdg::ControlDependencyGraph *>
            : public GraphTraits<pdg::DepGraph*> {
        static NodeRef getEntryNode(pdg::ControlDependencyGraph *CG) {
            return *(CG->CDG->begin_children());
        }

        static nodes_iterator nodes_begin(pdg::ControlDependencyGraph *CG) {
            return CG->CDG->begin_children();
        }

        static nodes_iterator nodes_end(pdg::ControlDependencyGraph *CG) {
            return CG->CDG->end_children();
        }
    };
}

#endif
