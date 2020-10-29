#pragma once

#include "sm.hh"

template<SM M>
auto make_box() {
    return [s = M::init] (M::input_t i) mutable {
        auto [new_s, o] = M::delta(std::move(s), std::move(i));
        s = std::move(new_s);
        return o;
    };
}
