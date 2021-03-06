#pragma once
#include <eigen3/Eigen/Dense>
#include "commands/command.h"
#include "scene_model.h"

namespace commands {

using namespace Eigen;
class AddBBoxCommand : public Command {
private:
  BBox bbox;

public:
  AddBBoxCommand(const BBox& bbox) : bbox(bbox) {}
  void execute(SceneModel& sceneModel) override {
    sceneModel.addBoundingBox(bbox);
    sceneModel.activeBBox = bbox.id;
  };
  void undo(SceneModel& sceneModel) override {
    sceneModel.removeBoundingBox(bbox.id);
    auto bboxes = sceneModel.getBoundingBoxes();
    if (sceneModel.getBoundingBox(bbox.id).has_value()) {
      sceneModel.activeBBox = bbox.id;
    } else if (!bboxes.empty()) {
      bboxes.back().id;
    }
  };
};

class MoveBBoxCommand : public Command {
private:
  BBox bbox;
  const Vector3f oldPosition;
  const Vector3f newPosition;
  const Quaternionf oldOrientation;
  const Quaternionf newOrientation;

public:
  MoveBBoxCommand(const BBox& box, const Vector3f& position, const Quaternionf& orientation) :
      bbox(box),
      oldPosition(bbox.position),
      newPosition(position),
      oldOrientation(bbox.orientation),
      newOrientation(orientation) {}
  void execute(SceneModel& sceneModel) override {
    bbox.position = newPosition;
    bbox.orientation = newOrientation;
    sceneModel.updateBoundingBox(bbox);
    sceneModel.activeBBox = bbox.id;
  };
  void undo(SceneModel& sceneModel) override {
    bbox.position = oldPosition;
    bbox.orientation = oldOrientation;
    sceneModel.updateBoundingBox(bbox);
    auto bboxes = sceneModel.getBoundingBoxes();
    if (sceneModel.getBoundingBox(bbox.id).has_value()) {
      sceneModel.activeBBox = bbox.id;
    } else if (!bboxes.empty()) {
      sceneModel.activeBBox = bboxes.back().id;
    }
  };
};

class ResizeBBoxCommand : public Command {
private:
  BBox bbox;
  const Vector3f oldDimensions;
  const Vector3f newDimensions;
  const Vector3f oldPosition;
  const Vector3f newPosition;

public:
  ResizeBBoxCommand(const BBox& box, const Vector3f& dimensions, const Vector3f& position) : bbox(box), oldDimensions(bbox.dimensions), newDimensions(dimensions), oldPosition(bbox.position),
                                                                                             newPosition(position){};
  void execute(SceneModel& sceneModel) override {
    bbox.position = newPosition;
    bbox.dimensions = newDimensions;
    sceneModel.updateBoundingBox(bbox);
    sceneModel.activeBBox = bbox.id;
  };
  void undo(SceneModel& sceneModel) override {
    bbox.position = oldPosition;
    bbox.dimensions = oldDimensions;
    sceneModel.updateBoundingBox(bbox);
    sceneModel.activeBBox = bbox.id;
  };
};

class ChangeBBoxClassIdCommand : public Command {
private:
  BBox bbox;
  const int newClassId;

public:
  ChangeBBoxClassIdCommand(const BBox bbox, const int newClassId) : newClassId(newClassId){};
  void execute(SceneModel& model) {
    BBox updated = bbox;
    updated.classId = newClassId;
    model.updateBoundingBox(updated);
  }

  void undo(SceneModel& model) {
    model.updateBoundingBox(bbox);
  }
};

} // namespace commands
