#include "utils.hh"
#include "trace.hh"
#include "linearize.hh"
#include "box.hh"

#include <iostream>

using namespace std;

struct cas { int x; int y; };
struct ok {};
struct fail {};
struct write { int x; };
struct ack {};
struct read {};
struct ret { int x; };

bool operator==(ok, ok) { return true; };
bool operator==(fail, fail) { return true; };
bool operator==(ack, ack) { return true; };
bool operator==(ret r1, ret r2) { return r1.x == r2.x; };
ostream& operator<<(ostream& os, cas c) { return os << "cas(" << c.x << ", " << c.y << ")"; }
ostream& operator<<(ostream& os, ok) { return os << "ok"; }
ostream& operator<<(ostream& os, fail) { return os << "fail"; }
ostream& operator<<(ostream& os, write w) { return os << "write(" << w.x << ")"; }
ostream& operator<<(ostream& os, ack) { return os << "ack"; }
ostream& operator<<(ostream& os, read) { return os << "read"; }
ostream& operator<<(ostream& os, ret r) { return os << "ret(" << r.x << ")"; }

struct RWCASRegister {
    using state_t = int;
    using input_t = variant<write, read, cas>;
    using output_t = variant<ok, fail, ack, ret>; 

    static pair<state_t, output_t> delta(state_t s, input_t i) {
        return visit(overloaded {
            [] (write w) -> pair<state_t, output_t> { return {w.x, ack{}}; },
            [s] (read) -> pair<state_t, output_t> { return {s, ret{s}}; },
            [s] (cas c) -> pair<state_t, output_t> {
                if (c.x == s)
                    return {c.y, ok{}};
                return {s, fail{}};
            },
        }, i);
    }

    static const state_t init = 0;
};

int main() {
    auto box = make_box<RWCASRegister>();
    cout << box(read{}) << " " << box(write{1}) << " "
         << box(cas{0, 2}) << " " << box(cas{1, 2}) << endl;

    print_is_valid_trace<RWCASRegister>({{cas{1, 3}, fail{}}, {write{1}, ack{}}, {cas{1, 3}, ok{}}, {read{}, ret{3}}, {write{42}, ack{}}, {read{}, ret{42}}});
    print_is_valid_trace<RWCASRegister>({{cas{1, 3}, ok{}}, {write{1}, ack{}}, {cas{1, 3}, ok{}}, {read{}, ret{3}}, {write{42}, ack{}}, {read{}, ret{42}}});

    /*
    print_linearizations<RWCASRegister>({{cas{0, 1}, 1}, {write{2}, 2}, {ack{}, 2}, {ok{}, 1}, {read{}, 3}, {ret{1}, 3}});
    print_linearizations<RWCASRegister>({{cas{0, 1}, 1}, {write{2}, 2}, {ack{}, 2}, {ok{}, 1}, {read{}, 3}, {ret{2}, 3}});
*/
}
