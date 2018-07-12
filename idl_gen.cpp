#include "ProgramDependencies.h"

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
            this->imported_functions = {
                    "___might_sleep",
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
                    "llvm.dbg.declare",
                    "netif_carrier_off",
                    "netif_carrier_on",
                    "nla_data",
                    "nla_len",
                    "register_netdevice",
                    "rtnl_link_unregister",
                    "rtnl_lock",
                    "rtnl_unlock",
                    "set_multicast_list",
                    "strlcpy",
                    "u64_stats_fetch_begin_irq",
                    "u64_stats_fetch_retry_irq",
                    "u64_stats_update_begin",
                    "u64_stats_update_end"
            };
        }

        // do main work here
        bool runOnModule(Module &M) {
            pdg::ProgramDependencyGraph &pdggraph = getAnalysis<pdg::ProgramDependencyGraph>();
        }

        void getArgDependencies(Function &F, pdg::ProgramDependencyGraph &pdggraph) {
//            std::map<const Function*, pdg::FunctionWrapper *> func_map = pdggraph.getFuncMap();
//            std::map<const Instruction*, pdg::InstructionWrapper*> instMap = pdggraph.getInstMap();
//
//            for (std::map<const Function*, pdg::FunctionWrapper *>::iterator it = func_map.begin();
//                 it != func_map.end(); ++it) {
//                errs() << "Function Name: " << it->first->getName() << "\n";
//            }
        }

        void recursiveQuery(pdg::InstructionWrapper *InstW, pdg::ProgramDependencyGraph &pdggraph) {
            pdggraph.PDG->printDependencies(InstW);
        }

        // Analysis Usage, specify PDG at this time
        void getAnalysisUsage(AnalysisUsage &AU) const {
            AU.addRequired<pdg::ProgramDependencyGraph>();
            AU.setPreservesAll();
        }

    private:
        std::set<std::string> imported_functions;
    };

    char IdlGenerator::ID = 0;
    static RegisterPass<IdlGenerator> IdlGenerator("idl-gen", "IDL generation for kernel", false, true);
}
