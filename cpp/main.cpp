#include <iostream>
#include <omp.h>

#include "patch.h"
#include "patchwork_player.h"
#include "patchwork_state.h"
#include "patchwork.h"
#include "agent_random.h"
#include "agent_mm.h"
#include "agent_heu.h"
#include "agent_pvs.h"

#define NUM_THREADS 1

using namespace std;


void arena_parallel(int rounds) {
    std::cout << "# of rounds: " << rounds << endl << endl;
    
    int win_sum[NUM_THREADS] = {0};
    int win = 0;
    double start = omp_get_wtime();

#pragma omp parallel for num_threads(NUM_THREADS) schedule(dynamic)
    for (int i = 0; i < rounds; ++i) {
        int thread_id = omp_get_thread_num();
        srand(i);

        auto a = Patchwork();
        // auto p0 = agent_heu::Agent(0);
        // auto p0 = agent_random::Agent();
        // auto p1 = agent_random::Agent();

        auto p0 = agent_mm::Agent(6, 0);
        auto p1 = agent_heu::Agent(1);    

        // auto p0 = agent_mm::Agent(4, 0);
        // auto p1 = agent_mm::Agent(3, 1);
        a.play(p0, p1);
        std::cout
        << "thread " << thread_id << " "
        << "match " << i 
        << ", winner " << a.ps.winner()
        << ", score: " << a.ps.get_player(0).score() << " vs " << a.ps.get_player(1).score() << endl;
        // cout << a.ps.cur_player.id << " " << a.ps.cur_player.score() << " " << a.ps.cur_player.cur_pos << endl;
        // cout << a.ps.opponent.id << " " << a.ps.opponent.score() << endl;
        auto obs = a.ps.cur_player.observe();
        // cout << obs << endl;
        win_sum[thread_id] += ((a.ps.winner() == 0)? 1 : 0);
    }
    int sum = 0;
    double end = omp_get_wtime();
    cout << "winning ratio: " << double(accumulate(win_sum, win_sum+NUM_THREADS, sum)) / double(rounds) << ", time: " << end - start << endl;
    // std::cout << "winning ratio: " << double(win) / double(rounds) << ", time: " << end - start << endl;
}

void arena(int rounds) {

    int win = 0;
    srand(0);
    std::cout << "# of rounds: " << rounds << endl << endl;
    double start = omp_get_wtime();
    for (int i = 0; i < rounds; ++i) {
        auto a = Patchwork();

        // auto p0 = agent_heu::Agent(0);
        // auto p1 = agent_random::Agent();

        auto p0 = agent_mm::Agent(1, 0);   
        // auto p1 = agent_mm::Agent(3, 1);    
        auto p1 = agent_heu::Agent(1);    

        // auto p0 = agent_heu::Agent(0);   
        // auto p1 = agent_heu::Agent(1);   

        // auto p0 = agent_pvs::Agent(4, 0);
        // auto p1 = agent_pvs::Agent(3, 1);
        
        a.play(p0, p1);
        std::cout
        << "match " << i 
        << ", winner " << a.ps.winner()
        << ", score: " << a.ps.get_player(0).score() << " vs " << a.ps.get_player(1).score() << endl;
        // cout << a.ps.cur_player.id << " " << a.ps.cur_player.score() << " " << a.ps.cur_player.cur_pos << endl;
        // cout << a.ps.opponent.id << " " << a.ps.opponent.score() << endl;
        auto obs = a.ps.cur_player.observe();
        // cout << obs << endl;
        win += ((a.ps.winner() == 0)? 1 : 0);
    }
    double end = omp_get_wtime();
    std::cout << "winning ratio: " << double(win) / double(rounds) << ", time: " << end - start << endl;
}

int main(int argc, char *argv[]) {
    // int rounds = atoi(argv[1]);
    int rounds = 1;
    // bool display = atoi(argv[2])
    // arena(rounds);
    arena_parallel(rounds);
}