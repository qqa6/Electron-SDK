/*
 * @Author: zhangtao@agora.io
 * @Date: 2021-04-22 20:53:44
 * @Last Modified by: zhangtao@agora.io
 * @Last Modified time: 2021-10-10 11:05:34
 */
#pragma once
#include <exception>
#include "iris_rtc_engine.h"
#include "iris_rtc_raw_data.h"
#include "iris_rtc_raw_data_plugin_manager.h"
#include "node_base.h"
#include "node_iris_rtc_channel.h"
#include "node_iris_rtc_device_manager.h"
#include "node_screen_window_info.h"
#include "video_processer.h"
#include "video_source_proxy.h"

namespace agora {
namespace rtc {
namespace electron {

class NodeIrisEventHandler;

class NodeIrisRtcEngine : public Napi::ObjectWrap<NodeIrisRtcEngine> {
 public:
  explicit NodeIrisRtcEngine(const Napi::CallbackInfo& info);
  virtual ~NodeIrisRtcEngine();

  static Napi::Object Init(Napi::Env env, Napi::Object exports);
  Napi::Value CallApi(const Napi::CallbackInfo& info);
  Napi::Value CallApiWithBuffer(const Napi::CallbackInfo& info);
  Napi::Value OnEvent(const Napi::CallbackInfo& info);
  Napi::Value CreateChannel(const Napi::CallbackInfo& info);
  Napi::Value GetDeviceManager(const Napi::CallbackInfo& info);
  Napi::Value GetScreenWindowsInfo(const Napi::CallbackInfo& info);
  Napi::Value GetScreenDisplaysInfo(const Napi::CallbackInfo& info);
  Napi::Value PluginCallApi(const Napi::CallbackInfo& info);
  Napi::Value EnableVideoFrameCache(const Napi::CallbackInfo& info);
  Napi::Value DisableVideoFrameCache(const Napi::CallbackInfo& info);
  Napi::Value GetVideoStreamData(const Napi::CallbackInfo& info);
  Napi::Value VideoSourceInitialize(const Napi::CallbackInfo& info);
  Napi::Value VideoSourceRelease(const Napi::CallbackInfo& info);
  Napi::Value VideoSourceSetAddonLogFile(const Napi::CallbackInfo& info);
  Napi::Value SetAddonLogFile(const Napi::CallbackInfo& info);
  Napi::Value Release(const Napi::CallbackInfo& info);
  void OnApiError(const char* errorMessage);
  int VideoSourceRelease();

  static const char* _class_name;

 private:
  static const char* _ret_code_str;
  static const char* _ret_result_str;
  std::unique_ptr<VideoSourceProxy> _video_source_proxy;
  std::shared_ptr<VideoProcesser> _video_processer;
  std::shared_ptr<iris::rtc::IrisRtcEngine> _iris_engine;
  std::shared_ptr<NodeIrisEventHandler> _iris_event_handler;
  iris::rtc::IrisRtcRawDataPluginManager* _iris_raw_data_plugin_manager;
  iris::rtc::IrisRtcRawData* _iris_raw_data;
};
}  // namespace electron
}  // namespace rtc
}  // namespace agora
