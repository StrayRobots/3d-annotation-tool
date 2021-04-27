#include "label_studio.h"
#include <memory>
#include <iostream>
#include <filesystem>
#include <fstream>
#include <bgfx/bgfx.h>
#include "3rdparty/json.hpp"

LabelStudio::LabelStudio(const std::string& folder) : GLFWApp("Label Studio"), sceneModel(folder),

                                                      commandStack(), studioViewController(sceneModel, commandStack), datasetFolder(folder) {
  loadState();

  glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
    double x, y;
    glfwGetCursorPos(window, &x, &y);
    LabelStudio* w = (LabelStudio*)glfwGetWindowUserPointer(window);
    w->setInputModifier(mods);
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
      w->leftButtonDown(x, y);
    } else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
      w->leftButtonUp(x, y);
    }
  });

  glfwSetCursorPosCallback(window, [](GLFWwindow* window, double xpos, double ypos) {
    LabelStudio* w = (LabelStudio*)glfwGetWindowUserPointer(window);
    w->mouseMoved(xpos, ypos);
  });

  glfwSetScrollCallback(window, [](GLFWwindow* window, double xoffset, double yoffset) {
    LabelStudio* w = (LabelStudio*)glfwGetWindowUserPointer(window);
    w->scroll(xoffset, yoffset);
  });
  glfwSetWindowSizeCallback(window, [](GLFWwindow* window, int width, int height) {
    LabelStudio* w = (LabelStudio*)glfwGetWindowUserPointer(window);
    std::cout << "window resized: " << width << " " << height << std::endl;
    w->resize(width, height);
  });

  glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
    LabelStudio* w = (LabelStudio*)glfwGetWindowUserPointer(window);
    w->setInputModifier(mods);
    if (action == GLFW_PRESS) {
      if ((CommandModifier == mods) && (GLFW_KEY_S == key)) {
        w->sceneModel.save();
      } else if ((CommandModifier == mods) && (GLFW_KEY_Z == key)) {
        w->undo();
      } else {
        char characterPressed = key;
        w->studioViewController.keypress(characterPressed);
      }
    }
  });

  glfwGetWindowSize(window, &width, &height);
  studioViewController.viewWillAppear(width, height);
  loadState();
}

void LabelStudio::setInputModifier(int mods) {
  if (mods == CommandModifier) {
    inputModifier = InputModifier::command;
  } else {
    inputModifier = InputModifier::NONE;
  }
}

void LabelStudio::leftButtonDown(double x, double y) {
  studioViewController.leftButtonDown(x, y, inputModifier);
}

void LabelStudio::leftButtonUp(double x, double y) {
  studioViewController.leftButtonUp(x, y, inputModifier);
}

void LabelStudio::mouseMoved(double x, double y) {
  studioViewController.mouseMoved(x, y, inputModifier);
}

void LabelStudio::scroll(double xoffset, double yoffset) {
  studioViewController.scroll(xoffset, yoffset, inputModifier);
}

void LabelStudio::resize(int newWidth, int newHeight) {
  GLFWApp::resize(newWidth, newHeight);
  studioViewController.resize(newWidth, newHeight, inputModifier);
}

bool LabelStudio::update() const {
  bgfx::setViewRect(0, 0, 0, width, height);
  studioViewController.render();

  bgfx::frame();

  glfwWaitEventsTimeout(0.16);

  return !glfwWindowShouldClose(window);
}

void LabelStudio::undo() {
  if (commandStack.empty()) return;
  commandStack.back()->undo(studioViewController, sceneModel);
  commandStack.pop_back();
}

void LabelStudio::loadState() {
  std::filesystem::path keypointPath(datasetFolder / "keypoints.json");
  if (!std::filesystem::exists(keypointPath))
    return;
  std::ifstream file(keypointPath.string());
  nlohmann::json json;
  file >> json;
  for (auto& keypoint : json) {
    auto k = Vector3f(keypoint["x"].get<float>(), keypoint["y"].get<float>(), keypoint["z"].get<float>());
    std::unique_ptr<Command> command = std::make_unique<AddKeypointCommand>(k);
    command->execute(studioViewController, sceneModel);
    commandStack.push_back(std::move(command));
  }
}
