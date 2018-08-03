#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/InstIterator.h"

#include "llvm/Analysis/CDG/ProgramDependencies.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Debug.h"

#include "llvm/IR/ValueSymbolTable.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/DebugInfo.h"
#include "llvm/IR/DebugInfoMetadata.h"

#include "llvm/ADT/Statistic.h"

#include <vector>
#include <set>
#include <map>
#include <string.h>

namespace {
    using namespace llvm;

    class DSAGenerator : public ModulePass {
    public:
        static char ID;
        std::map<std::string, std::vector<std::string>> struct_field_name_map;
        typedef std::map<unsigned, std::pair<std::string, DIType *>> offsetNames;
        std::string moduleName = "[dsa-gen]";
        DSAGenerator() : ModulePass(ID) {}

        // basically, this function is used to get the "real" structure type from
        // the pointer type. Such as struct type ..
        DIType* getLowestDINode(DIType *Ty) {
            if (Ty->getTag() == dwarf::DW_TAG_pointer_type ||
                Ty->getTag() == dwarf::DW_TAG_member ||
                Ty->getTag() == dwarf::DW_TAG_typedef) {
                DIType *baseTy = dyn_cast<DIDerivedType>(Ty)->getBaseType().resolve();
                if (!baseTy) {
                    errs() << "Type : NULL - Nothing more to do\n";
                    return NULL;
                }

                //Skip all the DINodes with DW_TAG_typedef tag
                while ((baseTy->getTag() == dwarf::DW_TAG_typedef ||
                        baseTy->getTag() == dwarf::DW_TAG_const_type ||
                        baseTy->getTag() == dwarf::DW_TAG_pointer_type)) {

                    if (DITypeRef temp = dyn_cast<DIDerivedType>(baseTy)->getBaseType())
                        baseTy = temp.resolve();
                    else
                        break;
                }
                return baseTy;
            }
            return Ty;
        }

        void getAllNames(DIType *Ty, offsetNames &of, unsigned prev_off, std::string baseName, std::string indent, StringRef argName, std::string &structName) {
            std::string printinfo = moduleName + "[getAllNames]: ";
            DIType *baseTy = getLowestDINode(Ty);
            if (!baseTy)
                return;
            // If that pointer is a struct
            if (baseTy->getTag() == dwarf::DW_TAG_structure_type) {
                //*file << "projection <struct " << arg->getType()->getStructName().str() << "" ;
                structName = baseTy->getName().str();
                DICompositeType *compType = dyn_cast<DICompositeType>(baseTy);
                // Go thro struct elements and print them all
                for (DINode *Op : compType->getElements()) {
                    DIDerivedType *der = dyn_cast<DIDerivedType>(Op);
                    unsigned offset = der->getOffsetInBits() >> 3;
                    std::string new_name(baseName);
                    if (new_name != "") new_name.append(".");
                    new_name.append(der->getName().str());
                    errs()<< printinfo <<"type information:  "<<der->getBaseType().resolve()->getTag()<<"\n";
                    errs()<< printinfo << "Updating [of] on line 192 with following pair:\n";
                    errs()<< printinfo << "first item [new_name] "<<new_name<<"\n";
                    of[offset + prev_off] = std::pair<std::string, DIType *>(
                            new_name, der->getBaseType().resolve());
                    /// XXX: crude assumption that we want to peek only into those members
                    /// whose sizes are greater than 8 bytes
                    if (((der->getSizeInBits() >> 3) > 1)
                        && der->getBaseType().resolve()->getTag()) {
                        std::string tempStructName("");
                        errs()<< printinfo <<"RECURSIVELY CALL getAllNames on 200\n";
                        getAllNames(dyn_cast<DIType>(der), of, prev_off + offset,
                                    new_name, indent, argName, tempStructName);
                    }
                    //errs() << "--------------- " << der->getName().str() << "\n";
                }
            } else {
                structName = "";
            }
        }

        offsetNames getArgFieldNames(Function *F, unsigned argNumber, StringRef argName, std::string& structName) {
            std::string printinfo = moduleName + "[getArgFieldNames]: ";
            offsetNames offNames;
            //didn't find any such case
            if (argNumber > F->arg_size()) {
                errs() << printinfo << "### WARN : requested data for non-existent element\n";
                return offNames;
            }

            SmallVector<std::pair<unsigned, MDNode *>, 4> MDs;
            //std::vector<MDNode *> MDs = getParameterNodeInFunction(F);
            F->getAllMetadata(MDs);
            //errs() << "MDNODE vector size: " << MDs.size() << "\n";
            for (auto &MD : MDs) {
                if (MDNode *N = MD.second) {
                    //if (DISubroutineType * subRoutine = dyn_cast<DISubroutineType>(MD)) {
                    if (DISubprogram *subprogram = dyn_cast<DISubprogram>(N)) {
                        auto *subRoutine = subprogram->getType();
                        //XXX:if a function takes in no arguments, how can we assume it is of type void?
                        if (!subRoutine->getTypeArray()[0]) {
                            errs() << printinfo << "return type \"void\" for Function : " << F->getName().str() << "\n";
                        }

                        const auto &TypeRef = subRoutine->getTypeArray();

                        /// XXX: When function arguments are coerced in IR, the corresponding
                        /// debugInfo extracted for that function from the source code will
                        /// not have the same number of arguments. Check the indexes to
                        /// prevent array out of bounds exception (segfault)

                        //did not encounter this case with dummy.c
                        if (argNumber >= TypeRef.size()) {
                            errs() << printinfo << "TypeArray request out of bounds. Are parameters coerced??\n";
                            goto done;
                        }

                        if (const auto &ArgTypeRef = TypeRef[argNumber]) {
                            // Resolve the type
                            DIType *Ty = ArgTypeRef.resolve();
                            // Handle Pointer type
                            if (F->getName() == "passF")errs() << "BEGIN WATCH\n";
                            errs() << printinfo << "CALL getAllNames on line 266 with these params:\n";
                            errs() << printinfo << "argName = " << argName << "\n";
                            getAllNames(Ty, offNames, 0, "", "  ", argName, structName);
                            errs() << printinfo << "structName = " << structName << "\n";
                        }
                    }
                }
            }

            done:
            return offNames;
        }

