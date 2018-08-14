#include "ProgramDependencies.h"
#include "DSAGenerator.h"

#define SUCCEED 0
#define NULLPTR 1
#define RECURSIVE_TYPE 2

using namespace llvm;
using namespace std;

char pdg::ProgramDependencyGraph::ID = 0;

static std::set<Type*> recursive_types;
//static std::set<Type*> unseparated_types;

tree<pdg::InstructionWrapper*>::iterator pdg::ProgramDependencyGraph::getInstInsertLoc(pdg::ArgumentWrapper *argW, TypeWrapper *tyW, TreeType treeTy) {
    tree<pdg::InstructionWrapper*>::iterator insert_loc;
    insert_loc = argW->getTree(treeTy).begin();
    size_t treeSize = argW->getTree(treeTy).size();
    while (treeSize > 1) {
        insert_loc++;
        treeSize -= 1;
    }
    return insert_loc;
}

pdg::ArgumentWrapper* getArgWrapperFromFunction(pdg::FunctionWrapper *funcW, Argument *arg){
    for(std::list<pdg::ArgumentWrapper*>::iterator argWI = funcW->getArgWList().begin(),
                argWE = funcW->getArgWList().end(); argWI != argWE; ++argWI){
        if( (*argWI)->getArg() == arg)
            return *argWI;
    }
    return nullptr;
}

void pdg::ProgramDependencyGraph::insertArgToTree(TypeWrapper *tyW, ArgumentWrapper *pArgW, TreeType treeTy,
                                                  tree<InstructionWrapper *>::iterator insertLoc) {
    Argument *arg = pArgW->getArg();
    TypeWrapper *tempTyW = new TypeWrapper(tyW->getType()->getPointerElementType(), id);
    llvm::Type *parent_type = nullptr;
    InstructionWrapper *typeFieldW = new InstructionWrapper(arg->getParent(), arg, tempTyW->getType(), parent_type, id++,PARAMETER_FIELD);
    instnodes.insert(typeFieldW);
    pArgW->getTree(treeTy).append_child(insertLoc,typeFieldW);
}

int pdg::ProgramDependencyGraph::buildFormalTypeTree(Argument *arg, TypeWrapper *tyW, TreeType treeTy, int field_pos ) {
    if(arg == nullptr){
        DEBUG(dbgs() << "In buildTypeTree, incoming arg is a nullptr\n");
        return NULLPTR;
    }

    if (tyW->getType()->isPointerTy() == false) {
        if(arg != nullptr)
            DEBUG(dbgs() << tyW->getType() << " is a Non-pointer type. arg = " << *arg << "\n");
        else
            DEBUG(dbgs() << "arg is nullptr!\n");
        return SUCCEED;
    }

    //kinds of complex processing happen when pointers come(recursion, FILE*, )...
    ArgumentWrapper *pArgW = getArgWrapperFromFunction(funcMap[arg->getParent()], arg);

    if(pArgW == nullptr){
        DEBUG(dbgs() << "getArgWrapper returns a NULL pointer!" << "\n");
        return NULLPTR;
    }

    // setup worklist to avoid recusion processing
    std::queue<TypeWrapper *> worklist;
    worklist.push(tyW);

    tree<InstructionWrapper*>::iterator insert_loc;
    while (!worklist.empty()) {
        TypeWrapper *curTyNode = worklist.front();
        worklist.pop();
        errs() << "current type node id: " << curTyNode->getId() << "\n";
        //errs() << "current node type is: " << curTyNode->getType()->getTypeID();
        insert_loc = getInstInsertLoc(pArgW, curTyNode, treeTy);

        if (curTyNode->getType()->isPointerTy() == false) {
            //insertArgToTree(curTyNode, pArgW, treeTy, insert_loc);
            continue;
        }

        // need to process the integer/float pointer differently
        if (curTyNode->getType()->getContainedType(0)->getNumContainedTypes() == 0) {
            insertArgToTree(curTyNode, pArgW, treeTy, insert_loc);
            return SUCCEED;
        }

        if(recursive_types.find(curTyNode->getType()) != recursive_types.end() ){
            DEBUG(dbgs() << curTyNode->getType() << " is a recursive type found in historic record!\n ");
            return RECURSIVE_TYPE;
        }

        //insert_loc = getInstInsertLoc(pArgW, curTyNode, treeTy);
        //if ty is a pointer, its containedType [ty->getContainedType(0)] means the type ty points to
        llvm::Type *parent_type = nullptr;

        for(unsigned int i = 0; i < curTyNode->getType()->getContainedType(0)->getNumContainedTypes(); i++) {
            int cur_field_offset = i;
            TypeWrapper *tempTyW = new TypeWrapper(curTyNode->getType()->getContainedType(0)->getContainedType(i),
                                                   cur_field_offset);

            // build a new instWrapper for each single argument and then insert the inst to instnodes.
            parent_type = curTyNode->getType();
            InstructionWrapper *typeFieldW = new InstructionWrapper(arg->getParent(), arg, tempTyW->getType(),
                                                                    parent_type, cur_field_offset, PARAMETER_FIELD);
            instnodes.insert(typeFieldW);
            funcInstWList[arg->getParent()].insert(typeFieldW);
            // start inserting formal tree instructions
            pArgW->getTree(treeTy).append_child(insert_loc, typeFieldW);
            //recursion, e.g. linked list, do backtracking along the path until reaching the root, if we can find an type that appears before,
            //use 1-limit to break the tree construction when insert_loc is not the root, it means we need to do backtracking the check recursion
            if (pArgW->getTree(treeTy).depth(insert_loc) != 0) {
                bool recursion_flag = false;
                tree<InstructionWrapper *>::iterator backTreeIt = pArgW->getTree(treeTy).parent(insert_loc);
                while (pArgW->getTree(treeTy).depth(backTreeIt) != 0) {
                    backTreeIt = pArgW->getTree(treeTy).parent(backTreeIt);
                    if ((*insert_loc)->getFieldType() == (*backTreeIt)->getFieldType()) {
                        errs() << "Find recursion type!" << "\n";
                        recursion_flag = true;
                        recursive_types.insert((*insert_loc)->getFieldType());
                        break;
                    }
                }

                if (recursion_flag) {
                    //process next contained type, because this type brings in a recursion
                    continue;
                }
            }

            //function ptr, FILE*, complex composite struct...
            if (tempTyW->getType()->isPointerTy()) {
                //if field is a function Ptr
                if (tempTyW->getType()->getContainedType(0)->isFunctionTy()) {
                    string Str;
                    raw_string_ostream OS(Str);
                    OS << *tempTyW->getType();
                    continue;
                }

                if (tempTyW->getType()->getContainedType(0)->isStructTy()) {
                    string Str;
                    raw_string_ostream OS(Str);
                    OS << *tempTyW->getType();
                    //FILE*, bypass, no need to buildTypeTree
                    if ("%struct._IO_FILE*" == OS.str() || "%struct._IO_marker*" == OS.str()) {
                        continue;
                    }
                }

                worklist.push(tempTyW);
            }
        }
    }

    return SUCCEED;
}

