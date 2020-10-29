#include "utils.hh"
#include "trace.hh"
#include "linearize.hh"
#include "box.hh"

#include <map>
#include <memory>
#include <iostream>

using std::string;
using std::variant;
using std::vector;
using std::map;
using std::visit;
using std::pair;
using std::unique_ptr;
using std::cout;
using std::endl;
using std::ostream;

struct get { string key; };
struct cnst { int val; };
using expr = variant<get, cnst>;

struct eq { expr e1; expr e2; };
struct le { expr e1; expr e2; };
using bexpr = variant<eq, le>;

struct stmt;
struct put { string key; expr e; };
struct branch { bexpr cond; vector<stmt> then; vector<stmt> els; };
struct print { expr e; };
struct stmt { variant<put, branch, print> s; };

using txn = vector<stmt>;

ostream& operator<<(ostream& os, get g) { return os << "get(" << g.key << ")"; }
ostream& operator<<(ostream& os, cnst c) { return os << "const(" << c.val << ")"; }
ostream& operator<<(ostream& os, eq e) { return os << e.e1 << " == " << e.e2; }
ostream& operator<<(ostream& os, le e) { return os << e.e1 << " <= " << e.e2; }
ostream& operator<<(ostream& os, put p) { return os << "put(" << p.key << ": " << p.e << ")"; }
ostream& operator<<(ostream& os, branch b) { return os << "if (" << b.cond << ") { " << b.then << " } else {" << b.els << "}"; }
ostream& operator<<(ostream& os, print p) { return os << "print(" << p.e << ")"; }
ostream& operator<<(ostream& os, stmt s) { return os << s.s; }

int eval(const expr& e, const map<string, int>& s) {
    return visit(overloaded {
        [&] (const get& g) {
            auto it = s.find(g.key);
            if (it == end(s)) {
                return 0;
            }
            return it->second;
        },
        [] (const cnst& c) { return c.val; }
    }, e);
}

bool eval(const bexpr& e, const map<string, int>& s) {
    return visit(overloaded {
        [&] (const eq& e) { return eval(e.e1, s) == eval(e.e2, s); },
        [&] (const le& e) { return eval(e.e1, s) <= eval(e.e2, s); },
    }, e);
}

void exec(const stmt& st, map<string, int>& s, vector<int>& out) {
    visit(overloaded {
        [&] (const put& p) {
            auto v = eval(p.e, s);
            s[p.key] = v;
        },
        [&] (const print& p) { out.push_back(eval(p.e, s)); },
        [&] (const branch& b) {
            if (eval(b.cond, s)) {
                for (auto& stmt: b.then) {
                    exec(stmt, s, out);
                }
            } else {
                for (auto& stmt: b.els) {
                    exec(stmt, s, out);
                }
            }
        }
    }, st.s);
}

struct KVStore {
    using state_t = map<string, int>;
    using input_t = txn;
    using output_t = vector<int>; 

    static pair<state_t, output_t> delta(state_t s, input_t i) {
        vector<int> out;
        for (auto& st: i) {
            exec(st, s, out);
        }
        return {s, out};
    }

    static const state_t init;
};

const KVStore::state_t KVStore::init = {};

int main() {
    txn t1 {
        {print { get {"y"} }}
    };
    txn t2 {
        {print { get {"x"} }},
        {put { "x", cnst{1} }},
        {branch { eq { get {"y"}, cnst{0} }, {
            {put { "y", cnst{2} }}
        }, {
            {put { "y", cnst{3} }}
        }}},
        {print { get {"y"} }}
    };

    print_is_valid_trace<KVStore>({{t1, {0}}, {t2, {0, 2}}, {t1, {2}}, {t2, {1, 3}}});
    print_linearizations<KVStore>({{t1, 1}, {t2, 2}, {vector<int>{0}, 1}, {vector<int>{0, 2}, 2}});
}
