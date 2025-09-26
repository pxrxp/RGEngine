#ifndef NDEBUG

#include "editor.h"
#include "../components/TransformComponent.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace Util {

void
draw_transform_component_editor(TransformComponent& component,
                                const char* label)
{
  if (!ImGui::CollapsingHeader(label)) {
    return;
  }

  glm::vec3 position = component.get_position();
  if (ImGui::DragFloat3("Position", glm::value_ptr(position), 0.1f)) {
    component.set_position(position);
  }

  glm::quat orientation = component.get_rotation();
  glm::vec3 euler_angles_rad = glm::eulerAngles(orientation);
  glm::vec3 euler_angles_deg = glm::degrees(euler_angles_rad);

  if (ImGui::DragFloat3("Rotation (Degrees)",
                        glm::value_ptr(euler_angles_deg))) {
    glm::vec3 new_euler_rad = glm::radians(euler_angles_deg);
    component.set_rotation(glm::quat(new_euler_rad));
  }

  glm::vec3 current_scale = component.get_scale();
  if (ImGui::DragFloat3("Scale", glm::value_ptr(current_scale), 0.05f)) {
    component.set_scale(current_scale);
  }

  ImGui::Separator();

  if (ImGui::TreeNode("Actions")) {
    static glm::vec3 translate_offset(0.0f);
    ImGui::InputFloat3("Translate Offset", glm::value_ptr(translate_offset));
    if (ImGui::Button("Translate")) {
      component.translate(translate_offset);
    }

    ImGui::Spacing();

    static glm::vec3 rotate_axis(0.0f, 1.0f, 0.0f);
    static float rotate_angle_deg = 45.0f;
    ImGui::InputFloat3("Rotation Axis", glm::value_ptr(rotate_axis));
    ImGui::SliderFloat("Rotation Angle", &rotate_angle_deg, -180.0f, 180.0f);
    if (ImGui::Button("Rotate")) {
      if (glm::length(rotate_axis) > 0.001f) {
        component.rotate(rotate_axis, glm::radians(rotate_angle_deg));
      }
    }

    ImGui::Spacing();

    static glm::vec3 scale_factors(1.0f);
    ImGui::InputFloat3("Scale Factors", glm::value_ptr(scale_factors));
    if (ImGui::Button("Scale")) {
      component.scale(scale_factors);
    }

    ImGui::TreePop();
  }
}

}
#endif