void pdg::ProgramDependencyGraph::buildFormalTree(Argument *arg, TreeType treeTy, int field_pos) {
    InstructionWrapper *treeTyW = nullptr;
    llvm::Type *parent_type = nullptr;
    treeTyW = new InstructionWrapper(arg->getParent(), arg, arg->getType(), parent_type, field_pos, FORMAL_IN);

    if(treeTyW != nullptr) {
        instnodes.insert(treeTyW);
    }
    else{
        DEBUG(dbgs() << "treeTyW is a null pointer!" <<"\n");
        exit(1);
    }

    //find the right arg, and set tree root
    std::list<pdg::ArgumentWrapper *>::iterator argWLoc;
    //find root node for formal trees(funcMap)
    for(auto argWI = funcMap[arg->getParent()]->getArgWList().begin(),
                argWE = funcMap[arg->getParent()]->getArgWList().end(); argWI != argWE; ++argWI){
        if((*argWI)->getArg() == arg)
            argWLoc = argWI;
    }

    auto treeRoot = (*argWLoc)->getTree(treeTy).set_head(treeTyW);

    TypeWrapper *tyW = new TypeWrapper(arg->getType(),field_pos++);

    string Str;
    raw_string_ostream OS(Str);
    OS << *tyW->getType();
    //FILE*, bypass, no need to buildTypeTree
    if("%struct._IO_FILE*" == OS.str() || "%struct._IO_marker*" == OS.str()){
        errs() << "OS.str() = " << OS.str() << " FILE* appears, stop buildTypeTree\n";
    }
    else if(tyW->getType()->isPointerTy() && tyW->getType()->getContainedType(0)->isFunctionTy()){
        errs() << *arg->getParent()->getFunctionType() << " DEBUG 312: in buildFormalTree: function pointer arg = " << *tyW->getType() << "\n";
    }
    else {
        errs() << "start building formal type tree!" << "\n";
        buildFormalTypeTree(arg, tyW, treeTy, field_pos);
    }
    id = 0;
}

void pdg::ProgramDependencyGraph::buildFormalParameterTrees(llvm::Function *callee) {
    auto argI = funcMap[callee]->getArgWList().begin();
    auto argE = funcMap[callee]->getArgWList().end();

    DEBUG(dbgs() << "Function: " << callee->getName() << " " << *callee->getFunctionType() << "\n");
    errs() << "Function: " << callee->getName() << " " << *callee->getFunctionType() << "\n";

    for(; argI != argE; ++argI){
        int field_pos = 0;
        buildFormalTree((*argI)->getArg(), FORMAL_IN_TREE, field_pos++);
        DEBUG(dbgs() << " F formalInTree size = " << (*argI)->getTree(FORMAL_IN_TREE).size() << "&&\n");
        //we use this copy way just for saving time for the tree construction
        (*argI)->copyTree((*argI)->getTree(FORMAL_IN_TREE), FORMAL_OUT_TREE);
    }

    funcMap[callee]->setTreeFlag(true);
}


void pdg::ProgramDependencyGraph::buildActualParameterTrees(CallInst *CI) {
    Function *called_func;
    // processing indirect call. Pick the first candidate function and copy its' 
    // formal parameter tree.
    if (CI->getCalledFunction() == nullptr) {
        std::vector<llvm::Function *> indirect_call_candidate = collectIndirectCallCandidates(CI->getFunctionType());
        if (indirect_call_candidate.size() == 0) { 
           errs() << "Parameter num 0, no need to build actual parameter tree" << "\n";
           return;
        }
        // get the first possible candidate
        called_func = indirect_call_candidate[0];
    } else {
        called_func = CI->getCalledFunction();
    }

    auto argI = callMap[CI]->getArgWList().begin();
    auto argE = callMap[CI]->getArgWList().end();

    auto argFI = funcMap[called_func]->getArgWList().begin();
    auto argFE = funcMap[called_func]->getArgWList().end();

    DEBUG(dbgs() << "Building actual parameter tree:" << called_func->getName() << "\n");
    //  errs() << "argFI FORMAL_IN_TREE size " << (*argFI)->getTree(FORMAL_IN_TREE).size() << "\n";

    //copy FormalInTree in callee to ActualIn/OutTree in callMap
    for(; argI != argE && argFI != argFE; ++argI,++argFI){
        (*argI)->copyTree((*argFI)->getTree(FORMAL_IN_TREE), ACTUAL_IN_TREE);
        (*argI)->copyTree((*argFI)->getTree(FORMAL_IN_TREE), ACTUAL_OUT_TREE);
        //errs() << "argIcopyTree size " << (*argI)->getTree(ACTUAL_IN_TREE).size() << " " << (*argI)->getTree(ACTUAL_OUT_TREE).size();
    }
}

void pdg::ProgramDependencyGraph::drawFormalParameterTree(Function *func,
                                                          TreeType treeTy) {
    for (std::list<ArgumentWrapper *>::iterator
                 argI = funcMap[func]->getArgWList().begin(),
                 argE = funcMap[func]->getArgWList().end();
         argI != argE; ++argI) {
        for (tree<InstructionWrapper *>::iterator
                     TI = (*argI)->getTree(treeTy).begin(),
                     TE = (*argI)->getTree(treeTy).end();
             TI != TE; ++TI) {
            for (unsigned i = 0; i < TI.number_of_children(); i++) {
                InstructionWrapper *childW = *(*argI)->getTree(treeTy).child(TI, i);
                DEBUG(dbgs() << "TreeType: ");
                DEBUG(dbgs() << treeTy);
                DEBUG(dbgs() << "\n");
                if (nullptr == *instnodes.find(*TI))
                    DEBUG(dbgs() << "DEBUG LINE 84 InstW NULL\n");
                if (nullptr == *instnodes.find(childW))
                    DEBUG(dbgs() << "DEBUG LINE 85 InstW NULL\n");
                PDG->addDependency(*instnodes.find(*TI), *instnodes.find(childW), PARAMETER);
            }
        } // end for tree
    }   // end for list
}

void pdg::ProgramDependencyGraph::drawActualParameterTree(CallInst *CI,
                                                          TreeType treeTy) {
    int ARG_POS = 0;
    for (list<ArgumentWrapper *>::iterator
                 argI = callMap[CI]->getArgWList().begin(),
                 argE = callMap[CI]->getArgWList().end();
         argI != argE; ++argI) {

        llvm::Value *tmp_val = CI->getOperand(ARG_POS); // get the corresponding argument
        if (llvm::Instruction *tmpInst = dyn_cast<llvm::Instruction>(tmp_val)) {
            auto TreeBegin = (*argI)->getTree(ACTUAL_IN_TREE).begin();
            PDG->addDependency(*instnodes.find(instMap[tmpInst]),
                               *instnodes.find(*TreeBegin), PARAMETER);
        }
        ARG_POS++;

        for (tree<InstructionWrapper *>::iterator
                     TI = (*argI)->getTree(treeTy).begin(),
                     TE = (*argI)->getTree(treeTy).end();
             TI != TE; ++TI) {
            for (unsigned i = 0; i < TI.number_of_children(); i++) {
                InstructionWrapper *childW = *(*argI)->getTree(treeTy).child(TI, i);

                DEBUG(llvm::dbgs() << "TreeType:");
                DEBUG(llvm::dbgs() << treeTy);
                DEBUG(llvm::dbgs() << "\n");

                if (nullptr == *instnodes.find(*TI))
                    DEBUG(llvm::dbgs() << "DEBUG LINE 103 InstW NULL\n");
                if (nullptr == *instnodes.find(childW))
                    DEBUG(llvm::dbgs() << "DEBUG LINE 104 InstW NULL\n");

                PDG->addDependency(*instnodes.find(*TI), *instnodes.find(childW), PARAMETER);
            }
        } // end for tree
    }   // end for list
}

