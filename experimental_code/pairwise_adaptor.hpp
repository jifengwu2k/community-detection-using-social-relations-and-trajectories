#ifndef PAIRWISE_ADAPTOR_HPP
#define PAIRWISE_ADAPTOR_HPP


#include <utility>

template <typename AdaptedFunction, typename T> struct PairwiseAdaptor {
    const AdaptedFunction adapted_function;
    T first;
    T second;
    bool has_received_first;
    
    PairwiseAdaptor(const AdaptedFunction& t_adapted_function):
        adapted_function(t_adapted_function),
        has_received_first(false) { }
    
    void operator()(const T& t) {
        if (has_received_first) {
            second = t;
            adapted_function(first, second);
            first = std::move(second);
        }
        else {
            first = t;
            has_received_first = true;
        }
    }
};


/*
#include <stdio.h>


auto f = [](int a, int b) { printf("%d %d\n", a, b); };

int main() {
    PairwiseAdaptor<decltype(f), int> pa(f);
    while (true) {
        int i;
        scanf("%d", &i);
        pa(i);
    }
    
    return 0;
}
*/

#endif

