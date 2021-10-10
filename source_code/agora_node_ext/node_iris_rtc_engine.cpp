/*
 * @Author: zhangtao@agora.io
 * @Date: 2021-04-22 20:53:37
 * @Last Modified by: zhangtao@agora.io
 * @Last Modified time: 2021-10-10 11:05:32
 */
#include "node_iris_rtc_engine.h"
#include <assert.h>
#include <memory>
#include "node_iris_event_handler.h"

namespace agora {
namespace rtc {
namespace electron {

#define RETURE_NAPI_OBJ()            \
  auto env = info.Env();             \
  auto obj = Napi::Object::New(env); \
  obj.Set(_ret_code_str, ret);       \
  obj.Set(_ret_result_str, result);  \
  return obj;

using namespace iris::rtc;

const char* NodeIrisRtcEngine::_class_name = "NodeIrisRtcEngine";
const char* NodeIrisRtcEngine::_ret_code_str = "retCode";
const char* NodeIrisRtcEngine::_ret_result_str = "result";

NodeIrisRtcEngine::NodeIrisRtcEngine(const Napi::CallbackInfo& info)
    : Napi::ObjectWrap<NodeIrisRtcEngine>(info) {
  LOG_F(INFO, "NodeIrisRtcEngine::NodeIrisRtcEngine()");
  _iris_event_handler = std::make_shared<NodeIrisEventHandler>(this);
  _iris_engine = std::make_shared<IrisRtcEngine>();
  _video_processer = std::make_shared<VideoProcesser>(_iris_engine);
  _iris_raw_data = _iris_engine->raw_data();
  _iris_raw_data_plugin_manager = _iris_raw_data->plugin_manager();
  _iris_engine->SetEventHandler(_iris_event_handler.get());
}

NodeIrisRtcEngine::~NodeIrisRtcEngine() {
  LOG_F(INFO, "NodeIrisRtcEngine::~NodeIrisRtcEngine");
}

Napi::Object NodeIrisRtcEngine::Init(Napi::Env env, Napi::Object exports) {
  Napi::Function func = DefineClass(
      env, "NodeIrisRtcEngine",
      {InstanceMethod("CallApi", &NodeIrisRtcEngine::CallApi),
       InstanceMethod("CallApiWithBuffer",
                      &NodeIrisRtcEngine::CallApiWithBuffer),
       InstanceMethod("OnEvent", &NodeIrisRtcEngine::OnEvent),
       InstanceMethod("CreateChannel", &NodeIrisRtcEngine::CreateChannel),
       InstanceMethod("GetDeviceManager", &NodeIrisRtcEngine::GetDeviceManager),
       InstanceMethod("GetScreenWindowsInfo",
                      &NodeIrisRtcEngine::GetScreenWindowsInfo),
       InstanceMethod("GetScreenDisplaysInfo",
                      &NodeIrisRtcEngine::GetScreenDisplaysInfo),
       InstanceMethod("EnableVideoFrameCache",
                      &NodeIrisRtcEngine::EnableVideoFrameCache),
       InstanceMethod("DisableVideoFrameCache",
                      &NodeIrisRtcEngine::DisableVideoFrameCache),
       InstanceMethod("GetVideoStreamData",
                      &NodeIrisRtcEngine::GetVideoStreamData),
       InstanceMethod("SetAddonLogFile", &NodeIrisRtcEngine::SetAddonLogFile),
       InstanceMethod("PluginCallApi", &NodeIrisRtcEngine::PluginCallApi),
       InstanceMethod("VideoSourceInitialize",
                      &NodeIrisRtcEngine::VideoSourceInitialize),
       InstanceMethod("VideoSourceRelease",
                      &NodeIrisRtcEngine::VideoSourceRelease),
       InstanceMethod("VideoSourceSetAddonLogFile",
                      &NodeIrisRtcEngine::VideoSourceSetAddonLogFile),
       InstanceMethod("Release", &NodeIrisRtcEngine::Release)});

  Napi::FunctionReference* constructor = new Napi::FunctionReference();
  *constructor = Napi::Persistent(func);
  env.SetInstanceData(constructor);

  exports.Set("NodeIrisRtcEngine", func);
  return exports;
}

Napi::Value NodeIrisRtcEngine::CallApi(const Napi::CallbackInfo& info) {
  int process_type = 0;
  int api_type = 0;
  std::string parameter = "";

  napi_get_value_int32(info[0], process_type);
  napi_get_value_int32(info[1], api_type);
  napi_get_value_utf8_string(info[2], parameter);

  char result[512];
  memset(result, '\0', 512);
  int ret = ERROR_PARAMETER_1;

  if (this->_iris_engine) {
    try {
      if (process_type == PROCESS_TYPE::MAIN) {
        ret = this->_iris_engine->CallApi((ApiTypeEngine)api_type,
                                          parameter.c_str(), result);
      } else {
        if (this->_video_source_proxy) {
          ret = this->_video_source_proxy->CallApi((ApiTypeEngine)api_type,
                                                   parameter.c_str(), result);
        } else {
          LOG_F(INFO, "CallApi parameter did not initialize videoSource yet");
        }
      }
      LOG_F(INFO, "CallApi parameter: type: %d, parameter: %s, ret: %d",
            process_type, parameter.c_str(), ret);
    } catch (std::exception& e) {
      this->OnApiError(e.what());
      LOG_F(INFO, "CallApi parameter: catch excepton msg: %s", e.what());
    }
  } else {
    ret = ERROR_NOT_INIT;
  }
  RETURE_NAPI_OBJ();
}

Napi::Value NodeIrisRtcEngine::CallApiWithBuffer(
    const Napi::CallbackInfo& info) {
  std::string parameter = "";
  std::string buffer = "";
  int process_type = 0;
  int api_type = 0;
  int length = 0;

  napi_get_value_int32(info[0], process_type);
  napi_get_value_int32(info[1], api_type);
  napi_get_value_utf8_string(info[2], parameter);
  napi_get_value_utf8_string(info[3], buffer);
  napi_get_value_int32(info[4], length);

  char result[512];
  int ret = ERROR_PARAMETER_1;
  memset(result, '\0', 512);

  if (this->_iris_engine) {
    try {
      switch (ApiTypeEngine(api_type)) {
        case kEngineRegisterPacketObserver: {
          break;
        }
        case kEngineSendStreamMessage: {
          if (process_type == PROCESS_TYPE::MAIN) {
            ret = this->_iris_engine->CallApi(
                (ApiTypeEngine)api_type, parameter.c_str(),
                const_cast<char*>(buffer.c_str()), result);
          } else {
            if (this->_video_source_proxy) {
              ret = this->_video_source_proxy->CallApi(
                  (ApiTypeEngine)api_type, parameter.c_str(), buffer.c_str(),
                  length, result);
            } else {
              LOG_F(INFO,
                    "CallApiWithBuffer parameter did not initialize "
                    "videoSource yet "
                    "source yet");
            }
          }
          break;
        }
        case kEngineSendMetadata: {
          break;
        }
        case kMediaPushAudioFrame: {
          break;
        }
        case kMediaPullAudioFrame: {
          break;
        }
        case kMediaPushVideoFrame: {
          break;
        }
        default: {
          break;
        }
      }
    } catch (std::exception& e) {
      this->OnApiError(e.what());
    }
  } else {
    ret = ERROR_NOT_INIT;
  }

  RETURE_NAPI_OBJ();
}

Napi::Value NodeIrisRtcEngine::OnEvent(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  std::string event_name = "";
  Napi::FunctionReference function;
  napi_get_value_utf8_string(info[0], event_name);
  napi_get_value_function(info[1], function);

  if (this->_iris_engine) {
    this->_iris_event_handler->addEvent(event_name, std::move(function));
  } else {
    LOG_F(INFO, "NodeIrisRtcEngine::OnEvent error Not Init Engine");
  }
  auto ret_value = env.Null();
  return ret_value;
}

Napi::Value NodeIrisRtcEngine::CreateChannel(const Napi::CallbackInfo& info) {
  LOG_F(INFO, " NodeIrisRtcEngine::CreateChannel");
  int process_type = 0;
  std::string channel_id = "";

  napi_get_value_int32(info[0], process_type);
  napi_get_value_utf8_string(info[1], channel_id);

  //  if (nodeIrisRtcEngine->_iris_engine) {
  //    auto iris_channel = nodeIrisRtcEngine->_iris_engine->channel();
  //    if (process_type == PROCESS_TYPE::MAIN) {
  //      auto _js_channel =
  //          NodeIrisRtcChannel::Init(_isolate, iris_channel,
  //          channel_id.c_str());
  //      args.GetReturnValue().Set(_js_channel);
  //    } else {
  //    }
  //  } else {
  //    LOG_F(INFO, "NodeIrisRtcEngine::CreateChannel error Not Init Engine");
  //  }
}

Napi::Value NodeIrisRtcEngine::GetDeviceManager(
    const Napi::CallbackInfo& info) {
  // NodeIrisRtcDeviceManager::Init(env);
  // napi_status status;
  // napi_value jsthis;
  // napi_get_cb_info(env, info, nullptr, nullptr, &jsthis, nullptr);
  // assert(status == napi_ok);

  // NodeIrisRtcEngine* nodeIrisRtcEngine;
  // status =
  // napi_unwrap(env, jsthis, reinterpret_cast<void**>(&nodeIrisRtcEngine));

  // if (!nodeIrisRtcEngine->_iris_engine) {
  //     LOG_F(INFO, "NodeIrisRtcEngine::GetDeviceManager error Not Init
  //     Engine"); napi_value value; return value;
  // }
  // auto _device_manager = nodeIrisRtcEngine->_iris_engine->device_manager();
  // NodeIrisRtcDeviceManager::_staticDeviceManager = _device_manager;
  // napi_value _js_device_manager =
  // NodeIrisRtcDeviceManager::NewInstance(env);
  // return _js_device_manager;
}

Napi::Value NodeIrisRtcEngine::GetScreenWindowsInfo(
    const Napi::CallbackInfo& info) {
  auto env = info.Env();
  auto _allWindows = getAllWindowInfo();
  auto arr_obj = Napi::Array::New(env, _allWindows.size());
  for (auto i = 0; i < _allWindows.size(); i++) {
    auto obj = Napi::Object::New(env);
    auto _windowInfo = _allWindows[i];
#ifdef _WIN32
    UINT32 windowId = (UINT32)_windowInfo.windowId;
#elif defined(__APPLE__)
    unsigned int windowId = _windowInfo.windowId;
#endif

    obj.Set("windowId", windowId);
    obj.Set("name", _windowInfo.name);
    obj.Set("ownerName", _windowInfo.ownerName);
    obj.Set("isOnScreen", _windowInfo.isOnScreen);
    obj.Set("width", _windowInfo.width);
    obj.Set("height", _windowInfo.height);
    obj.Set("originWidth", _windowInfo.originWidth);
    obj.Set("originHeight", _windowInfo.originHeight);

    if (_windowInfo.imageData) {
      auto arr_obj = Napi::ArrayBuffer::New(env, _windowInfo.imageData,
                                            _windowInfo.imageDataLength);
      obj.Set("image", arr_obj);
      free(_windowInfo.imageData);
    }
    arr_obj[i] = obj;
  }
  return arr_obj;
}

Napi::Value NodeIrisRtcEngine::GetScreenDisplaysInfo(
    const Napi::CallbackInfo& info) {
  auto env = info.Env();
  auto _allDisplays = getAllDisplayInfo();
  auto arr_obj = Napi::Array::New(env, _allDisplays.size());
  for (auto i = 0; i < _allDisplays.size(); i++) {
    auto displayObj = Napi::Object::New(env);
    auto _displayInfo = _allDisplays[i];
    auto _displayId = _displayInfo.displayId;
    auto obj = Napi::Object::New(env);
#ifdef _WIN32
    obj.Set("x", _displayId.x);
    obj.Set("y", _displayId.y);
    obj.Set("width", _displayId.width);
    obj.Set("height", _displayId.height);
#elif defined(__APPLE__)
    obj.Set("id", _displayId.idVal);
#endif
    displayObj.Set("displayId", obj);
    displayObj.Set("width", _displayInfo.width);
    displayObj.Set("height", _displayInfo.height);
    displayObj.Set("isMain", _displayInfo.isMain);
    displayObj.Set("isActive", _displayInfo.isActive);
    displayObj.Set("isBuiltin", _displayInfo.isBuiltin);
    if (_displayInfo.imageData) {
      auto arr_obj = Napi::ArrayBuffer::New(env, _displayInfo.imageData,
                                            _displayInfo.imageDataLength);
      displayObj.Set("image", arr_obj);
      free(_displayInfo.imageData);
    }
    arr_obj[i] = displayObj;
  }
  return arr_obj;
}

Napi::Value NodeIrisRtcEngine::VideoSourceInitialize(
    const Napi::CallbackInfo& info) {
  char result[512];
  int ret = ERROR_PARAMETER_1;
  memset(result, '\0', 512);

  if (!this->_video_source_proxy) {
    this->_video_source_proxy.reset(
        new VideoSourceProxy(this->_video_processer));
  }

  if (this->_video_source_proxy) {
    if (this->_video_source_proxy->Initialize(this->_iris_event_handler))
      ret = ERROR_OK;
  } else {
    ret = ERROR_NOT_INIT;
    LOG_F(INFO, "VideoSourceInitialize NodeIris Engine Not Init");
  }

  RETURE_NAPI_OBJ();
}

Napi::Value NodeIrisRtcEngine::VideoSourceSetAddonLogFile(
    const Napi::CallbackInfo& info) {
  std::string parameter = "";
  napi_get_value_utf8_string(info[0], parameter);

  char result[512];
  int ret = ERROR_PARAMETER_1;
  memset(result, '\0', 512);
  if (this->_video_source_proxy) {
    if (this->_video_source_proxy->SetAddonLogFile(parameter.c_str()))
      ret = ERROR_OK;
  } else {
    ret = ERROR_NOT_INIT;
    LOG_F(INFO,
          "VideoSourceSetAddonLogFile did not initialize videoSource yet");
  }

  RETURE_NAPI_OBJ();
}

Napi::Value NodeIrisRtcEngine::VideoSourceRelease(
    const Napi::CallbackInfo& info) {
  char result[512];
  int ret = ERROR_PARAMETER_1;
  memset(result, '\0', 512);
  if (this->_video_source_proxy) {
    ret = this->VideoSourceRelease();
  } else {
    ret = ERROR_NOT_INIT;
    LOG_F(INFO, "VideoSourceInitialize NodeIris Engine Not Init");
  }

  RETURE_NAPI_OBJ();
}

int NodeIrisRtcEngine::VideoSourceRelease() {
  LOG_F(INFO, "VideoSourceRelease");
  if (_video_source_proxy) {
    _video_source_proxy.reset();
  }
  return ERROR_OK;
}

Napi::Value NodeIrisRtcEngine::SetAddonLogFile(const Napi::CallbackInfo& info) {
  int process_type = 0;
  std::string file_path = "";
  napi_get_value_int32(info[0], process_type);
  napi_get_value_utf8_string(info[1], file_path);

  char result[512];
  int ret = ERROR_PARAMETER_1;
  memset(result, '\0', 512);

  if (process_type == PROCESS_TYPE::MAIN) {
    ret = startLogService(file_path.c_str());
  } else {
  }
  RETURE_NAPI_OBJ();
}

void NodeIrisRtcEngine::OnApiError(const char* errorMessage) {
  _iris_event_handler->OnEvent("onApiError", errorMessage);
}

Napi::Value NodeIrisRtcEngine::PluginCallApi(const Napi::CallbackInfo& info) {
  int process_type = 0;
  int api_type = 0;
  std::string parameter = "";
  napi_get_value_int32(info[0], process_type);
  napi_get_value_int32(info[1], api_type);
  napi_get_value_utf8_string(info[2], parameter);
  char result[512];
  memset(result, '\0', 512);
  LOG_F(INFO, "CallApi parameter: %s", parameter.c_str());
  int ret = ERROR_PARAMETER_1;

  if (this->_iris_engine) {
    try {
      if (process_type == PROCESS_TYPE::MAIN) {
        ret = this->_iris_raw_data_plugin_manager->CallApi(
            (ApiTypeRawDataPluginManager)api_type, parameter.c_str(), result);
      } else {
        if (this->_video_source_proxy) {
          ret = this->_video_source_proxy->PluginCallApi(
              (ApiTypeRawDataPluginManager)api_type, parameter.c_str(), result);
        } else {
          LOG_F(INFO,
                "PluginCallApi parameter did not initialize videoSource yet "
                "source yet");
        }
      }
    } catch (std::exception& e) {
      LOG_F(INFO, "PluginCallApi catch exception %s", e.what());
      this->OnApiError(e.what());
    }
  } else {
    ret = ERROR_NOT_INIT;
    LOG_F(INFO, "VideoSourceInitialize NodeIris Engine Not Init");
  }
  RETURE_NAPI_OBJ();
}

Napi::Value NodeIrisRtcEngine::EnableVideoFrameCache(
    const Napi::CallbackInfo& info) {
  int process_type = 0;
  Napi::Object frame_obj;
  napi_get_value_int32(info[0], process_type);
  napi_get_value_obj(info[1], frame_obj);

  unsigned int uid = 0;
  std::string channelId = "";
  int width = 0;
  int height = 0;

  napi_get_obj_uint32(frame_obj, "uid", uid);
  napi_get_obj_utf8_string(frame_obj, "channelId", channelId);
  napi_get_obj_int32(frame_obj, "width", width);
  napi_get_obj_int32(frame_obj, "height", height);

  char result[512];
  memset(result, '\0', 512);
  int ret = ERROR_PARAMETER_1;

  if (this->_iris_engine) {
    IrisRtcRendererCacheConfig config(VideoFrameType::kVideoFrameTypeYUV420,
                                      nullptr, width, height);
    try {
      if (process_type == PROCESS_TYPE::MAIN) {
        ret = this->_video_processer->EnableVideoFrameCache(config, uid,
                                                            channelId.c_str());
      } else {
        if (this->_video_source_proxy) {
          ret = this->_video_source_proxy->EnableVideoFrameCache(
              channelId.c_str(), uid, width, height);
        }
      }
    } catch (std::exception& e) {
      LOG_F(INFO, "PluginCallApi catch exception %s", e.what());
      this->OnApiError(e.what());
    }
  } else {
    ret = ERROR_NOT_INIT;
    LOG_F(INFO, "VideoSourceInitialize NodeIris Engine Not Init");
  }

  RETURE_NAPI_OBJ();
}

Napi::Value NodeIrisRtcEngine::DisableVideoFrameCache(
    const Napi::CallbackInfo& info) {
  int process_type = 0;
  Napi::Object frame_obj;

  napi_get_value_int32(info[0], process_type);
  napi_get_value_obj(info[1], frame_obj);

  unsigned int uid = 0;
  std::string channelId = "";

  napi_get_obj_uint32(frame_obj, "uid", uid);
  napi_get_obj_utf8_string(frame_obj, "channelId", channelId);

  char result[512];
  memset(result, '\0', 512);
  int ret = ERROR_PARAMETER_1;

  if (this->_iris_engine) {
    try {
      if (process_type == PROCESS_TYPE::MAIN) {
        ret = this->_video_processer->DisableVideoFrameCache(channelId.c_str(),
                                                             uid);
      } else {
        if (this->_video_source_proxy) {
          ret = this->_video_source_proxy->DisableVideoFrameCache(
              channelId.c_str(), uid);
        }
      }
    } catch (std::exception& e) {
      this->OnApiError(e.what());
    }
  } else {
    ret = ERROR_NOT_INIT;
    LOG_F(INFO, "VideoSourceInitialize NodeIris Engine Not Init");
  }

  RETURE_NAPI_OBJ();
}

Napi::Value NodeIrisRtcEngine::GetVideoStreamData(
    const Napi::CallbackInfo& info) {
  auto env = info.Env();
  int process_type = 0;
  Napi::Object obj;

  napi_get_value_int32(info[0], process_type);
  napi_get_value_obj(info[1], obj);

  unsigned int uid;
  std::string channel_id;
  Napi::Object y_buffer_obj;
  void* y_buffer;
  int y_length;
  Napi::Object u_buffer_obj;
  void* u_buffer;
  int u_length;
  Napi::Object v_buffer_obj;
  void* v_buffer;
  int v_length;

  int height;
  int y_stride;

  napi_get_obj_uint32(obj, "uid", uid);
  napi_get_obj_utf8_string(obj, "channelId", channel_id);

  napi_get_obj_obj(obj, "yBuffer", y_buffer_obj);
  napi_get_obj_node_buffer(y_buffer_obj, y_buffer, y_length);

  napi_get_obj_obj(obj, "uBuffer", u_buffer_obj);
  napi_get_obj_node_buffer(u_buffer_obj, u_buffer, u_length);

  napi_get_obj_obj(obj, "vBuffer", v_buffer_obj);
  napi_get_obj_node_buffer(v_buffer_obj, v_buffer, v_length);

  napi_get_obj_int32(obj, "height", height);
  napi_get_obj_int32(obj, "yStride", y_stride);

  IrisRtcVideoFrame _videoFrame = IrisRtcVideoFrame_default;
  _videoFrame.y_buffer = y_buffer;
  _videoFrame.u_buffer = u_buffer;
  _videoFrame.v_buffer = v_buffer;
  _videoFrame.height = height;
  _videoFrame.y_stride = y_stride;

  bool isFresh = false;
  bool ret = false;

  if (this->_iris_engine) {
    try {
      if (process_type == PROCESS_TYPE::MAIN) {
        ret = this->_video_processer->GetVideoFrame(_videoFrame, isFresh, uid,
                                                    channel_id.c_str());
      } else {
        ret = this->_video_processer->VideoSourceGetVideoFrame(
            _videoFrame, isFresh, uid, channel_id.c_str());
      }
    } catch (std::exception& e) {
      this->OnApiError(e.what());
    }
  } else {
    ret = false;
    LOG_F(INFO, "VideoSourceInitialize NodeIris Engine Not Init");
  }
  unsigned int rotation = 0;
  auto ret_obj = Napi::Object::New(env);
  ret_obj.Set("ret", ret);
  ret_obj.Set("isNewFrame", isFresh);
  ret_obj.Set("width", _videoFrame.width);
  ret_obj.Set("height", _videoFrame.height);
  ret_obj.Set("yStride", _videoFrame.y_stride);
  ret_obj.Set("rotation", rotation);
  ret_obj.Set("timestamp", _videoFrame.render_time_ms);
  return ret_obj;
}

Napi::Value NodeIrisRtcEngine::Release(const Napi::CallbackInfo& info) {
  auto env = info.Env();
  if (this->_iris_engine) {
    this->_video_processer.reset();
    this->_iris_event_handler.reset();
    this->_iris_raw_data_plugin_manager = nullptr;
    this->_iris_raw_data = nullptr;
    this->_iris_engine.reset();
    this->_video_source_proxy.reset();
    LOG_F(INFO, "NodeIrisRtcEngine::Release done");
  } else {
    LOG_F(INFO, "VideoSourceInitialize NodeIris Engine Not Init");
  }
  auto ret_value = env.Null();
  return ret_value;
}
}  // namespace electron
}  // namespace rtc
}  // namespace agora
