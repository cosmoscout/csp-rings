////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CSP_RINGS_PLUGIN_HPP
#define CSP_RINGS_PLUGIN_HPP

#include "../../../src/cs-core/PluginBase.hpp"
#include "Ring.hpp"

#include <VistaKernel/GraphicsManager/VistaOpenGLNode.h>

namespace csp::rings {

class RingRenderer;

/// This plugin introduces planetary rings. They can be rendered around any object that is
/// located at the center of a spice frame.
/// The configuration of this plugin is done via the provided json config. See README.md for
/// details.
class Plugin : public cs::core::PluginBase {
 public:
  struct Settings {
    /// Settings for a ring around a planet.
    struct Ring {
      std::string mTexture; ///< The path to the texture. The texture should represent a
                            ///< cross section of the ring.
      double mInnerRadius;  ///< The distance from the planets center to where the rings start in
                            ///< meter.
      double mOuterRadius;  ///< The distance from the planets center to where the rings end in
                            ///< meter.
    };

    std::map<std::string, Ring> mRings;
  };

  void init() override;
  void deInit() override;

 private:
  Settings                           mPluginSettings;
  std::vector<std::shared_ptr<Ring>> mRings;
  VistaOpenGLNode*                   mRenderNode;
  std::unique_ptr<RingRenderer>      mRingRenderer;
};

} // namespace csp::rings

#endif // CSP_RINGS_PLUGIN_HPP
