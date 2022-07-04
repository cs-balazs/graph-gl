#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <cglm/cglm.h>
#include "window.h"
#include "gui.h"
#include "gfx.h"
#include "parse_tree.h"

void update_positions(float stride, uint16_t num_of_positions, float *positions,
                      uint32_t *indicies, ParseTree *tree, float translate_x,
                      float translate_y)
{
  float x = -250.0f;
  for (size_t i = 0; i < num_of_positions * 2; i += 2) {
    positions[i] = x;
    positions[i + 1] =
      tree == NULL ? translate_y : compute(tree, x - translate_x) + translate_y;
    x += stride;
  }
  uint64_t index = 0;
  for (size_t i = 0; i < num_of_positions * 2; i += 2) {
    indicies[i] = index;
    // Condition: Last index is connected to itself
    indicies[i + 1] = index == (num_of_positions - 1) ? index : index + 1;
    index++;
  }
}

float scale_value = 10.0f;
void scroll_callback(GLFWwindow *window, double x_offset, double y_offset)
{
  if (y_offset > 0) {
    scale_value += .5f;
  } else {
    scale_value -= .5f;
  }
  scale_value = 1.0f > scale_value ? 1.0f : scale_value;
  scale_value = 20.0f < scale_value ? 20.0f : scale_value;
}

int main(int argc, char *argv[])
{
  GLFWwindow *window = get_window();
  if (window == NULL) {
    return -1;
  }

  uint8_t should_export_mermaid =
    argc > 1 &&
    (strcmp(argv[1], "-e") == 0 || strcmp(argv[1], "--export-on-render") == 0);

  float stride = 0.2f;
  uint32_t num_of_positions = (uint32_t)(500 / stride);
  float positions[num_of_positions * 2];
  uint32_t indicies[num_of_positions * 2];
  vec2 cursor_pos = { 0, 0 };
  vec2 translate = { 0, 0 };
  double new_cursor_pos[2] = { .0f, .0f };

  // OpenGL related setup
  uint32_t vertex_array = create_vertex_array();
  uint32_t shader = create_shader("line");
  glUseProgram(shader);
  uint32_t vertex_buffer = 0;
  uint32_t index_buffer = 0;
  BufferLayout layout = { stride: 0, length: 0 };
  BufferLayoutElement positions_element = { GL_FLOAT, 2, GL_FALSE };
  buffer_layout_add_element(&layout, &positions_element);
  mat4 mvp;
  mat4 proj;

  // Variables for parsing the input formula;
  Formula formula = { 0 };
  ParseTree *tree = malloc(sizeof(ParseTree));
  tree = NULL;

  // Maybe this should be in get_window(), and scale_value should be exported somehow?
  glfwSetScrollCallback(window, scroll_callback);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);
    gui_update();

    // Update translation data according to cursor position while middle mouse button is pressed
    glfwGetCursorPos(window, &new_cursor_pos[0], &new_cursor_pos[1]);
    uint8_t is_middle_mouse_button_predded =
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);
    if (is_middle_mouse_button_predded) {
      translate[0] -= ((cursor_pos[0] - new_cursor_pos[0]) / scale_value);
      translate[1] += ((cursor_pos[1] - new_cursor_pos[1]) / scale_value);
    }
    cursor_pos[0] = new_cursor_pos[0];
    cursor_pos[1] = new_cursor_pos[1];

    // Update MVP matrix, and set the corresponding uniform
    glm_ortho(-250.0f, 250.0f, -250.0f, 250.0f, -1.0f, 1.0f, proj);
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_scale(model, (vec3){ scale_value, scale_value, 1.0f });
    glm_mat4_mul(proj, model, mvp);
    setUniformMat4f(shader, "u_MVP", &mvp);

    // Set vertex & index buffer & fire draw call
    glBindVertexArray(vertex_array);
    vertex_buffer =
      create_vertex_buffer(positions, 2 * num_of_positions * sizeof(float));
    vertex_array_add_buffer(vertex_array, vertex_buffer, &layout);
    index_buffer = create_index_buffer(indicies, num_of_positions * 2);
    glDrawElements(GL_LINES, num_of_positions * 2, GL_UNSIGNED_INT, NULL);

    // cimgui updates
    igBegin("Settings", NULL, ImGuiWindowFlags_None);
    igInputText("Formula", formula, FORMULA_MAX_LENGTH, ImGuiWindowFlags_None,
                NULL, NULL);
    igSliderFloat("Stride", &stride, .15f, 5.0f, "%f", ImGuiActivateFlags_None);
    if (igButton("Render", (ImVec2){ 100, 25 })) {
      Formula formula_copy;
      strcpy(formula_copy, formula);
      tree = parse_formula(formula_copy);
      if (should_export_mermaid) {
        mermaid_export(tree);
      }
    }
    igSameLine(0, 10);
    if (igButton("Reset view", (ImVec2){ 100, 25 })) {
      translate[0] = .0f;
      translate[1] = .0f;
    }
    igSameLine(0, 10);
    if (igButton("Reset scale", (ImVec2){ 100, 25 })) {
      scale_value = 10.0f;
    }
    igEnd();

    // Finally update positions with all the new data
    update_positions(stride, num_of_positions, positions, indicies, tree,
                     translate[0], translate[1]);

    gui_render();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  free_parse_tree(tree);
  glDeleteBuffers(1, &vertex_buffer);
  glDeleteBuffers(1, &index_buffer);
  glDeleteProgram(shader);
  gui_terminate();
  glfwTerminate();
  return 0;
}