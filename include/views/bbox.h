#pragma once
#include <bgfx/bgfx.h>
#include "views/view.h"
#include "scene_model.h"

namespace views {

class BBoxView : public views::View3D {
private:
  bgfx::VertexBufferHandle vertexBuffer;
  bgfx::IndexBufferHandle indexBuffer;
  bgfx::VertexLayout layout;
  bgfx::ProgramHandle program;
  bgfx::UniformHandle u_scale, u_color;

public:
  BBoxView(int viewId = 0);
  ~BBoxView();
  void render(const BBox& bbox) const;
};

} // namespace views
