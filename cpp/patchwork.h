#pragma once

#include "patchwork_state.h"
#include "agent.h"

class Patchwork {
public:
    PatchworkState ps;

    map<int, PatchworkPlayer> players;

    void init() {
        ps = PatchworkState();
    }

    void play(AgentBase& agent0, AgentBase& agent1);
    // bool sewable(PatchworkPlayer& pp);
};