////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Plugin.hpp"

#include "RingRenderer.hpp"

#include "../../../src/cs-core/Settings.hpp"
#include "../../../src/cs-core/SolarSystem.hpp"
#include "../../../src/cs-utils/utils.hpp"

#include <VistaKernel/GraphicsManager/VistaSceneGraph.h>
#include <VistaKernel/GraphicsManager/VistaTransformNode.h>
#include <VistaKernelOpenSGExt/VistaOpenSGMaterialTools.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN cs::core::PluginBase* create() {
  return new csp::rings::Plugin;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN void destroy(cs::core::PluginBase* pluginBase) {
  delete pluginBase;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace csp::rings {

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings::Ring& o) {
  o.mTexture     = cs::core::parseProperty<std::string>("texture", j);
  o.mInnerRadius = cs::core::parseProperty<double>("innerRadius", j);
  o.mOuterRadius = cs::core::parseProperty<double>("outerRadius", j);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings& o) {
  cs::core::parseSection("csp-rings",
      [&] { o.mRings = cs::core::parseMap<std::string, Plugin::Settings::Ring>("rings", j); });
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::init() {
  std::cout << "Loading: CosmoScout VR Plugin Rings" << std::endl;

  mPluginSettings = mAllSettings->mPlugins.at("csp-rings");

  for (auto const& settings : mPluginSettings.mRings) {
    auto anchor = mAllSettings->mAnchors.find(settings.first);

    if (anchor == mAllSettings->mAnchors.end()) {
      throw std::runtime_error(
          "There is no Anchor \"" + settings.first + "\" defined in the settings.");
    }

    auto   existence       = cs::core::getExistenceFromSettings(*anchor);
    double tStartExistence = existence.first;
    double tEndExistence   = existence.second;

    auto ring = std::make_shared<Ring>(settings.second.mTexture, anchor->second.mCenter,
        anchor->second.mFrame, settings.second.mInnerRadius, settings.second.mOuterRadius,
        tStartExistence, tEndExistence);
    mSolarSystem->registerAnchor(ring);

    mRings.push_back(ring);
  }

  mRingRenderer = std::make_unique<RingRenderer>();
  mRingRenderer->setSun(mSolarSystem->getSun());
  mRingRenderer->setRings(mRings);

  mRenderNode = mSceneGraph->NewOpenGLNode(mSceneGraph->GetRoot(), mRingRenderer.get());
  VistaOpenSGMaterialTools::SetSortKeyOnSubtree(
      mRenderNode, static_cast<int>(cs::utils::DrawOrder::eAtmospheres) + 1);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
  for (auto const& ring : mRings) {
    mSolarSystem->unregisterAnchor(ring);
  }

  mSceneGraph->GetRoot()->DisconnectChild(mRenderNode);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::rings
