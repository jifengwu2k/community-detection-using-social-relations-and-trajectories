#ifndef IS_EDGE_DESCRIPTOR_IN_EDGE_SET_HPP
#define IS_EDGE_DESCRIPTOR_IN_EDGE_SET_HPP

template <typename EdgeSet> struct IsEdgeDescriptorInEdgeSet {
    EdgeSet* edge_set_pointer;
    
    IsEdgeDescriptorInEdgeSet(): edge_set_pointer(nullptr) { }
    IsEdgeDescriptorInEdgeSet(EdgeSet* t_edge_set_pointer): edge_set_pointer(t_edge_set_pointer) { }
    
    template <typename EdgeDescriptor> bool operator()(const EdgeDescriptor& edge_descriptor) const {
        return edge_set_pointer->count(edge_descriptor);
    }
};

# endif

