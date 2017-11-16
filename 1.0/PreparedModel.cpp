/*
 * Copyright (C) 2017 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "android.hardware.neuralnetworks@1.0-impl-hvx"

#include "HexagonUtils.h"
#include "PreparedModel.h"
#include "ValidateHal.h"
#include <android-base/logging.h>
#include <thread>

namespace android {
namespace hardware {
namespace neuralnetworks {
namespace V1_0 {
namespace implementation {

PreparedModel::PreparedModel(const Model& neuralNetworksModel,
                             const std::shared_ptr<hexagon::Model>& hexagonModel) :
        mNeuralNetworksModel(neuralNetworksModel), mHexagonModel(hexagonModel) {}

PreparedModel::~PreparedModel() {}

static void asyncExecute(const std::shared_ptr<hexagon::Model>& model, const Request& request,
                         const sp<IExecutionCallback>& callback) {
    ErrorStatus status = model->execute(request) == true ?
            ErrorStatus::NONE : ErrorStatus::GENERAL_FAILURE;
    callback->notify(status);
}

Return<ErrorStatus> PreparedModel::execute(const Request& request,
                                           const sp<IExecutionCallback>& callback) {
    if (callback.get() == nullptr) {
        LOG(ERROR) << "invalid callback passed to execute";
        return ErrorStatus::INVALID_ARGUMENT;
    }
    if (!nn::validateRequest(request, mNeuralNetworksModel)) {
        callback->notify(ErrorStatus::INVALID_ARGUMENT);
        return ErrorStatus::INVALID_ARGUMENT;
    }
    if (!hexagon::isHexagonAvailable()) {
        callback->notify(ErrorStatus::DEVICE_UNAVAILABLE);
        return ErrorStatus::DEVICE_UNAVAILABLE;
    }

    // TODO: once nnlib hanging issue is resolved, make this function
    // asynchronous again
    asyncExecute(mHexagonModel, request, callback);

    return ErrorStatus::NONE;
}

}  // namespace implementation
}  // namespace V1_0
}  // namespace neuralnetworks
}  // namespace hardware
}  // namespace android
