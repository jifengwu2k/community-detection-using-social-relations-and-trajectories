#ifndef MIN_ELEMENT_AND_VALUE_HPP
#define MIN_ELEMENT_AND_VALUE_HPP

#include <algorithm>
#include <type_traits>
#include <utility>


template <
    typename ForwardIterator,
    typename ValueOf
> std::pair<
    ForwardIterator,
    typename std::result_of<
        ValueOf(
            decltype(
                *(std::declval<ForwardIterator>())
            )
        )
    >::type
> min_element_and_value(
    ForwardIterator begin,
    ForwardIterator end,
    const ValueOf& value_of
) {
    ForwardIterator min_element = begin;
    typename std::result_of<
    ValueOf(
        decltype(
            *(std::declval<ForwardIterator>())
        )
    )
    >::type min_value = value_of(*begin);

    if (begin != end) {
        for (
            ++begin;
            begin != end;
            ++begin
        ) {
            typename std::result_of<
            ValueOf(
                decltype(
                    *(std::declval<ForwardIterator>())
                )
            )
            >::type value = value_of(*begin);
            if (value < min_value) {
                min_element = begin;
                min_value = value;
            }
        }
    }

    return { min_element, min_value };
}

#endif

