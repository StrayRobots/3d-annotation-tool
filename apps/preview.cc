#include <memory>
#include <thread>
#include <chrono>
#include <iostream>
#include <fstream>
#include <filesystem>
#include "3rdparty/cxxopts.h"
#include "glfw_app.h"
#include "views/image_pane.h"
#include "views/annotation_view.h"
#include "scene_model.h"
#include "camera.h"
#include "utils/dataset.h"
#include "3rdparty/json.hpp"
#include "camera.h"

namespace fs = std::filesystem;

void validateFlags(const cxxopts::ParseResult& flags) {
  bool valid = true;
  if (flags.count("dataset") == 0) {
    std::cout << "Dataset argument is required." << std::endl;
    valid = false;
  } else if (flags.count("dataset") > 1) {
    std::cout << "Only one dataset should be provided." << std::endl;
    valid = false;
  } else if (flags.count("dataset") == 1) {
    std::string dataset = flags["dataset"].as<std::vector<std::string>>()[0];
    if (!fs::exists(dataset)) {
      std::cout << "Dataset folder does not exist." << std::endl;
      valid = false;
    }
  }
  if (!valid) {
    exit(1);
  }
}

class PreviewApp : public GLFWApp {
private:
  const fs::path datasetPath;
  SceneModel scene;
  SceneCamera sceneCamera;
  ViewContext3D viewContext;
  views::AnnotationView annotationView;
  std::vector<fs::path> colorImages;
  std::vector<Matrix4f> cameraPoses;
  std::unique_ptr<views::ImagePane> imageView;
  int currentFrame = 0;
  bool paused = false;

public:
  PreviewApp(const std::string& folder) : GLFWApp("Stray Preview"),
                                          datasetPath(folder),
                                          scene(std::nullopt),
                                          sceneCamera(datasetPath / "camera_intrinsics.json"),
                                          viewContext(sceneCamera),
                                          annotationView(scene, 1) {
    colorImages = utils::dataset::getDatasetImagePaths(datasetPath / "color");
    scene.load(datasetPath / "annotations.json");
    cameraPoses = utils::dataset::getDatasetCameraTrajectory(datasetPath / "scene" / "trajectory.log");
    imageView = std::make_unique<views::ImagePane>(colorImages[0], 0);

    bgfx::setViewClear(imageView->viewId, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f);
    bgfx::setViewClear(annotationView.viewId, BGFX_CLEAR_DEPTH, 1.0f);

    viewContext.camera.reset(Vector3f::UnitZ(), Vector3f::Zero());

    viewContext.width = sceneCamera.imageWidth;
    viewContext.height = sceneCamera.imageHeight;

    glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int newWidth, int newHeight) {
      PreviewApp* w = (PreviewApp*)glfwGetWindowUserPointer(window);
      w->resize(newWidth, newHeight);
    });
    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
      PreviewApp* w = (PreviewApp*)glfwGetWindowUserPointer(window);
      char characterPressed = key;
      if (action == GLFW_PRESS) {
        if (characterPressed == ' ') {
          w->paused = !w->paused;
        }
      }
    });
  }

  void resize(int newWidth, int newHeight) override {
    GLFWApp::resize(newWidth, newHeight);
  }

  void advance() {
    if (paused) return;
    currentFrame++;
    if (currentFrame < int(colorImages.size())) {
      imageView->setImage(colorImages[currentFrame]);
      Matrix4f T_C = cameraPoses[currentFrame];
      Vector3f p_C = T_C.block<3, 1>(0, 3);
      Quaternionf R_C(T_C.block<3, 3>(0, 0));
      auto R_WC = AngleAxisf(M_PI, Vector3f::UnitX());
      viewContext.camera.setOrientation((R_C * R_WC).normalized());
      viewContext.camera.setPosition(p_C);
    }
  }

  bool update() const override {
    if (currentFrame >= int(colorImages.size())) return false;
    bgfx::setViewRect(imageView->viewId, 0, 0, width, height);
    imageView->render();
    bgfx::setViewRect(annotationView.viewId, 0, 0, width, height);
    annotationView.render(viewContext);
    bgfx::frame();
    glfwWaitEventsTimeout(0.025);
    return !glfwWindowShouldClose(window);
  }
};

int main(int argc, char* argv[]) {
  cxxopts::Options options("LabelStudio", "Annotate the world in 3D.");
  options.add_options()("dataset", "That path to folder of the dataset to annotate.",
                        cxxopts::value<std::vector<std::string>>());
  options.parse_positional({"dataset"});
  cxxopts::ParseResult flags = options.parse(argc, argv);
  validateFlags(flags);
  std::string dataset = flags["dataset"].as<std::vector<std::string>>()[0];

  PreviewApp window(dataset);

  while (window.update()) {
    window.advance();
  }

  return 0;
}
