#pragma once
#include "vec3.h"
#include "geometry.h"
#include "tgaimage.h"

    class model {
    private:
        std::vector<point3> verts{};
        std::vector<vec3> norms{};
        std::vector<point2> tex{};
        std::vector<int> facetVerts{};
        std::vector<int> faceNorms{};
        std::vector<int> faceTexs{};
        TGAImage normalMap{};
        TGAImage diffuseMap{};
        TGAImage specularMap{};

    public:
        model(const std::string filename);
        int nverts() const; // number of vertices
        int nnorms() const; // number of normals
        int nfaces() const; // number of triangles
        int nfaceNorms() const;
        point3 vert(const int i) const;                          // 0 <= i < nverts()
        point3 vert(const int iface, const int nthvert) const;
        vec3 normal(const int iface, const int nthvert) const;
        vec3 normal(const point2 &uv) const;
        point2 uv(const int iface, const int nthvert) const;
        const TGAImage& diffuse() const;
        const TGAImage& specular() const;
    };
