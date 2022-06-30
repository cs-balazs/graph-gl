#define CIMGUI_DEFINE_ENUMS_AND_STRUCTS
#include <cimgui.h>
#include "gui.h"

void gui_update(float colors[4])
{
  ImGui_ImplOpenGL3_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  igNewFrame();

  igBegin("Settings", NULL, ImGuiWindowFlags_None);
  igColorEdit4("Color", colors, ImGuiSliderFlags_None);
  igEnd();
}