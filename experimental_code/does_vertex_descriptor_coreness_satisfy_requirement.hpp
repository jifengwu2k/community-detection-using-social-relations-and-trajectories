#ifndef DOES_VERTEX_DESCRIPTOR_CORENESS_SATISFY_REQUIREMENT_HPP
#define DOES_VERTEX_DESCRIPTOR_CORENESS_SATISFY_REQUIREMENT_HPP

template <typename VertexDescriptorToCorenessMap, typename DegreeSizeType> struct DoesVertexDescriptorCorenessSatisfyRequirement {
    VertexDescriptorToCorenessMap* vertex_descriptor_to_coreness_map_pointer;
    DegreeSizeType vertex_descriptor_coreness_requirement;

    DoesVertexDescriptorCorenessSatisfyRequirement(): 
        vertex_descriptor_to_coreness_map_pointer(nullptr),
        vertex_descriptor_coreness_requirement(0) { }
    
    DoesVertexDescriptorCorenessSatisfyRequirement(
        VertexDescriptorToCorenessMap* t_vertex_descriptor_to_coreness_map_pointer,
        DegreeSizeType t_vertex_descriptor_coreness_requirement
    ):
        vertex_descriptor_to_coreness_map_pointer(t_vertex_descriptor_to_coreness_map_pointer),
        vertex_descriptor_coreness_requirement(t_vertex_descriptor_coreness_requirement) { }
    
    template <typename VertexDescriptor> bool operator()(const VertexDescriptor& vertex_descriptor) const {
        return vertex_descriptor_to_coreness_map_pointer->at(vertex_descriptor) >= vertex_descriptor_coreness_requirement;
    }
};

#endif

