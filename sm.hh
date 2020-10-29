#pragma once

#include <utility>

template <typename M> concept SM =
requires (M::state_t s, M::input_t i) {
    typename M::state_t;
    typename M::input_t;
    typename M::output_t;
    { M::delta(s, i) } -> std::same_as<std::pair<typename M::state_t, typename M::output_t>>;
    M::init;
    requires std::is_same_v<const typename M::state_t, decltype(M::init)>;
};
