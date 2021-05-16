#ifndef H_SCENE_MODEL
#define H_SCENE_MODEL
#include <memory>
#include <optional>
#include <filesystem>
#include "geometry/mesh.h"
#include "geometry/ray_trace_mesh.h"
#include "camera.h"

struct Keypoint {
  int id;
  Vector3f position;
  Keypoint(int id, const Vector3f& p) : id(id), position(p) {}
  Keypoint(int id) : id(id), position(Vector3f::Zero()) {}
};

struct BBox {
  int id;
  Vector3f position;
  Quaternionf orientation = Quaternionf::Identity();
  Vector3f dimensions = Vector3f::Ones() * 0.2;
};

enum ActiveTool {
  AddKeypointToolId,
  MoveKeypointToolId,
  BBoxToolId
};

class SceneModel {
private:
  std::filesystem::path datasetPath;

  std::shared_ptr<geometry::TriangleMesh> mesh;
  std::optional<geometry::RayTraceMesh> rtMesh;

  // Annotations.
  std::vector<Keypoint> keypoints;
  std::vector<BBox> boundingBoxes;
public:
  int activeKeypoint = -1;
  int activeBBox = -1;
  ActiveTool activeToolId = AddKeypointToolId;

  SceneModel(const std::string& datasetFolder, bool rayTracing = true);

  std::shared_ptr<geometry::TriangleMesh> getMesh() const;
  std::optional<Vector3f> traceRay(const Vector3f& origin, const Vector3f& direction);
  geometry::Intersection traceRayIntersection(const Vector3f& origin, const Vector3f& direction);

  // Keypoints
  const std::vector<Keypoint>& getKeypoints() const { return keypoints; };
  Keypoint addKeypoint(const Vector3f& p);
  void removeKeypoint(const Keypoint& keypoint);
  void updateKeypoint(int keypointId, Keypoint kp);
  std::optional<Keypoint> getKeypoint(int keypointId) const;
  void setKeypoint(const Keypoint& keypoint);
  void setActiveKeypoint(int id) { activeKeypoint = id; }

  // BBoxes.
  std::optional<BBox> getBoundingBox(int id) const;
  void addBoundingBox(BBox& bbox);
  void removeBoundingBox(int id);
  void updateBoundingBox(const BBox& bbox);
  const std::vector<BBox>& getBoundingBoxes() const { return boundingBoxes; };

  Camera sceneCamera() const;
  std::vector<Matrix4f> cameraTrajectory() const;

  void save() const;
  void load();

private:
  void initRayTracing();
};
#endif
