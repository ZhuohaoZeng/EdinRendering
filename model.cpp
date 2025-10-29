#include "model.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

model::model(const std::string filename)
{
    std::ifstream in;
    in.open(filename);
    if(in.fail()) return;
    std::string line;
    while(!in.eof())
    {
        std::getline(in, line);
        std::istringstream iss(line);
        char trash;
        if(!line.compare(0, 2, "v "))
        {
            iss >> trash;
            point3 v{0, 0, 0};
            for(int i : {0, 1, 2}) iss >> v[i];
            verts.push_back(v);
        }
        else if(!line.compare(0, 3, "vn "))
        {
            iss >> trash >> trash;
            point3 v{0, 0, 0};
            for(int i : {0, 1, 2}) iss >> v[i];
            norms.push_back(v);
        }
        else if(!line.compare(0, 3, "vt "))
        {
            iss >> trash >> trash;
            point2 v{};
            for(int i : {0, 1}) iss >> v[i];
            tex.push_back({v[0], 1 - v[1]});
        }
        else if(!line.compare(0, 2, "f "))
        {
            int v, t, n, count{0};
            iss >> trash;
            while(iss >> v >> trash >> t >> trash >> n)
            {
                facetVerts.push_back(--v);
                faceTexs.push_back(--t);
                faceNorms.push_back(--n);
                ++count;
            }
            if (3!=count) 
            {
                std::cerr << "Error: the obj file is supposed to be triangulated" << std::endl;
                return;
            }
        }
    }
    std::cout << "verts: " <<verts.size() << std::endl;
    std::cout << "norms: " <<norms.size() << std::endl;
    std::cout << "tex: " <<tex.size() << std::endl;
    std::cout << "faces: " <<facetVerts.size() << std::endl;
    std::cout << "faces norms: " <<faceNorms.size() << std::endl;
    std::cout << "faces tex: " << faceTexs.size() << std::endl;
    // auto load_texture = [&filename](const std::string suffix, TGAImage &img)
    // {
    //     size_t dot = filename.find_last_of('.');
    //     if (dot==std::string::npos) return;
    //     std::string texfile = filename.substr(0, dot) + suffix;
    //     std::cerr << "texture file " << texfile << " loading " << (img.read_tga_file(texfile.c_str()) ? "ok" : "failed") << std::endl;
    // };
    // load_texture("_nm_tangent.tga", normalMap);
    // load_texture("_diffuse.tga", diffuseMap);
    // load_texture("_spec.tga", specularMap);
}
int model::nverts() const{return verts.size();}
int model::nfaces() const{return facetVerts.size() / 3;}
int model::nnorms() const {return norms.size();}
int model::nfaceNorms() const {return faceNorms.size() / 3;}
point3 model::vert(const int i) const {return verts[i];}
point3 model::vert(const int iface, const int nthvert) const {return verts[facetVerts[iface*3+nthvert]];}
vec3 model::normal(const int iface, const int nthvert) const {return norms[faceNorms[iface*3+nthvert]];}
vec3 model::normal(const vec2 &uv) const
{
   TGAColor c = normalMap.get(uv[0] * normalMap.width(), uv[1] * normalMap.height());
   return unit_vector(vec3{(double)c[2],(double)c[1],(double)c[0]}*2./255. - vec3{1,1,1});
}
vec2 model::uv(const int iface, const int nthvert) const {return tex[faceTexs[iface*3+nthvert]];}
const TGAImage& model::diffuse() const {return diffuseMap;}
const TGAImage& model::specular() const {return specularMap;}