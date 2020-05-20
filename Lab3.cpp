#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <cctype>
#include <map>
#include <set>
using namespace std;


// the symbol representing null production rule in the input file
const char NULL_CHARACTER = '\\';


// .first - left-hand side; .second - right-hand side
using ProductionRule = pair<char, string>;
using Grammar = vector<ProductionRule>;


// i/o routine
Grammar read_grammar();
void show_grammar(const Grammar &g);

// context-free grammar conversion
Grammar eliminate_null_productions(const Grammar &g);
Grammar eliminate_unit_productions(const Grammar &g);
Grammar eliminate_useless_symbols(const Grammar &g);
Grammar transform_into_cnf(const Grammar &g);


int main() {
    freopen("Lab3Input15.txt", "r", stdin);
    Grammar g = read_grammar();

    g = eliminate_null_productions(g);
    g = eliminate_unit_productions(g);
    g = eliminate_useless_symbols(g);
    g = transform_into_cnf(g);

    show_grammar(g);
    return 0;
}


Grammar read_grammar() {
    vector<pair<char, string>> grammar;

    // read input line by line, finish when encountering an empty line
    string line;
    while (getline(cin, line) && !line.empty()) {
        int length = (int)line.size();

        // save left hand-side character
        char lhs = line[0];

        // skip arrow (->) characters and optional spaces around it
        int i = 1;
        while (i < length && isspace(line[i])) {
            i++;
        }
        i += 2;

        // read production rules which may be given either one per line or many, separated by a vertical bar
        while (i < length) {
            // skip spaces and vertical bars
            while (i < length && (isspace(line[i]) || line[i] == '|')) {
                i++;
            }

            // cut right hand-side of the production rule
            int l = 0;
            while ((i+l) < length && (isalpha(line[i+l]) || line[i+l] == NULL_CHARACTER)) {
                l++;
            }
            if (l != 0) {
                grammar.emplace_back(lhs, line.substr(i, l));
            }
            i += l;
        }
    }

    return grammar;
}

void show_grammar(const Grammar &g) {
    for (const pair<char, string> &rule : g) {
        cout << rule.first << " -> " << rule.second << '\n';
    }
}

Grammar eliminate_null_productions(const Grammar &g) {
    map<char, bool> nullable;
    nullable[NULL_CHARACTER] = true;

    // define a predicate to check if a symbol is "nullable"
    auto is_symbol_nullable = [&nullable](char c){ return nullable[c]; };

    // determine all "nullable" characters
    bool finished = false;
    while (!finished) {
        finished = true;

        for (const ProductionRule &rule : g) {
            // don't recompute, just skip it
            if (nullable[rule.first]) {
                continue;
            }

            bool all_nullable = all_of(rule.second.begin(), rule.second.end(), is_symbol_nullable);
            if (all_nullable) {
                nullable[rule.first] = true;
                finished = false;
            }
        }
    }

    // remove production rules containing "nullable" characters
    vector<ProductionRule> new_rules;
    for (auto rule : g) {
        bool contains_nullable = any_of(rule.second.begin(), rule.second.end(), is_symbol_nullable);

        if (!contains_nullable) {
            new_rules.emplace_back(rule);
            continue;
        }

        // ignore productions of form N -> \

        if (rule.second[0] == NULL_CHARACTER) {
            continue;
        }

        // handle productions of form X -> aNb
        new_rules.emplace_back(rule);
        string &s = rule.second;
        // verifying each segment of length l starting from position i
        for (int l = 1; l <= ((int)s.size()-1); l++) {
            for (int i = 0; i < ((int)s.size()-l+1); i++) {
                bool is_segment_nullable = all_of(s.begin()+i, s.begin()+i+l, is_symbol_nullable);
                if (is_segment_nullable) {
                    string new_rhs = s.substr(0, i) + s.substr(i+l);
                    new_rules.emplace_back(rule.first, new_rhs);
                }
            }
        }
    }

    return new_rules;
}

