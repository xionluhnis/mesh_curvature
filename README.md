# mesh_curvature
Utility to compute mesh curvature on mesh and export it to uv maps

## Compile

Compile this project using the standard cmake routine:

    mkdir build
    cd build
    cmake ..
    make

This should find and build the dependencies and create a `mesh_curvature` binary.

## Run

From within the `build` directory just issue:

    ./mesh_curvature

A glfw app should launch displaying a 3D cube.

## Dependencies

The only dependencies are stl, eigen, [libigl](libigl.github.io/libigl/) and
the dependencies of the `igl::viewer::Viewer` (mandatory: glfw and
opengl, optional: nanogui and nanovg).

If you clone the repository recursively, this should be taken care of:

    git clone --recursive https://github.com/xionluhnis/mesh_curvature.git


## License

See [libigl](libigl.github.io/libigl/) for its own license.
The code example here are distributed under the MIT License.
