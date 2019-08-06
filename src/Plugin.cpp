////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Plugin.hpp"

#include "../../../src/cs-core/Settings.hpp"
#include "../../../src/cs-core/SolarSystem.hpp"
#include "../../../src/cs-utils/convert.hpp"
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
  o.mTexture     = j.at("texture").get<std::string>();
  o.mInnerRadius = j.at("innerRadius").get<double>();
  o.mOuterRadius = j.at("outerRadius").get<double>();
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(const nlohmann::json& j, Plugin::Settings& o) {
  o.mRings = j.at("rings").get<std::map<std::string, Plugin::Settings::Ring>>();
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

    double tStartExistence = cs::utils::convert::toSpiceTime(anchor->second.mStartExistence);
    double tEndExistence   = cs::utils::convert::toSpiceTime(anchor->second.mEndExistence);

    auto ring = std::make_shared<Ring>(settings.second.mTexture, anchor->second.mCenter,
        anchor->second.mFrame, settings.second.mInnerRadius, settings.second.mOuterRadius,
        tStartExistence, tEndExistence);
    mSolarSystem->registerAnchor(ring);

    ring->setSun(mSolarSystem->getSun());

    VistaOpenGLNode* ringNode = mSceneGraph->NewOpenGLNode(mSceneGraph->GetRoot(), ring.get());
    VistaOpenSGMaterialTools::SetSortKeyOnSubtree(
        ringNode, static_cast<int>(cs::utils::DrawOrder::eAtmospheres) + 1);

    mRingNodes.push_back(ringNode);
    mRings.push_back(ring);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
  for (auto const& ring : mRings) {
    mSolarSystem->unregisterAnchor(ring);
  }

  for (auto const& ringNode : mRingNodes) {
    mSceneGraph->GetRoot()->DisconnectChild(ringNode);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::rings