int pdg::ProgramDependencyGraph::connectCallerAndCallee(InstructionWrapper *InstW, llvm::Function *callee) {
    if (InstW == nullptr || callee == nullptr) {
        return 1;
    }

    // callInst in caller --> Entry Node in callee
    PDG->addDependency(InstW, funcMap[callee]->getEntry(), CONTROL);

    Function *caller = InstW->getInstruction()->getFunction();
    // ReturnInst in callee --> CallInst in caller
    for (std::list<Instruction *>::iterator
                 RI = funcMap[callee]->getReturnInstList().begin(),
                 RE = funcMap[callee]->getReturnInstList().end();
         RI != RE; ++RI) {

        for (std::set<InstructionWrapper *>::iterator nodeIt = funcInstWList[caller].begin();
             nodeIt != funcInstWList[caller].end(); ++nodeIt) {
            if ((*nodeIt)->getInstruction() == (*RI)) {
                if (nullptr != dyn_cast<ReturnInst>((*nodeIt)->getInstruction())->getReturnValue())
                    PDG->addDependency(*instnodes.find(*nodeIt), InstW, DATA_GENERAL);
                    // TODO: indirect call, function name??
                else
                    errs() << "void ReturnInst: " << *(*nodeIt)->getInstruction();
            }
        }
    }

    // TODO: consider all kinds of connecting cases
    // connect caller InstW with ACTUAL IN/OUT parameter trees
    CallInst *CI = dyn_cast<CallInst>(InstW->getInstruction());

    for (std::list<ArgumentWrapper *>::iterator
                 argI = callMap[CI]->getArgWList().begin(),
                 argE = callMap[CI]->getArgWList().end();
         argI != argE; ++argI) {

        InstructionWrapper *actual_inW = *(*argI)->getTree(ACTUAL_IN_TREE).begin();
        InstructionWrapper *actual_outW = *(*argI)->getTree(ACTUAL_OUT_TREE).begin();

        if (nullptr == *instnodes.find(actual_inW))
            DEBUG(dbgs() << "DEBUG LINE 168 InstW NULL\n");
        if (nullptr == *instnodes.find(actual_outW))
            DEBUG(dbgs() << "DEBUG LINE 169 InstW NULL\n");

        if (InstW == *instnodes.find(actual_inW) || InstW == *instnodes.find(actual_outW)) {
            DEBUG(dbgs() << "DEBUG LINE 182 InstW duplicate");
            continue;
        }

        PDG->addDependency(InstW, *instnodes.find(actual_inW), PARAMETER);
        PDG->addDependency(InstW, *instnodes.find(actual_outW), PARAMETER);
    }

    // old way, process four trees at the same time, remove soon
    list<ArgumentWrapper *>::iterator formal_argI;
    formal_argI = funcMap[callee]->getArgWList().begin();

    list<ArgumentWrapper *>::iterator formal_argE;
    formal_argE = funcMap[callee]->getArgWList().end();

    list<ArgumentWrapper *>::iterator actual_argI;
    actual_argI = callMap[CI]->getArgWList().begin();

    list<ArgumentWrapper *>::iterator actual_argE;
    callMap[CI]->getArgWList().end();

    // increase formal/actual tree iterator simutaneously
    for (; formal_argI != formal_argE; ++formal_argI, ++actual_argI) {

        // intra-connection between ACTUAL/FORMAL IN/OUT trees
        for (tree<InstructionWrapper *>::iterator
                     actual_in_TI = (*actual_argI)->getTree(ACTUAL_IN_TREE).begin(),
                     actual_in_TE = (*actual_argI)->getTree(ACTUAL_IN_TREE).end(),
                     formal_in_TI = (*formal_argI)->getTree(FORMAL_IN_TREE).begin(), // TI2
                     formal_out_TI = (*formal_argI)->getTree(FORMAL_OUT_TREE).begin(), // TI3
                     actual_out_TI = (*actual_argI)->getTree(ACTUAL_OUT_TREE).begin(); // TI4
             actual_in_TI != actual_in_TE;
             ++actual_in_TI, ++formal_in_TI, ++formal_out_TI, ++actual_out_TI) {

            // connect trees: antual-in --> formal-in, formal-out --> actual-out
            PDG->addDependency(*instnodes.find(*actual_in_TI),
                               *instnodes.find(*formal_in_TI), PARAMETER);
            PDG->addDependency(*instnodes.find(*formal_out_TI),
                               *instnodes.find(*actual_out_TI), PARAMETER);

        } // end for(tree...) intra-connection between actual/formal

        // 3. ACTUAL_OUT --> LoadInsts in #Caller# function
        for (tree<InstructionWrapper *>::iterator
                     actual_out_TI = (*actual_argI)->getTree(ACTUAL_OUT_TREE).begin(),
                     actual_out_TE = (*actual_argI)->getTree(ACTUAL_OUT_TREE).end();
             actual_out_TI != actual_out_TE; ++actual_out_TI) {

            // must handle nullptr case first
            if (nullptr == (*actual_out_TI)->getFieldType()) {
                DEBUG(dbgs() << "DEBUG ACTUAL_OUT->LoadInst: actual_out_TI->getFieldType() "
                        "is empty!\n");
                break;
            }

            if (nullptr != (*actual_out_TI)->getFieldType()) {

                std::list<LoadInst *>::iterator LI =
                        funcMap[InstW->getFunction()]->getLoadInstList().begin();
                std::list<LoadInst *>::iterator LE =
                        funcMap[InstW->getFunction()]->getLoadInstList().end();

                for (; LI != LE; ++LI) {
                    if ((*actual_out_TI)->getFieldType() == (*LI)->getType()) {
                        std::set<InstructionWrapper *>::iterator nodeItB = funcInstWList[callee].begin();
                        std::set<InstructionWrapper *>::iterator nodeItE = funcInstWList[callee].begin();
                        for (; nodeItB != nodeItE; ++nodeItB) {
                            if ((*nodeItB)->getInstruction() == dyn_cast<Instruction>(*LI))
                                PDG->addDependency(*instnodes.find(*actual_out_TI),
                                                   *instnodes.find(*nodeItB), DATA_GENERAL);
                        }
                    }
                } // end for(; LI != LE; ++LI)
            }   // end if(nullptr != (*TI3)->...)
        }     // end for(tree actual_out_TI = getTree FORMAL_OUT_TREE)

    } // end for argI iteration

    return 0;
}

