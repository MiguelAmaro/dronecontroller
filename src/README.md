# LOGS & RESOURCES

### RESOURCES

**General Win32**
* https://github.com/grassator/win32-window-custom-titlebar

**DirectX**
* Beginning DirectX 11 Game Programming
* Physics Modeling for Game Programmers (DirectX 9)
* https://www.3dgep.com/introduction-to-directx-11/

**Orthographic Projection**
* https://docs.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixorthorh?redirectedfrom=MSDN
* https://blog.demofox.org/2017/03/31/orthogonal-projection-matrix-plainly-explained/
* https://docs.microsoft.com/en-us/windows/win32/direct3d9/d3dxmatrixorthooffcenterrh

**Packing Matrices/Sequentialization of Multiple Dimensional Data Structures (Column Vs Row Order)**
* https://en.wikipedia.org/wiki/Row-_and_column-major_order

**Spatial Partitioning**
* https://www.cs.cmu.edu/~mbz/personal/graphics/obj.html
* file:///C:/Users/mAmaro/OneDrive/Books/Computer%20Science/Games%20&%20Simulation/Space-filling%20curves%20an%20introduction%20with%20applications%20in%20scientific%20computing%20(%20PDFDrive%20).pdf
* https://www.youtube.com/c/Mr4thProgramming/videos
* https://en.wikipedia.org/wiki/Binary_space_partitioning
* https://en.wikipedia.org/wiki/Adaptive_mesh_refinement
* https://en.wikipedia.org/wiki/Quadtree

### RESOURCES

**Books:**
* The fundamentals of Computer Graphics
* [The Book of Shaders](https://thebookofshaders.com/)
* [Learn OpenGl](https://learnopengl.com/)

**Basis and Coordinate Systems:**

0. pg. 109 - 115
1. [[Khan Academy] Liniar Combinations & Span](https://youtu.be/Qm_OS-8COwU)
2. [[Khan Academy] Linear Independence](https://youtu.be/CrV1xCWdY-g)
3. [[HandmadeHero] Basis I](https://www.youtube.com/watch?v=lcmjmOfWPNU&feature=youtu.be)
4. [[HandmadeHero] Basis II](https://www.youtube.com/watch?v=2yKKcjBIaL0)
5. [[3Blue1Brown] Linear Combinations, Span, Basis](https://www.youtube.com/watch?v=k7RM-ot2NWY&list=PLZHQObOWTQDPD3MizzM2xVFitgF8hE_ab&index=2)
6. 5.1 Determinants: pg.89 - 91
7. [[3Blue1Brown] Linear Transformations and Matrices](https://www.youtube.com/watch?v=kYB8IZa5AuE&list=PLZHQObOWTQDPD3MizzM2xVFitgF8hE_ab&index=3)
8. [[HandmadeHero] Vertex and Fagment Shaders](https://www.youtube.com/watch?v=GtNvxxl3AK4)
9. [[Khan Academy] Linear Algebra Course](https://www.khanacademy.org/math/linear-algebra)                                                                       

**Graphics Resources:**
* http://www.songho.ca/opengl/gl_transform.html 
* http://davidlively.com/programming/graphics/opengl-matrices/row-major-vs-column-major/   
* http://www.songho.ca/opengl/gl_matrix.html                                                                               


**OpenGL:**
* https://web.archive.org/web/20150225192608/http://www.arcsynthesis.org/gltut/Positioning/Tutorial%2005.html

**IMGui:**
* http://silverspaceship.com/inner/imgui/
* https://www.youtube.com/watch?v=Z1qyvQsjK5Y          


**System Communication Resources:**
* https://en.wikipedia.org/wiki/Handshaking
* https://en.wikipedia.org/wiki/Flow_control_(data)
* https://en.wikipedia.org/wiki/Network_congestion#Congestion_control
* https://en.wikipedia.org/wiki/Augmented_Backus%E2%80%93Naur_form
* https://en.wikipedia.org/wiki/Finite-state_machine
* https://docs.microsoft.com/en-us/windows/win32/devio/communications-resources
* https://docs.microsoft.com/en-us/windows/win32/sync/synchronization
* https://docs.microsoft.com/en-us/windows/win32/devio/monitoring-communications-events                                         

### LOG

**NOTE(MIGUEL): (3/16/2022)**

This needs to change from from glID way  to d3d11 cbuffer way.
ids dont mean anything in d3d11 and cbuffer data should not be
just u32s. Use Map/Unmap to stream cbuffer data to gpu. Don't 
stream a bunch transform/proj matrice. instead compose and stream
only one. better would be to have multiple object that are viewed 
in the same way to share a precomputed projection matrix.


**NOTE(MIGUEL): (04/24/2022)**

I disabled some of the paths in the d3d11 renderer. I want to get things
working one at a time. The main issu at the moment is converting cpp pasted
function to the c d3d11 function calls. Problems with the bat file also fixed
and made to be able to build in any directory.


**NOTE(MIGUEL): (04/24/2022)**

I made some progress with the d3d11 port. Found out that the COBJMACROS macro need to be defined to 
use the c style COM interface. There's still a lot to do. Arenas are needed to hotload the shaders. 
DirectX3d is picky about the alignment of the const buffers so they need to be padded. GLSL shaders need
to be ported. The D3D11 Input Layout in the App code need to match the shader. There's probably more things
but this is all i can think of for now.


**NOTE(MIGUEL): (04/25/2022)**

Im porting glyph handling code. Now my focus is to set the alignment of glyph bitmap data
in to a d3d11 buffer resource. Actually that is a bit out of scope and is not neccesary to 
get the portion of code i want to port finised. Right now const buffers data are mapped but 
index buffer data and vertex buffer data are not mapped. So now the issues is how to map data 
from the correct offset and size to the gpu. 
