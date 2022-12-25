#include "litewq/mesh/Material.h"
#include "litewq/surface/WavefrontOBJ.h"
#include "litewq/utils/Loader.h"

using namespace litewq;

PhongMaterial *PhongMaterial::Create(MTLMaterial *mtl, 
                            Context *context)
{
    Texture *Diffuse = nullptr;
    Texture *Specular = nullptr;
    if (mtl->tex_map_[int(MTLTexMapType::Color)].isValid()) {
        MTLTexMap &DiffuseMap = mtl->tex_map_[int(MTLTexMapType::Color)];
        Diffuse = new Texture(0);
        Diffuse->LoadTexture(Loader::getFileFromPath(DiffuseMap.image_path_, DiffuseMap.mtl_dir_path));
    }
    if (mtl->tex_map_[int(MTLTexMapType::Specular)].isValid()) {
        MTLTexMap &SpecMap = mtl->tex_map_[int(MTLTexMapType::Specular)];
        Specular = new Texture(1);
        Specular->LoadTexture(Loader::getFileFromPath(SpecMap.image_path_, SpecMap.mtl_dir_path));
    }
    return new PhongMaterial(mtl->Ka_, mtl->Kd_, mtl->Ks_, Diffuse, Specular, mtl->Ns_);
}


void PhongMaterial::updateMaterial(GLShader *shader) {
    shader->Bind();
    if (diffuse_tex != nullptr) {
        shader->updateUniformInt("material.Kd", diffuse_tex->texture_unit_id_);
        diffuse_tex->BindTexture();
    } else {
        shader->updateUniformFloat3("material.Kd", Kd_);
    }
    if (spec_tex != nullptr) {
        shader->updateUniformInt("material.Ks", spec_tex->texture_unit_id_);
        spec_tex->BindTexture();
    } else {
        shader->updateUniformFloat3("material.Ks", Ks_);
    }
    shader->updateUniformFloat("material.highlight_decay", decay_);
}