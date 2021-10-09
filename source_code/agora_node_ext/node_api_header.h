/*
 * @Author: zhangtao@agora.io
 * @Date: 2021-04-22 20:53:01
 * @Last Modified by: zhangtao@agora.io
 * @Last Modified time: 2021-10-10 00:30:11
 */
#pragma once
#include <node_buffer.h>
#include <memory>
#include <string>
#include <vector>
#include "node_base.h"

namespace agora {
namespace rtc {
namespace electron {
void napi_get_value_float(const Napi::Value& value, float& result);

void napi_get_value_double(const Napi::Value& value, double& result);

void napi_get_value_int32(const Napi::Value& value, int& result);

void napi_get_value_uint32(const Napi::Value& value, unsigned int& result);

void napi_get_value_utf8_string(const Napi::Value& value, std::string& result);

void napi_get_value_bool(const Napi::Value& value, bool& result);

void napi_get_value_obj(const Napi::Value& value, Napi::Object& result);

void napi_get_value_node_buffer(const Napi::Value& value, void*& data, int& length);

void napi_get_value_function(const Napi::Value& value, Napi::FunctionReference& result);

void napi_get_obj_int32(const Napi::Object& obj, const char *key, int& result);

void napi_get_obj_utf8_string(const Napi::Object& obj, const char *key, std::string& result);

void napi_get_obj_uint32(const Napi::Object& obj, const char *key, unsigned int& result);

void napi_get_obj_obj(const Napi::Object& obj, const char *key, Napi::Object& result);

void napi_get_obj_node_buffer(const Napi::Object& obj, void*& data, int& length);
}  // namespace electron
}  // namespace rtc
}  // namespace agora
