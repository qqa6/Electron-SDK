#include "node_api_header.h"

namespace agora {
namespace rtc {
namespace electron {

void napi_get_value_float(const Napi::Value& value, float& result) {
  result = value.As<Napi::Number>().FloatValue();
}

void napi_get_value_double(const Napi::Value& value, double& result) {
  result = value.As<Napi::Number>().DoubleValue();
}

void napi_get_value_int32(const Napi::Value& value, int& result) {
  result = value.As<Napi::Number>().Int32Value();
}

void napi_get_value_uint32(const Napi::Value& value, unsigned int& result) {
  result = value.As<Napi::Number>().Uint32Value();
}

void napi_get_value_bool(const Napi::Value& value, bool& result) {
  result = value.As<Napi::Boolean>().Value();
}

void napi_get_value_utf8_string(const Napi::Value& value, std::string& result) {
  result = value.As<Napi::String>().Utf8Value();
}

void napi_get_value_obj(const Napi::Value& value, Napi::Object& result) {
  result = value.As<Napi::Object>();
}

void napi_get_value_node_buffer(const Napi::Value& value,
                                void*& data,
                                int& length) {
  data = value.As<Napi::Buffer<unsigned char>>().Data();
  length = value.As<Napi::Buffer<unsigned char>>().Length();
}

void napi_get_value_function(const Napi::Value& value,
                             Napi::FunctionReference& result) {
  result = Napi::Persistent(value.As<Napi::Function>());
}

void napi_get_obj_int32(const Napi::Object& obj, const char* key, int& result) {
  napi_get_value_int32(obj[key], result);
}

void napi_get_obj_utf8_string(const Napi::Object& obj,
                              const char* key,
                              std::string& result) {
  napi_get_value_utf8_string(obj[key], result);
}

void napi_get_obj_uint32(const Napi::Object& obj,
                         const char* key,
                         unsigned int& result) {
  napi_get_value_uint32(obj[key], result);
}

void napi_get_obj_obj(const Napi::Object& obj,
                      const char* key,
                      Napi::Object& result) {
  napi_get_value_obj(obj[key], result);
}

void napi_get_obj_node_buffer(const Napi::Object& obj,
                              void*& data,
                              int& length) {
  napi_get_value_node_buffer(obj, data, length);
}
}  // namespace electron
}  // namespace rtc
}  // namespace agora
