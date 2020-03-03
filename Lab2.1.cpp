#include <iostream>
#include <vector>
#include <set>
#include <map>
#include <queue>
using namespace std;

int main() {
    // describe initial non-deterministic finite automata
    const vector<int> states = { 0, 1, 2, 3 };
    const vector<char> alphabet = { 'a', 'b', 'c' };
    const int final_state = 3;
    // list of elements of form (from, (symbol, next_state))
    const vector<pair<int, pair<char, int>>> transition_table = {
            { 0, { 'a', 0 } },
            { 0, { 'a', 1 } },
            { 1, { 'b', 2 } },
            { 2, { 'a', 2 } },
            { 2, { 'b', 3 } },
            { 3, { 'a', 3 } }
    };

    // generate new deterministic finite automata

    // new state could be composed of several NDFA states
    // we would store it as a bitmask, where position of turned on bit indicates presence of according NDFA state
    set<int> new_states;
    set<pair<int, pair<char, int>>> new_transition_table;

    queue<int> need_review;

    // inserting first state
    // (1 << index) to mark state at index
    need_review.push(1 << 0); //need to be reviewed, state0
    new_states.insert(1 << 0); //already reviewed, state0

    while (!need_review.empty()) {
        int states_mask = need_review.front(); //we take the first state and work with it
        need_review.pop();                     //we remove the first state from the queue

        for (char symbol : alphabet) {
            int reachable_states_mask = 0;

            // deconstructing mask into a simple list
            // 32 because using 32-bit integers
            vector<int> states_indexes;      //states to be reviewed
            for (int i = 0; i < 32; i++) {
                if ((states_mask & (1 << i)) != 0) {
                    states_indexes.push_back(i);
                }
            }

            for (int state : states_indexes) {
                // find outgoing edges from this state labeled with symbol
                for (auto &row : transition_table) {
                    if (row.first != state || row.second.first != symbol)
                        continue;

                    reachable_states_mask |= 1 << (row.second.second);  //creating reachable states
                }
            }

            // if newly created  reachable state is not yet visited
            if (reachable_states_mask != 0) {
                if (new_states.find(reachable_states_mask) == new_states.end()) {
                    new_states.insert(reachable_states_mask);
                    need_review.push(reachable_states_mask);
                }

                new_transition_table.insert({ states_mask, { symbol, reachable_states_mask }});
            }
        }
    }

    // creating optional state mapping
    map<int, int> state_mapping;
    int last_index = 0;
    for (int state : new_states) {
        state_mapping.insert({ state, last_index });
        last_index++;
    }


    // display transition table
    for (auto &row : new_transition_table) {
        printf("s(%d, %c) = %d\n", state_mapping[row.first], row.second.first, state_mapping[row.second.second]);
    }
    return 0;
}
