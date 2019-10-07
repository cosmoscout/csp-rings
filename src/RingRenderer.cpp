////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "RingRenderer.hpp"

#include "Ring.hpp"

#include "../../../src/cs-utils/FrameTimings.hpp"
#include "../../../src/cs-utils/utils.hpp"

#include <VistaOGLExt/VistaTexture.h>

#include <glm/geometric.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>

namespace csp::rings {

////////////////////////////////////////////////////////////////////////////////////////////////////

const unsigned GRID_RESOLUTION = 200;

////////////////////////////////////////////////////////////////////////////////////////////////////

RingRenderer::RingRenderer() {
  std::vector<glm::vec2> vertices(GRID_RESOLUTION * 2);

  for (int i = 0; i < GRID_RESOLUTION; ++i) {
    auto x = (1.f * i / (GRID_RESOLUTION - 1.f));

    vertices[i * 2 + 0] = glm::vec2(x, 0.f);
    vertices[i * 2 + 1] = glm::vec2(x, 1.f);
  }

  mSphereVAO.Bind();

  mSphereVBO.Bind(GL_ARRAY_BUFFER);
  mSphereVBO.BufferData(vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);
  mSphereVBO.Release();

  mSphereVAO.EnableAttributeArray(0);
  mSphereVAO.SpecifyAttributeArrayFloat(
      0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), 0, &mSphereVBO);
  mSphereVAO.Release();

  // create sphere shader
  mShader.InitVertexShaderFromString(
      cs::utils::loadFileContentsToString("../share/resources/shaders/Ring.vert.glsl"));
  mShader.InitFragmentShaderFromString(
      cs::utils::loadFileContentsToString("../share/resources/shaders/Ring.frag.glsl"));
  mShader.Link();

  mUniforms.matModelView   = mShader.GetUniformLocation("uMatModelView");
  mUniforms.matProjection  = mShader.GetUniformLocation("uMatProjection");
  mUniforms.surfaceTexture = mShader.GetUniformLocation("uSurfaceTexture");
  mUniforms.radii          = mShader.GetUniformLocation("uRadii");
  mUniforms.farClip        = mShader.GetUniformLocation("uFarClip");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RingRenderer::setSun(std::shared_ptr<const cs::scene::CelestialObject> const& sun) {
  mSun = sun;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool RingRenderer::Do() {
  cs::utils::FrameTimings::ScopedTimer timer("Rings");

  // set uniforms
  mShader.Bind();
  mSphereVAO.Bind();

  // get modelview and projection matrices
  GLfloat glMatP[16];
  glGetFloatv(GL_PROJECTION_MATRIX, &glMatP[0]);
  glUniformMatrix4fv(mUniforms.matProjection, 1, GL_FALSE, glMatP);

  GLfloat glMatMV[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, &glMatMV[0]);

  mShader.SetUniform(mUniforms.surfaceTexture, 0);
  mShader.SetUniform(mUniforms.farClip, cs::utils::getCurrentFarClipDistance());

  glEnable(GL_BLEND);

  for (const auto& ring : mRings) {
    if (!ring->getIsInExistence()) {
      continue;
    }

    // cull invisible rings
    double size   = ring->mOuterRadius * glm::length(ring->matWorldTransform[0]);
    double dist   = glm::length(ring->matWorldTransform[3].xyz());
    double factor = size / dist;

    if (factor < 0.002) {
      return true;
    }

    auto matMV = glm::make_mat4x4(glMatMV) * glm::mat4(ring->getWorldTransform());
    glUniformMatrix4fv(mUniforms.matModelView, 1, GL_FALSE, glm::value_ptr(matMV));

    mShader.SetUniform(mUniforms.radii, (float)ring->mInnerRadius, (float)ring->mOuterRadius);

    ring->mTexture->Bind(GL_TEXTURE0);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw
    glDrawArrays(GL_TRIANGLE_STRIP, 0, GRID_RESOLUTION * 2);

    // clean up
    ring->mTexture->Unbind(GL_TEXTURE0);
  }

  glDisable(GL_BLEND);

  mSphereVAO.Release();
  mShader.Release();

  return true;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool RingRenderer::GetBoundingBox(VistaBoundingBox& bb) {
  return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void RingRenderer::setRings(const std::vector<std::shared_ptr<Ring>>& rings) {
  mRings = rings;
}

} // namespace csp::rings