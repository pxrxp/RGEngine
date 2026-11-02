#include <engine/components/TransformComponent.h>
#include <glm/gtc/quaternion.hpp>
#include <imgui.h>
#include <string>

#ifndef NDEBUG
namespace Util {

void
draw_transform_component_editor(TransformComponent& component,
                                const std::string& header)
{
  (void)header;

  glm::vec3 position = component.get_position();
  if (ImGui::DragFloat3("Position##Transform",
                        &position.x,
                        0.05f,
                        0.0f,
                        0.0f,
                        "%.3f"))
    component.set_position(position);

  glm::vec3 euler_deg =
    glm::degrees(glm::eulerAngles(component.get_rotation()));
  if (ImGui::DragFloat3("Rotation (deg)##Transform",
                        &euler_deg.x,
                        0.5f,
                        0.0f,
                        0.0f,
                        "%.1f"))
    component.set_rotation(glm::quat(glm::radians(euler_deg)));

  glm::vec3 scale = component.get_scale();
  if (ImGui::DragFloat3("Scale##Transform", &scale.x, 0.01f, 0.0f, 0.0f, "%.2f"))
    component.set_scale(scale);
}

}
#endif
