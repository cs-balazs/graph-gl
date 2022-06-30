#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <cglm/cglm.h>
#include "window.h"
#include "gui.h"
#include "gfx.h"

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

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

  float colors[4] = { 0.0f, 0.0f, 0.0f, 1.0f };

  setUniform4f(shader, "u_Color", colors[0], colors[1], colors[2], colors[3]);

  glBindVertexArray(0);
  glUseProgram(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  while (!glfwWindowShouldClose(window)) {
    glClear(GL_COLOR_BUFFER_BIT);

    gui_update(colors);

    setUniformMat4f(shader, "u_MVP", &proj);

    glUseProgram(shader);
    setUniform4f(shader, "u_Color", colors[0], colors[1], colors[2], colors[3]);

    glBindVertexArray(vertex_array);

    glDrawElements(GL_LINES, 8, GL_UNSIGNED_INT, NULL);

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