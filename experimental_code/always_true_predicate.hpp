#ifndef ALWAYS_TRUE_PREDICATE_HPP
#define ALWAYS_TRUE_PREDICATE_HPP


struct AlwaysTruePredicate {
    template <typename T> constexpr bool operator()(const T& t) const {
        return true;
    }
};

#endif
