#pragma once
#include <cppgl.h>
#include "../drawcollection.h"

namespace util {

    inline void print_mesh_info(cppgl::Drawelement prototype) {
        std::cout << prototype->name << "\n";
        std::cout << "textures:\n";
        for(auto tex : prototype->mesh->material->texture_map) {
            std::cout << "  " << tex.first << ": size:(" << tex.second->w << "," << tex.second->h << ")\n";
        }

        std::cout << "\n";
    }

    inline void print_mesh_info(DrawCollection collection) {
        for(auto &prot : collection->prototype) {
            print_mesh_info(prot);
        }
    }
};