void pdg::ProgramDependencyGraph::linkTypeNodeWithGEPInst(std::list<ArgumentWrapper *>::iterator argI,
                                                          tree<InstructionWrapper *>::iterator formal_in_TI) {
    // get all gep instructions
    auto GEPList = (*argI)->getGEPList();
    for (auto GEPInst : GEPList) {
        // get the offset of filed in the struct
        int operand_num = GEPInst->getInstruction()->getNumOperands();
        llvm::Value *last_idx = GEPInst->getInstruction()->getOperand(operand_num - 1);
        // cast the last_idx to int type
        if (llvm::ConstantInt* constInt = dyn_cast<ConstantInt>(last_idx)) {
            // make sure type is matched
            llvm::Type *parent_type = (*formal_in_TI)->getParentType();
            if (!isa<GetElementPtrInst>(GEPInst->getInstruction()) || parent_type == nullptr) continue;
            auto GEP = dyn_cast<GetElementPtrInst>(GEPInst->getInstruction());
            llvm::Type *GEPResTy = GEP->getResultElementType();
            llvm::Type *GEPSrcTy = GEP->getSourceElementType();
            if (parent_type != nullptr) {
                DEBUG(dbgs() << "Source Type" << GEPSrcTy->getTypeID() << "\n");
                DEBUG(dbgs() << parent_type->getTypeID() << "\n\n");
                //tree<InstructionWrapper *>::iterator parentIter = tree<InstructionWrapper *>::parent(formal_in_TI);
            }

            llvm::Type *TreeNodeTy = (*formal_in_TI)->getFieldType();
            // get access field idx from GEP
            int field_idx = constInt->getSExtValue();
            // plus one. Since for struct, the 0 index is used by the parent struct type parent_type must be a pointer. Since only sub fields can have parent that is not nullptr
            if (parent_type->isPointerTy()) {
                parent_type = parent_type->getPointerElementType();
            }

            // check the src type in GEP inst is equal to parent_type (GET FROM)
            bool match = GEPSrcTy == parent_type;
            // check if the offset is equal
            int field_offset = (*formal_in_TI)->getFieldId();
            //if (field_idx+1 == relative_idx && GEPResTy == TreeNodeTy && match) {
            if (field_idx == field_offset && GEPResTy == TreeNodeTy && match) {
                PDG->addDependency(*instnodes.find(*formal_in_TI), GEPInst, STRUCT_FIELDS);
                (*formal_in_TI)->setVisited(true);
            }
        }
    }
}

void pdg::ProgramDependencyGraph::connectFunctionAndFormalTrees(llvm::Function *callee) {

    for (list<ArgumentWrapper *>::iterator
                 argI = funcMap[callee]->getArgWList().begin(),
                 argE = funcMap[callee]->getArgWList().end();
         argI != argE; ++argI) {

        InstructionWrapper *formal_inW = *(*argI)->getTree(FORMAL_IN_TREE).begin();
        InstructionWrapper *formal_outW = *(*argI)->getTree(FORMAL_OUT_TREE).begin();

        // connect Function's EntryNode with formal in/out tree roots
        PDG->addDependency(funcMap[callee]->getEntry(), *instnodes.find(formal_inW),
                           PARAMETER);
        PDG->addDependency(funcMap[callee]->getEntry(),
                           *instnodes.find(formal_outW), PARAMETER);

        // two things: (1) formal-in --> callee's Store; (2) callee's Load --> formal-out
        for (tree<InstructionWrapper *>::iterator
                     formal_in_TI = (*argI)->getTree(FORMAL_IN_TREE).begin(),
                     formal_in_TE = (*argI)->getTree(FORMAL_IN_TREE).end(),
                     formal_out_TI = (*argI)->getTree(FORMAL_OUT_TREE).begin();
             formal_in_TI != formal_in_TE; ++formal_in_TI, ++formal_out_TI) {

            // connect formal-in and formal-out nodes formal-in --> formal-out
            PDG->addDependency(*instnodes.find(*formal_in_TI),
                               *instnodes.find(*formal_out_TI), PARAMETER);

            // must handle nullptr case first
            if ((*formal_in_TI)->getFieldType() == nullptr) {
                DEBUG(dbgs() << "connectFunctionAndFormalTrees: "
                        "formal_in_TI->getFieldType() == nullptr !\n");
                break;
            }

            if ((*formal_in_TI)->getFieldType() != nullptr) {
                // link specific field with GEP if there is any
                linkTypeNodeWithGEPInst(argI, formal_in_TI);
                // connect formal-in-tree type nodes with storeinst in call_func
                if ((*argI)->getTree(FORMAL_IN_TREE).depth(formal_in_TI) == 0) {
                    for (auto userIter = (*argI)->getArg()->user_begin();
                         userIter != (*argI)->getArg()->user_end(); ++userIter) {
                        if (llvm::Instruction *tmpInst = dyn_cast<Instruction>(*userIter)) {
                            PDG->addDependency(*instnodes.find(*formal_in_TI),
                                               instMap[tmpInst], DATA_GENERAL);
                            (*formal_in_TI)->setVisited(true);
                        }
                    }
                }
            }

            // 2. Callee's LoadInsts --> FORMAL_OUT in Callee function
            // must handle nullptr case first
            if ((*formal_out_TI)->getFieldType() == nullptr) {
                DEBUG(dbgs() << "connectFunctionAndFormalTrees: LoadInst->FORMAL_OUT: "
                        "formal_out_TI->getFieldType() == nullptr!\n");
                break;
            }

            //      errs() << "DEBUG 214\n";
            if ((*formal_out_TI)->getFieldType() == nullptr) {

                std::list<LoadInst *>::iterator LI =
                        funcMap[callee]->getLoadInstList().begin();
                std::list<LoadInst *>::iterator LE =
                        funcMap[callee]->getLoadInstList().end();

                for (; LI != LE; ++LI) {
                    if ((*formal_out_TI)->getFieldType() == (*LI)->getPointerOperand()->getType()->getContainedType(0)) {
                        for (std::set<InstructionWrapper *>::iterator nodeIt =
                                instnodes.begin();
                             nodeIt != instnodes.end(); ++nodeIt) {

                            if ((*nodeIt)->getInstruction() == dyn_cast<Instruction>(*LI)) {
                                PDG->addDependency(*instnodes.find(*nodeIt),
                                                   *instnodes.find(*formal_out_TI),
                                                   DATA_GENERAL);
                            }
                        }
                    }
                } // end for(; LI != LE; ++LI)
            }   // end if(nullptr != (*formal_out_TI)->...)
        } // end for (tree formal_in_TI...)
    } // end for arg iteration...
}

bool pdg::ProgramDependencyGraph::ifFuncTypeMatch(llvm::FunctionType *funcTy, llvm::FunctionType *indirectFuncCallTy) {
    if (funcTy->getNumParams() != indirectFuncCallTy->getNumParams()) {
        return false;
    }

    if (funcTy->getReturnType() != indirectFuncCallTy->getReturnType()) {
        return false;
    }

    unsigned param_len = funcTy->getNumParams();
    for (unsigned i = 0; i < param_len; ++i) {
        if (funcTy->getParamType(i) != indirectFuncCallTy->getParamType(i)) {
            return false;
        }
    }

    return true;
}

std::vector<llvm::Function *> pdg::ProgramDependencyGraph::collectIndirectCallCandidates(FunctionType *funcType) {
    std::vector<llvm::Function *> indirectCallList;
    std::map<const llvm::Function *, FunctionWrapper *>::iterator FI = funcMap.begin();
    std::map<const llvm::Function *, FunctionWrapper *>::iterator FE = funcMap.end();
    for (; FI != FE; ++FI) {
        llvm::Function *curFunc = const_cast<llvm::Function *>((*FI).first);
        llvm::FunctionType *curFuncType = curFunc->getFunctionType();

        if (curFunc->getFunctionType() == funcType && curFunc->getName() != "main") {
            errs() << curFunc->getName() << " function pointer! \n";
        }

        if (ifFuncTypeMatch(curFuncType, funcType)) {
           indirectCallList.push_back(curFunc);
        }
    }

    return indirectCallList;
}

