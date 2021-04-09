#ifndef H_VIEW
#define H_VIEW
#include "camera.h"

namespace views {

using namespace Eigen;
class View {
public:
  virtual void render(const Camera&) const = 0;
};

}

#endif

