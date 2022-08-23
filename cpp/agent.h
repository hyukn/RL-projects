#pragma once

#include "patchwork_state.h"

class AgentBase {
public:
    virtual action_t get_action(const PatchworkState& ps) {
        auto a = ps.legal_actions();
        return a[0];
    }
};
