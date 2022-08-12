#include "patchwork_state.h"
#include "agent.h"


namespace agent_random {
    class Agent : public AgentBase {
    public:
        action_t get_action(const PatchworkState& ps) {
            auto a = ps.legal_actions();
            if (! a.size() == 0) {
                auto choice = rand() % a.size();
                return a[choice];
            }
            else {
                throw "No legal actions.";
            }
        }
    };
}