
#include "open_spiel/games/patchwork/patchwork_state.h"

using namespace std;

const PatchLib& PatchworkState::pl = PatchworkPlayer::pl;
const place_t PatchworkState::full_board = place_t("2417851639229258349412351");

const vector<int> get_post_node_init() {
    auto post_node = vector<int>(PATCH_NUM, 0);
    iota(post_node.begin(), post_node.end(), 1);
    post_node.back() = 0;
    return post_node;
}

const vector<int> get_pre_node_init() {
    auto pre_node = vector<int>(PATCH_NUM, 0);
    iota(pre_node.begin()+1, pre_node.end(), 0);
    pre_node.front() = PATCH_NUM - 1;
    return pre_node;
}


PatchworkState::PatchworkState() {
    cur_player = PatchworkPlayer(0);
    opponent = PatchworkPlayer(1);
    
    post_node = vector<int>(PATCH_NUM, 0);
    iota(post_node.begin(), post_node.end(), 1);
    post_node.back() = 0;

    pre_node = vector<int>(PATCH_NUM, 0);
    iota(pre_node.begin()+1, pre_node.end(), 0);
    pre_node.front() = PATCH_NUM - 1;

    patch_remain = vector<bool>(PATCH_NUM, true);

    // initial patch_queue
    patch_queue = vector<int>(PATCH_NUM, 0);
    iota(patch_queue.begin(), patch_queue.end(), 1);
    random_shuffle(patch_queue.begin(), patch_queue.end());
}

PatchworkState::PatchworkState(const PatchworkState& other) {
    patch_queue.reserve(PATCH_NUM);
    post_node.reserve(PATCH_NUM);
    pre_node.reserve(PATCH_NUM);
    patch_remain.reserve(PATCH_NUM);

    copy(other.patch_queue.begin(), other.patch_queue.end(), back_inserter(patch_queue));
    cur_player = PatchworkPlayer(other.cur_player);
    opponent = PatchworkPlayer(other.opponent);

    phase = other.phase;
    square7 = other.square7;
    copy(other.post_node.begin(), other.post_node.end(), back_inserter(post_node));
    copy(other.pre_node.begin(), other.pre_node.end(), back_inserter(pre_node));
    copy(other.patch_remain.begin(), other.patch_remain.end(), back_inserter(patch_remain));
    // for_each(other.patch_remain.begin(), other.patch_remain.end(), 
        // [&](const auto& p) { patch_remain[p.first] = p.second;});
    cur_node = other.cur_node;
    next_sew = other.next_sew;
}

vector<action_t> PatchworkState::legal_actions() const {
    switch (phase) {
        case Phase::choose: {
            vector<action_t> choose;
            choose.reserve(4);
            int node = cur_node;
            for (int i = 0; i < 3; ++i) {
                int patch_id = patch_queue[node];
                const Patch& p = pl.get_patch(patch_id);
                if ((p.bc <= cur_player.buttons) && cur_player.is_sewable(patch_id)) {
                    choose.push_back(node);
                }
                node = post_node[node];
            }
            choose.push_back(-1);
            return choose;
            break;
        }
        case Phase::sew: {
            // cout << "sew " << next_sew << endl;
            return cur_player.all_sewable_ways(next_sew);
            break;
        }
        default: {
            return vector<action_t>();
            break;
        }
    }
}

void PatchworkState::do_apply_action(action_t action) {
    switch (phase) {
        case Phase::choose: {
            apply_choose_action(action);
            break;
        }
        case Phase::sew: {
            apply_sew_action(action);
            break;
        }
        default: {
            std::cout << "[ERROR]: Undefined action occurs in PatchworkState." << std::endl;
            break;
        }
    }
}

void PatchworkState::apply_choose_action(action_t action) {
    if (action == -1) {
        int move = opponent.cur_pos - cur_player.cur_pos + 1;
        cur_player.move_reward(move);
        player_move(move);
    }
    else {
        int node = action;
        int patch_id = patch_queue[node];
        const Patch& patch = PatchworkPlayer::pl.get_patch(patch_id);
        
        cur_player.choose(patch);

        int post = post_node[node];
        int pre = pre_node[node];
        
        post_node[pre] = post;
        pre_node[post] = pre;

        cur_node = post;
        next_sew = patch_id;
        
        // remove patches
        patch_remain[patch_id] = false;
        phase = Phase::sew;
    }
}

void PatchworkState::apply_sew_action(action_t action) {
    int patch_id = pl.pl_id_2_pa_id(action);
    cur_player.sew(patch_id, action);
    if ((!square7) && cur_player.area > 49) {
        for (auto& p: pl.square7) {
            if (p ^ cur_player.board == p) {
                cur_player.square7 = 7;
                square7 = true;
                break;
            }
        }
    }

    player_move(pl.get_patch(patch_id).tc);
}

void PatchworkState::player_move(int move) {
    cur_player.move(move);
    if (cur_player.cross_spp(opponent)) {
        if (cur_player.board == full_board) {
            if (cur_player.cur_pos > opponent.cur_pos) {
                // swap two player;
                swap(cur_player, opponent);
            }
            phase = Phase::choose;
        }
        else {
            phase = Phase::sew;
            next_sew = pl.get_patch(PatchLib::ONExONE_ID).id;
        }
    }
    else {
        if (cur_player.cur_pos > opponent.cur_pos) {
            // swap two player;
            swap(cur_player, opponent);
        }
        phase = Phase::choose;
    }
}