#pragma once

#include "NumCpp.hpp"
#include <map>
#include <unordered_map>
#include <vector>

#include <boost/multiprecision/cpp_int.hpp>

#define BOARD_SIZE 9

using namespace std;

using piece_t = nc::NdArray<int>;
using place_t = boost::multiprecision::uint128_t;
// using place_t = long long;

class Patch {
public:
    int id = 0;
    int tc = 0;
    int bc = 0;
    int br = 0;
    piece_t piece;

    int area;
    vector<piece_t> trans;

    Patch() {}
    Patch(int id, int bc, int tc, int br, const piece_t& p):
        id(id), bc(bc), tc(tc), br(br), piece(p) {
            area = count(p.begin(), p.end(), 1);
            transpose();
        }

    void transpose();
    const vector<piece_t>& get_trans() const;
    nc::Shape shape() const { return piece.shape(); }
};

class PatchLib {
public:
    static int ONExONE_ID;
    map<int, Patch> lib;
    // map<int, map<tuple<int, int, int>, board_t>> patch_alter;
    vector<place_t> patch_alter_vec;

    // patch id -> all placements
    unordered_map<int, vector<place_t>> place_map;

    nc::NdArray<place_t> patch_alter_array;
    vector<int> id_vec;

    // patch id -> all placements
    unordered_map<int, vector<int>> id_map;
    vector<place_t> square7;

    // nc::NdArray<int> id_array;
    map<tuple<int, int, int, int>, int> patch_xy2idx;
    map<tuple<int, int, int, int>, place_t> patch_xy2place;

    unordered_map<int, vector<pair<int, place_t>>> patch_xy2pi_sorted;
    unordered_map<int, vector<vector<pair<int, place_t>>>> patch_x2y2pi_sorted;

    vector<unordered_map<int, vector<place_t>>> overlap;
    pair<place_t, int> regrets[BOARD_SIZE][BOARD_SIZE];

    PatchLib();
    void add_patch(const Patch& patch) { lib[patch.id] = patch; }
    void add_all_patch();
    void init();

    inline const Patch& get_patch(int id) const { return lib.find(id)->second; }
    const int pl_id_2_pa_id(int pl_id) const { return id_vec[pl_id]; }
    inline const vector<int>& get_placeid(int id) const { return id_map.find(id)->second; }
    vector<int> get_unique_patch_id() const { 
        vector<int> q;
        for (auto& p: lib) {
            if (p.first != ONExONE_ID) {
                q.push_back(p.first);
            }
        }
        return q;
     }
};