void pdg::ProgramDependencyGraph::connectAllPossibleFunctions( CallInst *CI, std::vector<llvm::Function *> indirect_call_candidates) {
    InstructionWrapper *CInstW = instMap[CI];
    for (llvm::Function *func : indirect_call_candidates) {
        connectCallerAndCallee(CInstW, func);
    }


//   //llvm::CallInst *callInst;
//   for (list<ArgumentWrapper *>::iterator
//            argI = funcMap[func]->getArgWList().begin(),
//            argE = funcMap[func]->getArgWList().end();
//        argI != argE; ++argI) {

//     InstructionWrapper *formal_inW = *(*argI)->getTree(FORMAL_IN_TREE).begin();
//     InstructionWrapper *formal_outW = *(*argI)->getTree(FORMAL_OUT_TREE).begin();
//     errs() << "Validating formal nodes:" << "\n";

//     if (instnodes.find(formal_inW) == instnodes.end() ||
//         instnodes.find(formal_outW) == instnodes.end()) {
//       continue;
//     }
//     // connect Function's EntryNode with formal in/out tree roots
//     PDG->addDependency(funcMap[func]->getEntry(), *instnodes.find(formal_inW),
//                        PARAMETER);
//     PDG->addDependency(funcMap[func]->getEntry(), *instnodes.find(formal_outW),
//                        PARAMETER);
//   }
}

void pdg::ProgramDependencyGraph::collectGlobalInstList() {
  // Get the Module's list of global variable and function identifiers
  DEBUG(dbgs() << "======Global List: ======\n");
  for (llvm::Module::global_iterator globalIt = module->global_begin();
       globalIt != module->global_end(); ++globalIt) {
    InstructionWrapper *globalW =
        new InstructionWrapper(nullptr, nullptr, &(*globalIt), GLOBAL_VALUE);
    instnodes.insert(globalW);
    globalList.insert(globalW);

    // find all global pointer values and insert them into a list
    if (globalW->getValue()->getType()->getContainedType(0)->isPointerTy()) {
      gp_list.push_back(globalW);
    }
  }
}

void pdg::ProgramDependencyGraph::categorizeInstInFunc(llvm::Function *func) {
  // find all Load/Store instructions for each F, insert to F's storeInstList
  // and loadInstList
  for (inst_iterator I = inst_begin(func), IE = inst_end(func); I != IE; ++I) {
    Instruction *pInstruction = dyn_cast<Instruction>(&*I);

    if (isa<StoreInst>(pInstruction)) {
      StoreInst *SI = dyn_cast<StoreInst>(pInstruction);
      funcMap[func]->getStoreInstList().push_back(SI);
      funcMap[func]->getPtrSet().insert(SI->getPointerOperand());
      if (SI->getValueOperand()->getType()->isPointerTy()) {
        funcMap[func]->getPtrSet().insert(SI->getValueOperand());
      }
    }
    if (isa<LoadInst>(pInstruction)) {
      LoadInst *LI = dyn_cast<LoadInst>(pInstruction);
      funcMap[func]->getLoadInstList().push_back(LI);
      funcMap[func]->getPtrSet().insert(LI->getPointerOperand());
    }

    if (isa<ReturnInst>(pInstruction))
      funcMap[func]->getReturnInstList().push_back(pInstruction);

    if (isa<CallInst>(pInstruction)) {
      funcMap[func]->getCallInstList().push_back( dyn_cast<CallInst>(pInstruction));
    }
  }
}

bool pdg::ProgramDependencyGraph::processingCallInst(InstructionWrapper *instW) {
  llvm::Instruction *pInstruction = instW->getInstruction();
  if (pInstruction != nullptr && instW->getType() == INST &&
      isa<CallInst>(pInstruction) && !instW->getVisited()) {
    // InstructionWrapper *CallInstW = instW;
    CallInst *CI = dyn_cast<CallInst>(pInstruction);
    Function *callee = CI->getCalledFunction();

    if (callee == nullptr) {
        // indirect function call
        // get func type for indirect call inst
        Type *t = CI->getCalledValue()->getType();
        FunctionType *funcTy = cast<FunctionType>(cast<PointerType>(t)->getElementType());
        // collect all possible function with same function signature
        std::vector<llvm::Function *> indirect_call_candidates = collectIndirectCallCandidates(funcTy);
        CallWrapper *callW = new CallWrapper(CI, indirect_call_candidates);
        callMap[CI] = callW;
        errs() << "indirect call, called Type t = " << *t << "\n";
        // build formal tree for all candidiates.
        for (llvm::Function *func :  indirect_call_candidates) {
            if (func->isDeclaration()) {
                continue;
            }

            if (func->arg_empty()) {
                continue;
            }

            if (funcMap[func]->hasTrees()) {
                continue;
            }

            buildFormalParameterTrees(func);
            drawFormalParameterTree(func, FORMAL_IN_TREE);
            drawFormalParameterTree(func, FORMAL_OUT_TREE);
            connectFunctionAndFormalTrees(func);
            funcMap[func]->setTreeFlag(true);
        }
        buildActualParameterTrees(CI);
        drawActualParameterTree(CI, ACTUAL_IN_TREE);
        drawActualParameterTree(CI, ACTUAL_OUT_TREE);
        // connect actual tree with all possible candidaites.
        connectAllPossibleFunctions(CI, indirect_call_candidates);

        DEBUG(dbgs() << "call_func = null: " << *CI << "\n");
        DEBUG(dbgs() << "indirect call, called Type t = " << *t << "\n");
        DEBUG(dbgs() << "after cast<FunctionType>, ft = " << *funcTy << "\n");
        return true;
    }

    // handle intrinsic functions
    if (callee->isIntrinsic()) {
      if (callee->getIntrinsicID() == Intrinsic::var_annotation) {
        Value *v = CI->getArgOperand(0);
        DEBUG(dbgs() << "Intrinsic var_annotation: " << *v << "\n");
        if (isa<BitCastInst>(v)) {
          Instruction *tempI = dyn_cast<Instruction>(v);
          DEBUG(dbgs() << "******** BitInst opcode: " << *tempI
                       << "BitCast \n");

          for (llvm::Use &U : tempI->operands()) {
            Value *v_for_cast = U.get();
          }
        }
      }
      return false;
    }

    // special cases done, common function
    CallWrapper *callW = new CallWrapper(CI);
    callMap[CI] = callW;

    if (!callee->isDeclaration()) {
      if (!callee->arg_empty()) {
        if (funcMap[callee]->hasTrees() != true) {
          errs() << "Buiding parameter tree" << "\n";
          buildFormalParameterTrees(callee);
          drawFormalParameterTree(callee, FORMAL_IN_TREE);
          drawFormalParameterTree(callee, FORMAL_OUT_TREE);
          connectFunctionAndFormalTrees(callee);
          funcMap[callee]->setTreeFlag(true);
        }
        buildActualParameterTrees(CI);
        drawActualParameterTree(CI, ACTUAL_IN_TREE);
        drawActualParameterTree(CI, ACTUAL_OUT_TREE);
      } // end if !callee

      if (0 == connectCallerAndCallee(instW, callee)) {
        instW->setVisited(true);
      }

      // link typenode inst with argument inst
      std::list<ArgumentWrapper *> argList = funcMap[callee]->getArgWList();
      for (ArgumentWrapper *argW : argList) {
        tree<InstructionWrapper *>::iterator TI =
            argW->getTree(FORMAL_IN_TREE).begin();
        tree<InstructionWrapper *>::iterator TE =
            argW->getTree(FORMAL_IN_TREE).end();
        for (; TI != TE; ++TI) {
          if (PDG->depends(instW, *TI)) {
            PDG->addDependency(instW, *TI, STRUCT_FIELDS);
          }
        }
      }
    }
  }
  return true;
}

