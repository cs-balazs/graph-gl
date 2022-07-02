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

int main(void)
{
  GLFWwindow *window = get_window();
  if (window == NULL) {
    return -1;
  }

  printf("OpenGL version: %s\n", glGetString(GL_VERSION));

  // clang-format off
  float positions[8] = {
    -100.0f,  5.0f,
     100.0f,  5.0f,
     100.0f, -5.0f,
    -100.0f, -5.0f,
  };

  uint32_t indicies[8] = {
    0, 1,
    1, 2,
    2, 3,
    3, 0
  };
  // clang-format on

  uint32_t vertex_array = create_vertex_array();

  uint32_t vertex_buffer =
    create_vertex_buffer(positions, 2 * 4 * sizeof(float));

  BufferLayout layout = { stride: 0, length: 0 };
  BufferLayoutElement positions_element = { GL_FLOAT, 2, GL_FALSE };
  buffer_layout_add_element(&layout, &positions_element);

  vertex_array_add_buffer(vertex_array, vertex_buffer, &layout);

  uint32_t index_buffer = create_index_buffer(indicies, 8);

  mat4 proj;
  glm_ortho(-250.0f, 250.0f, -250.0f, 250.0f, -1.0f, 1.0f, proj);

  uint32_t shader = create_shader("line");
  glUseProgram(shader);

  Formula formula = { 0 };
  ParseTree *tree = malloc(sizeof(ParseTree));

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);

    gui_update();

    igBegin("Settings", NULL, ImGuiWindowFlags_None);
    igInputText("Formula", formula, FORMULA_MAX_LENGTH, ImGuiWindowFlags_None,
                NULL, NULL);

    setUniformMat4f(shader, "u_MVP", &proj);
    glUseProgram(shader);
    glBindVertexArray(vertex_array);
    glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, NULL);

    if (igButton("Render", (ImVec2){ 100, 25 })) {
      Formula formula_copy;
      strcpy(formula_copy, formula);
      tree = parse_formula(formula_copy);
      mermaid_export(tree);
      printf("Tree constructed!!!!!\n");
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