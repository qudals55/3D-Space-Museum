#pragma once
#include <string>
#include <vector>
#include <map>
#include <cmath>
#include "SOIL.h"

#include "vec.hpp"

struct Group
{
public:
  Group(const std::string& name);

public:
  std::string               m_name;

  std::vector<kmuvcl::math::vec3f>    m_vertices;
  std::vector<kmuvcl::math::vec3f>    m_normals;
  std::vector<kmuvcl::math::vec2f>    m_texcoords;
  std::string               m_mtl_name;
};

struct Material
{
public:
  Material();
  Material(const std::string& name,
    kmuvcl::math::vec4f& ambient, kmuvcl::math::vec4f& diffuse, kmuvcl::math::vec4f& specular, float& shininess);

public:
  std::string     m_name;

  kmuvcl::math::vec4f       m_ambient;
  kmuvcl::math::vec4f       m_diffuse;
  kmuvcl::math::vec4f       m_specular;
  float           m_shininess;

};

class Object
{
public:
  GLuint    textureid;
  GLfloat offsetX, offsetY, offsetZ;

  Object()
   :position_(0,0,0),scale_(1.0f), rotate_(0.0f), min_x(1), min_y(1), min_z(1), max_x(-1), max_y(-1), max_z(-1)
   {}

 void set_value(const kmuvcl::math::vec3f& _position, const GLfloat _scale, const GLfloat _rotate)
 {
      position_=_position;
      scale_=_scale;
      rotate_=_rotate;
      set_offset();
    /*  std::cout << "scale_ = " << scale_ << std::endl;
      std::cout << "min_x = " << min_x << std::endl;
      std::cout << "min_y = " << min_y << std::endl;
      std::cout << "min_z = " << min_z << std::endl;
      std::cout << "max_x = " << max_x << std::endl;
      std::cout << "max_y = " << max_y << std::endl;
      std::cout << "max_z = " << max_z << std::endl;
      std::cout << "offsetX = " << offsetX << std::endl;
      std::cout << "offsetY = " << offsetY << std::endl;
      std::cout << "offsetZ = " << offsetZ << std::endl;*/

  }
  void set_offset()
  {
    offsetX =  (max_x-min_x)*scale_ + 0.5f;
    offsetY = (max_y-min_y)*scale_ + 0.5f;
    offsetZ =  (max_z-min_z)*scale_ + 0.5f;
  }

const kmuvcl::math::vec3f position() const
{
  return  kmuvcl::math::vec3f(position_(0),
                 position_(1),
                 position_(2));
}
const kmuvcl::math::vec3f m_position() const
{
  return  kmuvcl::math::vec3f(-position_(0),
                 -position_(1),
                 -position_(2));
}

const GLfloat scale() const{ return scale_;}
const GLfloat rotate() const{return rotate_;}

void set_position(const kmuvcl::math::vec3f& _position){
  position_=_position;
}
void X(GLfloat add){
  if(position_(0)<300&& position_(0)>-300){
    position_(0)+=add;
  }
  else {
    if(position_(0)>-150) position_(0)-=300;
    else position_(0)+=700;
  }
}
void Y(GLfloat add){
  position_(1)+=add;
}
void Z(GLfloat add){
  if(position_(2)<300 && position_(2)>-300){
  position_(2)+=add;}

  else {
    if(position_(2)>150) position_(2)-=300;
    else position_(2)+=700;
  }
}
void R(GLfloat add){
  rotate_ += add;
}
void S(GLfloat a){
  scale_=a;
}

  void draw(int loc_a_vertex, int loc_a_normal,
    int loc_u_material_ambient, int loc_u_material_diffuse,
    int loc_u_material_specular, int loc_u_material_shininess);

 void draw(int loc_a_vertex, int loc_a_normal,  int loc_a_texcoord,
    int loc_u_material_ambient, int loc_u_material_diffuse,
    int loc_u_material_specular, int loc_u_material_shininess);

  void print();

void load_texture(const std::string& filename);
 bool load_simple_obj(const std::string& filename);
 bool load_simple_mtl(const std::string& filename);

private:
  kmuvcl::math::vec3f position_;

  GLfloat   scale_, rotate_;
  float min_x, min_y, min_z;
  float max_x, max_y, max_z;

  std::string PATH;
  std::vector<Group>              m_groups;
  std::map<std::string, Material> m_materials;
};
