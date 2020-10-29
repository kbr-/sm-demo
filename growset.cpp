#include "utils.hh"
#include "trace.hh"
#include "linearize.hh"

#include <set>
#include <iostream>

using namespace std;

struct add { int x; };
struct ack {};
struct has { int x; };
struct yes {};
struct no {};

bool operator==(add a1, add a2) { return a1.x == a2.x; };
bool operator==(ack, ack) { return true; };
bool operator==(has h1, has h2) { return h1.x == h2.x; };
bool operator==(yes, yes) { return true; };
bool operator==(no, no) { return true; };
ostream& operator<<(ostream& os, add a) { return os << "add(" << a.x << ")"; }
ostream& operator<<(ostream& os, ack) { return os << "ack"; }
ostream& operator<<(ostream& os, has h) { return os << "has(" << h.x << ")"; }
ostream& operator<<(ostream& os, yes) { return os << "yes"; }
ostream& operator<<(ostream& os, no) { return os << "no"; }

struct GSet {
    using state_t = set<int>;
    using input_t = variant<add, has>;
    using output_t = variant<ack, yes, no>; 

    static pair<state_t, output_t> delta(state_t s, input_t i) {
        return visit(overloaded {
            [s] (add v) mutable -> pair<state_t, output_t> {
                s.insert(v.x);
                return {s, ack{}};
            },
            [s] (has v) -> pair<state_t, output_t> {
                if (s.contains(v.x)) {
                    return {s, yes{}};
                }
                return {s, no{}};
            },
        }, i);
    }

    static const state_t init;
};

const GSet::state_t GSet::init = {};

int main() {
    cout << is_valid_trace<GSet>({{add{1}, ack{}}, {has{1}, yes{}}, {has{0}, no{}}}) << endl;
    cout << is_valid_trace<GSet>({{add{1}, ack{}}, {has{1}, no{}}}) << endl;

    print_linearizations<GSet>({{add{1}, 1}, {add{2}, 2}, {has{1}, 3}, {yes{}, 3}});
    print_linearizations<GSet>({{add{1}, 1}, {add{2}, 2}, {ack{}, 2}, {has{1}, 3}, {no{}, 3}});
}
