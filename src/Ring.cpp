////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Ring.hpp"

#include "../../../src/cs-graphics/TextureLoader.hpp"
#include "../../../src/cs-utils/FrameTimings.hpp"
#include "../../../src/cs-utils/utils.hpp"

#include <VistaMath/VistaBoundingBox.h>
#include <VistaOGLExt/VistaOGLUtils.h>

#include <glm/gtc/type_ptr.hpp>

namespace csp::rings {

Ring::Ring(std::string const& sTexture, std::string const& sCenterName,
    std::string const& sFrameName, double dInnerRadius, double dOuterRadius, double tStartExistence,
    double tEndExistence)
    : cs::scene::CelestialObject(sCenterName, sFrameName, tStartExistence, tEndExistence)
    , mTexture(cs::graphics::TextureLoader::loadFromFile(sTexture))
    , mInnerRadius(dInnerRadius)
    , mOuterRadius(dOuterRadius) {
}

} // namespace csp::rings
