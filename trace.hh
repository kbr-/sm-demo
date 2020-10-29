#pragma once

#include <vector>
#include <tuple>
#include <iostream>

#include "sm.hh"

template <SM M>
using trace = std::vector<std::tuple<typename M::input_t, typename M::output_t>>;

template <SM M>
std::ostream& operator<<(std::ostream& os, const trace<M>& trace) {
    os << "trace{";
    for (auto& [i, o]: trace) {
        os << i << ", " << o << ", ";
    }
    return os << "}";
}

template <SM M>
void print_trace(const trace<M>& t) {
    operator<< <M>(std::cout, t) << std::endl;
}

template <SM M>
bool is_valid_trace(const trace<M>& trace) {
    auto s = M::init;
    for (auto& [i, o]: trace) {
        auto [next_s, next_o] = M::delta(s, i);
        if (next_o != o) {
            return false;
        }
        s = std::move(next_s);
    }
    return true;
}

template <SM M>
void print_is_valid_trace(const trace<M>& trace) {
    operator<< <M>(std::cout << "The sequence ", trace) << " is " << (is_valid_trace<M>(trace) ? "a valid trace." : "not a valid trace.") << std::endl;
}