bool pdg::ProgramDependencyGraph::addNodeDependencies(
    InstructionWrapper *instW1) {
  // processing Global instruction
  if (instW1->getInstruction() != nullptr) {
    if (llvm::LoadInst *LDInst =
            dyn_cast<llvm::LoadInst>(instW1->getInstruction())) {
      for (auto GlobalInstW : globalList) {
        // iterate users of the global value
        for (User *U : GlobalInstW->getValue()->users()) {
          if (Instruction *userInst = dyn_cast<Instruction>(U)) {
            InstructionWrapper *userInstW = instMap[userInst];
            PDG->addDependency(GlobalInstW, userInstW, GLOBAL_VALUE);
          }
        }
      }
    }
  }

  // processing ddg nodes
  pdg::DependencyNode<InstructionWrapper> *dataDNode =
      ddg->DDG->getNodeByData(instW1);
  pdg::DependencyNode<InstructionWrapper>::DependencyLinkList dataDList =
      dataDNode->getDependencyList();
  for (auto dependencyPair : dataDList) {
    InstructionWrapper *DNodeW2 =
        const_cast<InstructionWrapper *>(dependencyPair.first->getData());
    PDG->addDependency(instW1, DNodeW2, dependencyPair.second);
  }

  // processing cdg nodes
#if 0
    pdg::DependencyNode<InstructionWrapper> *controlDNode = cdg->CDG->getNodeByData(instW1);
    pdg::DependencyNode<InstructionWrapper>::DependencyLinkList ControlDList = controlDNode->getDependencyList();
    for (auto dependencyPair : dataDList) {
        InstructionWrapper *DNode2 = const_cast<InstructionWrapper *>(dependencyPair.first->getData());
        PDG->addDependency(instW1, DNode2, CONTROL);
    }
#endif

  // Control Dep entry
  if (instW1->getType() == ENTRY) {
    llvm::Function *parentFunc = instW1->getFunction();
    for (InstructionWrapper *instW2 : funcInstWList[parentFunc]) {
      PDG->addDependency(instW1, instW2, CONTROL);
    }
  }

  return true;
}

// void pdg::ProgramDependencyGraph::printSensitiveFunctions() {
//    std::vector<InstructionWrapper*> sensitive_nodes;
//    std::set<llvm::GlobalValue *> senGlobalSet;
//    //std::set<llvm::Instruction *> senInstSet;
//    std::set<llvm::Function *> async_funcs;
//    std::deque<const InstructionWrapper*> queue;
//    std::set<InstructionWrapper* > coloredInstSet;
//
//    auto global_annos =
//    this->module->getNamedGlobal("llvm.global.annotations"); if(global_annos){
//        auto casted_array = cast<ConstantArray>(global_annos->getOperand(0));
//        for (int i = 0; i < casted_array->getNumOperands(); i++) {
//            auto casted_struct =
//            cast<ConstantStruct>(casted_array->getOperand(i)); if (auto sen_gv
//            =
//            dyn_cast<GlobalValue>(casted_struct->getOperand(0)->getOperand(0)))
//            {
//                auto anno =
//                cast<ConstantDataArray>(cast<GlobalVariable>(casted_struct->getOperand(1)->getOperand(0))->getOperand(0))->getAsCString();
//                if (anno == "sensitive") {
//                    errs() << "sensitive global found! value = " << *sen_gv <<
//                    "\n"; senGlobalSet.insert(sen_gv);
//                }
//            }
//bool pdg::ProgramDependencyGraph::ifFuncTypeMatch(llvm::FunctionType *funcTy, llvm::FunctionType *indirectFuncCallTy) {
//     if (funcTy->getNumParams() != indirectFuncCallTy->getNumParams()) {
//         return false;
//     }

//     if (funcTy->getReturnType() != indirectFuncCallTy->getReturnType()) {
//         return false;
//     }

//     unsigned param_len = funcTy->getNumParams();
//     for (unsigned i = 0; i < param_len; ++i) {
//         if (funcTy->getParamType(i) != indirectFuncCallTy->getParamType(i)) {
//             return false;
//         }
//     }

//     return true;
// }
//            if (auto fn =
//            dyn_cast<Function>(casted_struct->getOperand(0)->getOperand(0))) {
//                auto anno =
//                cast<ConstantDataArray>(cast<GlobalVariable>(casted_struct->getOperand(1)->getOperand(0))->getOperand(0))->getAsCString();
//                if (anno == "sensitive") {
//                    async_funcs.insert(fn);
//                    errs() << "async_funcs ++ sensitive " << fn->getName() <<
//                    "\n";
//                }
//            }
//        }
//    }
//
//    for(std::set<InstructionWrapper*>::iterator nodeIt = instnodes.begin();
//    nodeIt != instnodes.end(); ++nodeIt){
//        InstructionWrapper *InstW = *nodeIt;
//        llvm::Instruction *pInstruction = InstW->getInstruction();
//
//        for(int i = 0; i < sensitive_values.size(); i++){
//            if(sensitive_values[i] == pInstruction){
//                errs() << "sensitive_values " << i << " == "<< *pInstruction
//                << "\n"; sensitive_nodes.push_back(InstW);
//            }
//        }
//
//        if(InstW->getType() == GLOBAL_VALUE){
//            GlobalValue *gv = dyn_cast<GlobalValue>(InstW->getValue());
//            if(senGlobalSet.end() != senGlobalSet.find(gv)){
//                errs() << "sensitive_global: " << *gv <<"\n";
//                sensitive_nodes.push_back(InstW);
//            }
//        }
//    }
//
//    for(int i = 0; i < sensitive_nodes.size(); i++){
//        queue.push_back(sensitive_nodes[i]);
//    }
//
//    errs() << "queue.size = " << queue.size() << "\n";
//
//    while(!queue.empty()){
//        InstructionWrapper *InstW =
//        const_cast<InstructionWrapper*>(queue.front()); if(InstW->getType() ==
//        INST){
//            funcMap[InstW->getFunction()]->setVisited(true);
//        }
//        queue.pop_front();
//
//        if(InstW->getValue() != nullptr) {
//            coloredInstSet.insert(InstW);
//        }
//
//        DependencyNode<InstructionWrapper>* DNode = PDG->getNodeByData(InstW);
//        for(int i = 0; i < DNode->getDependencyList().size(); i++){
//            if(nullptr != DNode->getDependencyList()[i].first->getData()){
//                InstructionWrapper *adjacent_InstW =
//                *instnodes.find(const_cast<InstructionWrapper*>(DNode->getDependencyList()[i].first->getData()));
//                if(true != adjacent_InstW->getFlag()){
//                    queue.push_back(DNode->getDependencyList()[i].first->getData());
//                    adjacent_InstW->setFlag(true); //label the adjacent node
//                    visited
//                }
//            }
//        }
//    }
//
//    errs() << "\n\n++++++++++The FUNCTION List is as follows:++++++++++\n\n";
//
//    unsigned int funcs_count = 0;
//    unsigned int sen_funcs_count = 0;
//    unsigned int ins_funcs_count = 0;
//
//    std::set<FunctionWrapper*> sen_FuncSet;
//    std::set<FunctionWrapper*> ins_FuncSet;
//
//    std::map<const llvm::Function *, FunctionWrapper *>::iterator FI =
//            funcMap.begin();
//    std::map<const llvm::Function *, FunctionWrapper *>::iterator FE =
//            funcMap.end();
//    for (; FI != FE; ++FI) {
//        if (!(*FI).first->isDeclaration()) {
//            funcs_count++;
////            if ((*FI).second->hasFuncOrFilePtr()) {
////                errs() << (*FI).first->getName() << " hasFuncOrFilePtr()\n";
////            }
//
//            if ((*FI).second->isVisited()) {
//                errs() << (*FI).first->getName() << " is
//                colored(sensitive)\n";
//
//                Function* func = (*FI).second->getFunction();
//                errs() << "func name = " << func->getName() << "\n";
//
//                sen_FuncSet.insert((*FI).second);
//            } else {
//                errs() << (*FI).first->getName() << " is uncolored\n";
//                ins_FuncSet.insert((*FI).second);
//            }
//        }
//    }
//}

