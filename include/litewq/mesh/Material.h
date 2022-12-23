#pragma once 

#include "litewq/mesh/Texture.h"
#include "litewq/platform/OpenGL/GLShader.h"

#include <memory>
#include <map>

/// \file Material.h
/// \brief Material is a collection of bound shader and one or multiple textures.


namespace litewq {

struct MTLMaterial;
struct Context;

/// \brief General
class Material {
protected:
    enum MaterialType {
        BlingPhong = 0,
        Count
    };
    constexpr static MaterialType mat_type_tag = Count;


public:
    MaterialType material_type_;
    GLShader *shader;

    template <typename T> bool is() {
        return material_type_ == std::remove_pointer<T>::mat_type_tag;
    }
    template <typename T, 
            typename std::enable_if<std::is_pointer<T>::value, T>::type> 
    T as() {
        if (is<T>())
            return static_cast<T>(this);
        return nullptr;
    }

    Material() = default;
    
    virtual void updateMaterial();
    virtual void deactivateMaterial();

};

/// \brief Bare material with no lighting texture,
/// only for testing bling-phong lighting model.
class PhongMaterial : Material {
protected:
    constexpr static MaterialType mat_type_tag = BlingPhong;
public:
    PhongMaterial(const glm::vec3 &Ka, const glm::vec3 &Kd, const glm::vec3
        &Ks, Texture *map_d, Texture *map_Ks, float decay = 16.0f)
        : Ka_(Ka), Kd_(Kd), Ks_(Ks), diffuse_tex(map_d), spec_tex(map_Ks), decay_(decay) {}
    static PhongMaterial *Create(MTLMaterial *mtl, 
                                Context *context);
    
    void updateBareMaterial();
    void updateMaterial() override;

    glm::vec3 Ka_ = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 Kd_ = glm::vec3(0.8f, 0.8f, 0.8f);
    glm::vec3 Ks_ = glm::vec3(0.5f, 0.5f, 0.5f);
    // diffuse texture and specular texture, maybe null.
    Texture *diffuse_tex = nullptr;
    Texture *spec_tex = nullptr;
    float decay_ = 16.0;
};


} // end namespace litewq