        std::vector<DbgDeclareInst *> getDbgDeclareInstInFunction(Function *F) {
            std::vector<DbgDeclareInst *> dbg_decl_inst;
            for (inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {
                if (!isa<CallInst>(*I)) {
                    continue;
                }
                Instruction *pInstruction = dyn_cast<Instruction>(&*I);
                if (DbgDeclareInst *ddi = dyn_cast<DbgDeclareInst>(pInstruction)) {
                    dbg_decl_inst.push_back(&*ddi);
                }
            }
            return dbg_decl_inst;
        }

        std::vector<MDNode*> getParameterNodeInFunction(Function *F) {
            std::vector<MDNode*> mdnodes;
            std::vector<DbgDeclareInst *> dbg_decl_insts = getDbgDeclareInstInFunction(F);
            for (DbgDeclareInst *ddi : dbg_decl_insts) {
                //DILocalVariable *div = ddi->getVariable();
                if (MDNode *MD = dyn_cast<MDNode>(ddi->getRawExpression())) {
                //if (MDNode *MD = dyn_cast<MDNode>(ddi->getRawVariable())) {
                    mdnodes.push_back(MD);
                }
            }

            return mdnodes;
        }

        /// Prints it in console. Solely for debugging purposes
        void dumpOffsetNames(offsetNames &of) {
            std::string printinfo = moduleName + "[dumpOffsetNames]: ";
            // errs() << printinfo<<"Entered function\n";
            for (auto off : of) {
                errs() << printinfo<< "offset : " << off.first << "\n";
                errs() << printinfo<< "name : " << std::get<0>(off.second) << "\n";
                if (std::get<0>(off.second)=="Block")errs()<<"END WATCH\n";
            }
        }

        bool runOnModule(Module &M) {
            for (Module::iterator FF = M.begin(), E = M.end(); FF != E; ++FF) {
                std::string printinfo = moduleName + "[runOnModule]: ";
                // collect dgb inst for
                Function *F = dyn_cast<Function>(FF);
                if (F->isDeclaration()) {
                    continue;
                }
                // iterate dgb and
#if 0
                for (DbgDeclareInst *ddi : dbg_declare_call_inst_set) {
                    DILocalVariable *div = ddi->getVariable();

                    if (div->isParameter() == 0) {
                        continue;
                    }

                    //DIType *diType = div->getType().resolve();
                    DIType *baseType = getLowestDINode(div->getType().resolve());
                    if (diType->getTag() == dwarf::DW_TAG_structure_type) {
                        DICompositeType *dct = dyn_cast<DICompositeType>(baseType);
                        std::string struct_name = dct->getName().str();
                        errs() << "[Parent Struct Name]" << struct_name << "\n";
                        for (DINode *node : dct->getElements()) {
                            // retrive the name in the struct
                            DIDerivedType *didt = dyn_cast<DIDerivedType>(node);
                            std::string var_name = didt->getName().str();
                            if (struct_field_name_map.find(struct_name) == struct_field_name_map.end()) {
                                struct_field_name_map[struct_name] = std::vector<std::string>();
                                struct_field_name_map[struct_name].push_back(var_name);
                            } else {
                                struct_field_name_map[struct_name].push_back(var_name);
                            }
                            errs() << "Variable name: " << var_name << "\n";
                        }
                    }
                }
#endif
                for (Argument &arg : F->args()) {
                    if (arg.getType()->isPointerTy()) {
                        std::string structName;
                        errs()<<printinfo << "arg.getArgNo(){"<<arg.getArgNo()<<"}\n";
                        errs() << printinfo<<"CALL getArgFieldNames on line 521 with these parameters:\n";
                        errs() << printinfo<<"F, s.t F.getName() = "<<F->getName()<<"\n";
                        errs() << printinfo<<"arg.getArgNo() + 1 = "<<arg.getArgNo() + 1<<"\n";
                        errs() << printinfo<<"arg.getName() = "<<arg.getName()<<"\n";
                        offsetNames of = getArgFieldNames(F, arg.getArgNo()+1, arg.getName(), structName);
                        errs() << printinfo<< "structName = " << structName <<"\n";
                        errs() << printinfo<<"CALL dumpOffsetNames on line 523\n";
                        dumpOffsetNames(of);
                    }
                }
            }
        }

    };

    char DSAGenerator::ID = 0;
    static RegisterPass<DSAGenerator> DSAGenerator("dsa-gen", "DSA struct field name generation for kernel", false, true);
}
