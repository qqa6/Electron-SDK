/*
 * @Author: zhangtao@agora.io
 * @Date: 2021-04-22 20:52:36
 * @Last Modified by: zhangtao@agora.io
 * @Last Modified time: 2021-10-10 01:45:10
 */

#include "node_iris_rtc_engine.h"

using namespace agora::rtc::electron;

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    LOG_F(INFO, "Agora NAPI Init ");
    return NodeIrisRtcEngine::Init(env, exports);
}
NODE_API_MODULE(agora, Init)