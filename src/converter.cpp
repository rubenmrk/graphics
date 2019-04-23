#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>
#include <glm/glm.hpp>

#include <iostream>
#include <algorithm>
#include <unordered_map>
#include "application/mesh.hpp"

using namespace std;

static uint32_t rotl(const uint32_t value, int shift) 
{
    if ((shift &= sizeof(value)*8 - 1) == 0)
      return value;
    return (value << shift) | (value >> (sizeof(value)*8 - shift));
}

namespace std
{
    // SHA-1 like function on game::meshfile::vertexData
    template<>
    struct hash<game::meshfile::vertexData> 
    {
        uint64_t operator()(const game::meshfile::vertexData& v) const
        {
            static_assert(sizeof(v) % 4 == 0 && sizeof(v) <= 512-96);
            
            // Prep the data stream
            uint32_t block[80] = {};
            std::copy(reinterpret_cast<const char*>(&v), reinterpret_cast<const char*>(&v)+sizeof(v), block);
            block[sizeof(v)/4] = 0x80;
            block[14] = sizeof(v);

            // Initialize the variables
            uint32_t h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE, h3 = 0x10325476, h4 = 0xC3D2E1F0;
            for (auto i = 16; i < 80; ++i) {
                block[i] = rotl(block[i-3] ^ block[i-8] ^ block[i-14] ^ block[i-16], 1);
            }

            // Main algorithm
            uint32_t a = h0, b = h1, c = h2, d = h3, e = h4, f, k;
            for (auto i = 0; i < 80; ++i) {
                if (i < 20) {
                    f = (b & c) | ((~b) & d);
                    k = 0x5A827999;
                }
                else if (i < 40) {
                    f = b ^ c ^ d;
                    k = 0x6ED9EBA1;
                }
                else if (i < 60) {
                    f = (b & c) | (b & d) | (c & d);
                    k = 0x8F1BBCDC;
                }
                else if (i < 80) {
                    f = b ^ c ^ d;
                    k = 0xCA62C1D6;
                }

                uint32_t tmp = rotl(a, 5) + f + e + k + block[i];
                e = d;
                d = c;
                c = rotl(b, 30);
                b = a;
                a = tmp;
            }

            h0 = h0 + a;
            h1 = h1 + b;
            h2 = h2 + c;
            h3 = h3 + d;
            h4 = h4 + e;

            // Only line that differs from the original algorithm (64 bit output should be fine for input that is only 32 bytes)
            return (static_cast<uint64_t>(h0 ^ h4 ) << 32) | static_cast<uint64_t>(((h1 ^ h2) << 15) ^ h3);
        }
    };
}

int main(int argc, char *argv[])
{
    ios_base::sync_with_stdio(false);

    if (argc < 2) {
        cout << "No input file given" << endl;
        return 0;
    }
    else if (argc > 2) {
        cout << "Unexpected input" << endl;
        return 0;
    }

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string warn, err;

    if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, argv[1])) {
        cout << "Error loading file: " << err << endl;
        return 0;
    }

    std::unordered_map<game::meshfile::vertexData, unsigned int> outhash;
    std::vector<game::meshfile::vertexData> outdata;
    std::vector<unsigned int> outindices;

    for (const auto& shape : shapes) {
        for (const auto& index : shape.mesh.indices) {
            game::meshfile::vertexData tmp;

            tmp.position = {
                attrib.vertices[3 * index.vertex_index + 0],
                attrib.vertices[3 * index.vertex_index + 1],
                attrib.vertices[3 * index.vertex_index + 2]
            };

            tmp.texcoord = {
                attrib.texcoords[2 * index.texcoord_index + 0],
                1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
            };

			tmp.normal = {
				attrib.normals[3 * index.normal_index + 0],
				attrib.normals[3 * index.normal_index + 1],
				attrib.normals[3 * index.normal_index + 2]
			};

            if (outhash.count(tmp) == 0) {
                outhash[tmp] = outhash.size();
                outdata.push_back(tmp);
            }
            outindices.push_back(outhash[tmp]);
        }
    }

    std::string name(argv[1]);
    std::string outname = name.substr(0, name.find_last_of('.')).append(".msh");
    
    game::meshfile mf;
    mf.head.dataCount = outdata.size();
    mf.head.indexCount = outindices.size();
    mf.data = outdata.data();
    mf.indices = outindices.data();

    mf.toFile(outname);

    mf.data = nullptr;
    mf.indices = nullptr;

    return 0;
}