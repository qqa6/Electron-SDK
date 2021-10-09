/*
 * @Author: zhangtao@agora.io
 * @Date: 2021-04-22 20:52:36
 * @Last Modified by: zhangtao@agora.io
 * @Last Modified time: 2021-07-29 15:48:30
 */

#include "node_iris_rtc_engine.h"

using namespace agora::rtc::electron;

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    return NodeIrisRtcEngine::Init(env, exports);
}
NODE_API_MODULE(agora, Init)