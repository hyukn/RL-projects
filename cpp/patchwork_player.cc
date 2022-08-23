#include "patchwork_player.h"

TimeLine::TimeLine() {
    spp.clear();
    brp.clear();

    int spp_r = 0;
    int brp_r = 0;
    for (int i = 0; i <= TIMELINE_END; ++i) {
        if (brp_r < BUTTON_REWARD_POS.size() and i > BUTTON_REWARD_POS[brp_r]) {
            brp_r++;
        }
        if (spp_r < SQUARE_PATCH_POS.size() and i > SQUARE_PATCH_POS[spp_r]) {
            spp_r++;
        }

        spp.push_back(spp_r);
        brp.push_back(brp_r);
    }
}

const TimeLine PatchworkPlayer::tl = TimeLine();
const PatchLib PatchworkPlayer::pl = PatchLib();