bool pdg::ProgramDependencyGraph::runOnModule(Module &M) {
  std::set<std::string> funcList = {"___might_sleep",
                                    "__alloc_percpu",
                                    "__rtnl_link_register",
                                    "__rtnl_link_unregister",
                                    "_cond_resched",
                                    "alloc_netdev_mqs",
                                    "consume_skb",
                                    "dummy_change_carrier",
                                    "dummy_cleanup_module",
                                    "dummy_dev_init",
                                    "dummy_dev_uninit",
                                    "dummy_get_drvinfo",
                                    "dummy_get_stats64",
                                    "dummy_init_module",
                                    "dummy_init_one",
                                    "dummy_setup",
                                    "dummy_validate",
                                    "dummy_xmit",
                                    "eth_hw_addr_random",
                                    "eth_mac_addr",
                                    "eth_random_addr",
                                    "eth_validate_addr",
                                    "ether_setup",
                                    "free_netdev",
                                    "free_percpu",
                                    "get_random_bytes",
                                    "is_multicast_ether_addr",
                                    "is_valid_ether_addr",
                                    "is_zero_ether_addr",
                                    "netif_carrier_off",
                                    "netif_carrier_on",
                                    "nla_data",
                                    "nla_len",
                                    "register_netdevice",
                                    "rtnl_link_unregister",
                                    "rtnl_lock",
                                    "rtnl_unlock",
                                    "set_multicast_list",
                                    "u64_stats_fetch_begin_irq",
                                    "u64_stats_fetch_retry_irq",
                                    "u64_stats_update_begin",
                                    "u64_stats_update_end"};

  DEBUG(dbgs() << "ProgramDependencyGraph::runOnModule" << '\n');
  module = &M;
  constructFuncMap(M);
  collectGlobalInstList();
  auto t1 = std::chrono::high_resolution_clock::now();
  int funcs = 0;
  // process a module function by function
  for (Module::iterator FF = M.begin(), E = M.end(); FF != E; ++FF) {
    DEBUG(dbgs() << "Module Size: " << M.size() << "\n");
    Function *F = dyn_cast<Function>(FF);
    // if (funcList.find(F->getName()) == funcList.end()) {
    //     continue;
    // }

    if (F->isDeclaration()) {
      DEBUG(dbgs() << (*F).getName() << " is defined outside!"
                   << "\n");
      continue;
    }

    funcs++; // label this author-defined function

    DEBUG(dbgs() << "PDG " << 1.0 * funcs / M.getFunctionList().size() * 100
                 << "% completed\n");

    categorizeInstInFunc(F);

    cdg = &getAnalysis<ControlDependencyGraph>(*F);
    ddg = &getAnalysis<DataDependencyGraph>(*F);

    // process call nodes, one call node will only be touched
    // once(!InstW->getAccess)
    for (std::set<InstructionWrapper *>::iterator nodeIt =
             funcInstWList[F].begin();
         nodeIt != funcInstWList[F].end(); ++nodeIt) {
      InstructionWrapper *instW1 = *nodeIt;

      // processing CallInst
      if (!processingCallInst(instW1)) {
        continue;
      }

      if (!addNodeDependencies(instW1)) {
        continue;
      }
    } // end the iteration for finding CallInst
  }   // end for(Module...

  auto t2 = std::chrono::high_resolution_clock::now();
  auto int_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1);

  errs() << "Adding dependency time: " << int_ms.count() << "\n";

  DEBUG(dbgs() << "\n\n PDG construction completed! ^_^\n\n");
  DEBUG(dbgs() << "funcs = " << funcs << "\n");
  DEBUG(dbgs() << "+++++++++++++++++++++++++++++++++++++++++++++\n");

  // printSensitiveFunctions();
  printArgUseInfo(M, funcList);
  cleanupGlobalVars();
  return false;
}

const StructLayout *
pdg::ProgramDependencyGraph::getStructLayout(llvm::Module &M,
                                             InstructionWrapper *curTyNode) {
  DataLayout DL = M.getDataLayout();
  llvm::Type *curNodeTy = curTyNode->getFieldType();

  PointerType *pt = dyn_cast<PointerType>(curNodeTy);
  if (curNodeTy->isPointerTy()) {
    if (!pt->getElementType()->isStructTy()) {
      return nullptr;
    }
  }

  StructType *st = nullptr;

  if (curNodeTy->isPointerTy()) {
    st = dyn_cast<StructType>(pt->getElementType());
  } else {
    st = dyn_cast<StructType>(curNodeTy);
  }

  const StructLayout *StL = DL.getStructLayout(st);
  return StL;
}

std::map<std::string, bool> pdg::ProgramDependencyGraph::getArgUseInfoInFunc(llvm::Module &M, llvm::Function *start_func) {
    std::queue<llvm::Function *> func_queue;
    func_queue.push(start_func);
    
    while (!func_queue.empty()) {
        llvm::Function *cur_func = func_queue.front();        
        func_queue.pop();

        // TODO: getArgUseInfoMap lacking
        std::map<std::string, bool> new_arg_use_info_map = getArgUseInfoMap(M, cur_func);

        // TODO: implementation of global arg use info lacking
        std::map<std::string, bool> old_arg_use_info_map = global_arg_use_info_map[cur_func];

        // TODO: implementation of compare_map_equality
        if (new_arg_use_info_map == old_arg_use_info_map) {
            continue;
        }         

        // get all dependent functions. If arg use for this function change, also
        // update all arg use info for all dependent functions
        // TODO: getDependentFuncList lacking
        std::set<llvm::Function *> dependent_func_list = getDependentFuncList(cur_func);
        for (llvm::Function * func : dependent_func_list) {
            func_queue.push(func);
            update_arg_use_info(func, new_arg_use_info_map);
        }  
    }

    return global_arg_use_info_map[start_func];
}

