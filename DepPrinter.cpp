// dependency graph is in functionwrapper.h
#include "ControlDependencies.h"
#include "DataDependencies.h"
#include "llvm/Analysis/DOTGraphTraitsPass.h"


namespace llvm {

    template <>
    struct DOTGraphTraits<DepGraphNode *> : public DefaultDOTGraphTraits {
        DOTGraphTraits (bool isSimple = false): DefaultDOTGraphTraits(isSimple) {}

        std::string getNodeLabel(DepGraphNode *Node, DepGraphNode *Graph){

            const InstructionWrapper *instW = Node->getData();

            //TODO: why nullptr for Node->getData()?
            if(instW == nullptr){
                errs() <<"instW " << instW << "\n";
                return "null instW";
            }

            std::string Str;
            raw_string_ostream OS(Str);

            switch(instW->getType()) {
                case ENTRY:
                    return ("<<ENTRY>> " + instW->getFunctionName());

                case GLOBAL_VALUE:{
                    OS << *instW->getValue();
                    return ("GLOBAL_VALUE:" + OS.str());
                }

                case FORMAL_IN:{
                    OS << *instW->getArgument()->getType();
                    return ("FORMAL_IN:" + OS.str());
                }

                case ACTUAL_IN:{
                    OS << *instW->getArgument()->getType();
                    return ("ACTUAL_IN:" + OS.str() );
                }
                case FORMAL_OUT:{
                    OS << *instW->getArgument()->getType();
                    return ("FORMAL_OUT:" + OS.str());
                }

                case ACTUAL_OUT:{
                    OS << *instW->getArgument()->getType();
                    return ("ACTUAL_OUT:" + OS.str());
                }

                case PARAMETER_FIELD:{
                    OS << instW->getFieldId() << " " << *instW->getFieldType();
                    return OS.str();
                }

                    //for pointer node, add a "*" sign before real node content
                    //if multi level pointer, use a loop instead here
                case POINTER_RW:{
                    OS << *instW->getArgument()->getType();
                    return ("POINTER READ/WRITE : *" + OS.str());
                }

                    break;
            }

            const Instruction *inst = Node->getData()->getInstruction();

            if (isSimple() && !inst->getName().empty()) {
                return inst->getName().str();
            } else {
                std::string Str;
                raw_string_ostream OS(Str);
                OS << *inst;
                return OS.str();}
        }
    };

    template <>
    struct DOTGraphTraits<DepGraph *>
            : public DOTGraphTraits<DepGraphNode *>
    {
        DOTGraphTraits (bool isSimple = false)
                : DOTGraphTraits<DepGraphNode *>(isSimple) {}

        std::string getNodeLabel(DepGraphNode *Node, DepGraph *Graph) {
          return DOTGraphTraits<DepGraphNode *>::getNodeLabel(
              Node, *(Graph->begin_children()));
        }
    };

    template <>
    struct DOTGraphTraits<ControlDependencyGraph *>
        : public DOTGraphTraits<DepGraph *> {
      DOTGraphTraits(bool isSimple = false)
          : DOTGraphTraits<DepGraph *>(isSimple) {}

      static std::string getGraphName(ControlDependencyGraph *) {
        return "Instruction-Level Control dependency graph";
      }

      std::string getNodeLabel(DepGraphNode *Node,
                               ControlDependencyGraph *Graph) {
        return DOTGraphTraits<DepGraph *>::getNodeLabel(Node, Graph->CDG);
      }

      static std::string
      getEdgeSourceLabel(DepGraphNode *Node,
                         DependencyLinkIterator<InstructionWrapper> EI) {
        //    errs() << "getEdgeSourceLabel(): type = " <<
        //    EI.getDependencyType() << "\n";
        switch (EI.getDependencyType()) {

        default:
          return "";
        }
      }
    };

    } // namespace llvm


struct ControlDependencyViewer
                : public DOTGraphTraitsViewer<ControlDependencyGraph, false>{
            static char ID;
            ControlDependencyViewer() :
                    DOTGraphTraitsViewer<ControlDependencyGraph, false>("cdgraph", ID) {} };


char ControlDependencyViewer::ID = 0;
static RegisterPass<ControlDependencyViewer> CDGViewer("view-cdg", "View control dependency graph of function", false, false);

struct ControlDependencyPrinter
                : public DOTGraphTraitsPrinter<ControlDependencyGraph, false>
        {
            static char ID;
            ControlDependencyPrinter()
                    : DOTGraphTraitsPrinter<ControlDependencyGraph, false>("cdgragh", ID) {}
        };

char ControlDependencyPrinter::ID = 0;
static RegisterPass<ControlDependencyPrinter> CDGPrinter("dot-cdg", "Print control dependency graph of function to 'dot' file", false, false);



