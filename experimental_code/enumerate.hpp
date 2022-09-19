#ifndef ENUMERATE_HPP
#define ENUMERATE_HPP


template <typename Iterator, typename Callback> void enumerate(
    Iterator begin,
    const Iterator end,
    const Callback& callback
) {
    size_t index = 0;
    for (
        ;
        begin != end;
        ++begin, ++index
    ) {
        callback(index, *begin);
    }
}

#endif

