# [WIP] graph-gl

Simple graphing calculator in OpenGL.

## Todo list

[x] Remove brackets from formula after calculating the weights (also shift the weights char[])  
[x] Drag to move in space  
[x] Scroll to scale (buggy)  
[ ] Support plotting multiple grapts so it's more like an actual calculator (add color picking for them as well)  
[x] Reset translation and scale buttons  
[ ] Draw a grid (with numbers?)  
[ ] Adjust projection martix on window resize (have a stashed version, which wouldn't work :/)
[ ] Different (and potentially simpler) parser using reverse polish notation?

### Parser

[x] Mermaid export of the tree  
[ ] Support spaces (remove them like brackets)  
[ ] Support "2x" instead of 2\*x (also similar transformation to removal)  
[ ] Support negative numbers, currently '-' is being interpreted as minus operator, so (0 - 3) is -3 now  
[ ] sin()  
[ ] cos()  
[ ] log()  
[ ] x!  
[ ] |x|

## Todo list

[x] Lines start to disappear when dragging the view too far on the x axis. Make sure that the rendered positions are always the ones, showed in the frame

> I'm sure there's more

## Building

```
mkdir build
cd build
cmake ..
make all
cd ..
./build/app
```

## External dependencies

- [glfw](https://github.com/glfw/glfw)
- [glad](https://github.com/Dav1dde/glad)
- [cimgui](https://github.com/cimgui/cimgui)
- [cglm](https://github.com/recp/cglm)
