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

    ./mesh_curvature ../models/piggy.obj

A glfw app should launch displaying a 3D cube.

## Dependencies

The only dependencies are stl, eigen, [libigl](libigl.github.io/libigl/) and
the dependencies of the `igl::viewer::Viewer` (mandatory: glfw and
opengl, optional: nanogui and nanovg).

If you clone the repository recursively, this should be taken care of:

    git clone --recursive https://github.com/xionluhnis/mesh_curvature.git

## Generating texture data

    ./mesh_curvature ../models/piggy.obj all 0

will creates the following files in `models`:
 * `piggy.obj-H.tsv` - the mean curvature at every corner
 * `piggy.obj-G.tsv` - the gaussian curvature at every corner
 * `piggy.obj-Kx.tsv` - the X=1|2-th principal curvature at every corner
 * `piggy.obj-Kxd.tsv` - the X=1|2-th principal curvature 3d vectors at every corner

These tsv files are just list of values at corners (vertex per face).
To generate a full image, use matlab:

    K1 = dlmread('models/piggy.obj-K1.tsv');
    texK1 = generate_texture(K1, 1024);

which you can then save as a texture image.

## License

See [libigl](libigl.github.io/libigl/) for its own license.
The code example here are distributed under the MIT License.
