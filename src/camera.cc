#include "camera.h"
#include <iostream>
#include <fstream>
#include "3rdparty/json.hpp"

using json = nlohmann::json;

SceneCamera::SceneCamera(const std::filesystem::path intrinsicsPath) {
  if (!std::filesystem::exists(intrinsicsPath)) {
    std::cout << "camera intrinsics do not exist at " << intrinsicsPath.string() << std::endl;
    exit(1);
  }
  std::ifstream file(intrinsicsPath.string());
  nlohmann::json json;
  file >> json;
  auto matrix = json["intrinsic_matrix"];
  for (int i = 0; i < 3; i++) {
    for (int j = 0; j < 3; j++) {
      cameraMatrix(i, j) = matrix[j * 3 + i];
    }
  }
  imageHeight = json["height"];
  imageWidth = json["width"];
}

SceneCamera::SceneCamera(const Eigen::Matrix3f cameraMatrix, double width, double height) : cameraMatrix(cameraMatrix), imageWidth(width), imageHeight(height) {}

Camera::Camera() {
  reset(Vector3f(-0.1, -0.1, -0.1), Vector3f(0.0, 0.0, 0.0));
}

Camera::Camera(const SceneCamera& sceneCamera) : fov(2.0 * std::atan(sceneCamera.imageHeight / (2.0 * sceneCamera.cameraMatrix(1, 1))) * 180.0 / M_PI) {
}

void Camera::reset(const Vector3f resetLookat, const Vector3f resetPosition) {
  lookat = resetLookat;
  position = resetPosition;
  viewMatrix.setIdentity();
  orientation.setIdentity();
  orientation = Quaternionf(getCameraRotation(lookat));
  Quaternionf q = orientation.conjugate();
  viewMatrix.linear() = q.toRotationMatrix();
  viewMatrix.translation() = -(viewMatrix.linear() * position);
}

Matrix3f Camera::getCameraRotation(const Vector3f& forwardVector) const {
  Matrix3f cameraRotation;
  cameraRotation.col(2) = (-forwardVector).normalized();
  cameraRotation.col(0) = (-getUpVector()).cross(cameraRotation.col(2)).normalized();
  cameraRotation.col(1) = cameraRotation.col(2).cross(cameraRotation.col(0)).normalized();
  return cameraRotation;
}

Vector3f Camera::getForwardVector(void) const {
  return -(orientation * Vector3f::UnitZ());
}

Vector3f Camera::getUpVector(void) const {
  return orientation * Vector3f::UnitY();
}

Vector3f Camera::getRightVector(void) const {
  return orientation * Vector3f::UnitX();
}

void Camera::setPosition(const Vector3f& p) {
  position = p;
  updateViewMatrix();
}

void Camera::setOrientation(const Quaternionf& q) {
  orientation = q;
  updateViewMatrix();
}

void Camera::updateViewMatrix() {
  viewMatrix.setIdentity();
  Quaternionf q = orientation.conjugate();
  viewMatrix.linear() = q.toRotationMatrix();
  viewMatrix.translation() = -(viewMatrix.linear() * position);
}

Vector3f Camera::computeRayWorld(float width, float height, double x, double y) const {
  float aspectRatio = width / height;
  float pX = (2.0f * ((x + 0.5f) / width) - 1.0f) * std::tan(fov / 2.0f * M_PI / 180) * aspectRatio;
  float pY = (1.0f - 2.0f * ((y + 0.5f) / height)) * std::tan(fov / 2.0f * M_PI / 180);
  Vector3f ray_C(pX, pY, -1.0f);
  return orientation * ray_C.normalized();
}

Vector2f Camera::projectPoint(const Vector3f& point) const {
  Vector3f viewVector = viewMatrix * point;
  return (viewVector / viewVector[2]).head<2>();
}

void Camera::updateLookat(const Vector3f& newLookat) {
  setLookat(newLookat);
  if (!lookat.isApprox(position)) {
    Vector3f newForwardVector = (lookat - position).normalized();

    Matrix3f cameraRotation(getCameraRotation(newForwardVector));
    setOrientation(Quaternionf(cameraRotation));
  }
}

void Camera::translate(const Vector3f& t) {
  Vector3f trans = orientation * t;
  Affine3f newViewMatrix;
  position = position + trans;
  setLookat(lookat + trans);
  Quaternionf q = getOrientation().conjugate();
  newViewMatrix.linear() = q.toRotationMatrix();
  newViewMatrix.translation() = -(viewMatrix.linear() * getPosition());
  setViewMatrix(newViewMatrix);
}

void Camera::rotateAroundTarget(const Quaternionf& q) {
  Vector3f t = viewMatrix * lookat;
  Affine3f newViewMatrix;
  newViewMatrix = Translation3f(t) * q * Translation3f(-t) * viewMatrix;
  Quaternionf qa(newViewMatrix.linear());
  qa = qa.conjugate();
  orientation = qa;
  position = -(qa * viewMatrix.translation());
  setViewMatrix(newViewMatrix);
}

void Camera::zoom(float d) {
  Affine3f newViewMatrix;
  position += getForwardVector() * d;
  Quaternionf q = getOrientation().conjugate();
  newViewMatrix.linear() = q.toRotationMatrix();
  newViewMatrix.translation() = -(viewMatrix.linear() * getPosition());
  setViewMatrix(newViewMatrix);
}
