#ifndef DSAGENERATOR_H
#define DSAGENERATOR_H

#include "ProgramDependencies.h"
#include "DSAGenerator.h"

#include "llvm/Support/raw_ostream.h"
#include <vector>
#include <set>
#include <string.h>

namespace {
    using namespace llvm;

    class IdlGenerator : public ModulePass {
    public:
        static char ID;

        IdlGenerator() : ModulePass(ID) {
            
        }

        // do main work here
        bool runOnModule(Module &M) {
            // pdg::ProgramDependencyGraph &pdggraph = getAnalysis<pdg::ProgramDependencyGraph>();
            // pdggraph.printArgUseInfo(M, imported_functions);

            // CallGraph &call_graph = getAnalysis<CallGraphWrapperPass>().getCallGraph();
            // for (llvm::Function &F : M) {
            //     if (F.isDeclaration()) {
            //         continue;
            //     }
            //     auto call_node = call_graph[&F];
            //     errs() << "Printing info for func: " << F.getName() << "\n";
            //     for (auto callerNodeIter = call_node->begin(); callerNodeIter
            //     != call_node->end(); ++callerNodeIter) {
            //         //errs() <<
            //         (*callerNodeIter).second->getFunction()->getName() <<
            //         "\n"; errs() << (*callerNodeIter).first;
            //     }
            // }

            for (llvm::Function &F : M) {
              if (F.isDeclaration()) {
                continue;
              }

              errs() << F.getName() << "\n";
              for (llvm::inst_iterator instIter = inst_begin(F); instIter != inst_end(F); ++instIter) {
                  constructTypeInfoForCallInst(&*instIter);
              }
            }
        }

        void constructTypeInfoForCallInst(Instruction *inst) {
            if (CallInst *callinst = dyn_cast<CallInst>(inst)) {
              for (unsigned i = 0; i < callinst->getNumArgOperands(); ++i) {
                if (Value *tmp_val = dyn_cast<Value>(callinst->getArgOperand(i))) {
                    errs() << tmp_val->getType()->getTypeID() << "\n";
                }
              }
            }
        }

        // Analysis Usage, specify PDG at this time
        void getAnalysisUsage(AnalysisUsage &AU) const {
          // AU.addRequired<DSAGenerator>();
          // AU.addRequired<pdg::ProgramDependencyGraph>();
          // AU.addRequired<CallGraphWrapperPass>();
          AU.setPreservesAll();
        }

      private:
        std::set<std::string> imported_functions;
        std::map<llvm::Function, std::map<std::string, bool>> arg_use_info_map;
    };

    char IdlGenerator::ID = 0;
    static RegisterPass<IdlGenerator>
        IdlGenerator("idl-gen", "IDL generation for kernel", false, true);
    } // namespace

#endif
