////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CSP_RINGS_RING_HPP
#define CSP_RINGS_RING_HPP

#include <VistaKernel/GraphicsManager/VistaOpenGLDraw.h>
#include <VistaOGLExt/VistaBufferObject.h>
#include <VistaOGLExt/VistaGLSLShader.h>
#include <VistaOGLExt/VistaVertexArrayObject.h>

#include "../../../src/cs-scene/CelestialObject.hpp"

class VistaTexture;

namespace csp::rings {

/// A single planetary ring. It renders around the frames center.
class Ring : public cs::scene::CelestialObject, public IVistaOpenGLDraw {
 public:
  Ring(std::string const& sTexture, std::string const& sCenterName, std::string const& sFrameName,
      double dInnerRadius, double dOuterRadius, double tStartExistence, double tEndExistence);
  ~Ring() override = default;

  void setSun(std::shared_ptr<const cs::scene::CelestialObject> const& sun);

  bool Do() override;
  bool GetBoundingBox(VistaBoundingBox& bb) override;

 private:
  std::shared_ptr<VistaTexture> mTexture;

  VistaGLSLShader        mShader;
  VistaVertexArrayObject mSphereVAO;
  VistaBufferObject      mSphereVBO;

  std::shared_ptr<const cs::scene::CelestialObject> mSun;

  double mInnerRadius;
  double mOuterRadius;

  static const std::string SPHERE_VERT;
  static const std::string SPHERE_FRAG;
};
} // namespace csp::rings

#endif // CSP_RINGS_RING_HPP