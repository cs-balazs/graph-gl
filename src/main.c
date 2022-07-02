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

#define AUTO_EXPORT_MERMAID

void update_positions(float stride, uint16_t num_of_positions, float *positions,
                      uint32_t *indicies, ParseTree *tree)
{
  int16_t x = -250;
  for (size_t i = 0; i < num_of_positions * 2; i += 2) {
    positions[i] = x;
    positions[i + 1] = tree == NULL ? 1 : compute(tree, x);
    x += stride;
  }
  uint16_t index = 0;
  for (size_t i = 0; i < num_of_positions * 2 - 2; i += 2) {
    indicies[i] = index;
    indicies[i + 1] = index + 1;
    index++;
  }
}

int main(void)
{
  GLFWwindow *window = get_window();
  if (window == NULL) {
    return -1;
  }

  printf("OpenGL version: %s\n", glGetString(GL_VERSION));

  float stride = 1.0f;

  uint16_t num_of_positions = (uint16_t)(500 / stride);

  float positions[num_of_positions * 2];
  /* int16_t x = -250;
  for (size_t i = 0; i < num_of_positions * 2; i += 2) {
    positions[i] = x;
    positions[i + 1] = 1;
    x += stride;
  } */

  uint32_t indicies[num_of_positions * 2];
  /* uint16_t index = 0;
  for (size_t i = 0; i < num_of_positions * 2; i += 2) {
    indicies[i] = index;
    indicies[i + 1] = index + 1;
    index++;
  } */
  update_positions(stride, num_of_positions, positions, indicies, NULL);

  uint32_t vertex_array = create_vertex_array();

  mat4 proj;
  glm_ortho(-250.0f, 250.0f, -250.0f, 250.0f, -1.0f, 1.0f, proj);

  uint32_t shader = create_shader("line");
  glUseProgram(shader);

  Formula formula = { 0 };
  ParseTree *tree = malloc(sizeof(ParseTree));

  uint32_t vertex_buffer = 0;
  uint32_t index_buffer = 0;

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);

    gui_update();

    igBegin("Settings", NULL, ImGuiWindowFlags_None);
    igInputText("Formula", formula, FORMULA_MAX_LENGTH, ImGuiWindowFlags_None,
                NULL, NULL);

    setUniformMat4f(shader, "u_MVP", &proj);
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
      update_positions(stride, num_of_positions, positions, indicies, tree);
#ifdef AUTO_EXPORT_MERMAID
      mermaid_export(tree);
#endif
    }
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