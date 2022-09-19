#ifndef WRITE_VECTOR_HPP
#define WRITE_VECTOR_HPP

template <typename T> std::ostream& operator<<(std::ostream& ostream, const std::vector<T>& vector) {
    ostream << '[';
    
    auto it = vector.cbegin(), end = vector.cend();
    if (it != end) {
        // write first member
        ostream << *it;
        
        // write remaining members
        for (
            ++it;
            it != end;
            ++it
        ) {
            ostream << ',' << *it;
        }
    }
    
    ostream << ']';
    return ostream;
}

#endif
