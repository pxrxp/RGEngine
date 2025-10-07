#ifndef NDEBUG

#include "editor.h"
#include "../components/PhysicsComponent.h"
#include "../components/TransformComponent.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>

namespace Util {

void
draw_transform_component_editor(TransformComponent& component, const std::string& header)
{
  std::string id = header + "#Transform";
  ImGui::PushID(id.c_str());
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
  ImGui::PopID();
}

void
draw_physics_component_editor(PhysicsComponent& component, const std::string& header)
{
  std::string id = header + "#Physics";
  ImGui::PushID(id.c_str());
  bool enabled = component.is_enabled();
  if (ImGui::Checkbox("Enabled", &enabled)) {
    if (enabled)
      component.enable();
    else
      component.disable();
  }

  float mass = component.get_mass();
  if (ImGui::DragFloat("Mass", &mass, 0.1f, 0.001f, 1000.0f)) {
    component.set_mass(mass);
  }

  float restitution = component.get_restitution();
  if (ImGui::SliderFloat(
        "Restitution (Bounciness)", &restitution, 0.0f, 1.0f)) {
    component.set_restitution(restitution);
  }

  bool has_gravity = component.has_gravity();
  if (ImGui::Checkbox("Has Gravity", &has_gravity)) {
    component.set_gravity(has_gravity);
  }

  ImGui::SeparatorText("State (Read-Only)");

  glm::vec3 velocity = component.get_velocity();
  ImGui::InputFloat3(
    "Velocity", glm::value_ptr(velocity), "%.3f", ImGuiInputTextFlags_ReadOnly);

  glm::vec3 angular_velocity = component.get_angular_velocity();
  ImGui::InputFloat3("Angular Velocity",
                     glm::value_ptr(angular_velocity),
                     "%.3f",
                     ImGuiInputTextFlags_ReadOnly);

  ImGui::Separator();

  if (ImGui::TreeNode("Actions")) {
    static glm::vec3 force_to_apply(0.0f, 10.0f, 0.0f);
    ImGui::InputFloat3("Force Vector", glm::value_ptr(force_to_apply));
    if (ImGui::Button("Apply Force")) {
      component.apply_force(force_to_apply);
    }

    ImGui::SameLine();

    static glm::vec3 impulse_to_apply(0.0f, 5.0f, 0.0f);
    ImGui::InputFloat3("Impulse Vector", glm::value_ptr(impulse_to_apply));
    if (ImGui::Button("Apply Linear Impulse")) {
      component.apply_linear_impulse(impulse_to_apply);
    }

    ImGui::Spacing();

    if (ImGui::TreeNode("Apply Force at Point")) {
      static glm::vec3 force(10.0f, 0.0f, 0.0f);
      static glm::vec3 center_of_mass(0.0f);
      static glm::vec3 apply_point(0.0f, 1.0f, 0.0f);

      ImGui::InputFloat3("Force##AtPoint", glm::value_ptr(force));
      ImGui::InputFloat3("Center of Mass", glm::value_ptr(center_of_mass));
      ImGui::InputFloat3("Apply Point", glm::value_ptr(apply_point));

      if (ImGui::Button("Apply Torque")) {
        component.apply_force_at_point(force, center_of_mass, apply_point);
      }
      ImGui::TreePop();
    }

    ImGui::Spacing();

    if (ImGui::Button("Clear Forces & Torques")) {
      component.clear_forces();
    }

    ImGui::TreePop();
  }
  ImGui::PopID();
}

}
#endif
