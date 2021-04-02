#include <memory>
#include <thread>
#include <chrono>
#include <bgfx/bgfx.h>
#include <bgfx/platform.h>
#include <3rdparty/json.hpp>
#include <iostream>
#include <fstream>
#include "3rdparty/cxxopts.h"
#include "views/mesh_view.h"
#include "glfw_app.h"
#include "scene_model.h"
#include <filesystem>

#if BX_PLATFORM_OSX
const unsigned int CommandModifier = GLFW_MOD_SUPER;
#else
const unsigned int CommandModifier = GLFW_MOD_CONTROL;
#endif

class LabelStudio : public GLFWApp {
private:
  std::shared_ptr<views::MeshView> meshView;
  std::shared_ptr<views::MeshDrawable> meshDrawable;

  // Changing view point.
  double prevX, prevY;
  bool dragging = false, moved = false;
  std::optional<Vector3f> pointingAt;
public:
  SceneModel sceneModel;

  LabelStudio(const std::string& scene) : GLFWApp("LabelStudio"), sceneModel(scene) {
    meshView = std::make_shared<views::MeshView>();
    meshDrawable = std::make_shared<views::MeshDrawable>(sceneModel.getMesh());
    meshView->addObject(meshDrawable);

    glfwSetMouseButtonCallback(window, [](GLFWwindow *window, int button, int action, int mods) {
      double x, y;
      glfwGetCursorPos(window, &x, &y);
      LabelStudio* w = (LabelStudio*)glfwGetWindowUserPointer(window);
      if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        w->leftButtonDown(x, y);
      } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        w->leftButtonUp(x, y);
      }
    });

    glfwSetCursorPosCallback(window, [](GLFWwindow *window, double xpos, double ypos) {
      LabelStudio* w = (LabelStudio*)glfwGetWindowUserPointer(window);
      w->mouseMoved(xpos, ypos);
    });

    glfwSetScrollCallback(window, [](GLFWwindow *window, double xoffset, double yoffset) {
      LabelStudio* w = (LabelStudio*)glfwGetWindowUserPointer(window);
      w->scroll(xoffset, yoffset);
    });

    glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
      LabelStudio* w = (LabelStudio*)glfwGetWindowUserPointer(window);
      if (action == GLFW_PRESS) {
        if((CommandModifier == mods) && (GLFW_KEY_S == key)) {
          LabelStudio* w = (LabelStudio*)glfwGetWindowUserPointer(window);
          w->sceneModel.save();
        } else if ((CommandModifier == mods) && (GLFW_KEY_Z == key)) {
          w->undo();
        }
      }
    });
  }

  void leftButtonDown(double x, double y) {
    dragging = true;
    moved = false;
    prevX = x;
    prevY = y;
  }

  void leftButtonUp(double x, double y) {
    dragging = false;
    if (!moved && pointingAt.has_value()) {
      auto sphere = sceneModel.addKeypoint(pointingAt.value());
      auto sphereDrawable = std::make_shared<views::MeshDrawable>(sphere, Vector4f(1.0, 0.5, 0.5, 1.0));
      meshView->addObject(sphereDrawable);
      std::cout << "Added keypoint: " << pointingAt.value().transpose() << std::endl;
    }
  }

  void mouseMoved(double x, double y) {
    moved = true;
    if (dragging) {
      float diffX = (x - prevX) * M_PI / 1000.0;
      float diffY = (y - prevY) * M_PI / 1000.0;
      Quaternionf rotationX, rotationY;
      const auto& camera = sceneModel.getCamera();
      rotationY = AngleAxis(diffY, camera.getOrientation() * Vector3f::UnitX());
      rotationX = AngleAxis(diffX, camera.getOrientation() * Vector3f::UnitY());
      sceneModel.setCameraOrientation(rotationX * rotationY * camera.getOrientation());

      prevX = x;
      prevY = y;
    }
    pointingAt = sceneModel.traceRay(x, y);
  }

  void scroll(double xoffset, double yoffset) {
    (void)xoffset;
    double diff = yoffset * 0.05;
    const auto& cameraPosition = sceneModel.getCamera().getPosition();
    double newNorm = std::max(cameraPosition.norm() + diff, 0.1);
    sceneModel.setCameraPosition(newNorm * cameraPosition.normalized());
  }

  void undo() {
    if (!sceneModel.getKeypoints().empty()) {
      meshView->popObject();
    }
    sceneModel.popKeypoint();
  }

  bool update() const override {
    bgfx::setViewRect(0, 0, 0, uint16_t(Width), uint16_t(Height));
    glfwWaitEventsTimeout(0.02);
    meshView->render(sceneModel.getCamera());

    bgfx::frame();

    return !glfwWindowShouldClose(window);
  }
};

void validateFlags(const cxxopts::ParseResult& flags) {
  bool valid = true;
  if (flags.count("scene") == 0) {
    std::cout << "Scene argument is required." << std::endl;
    valid = false;
  } else if (flags.count("scene") > 1) {
    std::cout << "Only one scene should be provided." << std::endl;
    valid = false;
  } else if (flags.count("scene") == 1) {
    std::string scene = flags["scene"].as<std::vector<std::string>>()[0];
    if (!std::filesystem::exists(scene)) {
      std::cout << "Scene file does not exist." << std::endl;
      valid = false;
    }
  }
  if (!valid) {
    exit(1);
  }
}

int main(int argc, char* argv[]) {
  cxxopts::Options options("LabelStudio", "Annotate the world in 3D.");
  options.add_options()("scene", "That path to the scene .ply file to be loaded for annotation.",
      cxxopts::value<std::vector<std::string>>());
  options.parse_positional({"scene"});
  cxxopts::ParseResult flags = options.parse(argc, argv);
  validateFlags(flags);
  std::string scene = flags["scene"].as<std::vector<std::string>>()[0];

  auto window = std::make_shared<LabelStudio>(scene);

  while (window->update()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(20));
  }

  window = nullptr;

  return 0;
}

