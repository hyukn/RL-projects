
#include "patch.h"
#include <unordered_map>

using namespace std;

// candidates utilities
// for_each(board.begin(), board.end(), [](int x) { cout << x << " ";});

place_t toplace(const piece_t& piece, int x, int y) {
    auto board = nc::zeros<int> (nc::Shape(BOARD_SIZE, BOARD_SIZE));
    board.put(nc::Slice(x, x + piece.numCols()), nc::Slice(y, y + piece.numRows()), piece);
    place_t place = 0;
    place_t one = 1;
    // cout << board << endl;
    // cout << piece << endl;
    for (auto entry: board) {
        place = place + place_t(one * entry);
        one <<= 1;
        // cout << "piece: " << piece << x << " " << y << endl;
        // cout << "one " << one << " place " << place << endl;
    }
    return place;
}

void Patch::transpose() {
    trans.clear();
    auto t = piece;
    for (int i = 0; i < 2; ++i) { 
        for (int j = 0; j < 4; ++j) {
            t = nc::rot90(t);
            auto contains = find_if(trans.begin(), trans.end(), [t](const piece_t& a) { return nc::array_equal(t, a); } );
            if (contains == trans.end()) {
                trans.push_back(piece_t(t));
            }
        }
        t = nc::transpose(t);
    }
}

const vector<piece_t>& Patch::get_trans() const {
    return trans;
}

int PatchLib::ONExONE_ID = 0;

PatchLib::PatchLib() {
    init();
}

// void PatchLib::add_all_patch_prev() {
//     add_patch(Patch(11, 0, 0, 0, piece_t({1})));
//     add_patch(Patch(21, 2, 1, 0, piece_t({1, 1})));
//     add_patch(Patch(31, 2, 2, 0, piece_t({1, 1, 1})));
//     add_patch(Patch(32, 1, 3, 0, piece_t({{1, 1}, {1, 0}})));
//     add_patch(Patch(33, 3, 1, 0, piece_t({{1, 1}, {1, 0}})));
//     add_patch(Patch(41, 2, 2, 0, piece_t({{1, 1, 1}, {0, 1, 0}})));
//     add_patch(Patch(42, 3, 2, 1, piece_t({{0, 1, 1}, {1, 1, 0}})));
//     add_patch(Patch(43, 3, 3, 1, piece_t({{1, 1, 1, 1}})));
//     add_patch(Patch(44, 4, 2, 1, piece_t({{1, 1, 1}, {1, 0, 0}})));
//     add_patch(Patch(45, 6, 5, 2, piece_t({{1, 1}, {1, 1}})));
//     add_patch(Patch(46, 4, 6, 2, piece_t({{1, 1, 1}, {1, 0, 0}})));
//     add_patch(Patch(47, 7, 6, 3, piece_t({{0, 1, 1}, {1, 1, 0}})));
//     add_patch(Patch(51, 2, 2, 0, piece_t({{1, 1, 1}, {1, 1, 0}})));
//     add_patch(Patch(52, 1, 2, 0, piece_t({{1, 1, 1}, {1, 0, 1}})));
//     add_patch(Patch(53, 7, 1, 1, piece_t({1, 1, 1, 1, 1})));
//     add_patch(Patch(54, 3, 4, 1, piece_t({{1, 1, 1, 1}, {0, 1, 0, 0}})));
//     add_patch(Patch(55, 2, 3, 1, piece_t({{0, 1, 1, 1}, {1, 1, 0, 0}})));
//     add_patch(Patch(56, 10, 3, 2, piece_t({{1, 1, 1, 1}, {1, 0, 0, 0}})));
//     add_patch(Patch(57, 5, 4, 2, piece_t({{0, 1, 0}, {1, 1, 1}, {0, 1, 0}})));
//     add_patch(Patch(58, 5, 5, 2, piece_t({{1, 0, 0}, {1, 1, 1}, {1, 0, 0}})));
//     add_patch(Patch(59, 10, 4, 3, piece_t({{0, 0, 1}, {0, 1, 1}, {1, 1, 0}})));
//     add_patch(Patch(61, 4, 2, 0, piece_t({{0, 1, 1, 1}, {1, 1, 1, 0}})));
//     add_patch(Patch(62, 2, 1, 0, piece_t({{0, 0, 1, 0}, {1, 1, 1, 1}, {0, 1, 0, 0}})));
//     add_patch(Patch(63, 1, 2, 0, piece_t({{0, 0, 0, 1}, {1, 1, 1, 1}, {1, 0, 0, 0}})));
//     add_patch(Patch(60, 0, 3, 1, piece_t({{0, 1, 0, 0}, {1, 1, 1, 1}, {0, 1, 0, 0}})));
//     add_patch(Patch(64, 1, 5, 1, piece_t({{1, 1, 1, 1}, {1, 0, 0, 1}})));
//     add_patch(Patch(65, 7, 4, 2, piece_t({{1, 1, 1, 1}, {0, 1, 1, 0}})));
//     add_patch(Patch(66, 3, 6, 2, piece_t({{0, 1, 0}, {1, 1, 1}, {1, 0, 1}})));
//     add_patch(Patch(67, 7, 2, 2, piece_t({{1, 0, 0, 0}, {1, 1, 1, 1}, {1, 0, 0, 0}})));
//     add_patch(Patch(68, 10, 5, 3, piece_t({{1, 1, 1, 1}, {1, 1, 0, 0}})));
//     add_patch(Patch(69, 8, 6, 3, piece_t({{0, 0, 1}, {1, 1, 1}, {1, 1, 0}})));
//     add_patch(Patch(71, 2, 3, 0, piece_t({{1, 1, 1}, {0, 1, 0}, {1, 1, 1}})));
//     add_patch(Patch(72, 1, 4, 1, piece_t({{0, 0, 1, 0, 0}, {1, 1, 1, 1, 1}, {0, 0, 1, 0, 0}})));
//     add_patch(Patch(81, 5, 3, 1, piece_t({{0, 1, 1, 0}, {1, 1, 1, 1}, {0, 1, 1, 0}})));
// }

