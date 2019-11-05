////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ring.hpp"

#include "../../../src/cs-core/GraphicsEngine.hpp"
#include "../../../src/cs-core/SolarSystem.hpp"
#include "../../../src/cs-graphics/TextureLoader.hpp"
#include "../../../src/cs-utils/FrameTimings.hpp"
#include "../../../src/cs-utils/utils.hpp"

#include <VistaMath/VistaBoundingBox.h>
#include <VistaOGLExt/VistaOGLUtils.h>

#include <glm/gtc/type_ptr.hpp>

namespace csp::rings {

////////////////////////////////////////////////////////////////////////////////////////////////////

const unsigned GRID_RESOLUTION = 200;

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string Ring::SPHERE_VERT = R"(
#version 330

uniform vec3 uSunDirection;
uniform vec2 uRadii;
uniform mat4 uMatModelView;
uniform mat4 uMatProjection;

// inputs
layout(location = 0) in vec2 iGridPos;

// outputs
out vec2 vTexCoords;
out vec3 vPosition;
const float PI = 3.141592654;

void main()
{
    vTexCoords = iGridPos.yx;

    vec2 vDir = vec2(sin(iGridPos.x * 2.0 * PI), cos(iGridPos.x * 2.0 * PI));
    
    vec2 vPos = mix(vDir * uRadii.x, vDir * uRadii.y, iGridPos.y);

    vPosition   = (uMatModelView * vec4(vPos.x, 0, vPos.y, 1.0)).xyz;
    gl_Position =  uMatProjection * vec4(vPosition, 1);
}
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

const std::string Ring::SPHERE_FRAG = R"(
#version 330

uniform sampler2D uSurfaceTexture;
uniform float uSunIlluminance;
uniform float uFarClip;

// inputs
in vec2 vTexCoords;
in vec3 vSunDirection;
in vec3 vPosition;

// outputs
layout(location = 0) out vec4 oColor;

void main()
{
    oColor = texture(uSurfaceTexture, vTexCoords);
    oColor.rgb *= uSunIlluminance;
    gl_FragDepth = length(vPosition) / uFarClip;
}
)";

////////////////////////////////////////////////////////////////////////////////////////////////////

Ring::Ring(std::shared_ptr<cs::core::GraphicsEngine> const& graphicsEngine,
    std::shared_ptr<cs::core::SolarSystem> const& solarSystem, std::string const& sTexture,
    std::string const& sCenterName, std::string const& sFrameName, double dInnerRadius,
    double dOuterRadius, double tStartExistence, double tEndExistence)
    : cs::scene::CelestialObject(sCenterName, sFrameName, tStartExistence, tEndExistence)
    , mGraphicsEngine(graphicsEngine)
    , mSolarSystem(solarSystem)
    , mTexture(cs::graphics::TextureLoader::loadFromFile(sTexture))
    , mInnerRadius(dInnerRadius)
    , mOuterRadius(dOuterRadius) {

  // create sphere grid geometry
  std::vector<glm::vec2> vertices(GRID_RESOLUTION * 2);

  for (int i = 0; i < GRID_RESOLUTION; ++i) {
    auto x = (1.f * i / (GRID_RESOLUTION - 1.f));

    vertices[i * 2 + 0] = glm::vec2(x, 0.f);
    vertices[i * 2 + 1] = glm::vec2(x, 1.f);
  }

  mSphereVBO.Bind(GL_ARRAY_BUFFER);
  mSphereVBO.BufferData(vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);
  mSphereVBO.Release();

  mSphereVAO.EnableAttributeArray(0);
  mSphereVAO.SpecifyAttributeArrayFloat(
      0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0, &mSphereVBO);

  // create sphere shader
  mShader.InitVertexShaderFromString(SPHERE_VERT);
  mShader.InitFragmentShaderFromString(SPHERE_FRAG);
  mShader.Link();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Ring::setSun(std::shared_ptr<const cs::scene::CelestialObject> const& sun) {
  mSun = sun;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Ring::Do() {
  if (!getIsInExistence()) {
    return true;
  }

  cs::utils::FrameTimings::ScopedTimer timer("Rings");

  // cull invisible rings
  double size   = mOuterRadius * glm::length(matWorldTransform[0]);
  double dist   = glm::length(matWorldTransform[3].xyz());
  double factor = size / dist;

  if (factor < 0.002) {
    return true;
  }

  // set uniforms
  mShader.Bind();

  // get modelview and projection matrices
  GLfloat glMatMV[16], glMatP[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, &glMatMV[0]);
  glGetFloatv(GL_PROJECTION_MATRIX, &glMatP[0]);
  auto matMV = glm::make_mat4x4(glMatMV) * glm::mat4(getWorldTransform());
  glUniformMatrix4fv(
      mShader.GetUniformLocation("uMatModelView"), 1, GL_FALSE, glm::value_ptr(matMV));
  glUniformMatrix4fv(mShader.GetUniformLocation("uMatProjection"), 1, GL_FALSE, glMatP);

  mShader.SetUniform(mShader.GetUniformLocation("uSurfaceTexture"), 0);
  mShader.SetUniform(
      mShader.GetUniformLocation("uRadii"), (float)mInnerRadius, (float)mOuterRadius);
  mShader.SetUniform(
      mShader.GetUniformLocation("uFarClip"), cs::utils::getCurrentFarClipDistance());

  float sunIlluminance(1.f);

  if (mGraphicsEngine->pEnableHDR.get()) {
   sunIlluminance = mSolarSystem->getSunIlluminance(getWorldTransform()[3]);
  }

  mShader.SetUniform(mShader.GetUniformLocation("uSunIlluminance"), sunIlluminance);

  mTexture->Bind(GL_TEXTURE0);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // draw
  mSphereVAO.Bind();
  glDrawArrays(GL_TRIANGLE_STRIP, 0, GRID_RESOLUTION * 2);
  mSphereVAO.Release();

  // clean up
  mTexture->Unbind(GL_TEXTURE0);

  glDisable(GL_BLEND);
  mShader.Release();

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool Ring::GetBoundingBox(VistaBoundingBox& bb) {
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::rings
