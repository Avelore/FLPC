#include <iostream>
#include <vector>
#include <queue>
#include <utility>
#include <tuple>
using namespace std;


const vector<char> STATE_NAMES = { 'S', 'A', 'B', 'C', 'F' };
const vector<vector<char>> ADJ_MATRIX = {
        /* S    A    B    C    F  */ // matrice de adiacenta
/* S */ {  0,  'a', 'a',  0,   0  },
/* A */ { 'b',  0,   0,   0,   0  },
/* B */ {  0,   0,   0,  'a',  0  },
/* C */ { 'b',  0,   0,   0,  'a' },
/* F */ {  0,   0,   0,   0,   0  },
};
const int START_STATE = 0; /* A */
const int FINAL_STATE = 4; /* F */


int main() {
    string input_string;
    getline(cin, input_string);

    // pairs of (state, index)
    queue<pair<int, int>> next_states;
    next_states.push(make_pair(START_STATE, 0));
    bool is_valid = false;

    while (!next_states.empty()) {
        int state, index;
        tie(state, index) = next_states.front(); // takes the next element from the queue
        next_states.pop();

        //in case it reaches the final state
        if (state == FINAL_STATE && index == (int)input_string.length()) {
            is_valid = true;
            break;
        }
        //checks each character
        char current_character = input_string[index];
        for (int i = 0; i < (int)ADJ_MATRIX[state].size(); i++) {
            if (ADJ_MATRIX[state][i] == current_character) {
                next_states.push(make_pair(i, index + 1));
            }
        }
    }

    if (is_valid) cout << "is valid" << endl;
    else cout << "is not valid" << endl;


    return 0;
}
