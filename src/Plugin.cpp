////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Plugin.hpp"

#include "../../../src/cs-core/Settings.hpp"
#include "../../../src/cs-core/SolarSystem.hpp"
#include "../../../src/cs-utils/logger.hpp"
#include "../../../src/cs-utils/utils.hpp"
#include "Ring.hpp"
#include "logger.hpp"

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN cs::core::PluginBase* create() {
  return new csp::rings::Plugin;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

EXPORT_FN void destroy(cs::core::PluginBase* pluginBase) {
  delete pluginBase; // NOLINT(cppcoreguidelines-owning-memory)
}

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace csp::rings {

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(nlohmann::json const& j, Plugin::Settings::Ring& o) {
  cs::core::Settings::deserialize(j, "texture", o.mTexture);
  cs::core::Settings::deserialize(j, "innerRadius", o.mInnerRadius);
  cs::core::Settings::deserialize(j, "outerRadius", o.mOuterRadius);
}

void to_json(nlohmann::json& j, Plugin::Settings::Ring const& o) {
  cs::core::Settings::serialize(j, "texture", o.mTexture);
  cs::core::Settings::serialize(j, "innerRadius", o.mInnerRadius);
  cs::core::Settings::serialize(j, "outerRadius", o.mOuterRadius);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void from_json(nlohmann::json const& j, Plugin::Settings& o) {
  cs::core::Settings::deserialize(j, "rings", o.mRings);
}

void to_json(nlohmann::json& j, Plugin::Settings const& o) {
  cs::core::Settings::serialize(j, "rings", o.mRings);
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::init() {

  logger().info("Loading plugin...");

  mOnLoadConnection = mAllSettings->onLoad().connect([this]() { onLoad(); });
  mOnSaveConnection = mAllSettings->onSave().connect(
      [this]() { mAllSettings->mPlugins["csp-rings"] = mPluginSettings; });

  // Load settings.
  onLoad();

  logger().info("Loading done.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::deInit() {
  logger().info("Unloading plugin...");

  for (auto const& ring : mRings) {
    mSolarSystem->unregisterAnchor(ring);
  }

  mAllSettings->onLoad().disconnect(mOnLoadConnection);
  mAllSettings->onSave().disconnect(mOnSaveConnection);

  logger().info("Unloading done.");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

void Plugin::onLoad() {
  // Read settings from JSON.
  from_json(mAllSettings->mPlugins.at("csp-rings"), mPluginSettings);

  // First try to re-configure existing rings.
  for (auto&& ring : mRings) {
    auto settings = mPluginSettings.mRings.find(ring->getCenterName());
    if (settings != mPluginSettings.mRings.end()) {
      // If there are settings for this ring, reconfigure it.
      auto anchor                           = mAllSettings->mAnchors.find(settings->first);
      auto [tStartExistence, tEndExistence] = anchor->second.getExistence();
      ring->setStartExistence(tStartExistence);
      ring->setEndExistence(tEndExistence);
      ring->setFrameName(anchor->second.mFrame);
      ring->configure(settings->second);
    } else {
      // Else delete it.
      mSolarSystem->unregisterAnchor(ring);
      ring.reset();
    }
  }

  // Then remove all which have been set to null.
  mRings.erase(
      std::remove_if(mRings.begin(), mRings.end(), [](auto const& p) { return !p; }), mRings.end());

  // Then add new rings.
  for (auto const& ringSettings : mPluginSettings.mRings) {
    auto existing = std::find_if(mRings.begin(), mRings.end(),
        [&](auto val) { return val->getCenterName() == ringSettings.first; });
    if (existing != mRings.end()) {
      continue;
    }

    auto anchor = mAllSettings->mAnchors.find(ringSettings.first);

    if (anchor == mAllSettings->mAnchors.end()) {
      throw std::runtime_error(
          "There is no Anchor \"" + ringSettings.first + "\" defined in the settings.");
    }

    auto [tStartExistence, tEndExistence] = anchor->second.getExistence();

    auto ring = std::make_shared<Ring>(mAllSettings, mSolarSystem, anchor->second.mCenter,
        anchor->second.mFrame, tStartExistence, tEndExistence);

    ring->configure(ringSettings.second);

    mSolarSystem->registerAnchor(ring);

    mRings.emplace_back(ring);
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace csp::rings
