#pragma once

#include <vector>
#include <utility>
#include <variant>

using id = int;

template <SM M>
using history = std::vector<std::pair<std::variant<typename M::input_t, typename M::output_t>, id>>;

template <SM M>
std::ostream& operator<<(std::ostream& os, const history<M>& history) {
    os << "history{";
    for (auto& [e, id]: history) {
        os << "(" << e << ", id=" << id << "), ";
    }
    return os << "}";
}
