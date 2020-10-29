#include "utils.hh"
#include "trace.hh"
#include "linearize.hh"

#include <iostream>

using namespace std;

struct cas { int x; int y; };
struct ok {};
struct fail {};

bool operator==(ok, ok) { return true; };
bool operator==(fail, fail) { return true; };

ostream& operator<<(ostream& os, cas c) { return os << "cas(" << c.x << ", " << c.y << ")"; }
ostream& operator<<(ostream& os, ok) { return os << "ok"; }
ostream& operator<<(ostream& os, fail) { return os << "fail"; }

struct CASRegister {
    using state_t = int;
    using input_t = cas;
    using output_t = variant<ok, fail>;

    static pair<state_t, output_t> delta(state_t s, input_t i) {
        if (i.x == s)
            return {i.y, ok{}};
        return {s, fail{}};
    }

    static const state_t init = 0;
};

int main() {
    //cout << is_valid_trace<CASRegister>({{cas(1, 0), fail{}}, {cas{0, 1}, ok{}}, {cas{0, 2}, fail{}}, {cas{1, 2}, ok{}}}) << endl;
    //cout << is_valid_trace<CASRegister>({{cas(1, 0), fail{}}, {cas{0, 1}, ok{}}, {cas{0, 2}, ok{}}, {cas{1, 2}, ok{}}}) << endl;
    //print_trace<CASRegister>({{cas{0, 1}, ok{}}});

    print_linearizations<CASRegister>({{cas{0, 1}, 1}, {cas{0, 2}, 2}, {ok{}, 1}, {ok{}, 2}});

    print_linearizations<CASRegister>({{cas{0, 1}, 1}, {cas{0, 2}, 2}, {ok{}, 1}, {ok{}, 2}});
    print_linearizations<CASRegister>({{cas{0, 1}, 1}, {cas{0, 2}, 2}, {ok{}, 1}, {fail{}, 2}});
}