Grammar eliminate_unit_productions(const Grammar &g) {
    set<ProductionRule> new_rules, old_rules(g.begin(), g.end());

    // do this continuously while there are some unit productions
    bool finished = false;
    while (!finished) {
        finished = true;

        // traverse production rules list and find if there are unit productions
        for (auto &rule : old_rules) {
            // leave rules that are not unit productions (A -> B)
            if (rule.second.size() > 1 || !isupper(rule.second[0])) {
                new_rules.emplace(rule);
                continue;
            }

            // find all rules where B is on the left side
            char b = rule.second[0];
            for (auto& r : g) {
                if (r.first == b) {
                    new_rules.emplace(rule.first, r.second);
                }
            }
            finished = false;
        }

        old_rules = new_rules;
        new_rules.clear();
    }

    return vector<ProductionRule>(old_rules.begin(), old_rules.end());
}

Grammar eliminate_useless_symbols(const Grammar &g) {
    // find the set of accessible symbols
    set<char> accessible_symbols;
    accessible_symbols.insert('S');

    for (bool finished = false; !finished; ) {
        finished = true;

        for (auto &rule : g) {
            // skip if its not in the set of accessible symbols
            if (accessible_symbols.count(rule.first) == 0) {
                continue;
            }

            // update accessible set with symbols adjacent to the current rule
            for (char c : rule.second) {
                if (isupper(c) && accessible_symbols.count(c) == 0) {
                    accessible_symbols.insert(c);
                    finished = false;
                }
            }
        }
    }

    // find the set of productive symbols
    set<char> productive_symbols;

    // define predicate of a productive symbol
    auto is_symbol_productive = [&productive_symbols](char c){ return islower(c) || productive_symbols.count(c) != 0; };

    for (bool finished = false; !finished; ) {
        finished = true;

        for (auto &rule : g) {
            bool is_productive = all_of(rule.second.begin(), rule.second.end(), is_symbol_productive);

            if (is_productive && productive_symbols.count(rule.first) == 0) {
                productive_symbols.insert(rule.first);
                finished = false;
            }
        }
    }

    // filter production rules using calculated sets
    vector<ProductionRule> new_rules;
    for (auto &rule : g) {
        // if its left side symbol is inaccessible, skip it
        if (accessible_symbols.count(rule.first) == 0) {
            continue;
        }

        // if contains nonproductive symbols on its right side, skip it
        if (!all_of(rule.second.begin(), rule.second.end(), is_symbol_productive)) {
            continue;
        }

        new_rules.emplace_back(rule);
    }

    return new_rules;
}

Grammar transform_into_cnf(const Grammar &g) {
    // find letters that are used in at least one production rule
    set<int> used_letters;
    for (auto &rule : g) {
        used_letters.insert(rule.first);
    }

    // define a production that will give us next available variable
    auto next_unused_symbol = [&used_letters]() -> char {
        char x = 'A';
        for (; x <= 'Z' && used_letters.count(x) != 0; x++);
        used_letters.insert(x);
        return x;
    };

    // transform all production rules to these formats: A -> BC, A -> a
    vector<ProductionRule> new_rules;
    map<char, char> symbol_mapping;
    for (auto &rule : g) {

        // if it's a terminal character, no need to transform anything
        if (rule.second.size() == 1) {
            new_rules.emplace_back(rule);
            continue;
        }

        // rename terminal symbols
        string s = rule.second;
        for (char &c : s) {
            // get a new name for it
            if (islower(c)) {
                if (symbol_mapping.count(c) == 0) {
                    symbol_mapping[c] = next_unused_symbol();
                }
                c = symbol_mapping[c];
            }
        }

        // if it contains only 2 symbols, no need for rule chaining
        if (s.size() == 2) {
            new_rules.emplace_back(rule.first, s);
            continue;
        }

        // chain from the back to the front
        char last = next_unused_symbol();
        new_rules.emplace_back(last, s.substr(s.size()-2));

        for (int i = (int)s.size()-2-1; i > 0; i--) {
            char pr = next_unused_symbol();
            new_rules.emplace_back(pr, string(1, s[i]) + string(1, last));
            last = pr;
        }

        new_rules.emplace_back(rule.first, string(1, s[0]) + string(1, last));
    }

    // adding production rules for newly created symbols
    for (auto &p : symbol_mapping) {
        new_rules.emplace_back(p.second, string(1, p.first));
    }

    return new_rules;
}