void PatchLib::add_all_patch() {
    add_patch(Patch(0, 0, 0, 0, piece_t({1})));
    add_patch(Patch(1, 2, 1, 0, piece_t({1, 1})));
    add_patch(Patch(2, 2, 2, 0, piece_t({1, 1, 1})));
    add_patch(Patch(3, 1, 3, 0, piece_t({{1, 1}, {1, 0}})));
    add_patch(Patch(4, 3, 1, 0, piece_t({{1, 1}, {1, 0}})));
    add_patch(Patch(5, 2, 2, 0, piece_t({{1, 1, 1}, {0, 1, 0}})));
    add_patch(Patch(6, 3, 2, 1, piece_t({{0, 1, 1}, {1, 1, 0}})));
    add_patch(Patch(7, 3, 3, 1, piece_t({{1, 1, 1, 1}})));
    add_patch(Patch(8, 4, 2, 1, piece_t({{1, 1, 1}, {1, 0, 0}})));
    add_patch(Patch(9, 6, 5, 2, piece_t({{1, 1}, {1, 1}})));
    add_patch(Patch(10, 4, 6, 2, piece_t({{1, 1, 1}, {1, 0, 0}})));
    add_patch(Patch(11, 7, 6, 3, piece_t({{0, 1, 1}, {1, 1, 0}})));
    add_patch(Patch(12, 2, 2, 0, piece_t({{1, 1, 1}, {1, 1, 0}})));
    add_patch(Patch(13, 1, 2, 0, piece_t({{1, 1, 1}, {1, 0, 1}})));
    add_patch(Patch(14, 7, 1, 1, piece_t({1, 1, 1, 1, 1})));
    add_patch(Patch(15, 3, 4, 1, piece_t({{1, 1, 1, 1}, {0, 1, 0, 0}})));
    add_patch(Patch(16, 2, 3, 1, piece_t({{0, 1, 1, 1}, {1, 1, 0, 0}})));
    add_patch(Patch(17, 10, 3, 2, piece_t({{1, 1, 1, 1}, {1, 0, 0, 0}})));
    add_patch(Patch(18, 5, 4, 2, piece_t({{0, 1, 0}, {1, 1, 1}, {0, 1, 0}})));
    add_patch(Patch(19, 5, 5, 2, piece_t({{1, 0, 0}, {1, 1, 1}, {1, 0, 0}})));
    add_patch(Patch(20, 10, 4, 3, piece_t({{0, 0, 1}, {0, 1, 1}, {1, 1, 0}})));
    add_patch(Patch(21, 4, 2, 0, piece_t({{0, 1, 1, 1}, {1, 1, 1, 0}})));
    add_patch(Patch(22, 2, 1, 0, piece_t({{0, 0, 1, 0}, {1, 1, 1, 1}, {0, 1, 0, 0}})));
    add_patch(Patch(23, 1, 2, 0, piece_t({{0, 0, 0, 1}, {1, 1, 1, 1}, {1, 0, 0, 0}})));
    add_patch(Patch(24, 0, 3, 1, piece_t({{0, 1, 0, 0}, {1, 1, 1, 1}, {0, 1, 0, 0}})));
    add_patch(Patch(25, 1, 5, 1, piece_t({{1, 1, 1, 1}, {1, 0, 0, 1}})));
    add_patch(Patch(26, 7, 4, 2, piece_t({{1, 1, 1, 1}, {0, 1, 1, 0}})));
    add_patch(Patch(27, 3, 6, 2, piece_t({{0, 1, 0}, {1, 1, 1}, {1, 0, 1}})));
    add_patch(Patch(28, 7, 2, 2, piece_t({{1, 0, 0, 0}, {1, 1, 1, 1}, {1, 0, 0, 0}})));
    add_patch(Patch(29, 10, 5, 3, piece_t({{1, 1, 1, 1}, {1, 1, 0, 0}})));
    add_patch(Patch(30, 8, 6, 3, piece_t({{0, 0, 1}, {1, 1, 1}, {1, 1, 0}})));
    add_patch(Patch(31, 2, 3, 0, piece_t({{1, 1, 1}, {0, 1, 0}, {1, 1, 1}})));
    add_patch(Patch(32, 1, 4, 1, piece_t({{0, 0, 1, 0, 0}, {1, 1, 1, 1, 1}, {0, 0, 1, 0, 0}})));
    add_patch(Patch(33, 5, 3, 1, piece_t({{0, 1, 1, 0}, {1, 1, 1, 1}, {0, 1, 1, 0}})));
}

