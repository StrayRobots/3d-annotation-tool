#pragma once
#include "camera.h"

class ViewContext3D {
public:
  const Camera& camera;

  ViewContext3D(const Camera& cam) : camera(cam) {}
  double width = 800;
  double height = 600;
  double mousePositionX = 0.0;
  double mousePositionY = 0.0;
};
