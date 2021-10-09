/*
 * @Author: zhangtao@agora.io
 * @Date: 2021-04-22 20:53:49
 * @Last Modified by: zhangtao@agora.io
 * @Last Modified time: 2021-10-09 20:30:27
 */
#include "node_iris_event_handler.h"
#include <memory.h>
#include <string>
#include "node_iris_rtc_engine.h"

namespace agora {
namespace rtc {
namespace electron {
NodeIrisEventHandler::NodeIrisEventHandler(NodeIrisRtcEngine* engine)
    : _node_iris_engine(engine) {
  node_async_call::close(false);
}

NodeIrisEventHandler::~NodeIrisEventHandler() {
  node_async_call::close(true);
  _callbacks.clear();
  _node_iris_engine = nullptr;
}

void NodeIrisEventHandler::addEvent(const std::string& eventName,
                                    Napi::FunctionReference function) {
  auto callback = new EventCallback();
  callback->function = function;
  _callbacks[eventName] = callback;
}

void NodeIrisEventHandler::OnEvent(const char* event, const char* data) {
  std::string _eventName(event);
  std::string _eventData(data);
  node_async_call::async_call([this, _eventName, _eventData] {
    auto it = _callbacks.find("call_back");
    if (it != _callbacks.end()) {
      auto function_reference = it->second;
      auto env = function_reference.Env();
      auto param1 = Napi::String::New(env, event);
      auto param2 = Napi::String::New(env, data);
      std::vector<Napi::Value> args = {param1, param2};
      function_reference.Call(args);
    }
  });
}

void NodeIrisEventHandler::OnEvent(const char* event,
                                   const char* data,
                                   const void* buffer,
                                   unsigned int length) {
  std::string _eventName(event);
  std::string _eventData(data);
  std::vector<unsigned char> vec_buffer;
  vec_buffer.resize(length);
  memcpy(vec_buffer.data(), buffer, length);
  
  node_async_call::async_call([this, _eventName, _eventData, vec_buffer, length] {
    auto it = _callbacks.find("call_back_with_buffer");
    if (it != _callbacks.end()) {
      auto function_reference = it->second;
      auto env = function_reference.Env();
      auto param1 = Napi::String::New(env, _eventName);
      auto param2 = Napi::String::New(env, _eventData);
      auto param3 = Napi::ArrayBuffer::New(env, vec_buffer.data(), length);
      std::vector<Napi::Value> args = {param1, param2, param3};
      function_reference.Call(args);
    }
  });
}

void NodeIrisEventHandler::OnVideoSourceEvent(const char* eventName,
                                              const char* eventData) {
  std::string _eventName(eventName);
  std::string _eventData(eventData);
  node_async_call::async_call([this, _eventName, _eventData] {
    auto it = _callbacks.find("video_source_call_back");
    if (it != _callbacks.end()) {
      auto function_reference = it->second;
      auto env = function_reference.Env();
      auto param1 = Napi::String::New(env, _eventName);
      auto param2 = Napi::String::New(env, _eventData);
      std::vector<Napi::Value> args = {param1, param2};
      function_reference.Call(args);
    }
  });
}

void NodeIrisEventHandler::OnVideoSourceEvent(const char* event,
                                              const char* data,
                                              const char* buffer,
                                              int length) {
  std::string _eventName(event);
  std::string _eventData(data);
  std::vector<unsigned char> vec_buffer;
  vec_buffer.resize(length);
  memcpy(vec_buffer.data(), buffer, length);
  
  node_async_call::async_call([this, _eventName, _eventData, vec_buffer, length] {
    auto it = _callbacks.find("video_source_call_back_with_buffer");
    if (it != _callbacks.end()) {
      auto function_reference = it->second;
      auto env = function_reference.Env();
      auto param1 = Napi::String::New(env, _eventName);
      auto param2 = Napi::String::New(env, _eventData);
      auto param3 = Napi::ArrayBuffer::New(env, vec_buffer.data(), length);
      std::vector<Napi::Value> args = {param1, param2, param3};
      function_reference.Call(args);
    }
  });
}

void NodeIrisEventHandler::OnVideoSourceExit() {
  LOG_F(INFO, "NodeIrisEventHandler::OnVideoSourceExit");
  _node_iris_engine->VideoSourceRelease();
}
}  // namespace electron
}  // namespace rtc
}  // namespace agora