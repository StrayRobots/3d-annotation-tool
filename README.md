# Stray Studio - 3D Data Annotation Tool

This project is a graphical user interface for annotating point clouds and [Stray scenes](https://docs.strayrobots.io/formats/data.html).

Several different label types are supported, including:
- Bounding boxes
- Keypoints
- Rectangles

### Bounding Box Labels

![Bounding box label type](assets/bbox.jpg)

An example of a bounding box is shown above. They have the following properties:
- `class_id` the class id of the label.
- `position` the x, y, z position relative to the point cloud.
- `orientation` the rotation that transforms vectors in the box coordinate frame to the point cloud coordinate frame.
- `dimensions` three values denoting the width along the x, y and z axes in the bounding boxes local coordinate frame.

### Keypoint Labels

![Keypoint label type](assets/keypoint.jpg)

Keypoints are individual points in the global coordinate frame. They have the following properties:
- `class_id` the class id of the label.
- `position` the x, y, z position in the global frame.

### Rectangle Labels

![Oriented rectangle label type](assets/rectangle.jpg)

Rectangles, show above, are rectangular planes that have a size (height and width), an orientation and position.

The properties are:
- `class_id` the class id of the label.
- `position` the x, y, z position of the center in the global frame.
- `orientation` the rotation taking vectors in local frame to the world frame.
- `size` width and height of the rectangle.

## Installation

### Install Dependencies - Mac
1. Install Homebrew: https://brew.sh/
2. Run `brew update && brew install cmake libomp eigen boost git-lfs pkg-config`

### Install Dependencies - Linux
1. RUN `sudo apt install libeigen3-dev libglfw3-dev libomp-dev libxinerama-dev libxcursor-dev libxi-dev git-lfs cmake libboost-all-dev`

### Build and install the annotation tool
1. Get the source code `git clone https://github.com/StrayRobots/3d-annotation-tool.git` (requires [git](https://git-scm.com/))
2. Navigate to the `3d-annotation-tool` directory (e.g. `cd 3d-annotation-tool`)
3. Initialize git submodules with `git submodule update --init --recursive`
4. Create a build directory with `mkdir build`
5. Pull `git-lfs` objects (helper meshes etc) with `git lfs install && git lfs pull`
6. To build the project run `cd build && cmake .. && make -j8` (`-j8` specifies the number of parallel jobs, for a fewer jobs use a lower number (than `8`))
7. The executable is called `studio`, it can be executed with `./studio <path-to-pointcloud>`. You can find an example `cloud.ply` point cloud  from [here](https://stray-data.nyc3.digitaloceanspaces.com/tutorials/cloud.ply). `<path-to-pointcloud>` should then specify the absolute path to the downloaded file.

## Usage

Run `./studio <path-to-pointcloud>` to open a point cloud in the viewer. Currently only `.ply` point clouds are supported. Annotations are saved into a file of with the same filename but a `.json` file extension. When annotating point clouds, you can move to the next point cloud in the same directory as `<path-to-pointcloud>` using `tab`.

An example `cloud.ply` point cloud can be downloaded from [here](https://stray-data.nyc3.digitaloceanspaces.com/tutorials/cloud.ply).

The keyboard shortcuts are:
- `ctrl+s` to save the annotations.
- `k` switches to the keypoint tool.
- `b` switches to the bounding box tool.
- `r` switches to the rectangle tool.
- `v` switches to the move tool.

## The Stray Toolkit

This project is part of the [Stray command line interface](https://docs.strayrobots.io/), a toolkit to make building 3D computer vision applications easy. Stray Studio can be used through the `stray studio` command.

The Stray toolkit allows you to build point clouds out of handheld or robot mounted RGB-D scans. It also contains utility functions to manage datasets and export labels into commonly used formats, such as Yolo for object detection.

## Running tests

In your build directory, run:
```
cmake .. -DBUILD_TESTS=1
make build_tests
ctest
```

## Code formatting

Code can be formatted using clang-format.

Install clang-format on Mac (brew):

`brew install clang-format`

Install clang-format on Ubuntu:

`sudo apt install clang-format`

There's a script that runs formatting on all .h and .cc files based on the .clang-format file. Usage: `./run_formatting`

