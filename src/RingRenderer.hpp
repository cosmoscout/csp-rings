////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CSP_RINGS_RING_RENDERER_HPP
#define CSP_RINGS_RING_RENDERER_HPP

#include <VistaKernel/GraphicsManager/VistaOpenGLDraw.h>
#include <VistaOGLExt/VistaBufferObject.h>
#include <VistaOGLExt/VistaGLSLShader.h>
#include <VistaOGLExt/VistaVertexArrayObject.h>
#include <memory>

namespace cs::scene {
class CelestialObject;
}

namespace csp::rings {

class Ring;

class RingRenderer : public IVistaOpenGLDraw {
 public:
  RingRenderer();
  ~RingRenderer() override = default;

  void setSun(std::shared_ptr<const cs::scene::CelestialObject> const& sun);
  void setRings(const std::vector<std::shared_ptr<Ring>>& rings);

  bool Do() override;

  bool GetBoundingBox(VistaBoundingBox& bb) override;

 private:
  VistaGLSLShader        mShader;
  VistaVertexArrayObject mRingVAO;
  VistaBufferObject      mRingVBO;

  struct {
    uint32_t matProjection;
    uint32_t surfaceTexture;
    uint32_t farClip;
    uint32_t matModelView;
    uint32_t radii;
  } mUniforms{};

  std::vector<std::shared_ptr<Ring>>                mRings;
  std::shared_ptr<const cs::scene::CelestialObject> mSun;
};
} // namespace csp::rings
#endif // CSP_RINGS_RING_RENDERER_HPP
