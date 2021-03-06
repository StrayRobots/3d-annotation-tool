#pragma once
#include "scene_model.h"

class StudioViewController;

namespace commands {

class Command {
  /*
   * A command that changes the state of the scene model.
   * Has to implement both an execute and undo method. The undo method should restore
   * the state of the scene model back to where it was prior to executing the command.
   **/
public:
  virtual ~Command(){};
  virtual void execute(SceneModel& sceneModel) = 0;
  virtual void undo(SceneModel& sceneModel) = 0;
};

} // namespace commands
