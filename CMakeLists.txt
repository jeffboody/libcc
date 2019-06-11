cmake_minimum_required(VERSION 3.4.1)

# Compiler options
add_compile_options(-Wall)

# Submodule library
add_library(cc
            STATIC

            # Source
            cc_list.c
            cc_log.c
            cc_map.c
            cc_memory.c
            cc_multimap.c
            cc_timestamp.c
            cc_unit.c
            cc_workq.c)

# Linking
target_link_libraries(cc

                      # NDK libraries
                      log)
