////////////////////////////////////////////////////////////////////////////////////////////////////
//                               This file is part of CosmoScout VR                               //
//      and may be used under the terms of the MIT license. See the LICENSE file for details.     //
//                        Copyright: (c) 2019 German Aerospace Center (DLR)                       //
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef CSP_RINGS_LOGGER_HPP
#define CSP_RINGS_LOGGER_HPP

#include <spdlog/spdlog.h>

namespace csp::rings {

/// This creates the default singleton logger for "csp-rings" when called for the first time
/// and returns it. See cs-utils/logger.hpp for more logging details.
std::shared_ptr<spdlog::logger> logger();

} // namespace csp::rings

#endif // CSP_RINGS_LOGGER_HPP
