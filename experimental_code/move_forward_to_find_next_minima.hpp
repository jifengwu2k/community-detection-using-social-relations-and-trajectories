#ifndef MOVE_FORWARD_TO_FIND_NEXT_MINIMA_HPP
#define MOVE_FORWARD_TO_FIND_NEXT_MINIMA_HPP


template <typename Iterator, typename GetValue> inline Iterator move_forward_to_find_next_minima(
    const Iterator begin,
    const Iterator end,
    const GetValue& get_value
) {
    Iterator current_position = begin;
    if (current_position == end) return current_position;
    auto current_value = get_value(current_position);
    
    while (true) {
        Iterator next_position = current_position; ++next_position;
        if (next_position == end) return current_position;
        auto next_value = get_value(next_position);
        if (next_value > current_value) return current_position;
        current_position = next_position; current_value = next_value;
    }
}


/*
#include <math.h>
#include <stdio.h>


int main() {
    int numbers[] = {1, 3, 6, 7, 9};

    int* position = numbers;
    int* end = position + sizeof(numbers) / sizeof(int);
    
    while (true) {
        printf("enter target: ");
        int target;
        scanf("%d", &target);
        
        position = move_forward_to_find_next_minima(
            position,
            end,
            [target](const int* p) { return abs(target - *p); }
        );

        printf("%d matches %d\n", *position, target);
    }

    return 0;
}
*/

#endif

