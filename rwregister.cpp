#include "utils.hh"
#include "trace.hh"
#include "linearize.hh"
#include "box.hh"

#include <iostream>

using namespace std;

struct write { int x; };
struct ack {};
struct read {};
struct ret { int x; };

bool operator==(write w1, write w2) { return w1.x == w2.x; };
bool operator==(ack, ack) { return true; };
bool operator==(read, read) { return true; };
bool operator==(ret r1, ret r2) { return r1.x == r2.x; };

ostream& operator<<(ostream& os, write w) { return os << "write(" << w.x << ")"; }
ostream& operator<<(ostream& os, ack) { return os << "ack"; }
ostream& operator<<(ostream& os, read) { return os << "read"; }
ostream& operator<<(ostream& os, ret r) { return os << "ret(" << r.x << ")"; }

struct RWRegister {
    using state_t = int;
    using input_t = variant<write, read>;
    using output_t = variant<ack, ret>; 

    static pair<state_t, output_t> delta(state_t s, input_t i) {
        return visit(overloaded {
            [] (write w) -> pair<state_t, output_t> { return {w.x, ack{}}; },
            [s] (read) -> pair<state_t, output_t> { return {s, ret{s}}; },
        }, i);
    }

    static const state_t init = 0;
};

int main() {
    //auto box = make_box<RWRegister>();
    //cout << box(write{1}) << " " << box(read{}) << endl;

    //cout << is_valid_trace<RWRegister>({{write{1}, ack{}}, {read{}, ret{42}}}) << endl;

    print_linearizations<RWRegister>({{write{1}, 1}, {ack{}, 1}, {read{}, 2}, {ret{2}, 2}});
    print_linearizations<RWRegister>({{write{1}, 1}, {ack{}, 1}, {read{}, 2}, {ret{1}, 2}});
    print_linearizations<RWRegister>({{write{42}, 1}, {write{43}, 2}, {ack{}, 1}, {ack{}, 2}, {read{}, 3}, {ret{42}, 3}});
    print_linearizations<RWRegister>({{write{42}, 1}, {write{43}, 2}, {ack{}, 1}, {ack{}, 2}, {read{}, 3}, {ret{43}, 3}});

    print_linearizations<RWRegister>({{write{42}, 1}, {write{43}, 2}, {ack{}, 1}, {read{}, 3}, {ret{43}, 3}});
    print_linearizations<RWRegister>({{write{42}, 1}, {write{43}, 2}, {ack{}, 1}, {read{}, 3}, {ret{42}, 3}});
}
