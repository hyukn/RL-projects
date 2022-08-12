#pragma once

#include <vector>

#include "patch.h"
#include "patchwork_player.h"
#include "patchwork_state.h"
#include "agent.h"
#include "evaluate.h"

using solve_t = std::pair<double, action_t>;

namespace agent_pvs{
    class Solver {
    public:

        PWPlacer placer;
        PWChooser chooser;

        solve_t solve(
            const PatchworkState& ps, int depth, double alpha, double beta, int player_id
        );
    };

    class Agent: public AgentBase {
    public:
        Solver solver;
        
        int depth = 1;
        int player_id = 0;
        Agent(int depth_, int player_id_) : 
            depth(depth_), player_id(player_id_) {}
        
        action_t get_action(const PatchworkState& ps);
    };
}
