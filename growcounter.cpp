#include "utils.hh"
#include "trace.hh"
#include "linearize.hh"

#include <iostream>

using namespace std;

struct incr {};
struct ack {};
struct read {};
struct ret { unsigned x; };

bool operator==(incr, incr) { return true; };
bool operator==(ack, ack) { return true; };
bool operator==(read, read) { return true; };
bool operator==(ret r1, ret r2) { return r1.x == r2.x; };
ostream& operator<<(ostream& os, incr) { return os << "incr"; }
ostream& operator<<(ostream& os, ack) { return os << "ack"; }
ostream& operator<<(ostream& os, read) { return os << "read"; }
ostream& operator<<(ostream& os, ret r) { return os << "ret(" << r.x << ")"; }

struct GCounter {
    using state_t = unsigned;
    using input_t = variant<incr, read>;
    using output_t = variant<ack, ret>; 

    static pair<state_t, output_t> delta(state_t s, input_t i) {
        return visit(overloaded {
            [s] (incr) -> pair<state_t, output_t> { return {s + 1, ack{}}; },
            [s] (read) -> pair<state_t, output_t> { return {s, ret{s}}; },
        }, i);
    }

    static const state_t init = 0;
};

int main() {
    cout << is_valid_trace<GCounter>({{read{}, ret{0}}}) << endl;
    cout << is_valid_trace<GCounter>({{incr{}, ack{}}, {read{}, ret{1}}}) << endl;
    cout << is_valid_trace<GCounter>({{incr{}, ack{}}, {read{}, ret{0}}}) << endl;

    print_linearizations<GCounter>({{incr{}, 1}, {incr{}, 2}, {read{}, 3}, {ret{1}, 3}});
    print_linearizations<GCounter>({{incr{}, 1}, {incr{}, 2}, {incr{}, 3}, {read{}, 4}, {read{}, 5}, {read{}, 6}, {ret{3}, 4}, {ret{2}, 5}, {ret{1}, 6}, {ack{}, 1}, {ack{}, 2}, {ack{}, 3}});
}
