#pragma once

#include <variant>
#include <string>
#include <vector>
#include <ostream>

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

template <typename T, typename... Ts>
std::ostream& operator<<(std::ostream& os, const std::variant<T, Ts...>& v) {
    visit([&os] (auto&& v) { os << v; }, v);
    return os;
}

template <typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    os << "{";
    for (auto& e: v) {
        os << e << ", ";
    }
    return os << "}";
}
