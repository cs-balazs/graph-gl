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

float scale_value = 10.0f;

void update_positions(float stride, uint16_t num_of_positions, float *positions,
                      uint32_t *indicies, ParseTree *tree, float translate_x,
                      float translate_y)
{
  float x = -250.0f;
  for (size_t i = 0; i < num_of_positions * 2; i += 2) {
    positions[i] = x; // * (1.0f / scale_value);
    positions[i + 1] =
      tree == NULL ? 1 : compute(tree, x - translate_x) + translate_y;
    x += stride;
  }
  uint64_t index = 0;
  for (size_t i = 0; i < num_of_positions * 2 - 2; i += 2) {
    indicies[i] = index;
    indicies[i + 1] = index + 1;
    index++;
  }
}

void scroll_callback(GLFWwindow *window, double x_offset, double y_offset)
{
  if (y_offset > 0) {
    scale_value += 0.5f;
  } else {
    scale_value -= 0.5f;
  }
  scale_value = 1.0f > scale_value ? 1.0f : scale_value;
  scale_value = 20.0f < scale_value ? 20.0f : scale_value;
}

int main(void)
{
  GLFWwindow *window = get_window();
  if (window == NULL) {
    return -1;
  }

  printf("OpenGL version: %s\n", glGetString(GL_VERSION));

  float stride = 0.2f;
  uint32_t num_of_positions = (uint32_t)(500 / stride);
  float positions[num_of_positions * 2];
  uint32_t indicies[num_of_positions * 2];

  vec2 cursor_pos = { 0, 0 };
  vec2 translate = { 0, 0 };

  uint8_t is_mouse_button_pressed = 0;

  update_positions(stride, num_of_positions, positions, indicies, NULL,
                   translate[0], translate[1]);

  uint32_t vertex_array = create_vertex_array();

  uint32_t shader = create_shader("line");
  glUseProgram(shader);

  Formula formula = { 0 };
  ParseTree *tree = malloc(sizeof(ParseTree));
  tree = NULL;

  uint32_t vertex_buffer = 0;
  uint32_t index_buffer = 0;

  uint8_t is_middle_mouse_button_pressed = 0;

  double x = 0;
  double y = 0;

  glfwSetScrollCallback(window, scroll_callback);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);

    gui_update();

    glfwGetCursorPos(window, &x, &y);

    if (is_mouse_button_pressed) {
      translate[0] -= ((cursor_pos[0] - x) / scale_value);
      translate[1] += ((cursor_pos[1] - y) / scale_value);
    }
    cursor_pos[0] = x;
    cursor_pos[1] = y;

    is_mouse_button_pressed =
      glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE);

    mat4 mvp;
    mat4 proj;
    glm_ortho(-250.0f, 250.0f, -250.0f, 250.0f, -1.0f, 1.0f, proj);
    mat4 model = GLM_MAT4_IDENTITY_INIT;
    glm_scale(model, (vec3){ scale_value, scale_value, 0.0f });
    glm_mat4_mul(proj, model, mvp);

    igBegin("Settings", NULL, ImGuiWindowFlags_None);
    igInputText("Formula", formula, FORMULA_MAX_LENGTH, ImGuiWindowFlags_None,
                NULL, NULL);

    setUniformMat4f(shader, "u_MVP", &mvp);
    glUseProgram(shader);
    glBindVertexArray(vertex_array);

    vertex_buffer =
      create_vertex_buffer(positions, 2 * num_of_positions * sizeof(float));

    BufferLayout layout = { stride: 0, length: 0 };
    BufferLayoutElement positions_element = { GL_FLOAT, 2, GL_FALSE };
    buffer_layout_add_element(&layout, &positions_element);

    vertex_array_add_buffer(vertex_array, vertex_buffer, &layout);

    index_buffer = create_index_buffer(indicies, num_of_positions * 2);

    glDrawElements(GL_LINES, num_of_positions * 2, GL_UNSIGNED_INT, NULL);

    if (igButton("Render", (ImVec2){ 100, 25 })) {
      Formula formula_copy;
      strcpy(formula_copy, formula);
      tree = parse_formula(formula_copy);
      mermaid_export(tree);
    }

    update_positions(stride, num_of_positions, positions, indicies, tree,
                     translate[0], translate[1]);

    printf("( ( %lf, %lf ), ( %lf, %lf ) )\n", positions[0], positions[1],
           positions[num_of_positions * 2 - 2],
           positions[num_of_positions * 2 - 1]);

    printf("[ [ %d, %d ], [ %d, %d ] ]\n", indicies[0], indicies[1],
           indicies[num_of_positions * 2 - 2],
           indicies[num_of_positions * 2 - 1]);

    printf("Frame left: %lf\n", -250.0f - translate[0]);

    igEnd();

    gui_render();
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  glDeleteBuffers(1, &vertex_buffer);
  glDeleteBuffers(1, &index_buffer);
  glDeleteProgram(shader);

  gui_terminate();

  glfwTerminate();
  return 0;
}