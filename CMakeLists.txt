cmake_minimum_required(VERSION 3.4.1)

# Compiler options
add_compile_options(-Wall)

if(CC_USE_MATH)
    set(SOURCE_MATH
        math/cc_pow2n.c
        math/cc_orientation.c
        math/cc_quaternion.c
        math/cc_plane.c
        math/cc_fplane.c
        math/cc_ray.c
        math/cc_sphere.c
        math/cc_stack4f.c
        math/cc_rect12f.c
        math/cc_mat3f.c
        math/cc_mat4f.c
        math/cc_vec2f.c
        math/cc_vec3f.c
        math/cc_vec4f.c)
endif()

# Submodule library
add_library(cc
            STATIC

            # Source
            cc_jobq.c
            cc_list.c
            cc_log.c
            cc_map.c
            cc_memory.c
            cc_multimap.c
            cc_mumurhash3.c
            cc_timestamp.c
            cc_unit.c
            cc_workq.c
            ${SOURCE_MATH})

# Linking
target_link_libraries(cc

                      # NDK libraries
                      log)
