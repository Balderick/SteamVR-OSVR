/** @file
    @brief OSVR tracked device

    @date 2016

    @author
    Sensics, Inc.
    <http://sensics.com/osvr>
*/

// Copyright 2016 Sensics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

// Internal Includes
#include "OSVRTrackedDevice.h"
#include "Logging.h"

#include "osvr_compiler_detection.h"
#include "make_unique.h"
#include "matrix_cast.h"
#include "ValveStrCpy.h"
#include "platform_fixes.h" // strcasecmp
#include "make_unique.h"

// OpenVR includes
#include <openvr_driver.h>

// Library/third-party includes
#include <osvr/ClientKit/Display.h>
#include <osvr/Display/DisplayEnumerator.h>
#include <osvr/Util/EigenInterop.h>
#include <osvr/Client/RenderManagerConfig.h>
#include <util/FixedLengthStringFunctions.h>
#include <osvr/RenderKit/DistortionCorrectTextureCoordinate.h>

// Standard includes
#include <cstring>
#include <ctime>
#include <string>
#include <iostream>
#include <exception>
#include <fstream>
#include <algorithm>        // for std::find

OSVRTrackedDevice::OSVRTrackedDevice(osvr::clientkit::ClientContext& context, vr::ETrackedDeviceClass device_class, const std::string& name) : context_(context), deviceClass_(device_class), name_(name), pose_()
{
    OSVR_LOG(trace) << "OSVRTrackedDevice::OSVRTrackedDevice() called.";
}

OSVRTrackedDevice::~OSVRTrackedDevice()
{
    // do nothing
}

vr::EVRInitError OSVRTrackedDevice::Activate(uint32_t object_id)
{
    objectId_ = object_id;
    settings_ = std::make_unique<Settings>();

    propertyContainer_ = vr::VRProperties()->TrackedDeviceToPropertyContainer(objectId_);
    vr::VRProperties()->SetInt32Property(propertyContainer_, vr::Prop_DeviceClass_Int32, deviceClass_);
    vr::VRProperties()->SetStringProperty(propertyContainer_, vr::Prop_SerialNumber_String, name_.c_str());

    return vr::VRInitError_None;
}

void OSVRTrackedDevice::Deactivate()
{
    objectId_ = vr::k_unTrackedDeviceIndexInvalid;
}

void OSVRTrackedDevice::EnterStandby()
{
    // do nothing
}

void* OSVRTrackedDevice::GetComponent(const char* component_name_and_version)
{
    if (!strcasecmp(component_name_and_version, vr::IVRDisplayComponent_Version)) {
        auto component = dynamic_cast<vr::IVRDisplayComponent*>(this);
        if (!component)
            OSVR_LOG(warn) << name_ << "::GetComponent(): Requested component [" << component_name_and_version << "] but failed dynamic_cast.";
        return component;
    } else if (!strcasecmp(component_name_and_version, vr::IVRDriverDirectModeComponent_Version)) {
        auto component = dynamic_cast<vr::IVRDriverDirectModeComponent*>(this);
        if (!component)
            OSVR_LOG(warn) << name_ << "::GetComponent(): Requested component [" << component_name_and_version << "] but failed dynamic_cast.";
        return component;
    } else if (!strcasecmp(component_name_and_version, vr::IVRControllerComponent_Version)) {
        auto component = dynamic_cast<vr::IVRControllerComponent*>(this);
        if (!component)
            OSVR_LOG(warn) << name_ << "::GetComponent(): Requested component [" << component_name_and_version << "] but failed dynamic_cast.";
        return component;
    } else if (!strcasecmp(component_name_and_version, vr::IVRCameraComponent_Version)) {
        auto component = dynamic_cast<vr::IVRCameraComponent*>(this);
        if (!component)
            OSVR_LOG(warn) << name_ << "::GetComponent(): Requested component [" << component_name_and_version << "] but failed dynamic_cast.";
        return component;
    } else {
        OSVR_LOG(warn) << name_ << "::GetComponent(): Unknown component [" << component_name_and_version << "] requested.";
        return nullptr;
    }
}

void OSVRTrackedDevice::DebugRequest(const char* request, char* response_buffer, uint32_t response_buffer_size)
{
    // Log the requests just to see what info clients are looking for
    OSVR_LOG(debug) << name_ << ": Received debug request [" << request << "] with response buffer size of " << response_buffer_size << "].";

    // make use of (from vrtypes.h) static const uint32_t k_unMaxDriverDebugResponseSize = 32768;
    // return empty string for now
    if (response_buffer_size > 0) {
        response_buffer[0] = '\0';
    }
}

vr::DriverPose_t OSVRTrackedDevice::GetPose()
{
    return pose_;
}

const char* OSVRTrackedDevice::getId()
{
    return name_.c_str();
}

vr::ETrackedDeviceClass OSVRTrackedDevice::getDeviceClass() const
{
    return deviceClass_;
}

std::string OSVRTrackedDevice::getName() const
{
    return name_;
}

// ------------------------------------
// Protected Methods
// ------------------------------------

