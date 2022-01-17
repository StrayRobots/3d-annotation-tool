#ifndef H_SCENE_MODEL
#define H_SCENE_MODEL
#include <memory>
#include <optional>
#include <filesystem>
#include <map>
#include "model/rectangle.h"
#include "geometry/mesh.h"
#include "geometry/point_cloud.h"
#include "geometry/ray_trace_mesh.h"
#include "camera.h"

struct Keypoint {
  int id;
  int instanceId;
  Vector3f position;
  Keypoint(const Vector3f& p) : id(-1), instanceId(0), position(p) {}
  Keypoint(int id, int instance, const Vector3f& p) : id(id), instanceId(instance), position(p) {}
  Keypoint(int id, const Vector3f& p) : id(id), instanceId(0), position(p) {}
  Keypoint(int id) : id(id), instanceId(0), position(Vector3f::Zero()) {}
};

struct BBox {
  int id;
  int instanceId;
  Vector3f position;
  Quaternionf orientation = Quaternionf::Identity();
  Vector3f dimensions = Vector3f::Ones() * 0.2;
};

struct InstanceMetadata {
  std::string name = "";
  Vector3f size = Vector3f::Ones() * 0.2;
};

struct DatasetMetadata {
  int numClasses = 10;
  std::map<int, InstanceMetadata> instanceMetadata;
};

enum ActiveTool {
  AddKeypointToolId,
  MoveKeypointToolId,
  BBoxToolId,
  AddRectangleToolId
};

namespace active_view {
enum ActiveView {
  MeshView,
  PointCloudView
};
}

namespace fs = std::filesystem;

class SceneModel {
private:
  fs::path datasetPath;

  std::shared_ptr<geometry::TriangleMesh> mesh;
  std::shared_ptr<geometry::PointCloud> pointCloud;
  std::optional<geometry::RayTraceMesh> rtMesh;

  // Annotations.
  std::vector<Keypoint> keypoints;
  std::vector<BBox> boundingBoxes;
  std::vector<Rectangle> rectangles;
  int imageWidth;
  int imageHeight;
  Matrix3f cameraMatrix;

public:
  int activeKeypoint = -1;
  int activeBBox = -1;
  int currentInstanceId = 0;
  active_view::ActiveView activeView = active_view::MeshView;
  ActiveTool activeToolId = AddKeypointToolId;
  DatasetMetadata datasetMetadata;

  SceneModel(const std::string& datasetFolder, bool rayTracing = true);

  std::shared_ptr<geometry::TriangleMesh> getMesh() const;
  std::shared_ptr<geometry::PointCloud> getPointCloud();
  std::optional<Vector3f> traceRay(const Vector3f& origin, const Vector3f& direction);
  geometry::Intersection traceRayIntersection(const Vector3f& origin, const Vector3f& direction);

  // Keypoints
  const std::vector<Keypoint>& getKeypoints() const { return keypoints; };
  Keypoint addKeypoint(const Vector3f& kp);
  Keypoint addKeypoint(const Keypoint& kp);
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

  // Rectangle.
  std::vector<Rectangle>& getRectangles() { return rectangles; };
  const std::vector<Rectangle>& getRectangles() const { return rectangles; };
  void addRectangle(Rectangle& rectangle);
  void removeRectangle(int id);
  void updateRectangle(const Rectangle& rectangle);

  Camera sceneCamera() const;
  std::pair<int, int> imageSize() const;
  std::vector<fs::path> imagePaths() const;
  std::vector<Matrix4f> cameraTrajectory() const;

  void save() const;
  void load();

private:
  void initRayTracing();
  void loadCameraParams();
  void loadSceneMetadata();
  void loadPointCloud();
};
#endif