std::map<std::string, bool> pdg::ProgramDependencyGraph::getArgUseInfoMap(llvm::Module &M, llvm::Function *func) {
  std::map<std::string, bool> arg_use_info_map;

  typedef std::map<unsigned, std::pair<std::string, DIType *>> offsetNames;
  // get function to arg name map
  std::map<Function *, offsetNames> funcArgOffsetNames = getAnalysis<DSAGenerator>().getFuncArgOffsetNames();

  offsetNames funcOffsetNames = funcArgOffsetNames[func];
  std::list<ArgumentWrapper *> arg_list = funcMap[func]->getArgWList();

  for (auto argW : arg_list) {
    auto treeIter = argW->getTree(FORMAL_IN_TREE).begin();
    int prev_offset = 0;
    int accumulate_offset = 0;
    int curDepth = 0;

    // iterate through parameter tree.
    // summarize field name information with used information
    for (; treeIter != argW->getTree(FORMAL_IN_TREE).end(); ++treeIter) {
      int tmpDepth = argW->getTree(FORMAL_IN_TREE).depth(treeIter);
      // make sure we are fetching the code at the same level in the
      // parameter tree
      if (tmpDepth != curDepth) {
        // updating the depth to the new level
        curDepth = tmpDepth;
        prev_offset += accumulate_offset;
        accumulate_offset = 0;
      }

      InstructionWrapper *curTyNode = *treeIter;
      llvm::Type *parentType = curTyNode->getParentType();

      if (parentType != nullptr) {
        DEBUG(dbgs() << "Parent Type: "
                     << curTyNode->getParentType()->getTypeID() << "\n");
      } else {
        DEBUG(dbgs() << "This is the root type node. "
                     << "\n");
        errs() << "** Root type node **"
               << "\n";
        errs() << "Visited status: " << curTyNode->getVisited() << "\n";
        continue;
      }

      auto parentIter = tree<InstructionWrapper *>::parent(treeIter);
      if (parentType->isPointerTy()) {
        const StructLayout *stLayout = getStructLayout(M, *parentIter);
        if (stLayout == nullptr) {
          errs() << "Struct Layout is nullptr. Continue"
                 << "\n";
          continue;
        }

        // get current field offset
        int offset = stLayout->getElementOffset(curTyNode->getFieldId());

        // get field name and field type node visit information
        std::string field_name = funcOffsetNames[prev_offset + offset].first;
        bool isVisited = curTyNode->getVisited();

        arg_use_info_map[field_name] = isVisited;
        
        // check 
        if (tmpDepth == curDepth) {
          accumulate_offset = offset;
        }
      }
      errs() << "..................................................\n";
    }
  }
}

std::set<llvm::Function *> pdg::ProgramDependencyGraph::getDependentFuncList(llvm::Function *start_func) {
    std::set<llvm::Function *> dependen_func_list;

    std::queue<llvm::Function *> func_queue;
    func_queue.push(start_func);

    // CallGraph &call_graph = getAnalysis<CallGraphWrapperPass>().getCallGraph();
    // while (!func_queue.empty()) {
    //     llvm::Function *cur_func = func_queue.front();
    //     func_queue.pop();

        
    // }
}


void pdg::ProgramDependencyGraph::printArgUseInfo(
    llvm::Module &M, std::set<std::string> funcNameList) {
  typedef std::map<unsigned, std::pair<std::string, DIType *>> offsetNames;
  std::map<Function *, offsetNames> funcArgOffsetNames =
      getAnalysis<DSAGenerator>().getFuncArgOffsetNames();

  for (llvm::Function &func : M) {
    if (func.isDeclaration()) {
      continue;
    }
    // if (funcNameList.find(func.getName()) == funcNameList.end()) {
    //     continue;
    // }
    offsetNames funcOffsetNames = funcArgOffsetNames[&func];
    errs() << "==================== DashBoard ===================\n";
    errs() << "Arg use info for F: [ " << func.getName() << " ]\n";
    auto arg_list = funcMap[&func]->getArgWList();
    // get offset from each argument and accumulate the offsets to match DSA
    // generate result.
    for (auto argW : arg_list) {
      errs() << "..................................................\n";
      errs() << "\nFor arg [ " << argW->getArg()->getArgNo() << " ] \n";
      std::string printinfo = "[ " + func.getName().str() + " ] ";
      auto treeIter = argW->getTree(FORMAL_IN_TREE).begin();
      int prev_offset = 0;
      int accumulate_offset = 0;
      int curDepth = 0;
      for (; treeIter != argW->getTree(FORMAL_IN_TREE).end(); ++treeIter) {
        int tmpDepth = argW->getTree(FORMAL_IN_TREE).depth(treeIter);
        // make sure we are fetching the code at the same level in the
        // parameter tree
        if (tmpDepth != curDepth) {
          // updating the depth to the new level
          curDepth = tmpDepth;
          prev_offset += accumulate_offset;
          accumulate_offset = 0;
        }

        InstructionWrapper *curTyNode = *treeIter;
        llvm::Type *parentType = curTyNode->getParentType();

        if (parentType != nullptr) {
          DEBUG(dbgs() << "Parent Type: "
                       << curTyNode->getParentType()->getTypeID() << "\n");
        } else {
          DEBUG(dbgs() << "This is the root type node. "
                       << "\n");
          errs() << "** Root type node **"
                 << "\n";
          errs() << "Visited status: " << curTyNode->getVisited() << "\n";
          continue;
        }

        auto parentIter = tree<InstructionWrapper *>::parent(treeIter);
        if (parentType->isPointerTy()) {
          const StructLayout *stLayout = getStructLayout(M, *parentIter);
          if (stLayout == nullptr) {
            errs() << "Struct Layout is nullptr. Continue"
                   << "\n";
            continue;
          }
          int offset = stLayout->getElementOffset(curTyNode->getFieldId());
          errs() << printinfo
                 << "sub field: " << funcOffsetNames[prev_offset + offset].first
                 << "\n";
          errs() << "Visit status: " << curTyNode->getVisited() << "\n";
          if (tmpDepth == curDepth) {
            accumulate_offset = offset;
          }
        }
        errs() << "..................................................\n";
        // errs() << "Node Type: " << curTyNode->getFieldType()->getTypeID() <<
        // "\n";
      }
      errs() << "==================================================\n\n";
    }
  }
}

void pdg::ProgramDependencyGraph::getAnalysisUsage(AnalysisUsage &AU) const {
  AU.addRequired<ControlDependencyGraph>();
  AU.addRequired<DataDependencyGraph>();
  AU.addRequired<DSAGenerator>();
  AU.addRequired<llvm::CallGraphWrapperPass>();
  AU.setPreservesAll();
}

void pdg::ProgramDependencyGraph::print(llvm::raw_ostream &OS,
                                        const llvm::Module *) const {
  PDG->print(OS, (getPassName().data()));
}

static RegisterPass<pdg::ProgramDependencyGraph>
    PDG("pdg", "Program Dependency Graph Construction", false, true);
