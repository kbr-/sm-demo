#pragma once

#include "history.hh"

template <SM M>
using trace_ids = std::vector<std::tuple<typename M::input_t, typename M::output_t, id>>;

template <typename T>
std::vector<std::vector<T>> ins_each_pos(std::vector<T> v, T elem) {
    std::vector<std::vector<T>> ret;
    for (auto i = v.begin(); i != v.end(); ++i) {
        ret.push_back({});
        auto& vv = ret.back();

        auto j = v.begin();
        for (; j != i; ++j) {
            vv.push_back(*j);
        }
        vv.push_back(elem);
        for (; j != v.end(); ++j) {
            vv.push_back(*j);
        }
    }
    ret.push_back(v);
    ret.back().push_back(elem);
    return ret;
}

template <SM M>
struct prediction {
    std::vector<std::tuple<typename M::input_t, typename M::output_t, id>> past;
    M::state_t curr;
    std::vector<std::pair<typename M::input_t, id>> future;
};

template <SM M>
std::vector<prediction<M>> linearize(typename history<M>::const_iterator it, typename history<M>::const_iterator end, prediction<M> p) {
    if (it == end) {
        return {p};
    }

    auto [e, id] = *it;
    return std::visit(overloaded {
        [&] (M::input_t i) {
            auto new_futs = ins_each_pos(p.future, std::pair{i, id});
            new_futs.push_back(std::move(p.future));
            std::vector<prediction<M>> ret;
            for (auto& fut: new_futs) {
                auto ps = linearize<M>(std::next(it), end, {p.past, p.curr, std::move(fut)});
                ret.insert(ret.end(), make_move_iterator(ps.begin()), make_move_iterator(ps.end()));
            }
            return ret;
        },
        [&] (M::output_t o) -> std::vector<prediction<M>> {
            {
                auto j = std::find_if(p.past.begin(), p.past.end(), [id] (auto& e) { return std::get<2>(e) == id; });
                if (j != p.past.end()) {
                    if (std::get<1>(*j) != o) {
                        return {};
                    }
                    return linearize<M>(std::next(it), end, std::move(p));
                }
            }

            auto j = p.future.begin();
            for (; j != p.future.end() && j->second != id; ++j) {
                auto [st, _o] = M::delta(p.curr, j->first);
                p.past.emplace_back(j->first, _o, j->second);
                p.curr = st;
            }
            if (j == p.future.end()) {
                return {};
            }
            auto [st, _o] = M::delta(p.curr, j->first);
            if (o != _o) {
                return {};
            }
            p.past.emplace_back(j->first, o, id);
            p.curr = st;
            p.future.erase(p.future.begin(), std::next(j));
            return linearize<M>(std::next(it), end, std::move(p));
        },
    }, e);
}

template <SM M>
std::vector<trace_ids<M>> linearize(const history<M>& h) {
    auto ps = linearize<M>(h.begin(), h.end(), {{}, M::init, {}});

    std::vector<trace_ids<M>> ret;
    for (auto& p: ps) {
        ret.push_back({});
        for (auto [i, o, id]: p.past) {
            ret.back().emplace_back(i, o, id);
        }
        for (auto [i, id]: p.future) {
            auto [st, o] = M::delta(p.curr, i);
            p.curr = st;
            ret.back().emplace_back(i, o, id);
        }
    }
    return ret;
}

template <SM M>
std::ostream& operator<<(std::ostream& os, const trace_ids<M>& trace) {
    os << "trace{";
    for (auto& [i, o, id]: trace) {
        os << "(" << i << ", " << o << ", id=" << id << "), ";
    }
    return os << "}";
}

template <SM M>
void print_linearizations(const history<M>& h) {
    auto ts = linearize<M>(h);
    if (ts.empty()) {
        operator<< <M>(std::cout, h) << " has no linearizations" << std::endl;
    } else {
        operator<< <M>(std::cout << "linearizations of ", h) << " are: " << std::endl;
        for (auto& t: ts) {
            operator<< <M>(std::cout << "    ", t) << std::endl;
        }
    }
}