void PatchLib::init() {
    add_all_patch();
    auto array_idx = 0;
    for (auto& p: lib) {
        p.second.transpose();
        const vector<piece_t> trans = p.second.get_trans();
        auto id = p.first;
        auto trans_idx = 0;
        for (auto& piece: trans) {
            auto shape = piece.shape();
            for (auto x = 0; x < BOARD_SIZE; ++x) {
                for (auto y = 0; y < BOARD_SIZE; ++y) {
                    if (x + piece.numCols() <= BOARD_SIZE && y + piece.numRows() <= BOARD_SIZE) {
                        auto place = toplace(piece, x, y);
                        patch_alter_vec.push_back(place);
                        id_vec.push_back(p.first);
                        id_map[id].push_back(array_idx);
                        place_map[id].push_back(place);

                        patch_xy2idx[make_tuple(id, x, y, trans_idx)] = array_idx;
                        patch_xy2place[make_tuple(id, x, y, trans_idx)] = place;
                        array_idx++;
                    }
                }
            }
            trans_idx++;
        }
    }

    patch_alter_array = patch_alter_vec;

    // initial square 7
    auto piece7 = nc::ones<int>(nc::Shape(7, 7));
    for (auto x = 0; x < BOARD_SIZE - 7 + 1; ++x) {
        for (auto y = 0; y < BOARD_SIZE - 7 + 1; ++y) {
            auto place = toplace(piece7, x, y);
            square7.push_back(place);
        }
    }

    auto patch_xy2place_sorted = unordered_map<int, place_t>();

    // sorted patch on board
    // sorted according to different x, y
    for (auto p: lib) {
        for (auto x = 0; x < BOARD_SIZE; ++x) {
            auto x2y2pi = vector<pair<int, place_t>>();
            for (auto y = 0; y < BOARD_SIZE; ++y) {
                auto trans = p.second.get_trans();
                for (int t = 0; t < trans.size(); t++) {
                    auto key = make_tuple(p.second.id, x, y, t);
                    auto a = patch_xy2place.find(key);
                    if (patch_xy2place.find(key) != patch_xy2place.end()) {
                        patch_xy2pi_sorted[p.second.id].push_back(
                            pair<int, place_t>(patch_xy2idx.find(key)->second, patch_xy2place.find(key)->second)
                        );

                        x2y2pi.push_back(
                            pair<int, place_t>(patch_xy2idx.find(key)->second, patch_xy2place.find(key)->second)
                        );

                    }
                }
            }
            patch_x2y2pi_sorted[p.second.id].push_back(x2y2pi);
        }
    }

    // overlap between different pieces
    // place_id 2 all overlap placements
    vector<int> entry2place[BOARD_SIZE * BOARD_SIZE];
    vector<int> place2entry[patch_alter_vec.size()];

    place_t board = 1;
    for (auto entry = 0; entry < BOARD_SIZE * BOARD_SIZE; ++entry) {
        for (auto pid = 0; pid < patch_alter_vec.size(); pid++) {
            auto& p = patch_alter_vec[pid];
            if ((board & p) != 0) {
                entry2place[entry].push_back(pid);
                place2entry[pid].push_back(entry);
            }
        }
        board <<= 1;
    }

    for (int i = 0; i < patch_alter_vec.size(); i++) {
        // cout << i << " ";
        auto p_overlap = unordered_map<int, vector<place_t>>();
        auto& patch_id = id_vec[i];
        for (auto& entry: place2entry[i]) {
            for (auto& pid: entry2place[entry]) {
                if (id_vec[pid] != patch_id) {
                    p_overlap[id_vec[pid]].push_back(patch_alter_vec[pid]);
                }
            }
        }
        overlap.push_back(p_overlap);
    }

    // for (int i = 0; i < patch_alter_vec.size(); i++) {
    //     auto& pid = id_vec[i];
    //     auto& place = patch_alter_vec[i];
    //     auto p_overlap = unordered_map<int, vector<place_t>>();
    //     cout << i << " ";
    //     for (auto& p_: place_map) {
    //         if (p_.first != pid) {
    //             for (auto& place_: p_.second) {
    //                 auto a = place_ & place;
    //                 // cout << a << " " << place_ << " " << place << endl;
    //                 if ((place_ & place) != 0) {
    //                     p_overlap[p_.first].push_back(place_);
    //                 }
    //             }
    //         }
    //     }
    //     overlap.push_back(p_overlap);
    // }
    // cout << endl;

    // for (auto& p: place_map) {
    //     auto& pid = p.first;
    //     auto& places = p.second;
    //     auto p_overlap = unordered_map<int, vector<place_t>>();
    //     cout << pid << endl;
    //     for (auto& place: places) {
    //         for (auto& p_: place_map) {
    //             if (p_.first != pid) {
    //                 for (auto& place_: p_.second) {
    //                     auto a = place_ & place;
    //                     // cout << a << " " << place_ << " " << place << endl;
    //                     if ((place_ & place) != 0) {
    //                         p_overlap[p_.first].push_back(place_);
    //                     }
    //                 }
    //             }
    //         }
    //         overlap.push_back(p_overlap);
    //     }
    // }
    
    auto piece = nc::zeros<int>(nc::Shape(BOARD_SIZE, BOARD_SIZE));
    for (auto x = 0; x < BOARD_SIZE; ++x) {
        for (auto y = 0; y < BOARD_SIZE; ++y) {
            place_t one = 1;
            place_t board = (one << (x * BOARD_SIZE + y));
            int regret = 0;
            for (auto& p: patch_alter_vec) {
                regret += ((board & p) != 0);
            }
            regrets[x][y] = pair<place_t, int>(board, regret);
            // piece[x, y] = 0;
        }
    }

    cout << "PatchLib initialized." << endl;
}
