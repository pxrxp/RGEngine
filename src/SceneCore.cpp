#include "Scene.h"
#include "Material.h"
#include "Renderer.h"
#include "SceneObject.h"
#include "ShaderProgram.h"

void
Scene::init()
{
  SceneObjectStrongPtr player = std::make_shared<SceneObject>();
  player->get_physics_component().set_gravity(false);
  player->get_local_transform().translate(AXIS_Y);
  m_scene_ptr->add_child(player);

  m_app_cache.get_camera().setup(player);
}

void
Scene::set_view_matrix(RenderPacketStrongPtr render_packet,
                       const char* view_uniform_name,
                       const glm::mat4& view_matrix)
{
  glm::mat4 V = (view_matrix != glm::mat4(1.f))
                  ? view_matrix
                  : m_app_cache.get_camera().get_view_matrix();

  render_packet->material->get_shader_program()->set_uniform(view_uniform_name,
                                                             V);
}

void
Scene::set_projection_matrix(RenderPacketStrongPtr render_packet,
                             const char* proj_uniform_name,
                             const glm::mat4& proj_matrix)
{
  glm::mat4 P = (proj_matrix != glm::mat4(1.f))
                  ? proj_matrix
                  : m_app_cache.get_camera().get_projection_matrix();

  render_packet->material->get_shader_program()->set_uniform(proj_uniform_name,
                                                             P);
}

void
Scene::set_model_matrix(RenderPacketStrongPtr render_packet,
                        const glm::mat4& model_matrix,
                        const char* uniform_name)
{
  render_packet->material->get_shader_program()->set_uniform(uniform_name,
                                                             model_matrix);
}

void
Scene::submit_to_renderer()
{
  m_app_cache.get_renderer().submit(get_scene_ptr());
}

SceneObjectStrongPtr
Scene::get_scene_ptr() const
{
  return m_scene_ptr;
}
