/*
* Copyright (c) 2017 Agora.io
* All rights reserved.
* Proprietry and Confidential -- Agora.io
*/

/*
*  Created by Wang Yongli, 2017
*/

#include "node_napi_api.h"
#include "node_uid.h"
#include "node_log.h"
#include "node_video_stream_channel.h"
#include "node_async_queue.h"
#include "libyuv.h"
#include <chrono>
#include <string>
using namespace libyuv;
NodeVideoFrameTransporter g_transport;

NodeVideoFrameTransporter* getNodeVideoFrameTransporter()
{
    return &g_transport;
}

NodeVideoFrameTransporter::NodeVideoFrameTransporter()
: init(false)
, env(nullptr)
, m_FPS(10)
{
    
}

NodeVideoFrameTransporter::~NodeVideoFrameTransporter()
{
    deinitialize();
}

bool NodeVideoFrameTransporter::initialize(v8::Isolate *isolate, const Nan::FunctionCallbackInfo<v8::Value> &callbackinfo)
{
    if (init) {
        deinitialize();
    }
    m_stopFlag = false;
    env = isolate;
    callback.Reset(callbackinfo[0].As<Function>());
    js_this.Reset(callbackinfo.This());
    m_thread.reset(new std::thread(&NodeVideoFrameTransporter::FlushVideo, this));
    init = true;
    return true;
}

bool NodeVideoFrameTransporter::deinitialize()
{
    if (!init)
        return true;
    m_stopFlag = 1;
    if (m_thread->joinable())
        m_thread->join();
    init = false;
    m_thread.reset();
    env = nullptr;
    callback.Reset();
    js_this.Reset();
    return true;
}

int NodeVideoFrameTransporter::setVideoDimension(NodeRenderType type, agora::rtc::uid_t uid, std::string channelId, uint32_t width, uint32_t height)
{
    if (!init)
        return -1;
    std::lock_guard<std::mutex> lck(m_lock);
    VideoFrameInfo& info = getVideoFrameInfo(type, uid, channelId);
    info.m_destWidth = width;
    info.m_destHeight = height;
    return 0;
}

VideoFrameInfo& NodeVideoFrameTransporter::getVideoFrameInfo(NodeRenderType type, agora::rtc::uid_t uid, std::string channelId)
{
    if (type == NodeRenderType::NODE_RENDER_TYPE_LOCAL) {
        if (!m_localVideoFrame.get())
            m_localVideoFrame.reset(new VideoFrameInfo(NODE_RENDER_TYPE_LOCAL));
        return *m_localVideoFrame.get();
    }
    else if (type == NODE_RENDER_TYPE_REMOTE) {
        auto cit = m_remoteVideoFrames.find(channelId);
        if(cit == m_remoteVideoFrames.end()){
            m_remoteVideoFrames[channelId] = std::unordered_map<agora::rtc::uid_t, VideoFrameInfo>();
        }
        auto it = m_remoteVideoFrames[channelId].find(uid);
        if (it == m_remoteVideoFrames[channelId].end()) 
            m_remoteVideoFrames[channelId][uid] = VideoFrameInfo(NODE_RENDER_TYPE_REMOTE, uid, channelId);

        return m_remoteVideoFrames[channelId][uid];
    }
    else if (type == NODE_RENDER_TYPE_DEVICE_TEST) {
        if (!m_devTestVideoFrame.get())
            m_devTestVideoFrame.reset(new VideoFrameInfo(NODE_RENDER_TYPE_DEVICE_TEST));
        return *m_devTestVideoFrame.get();
    }
    else {
        if (!m_videoSourceVideoFrame.get())
            m_videoSourceVideoFrame.reset(new VideoFrameInfo(NODE_RENDER_TYPE_VIDEO_SOURCE));
        return *m_videoSourceVideoFrame.get();
    }
}

void NodeVideoFrameTransporter::updateVideoBuffer(unsigned char* ybuffer, unsigned char* ubuffer, unsigned char* vbuffer, unsigned char* hbuffer, NodeRenderType type, agora::rtc::uid_t uid, std::string channelId)
{
    LOG_F(INFO, "updateVideoBuffer ");
    VideoFrameInfo&  v = getVideoFrameInfo(type, uid, channelId);
    v.m_ybuffer = ybuffer;
    v.m_ubuffer = ubuffer;
    v.m_vbuffer = vbuffer;
    v.m_headbuffer = hbuffer;
    v.m_videoSizeChanged = false;
}

void NodeVideoFrameTransporter::clearVideoBuffer(NodeRenderType type, agora::rtc::uid_t uid, std::string channelId)
{
    std::lock_guard<std::mutex> lck(m_lock);
    LOG_F(INFO, "clearVideoBuffer ");
    VideoFrameInfo&  v = getVideoFrameInfo(type, uid, channelId);
    v.m_ybuffer = nullptr;
    v.m_ubuffer = nullptr;
    v.m_vbuffer = nullptr;
    v.m_headbuffer = nullptr;
    v.m_videoSizeChanged = false;
}

int NodeVideoFrameTransporter::deliverVideoSourceFrame(const char* payload, int len)
{
    if (!init)
        return -1;
    image_frame_info *info = (image_frame_info*)payload;
    image_header_type *hdr = (image_header_type*)(payload + sizeof(image_frame_info));
    unsigned int uv_len = (info->stride / 2) * (info->height / 2);
    char* y = (char*)(hdr + 1);
    char* u = y + uv_len * 4;
    char* v = u + uv_len;
    std::lock_guard<std::mutex> lck(m_lock);

    VideoFrameInfo& videoInfo = getVideoFrameInfo(NODE_RENDER_TYPE_VIDEO_SOURCE, 0, "");
    if (videoInfo.m_videoSizeChanged) {
        return 0;
    }
    int destWidth = videoInfo.m_destWidth ? videoInfo.m_destWidth : info->width;
    int destHeight = videoInfo.m_destHeight ? videoInfo.m_destHeight : info->height;

    if (destWidth != videoInfo.m_videoStride || destWidth != videoInfo.m_videoWidth || destHeight != videoInfo.m_videoHeight) {
        videoInfo.m_videoSizeChanged = true;
        videoInfo.m_videoStride = destWidth;
        videoInfo.m_videoWidth = destWidth;
        videoInfo.m_videoHeight = destHeight;
        videoInfo.m_needUpdate = true;
        return 0;
    }

    if (videoInfo.m_ybuffer == nullptr || videoInfo.m_ubuffer == nullptr || videoInfo.m_vbuffer == nullptr || videoInfo.m_headbuffer == nullptr) {
        return 0;
    }

    image_header_type *localHdr = (image_header_type*)videoInfo.m_headbuffer;
    localHdr->format = hdr->format;
    localHdr->mirrored = hdr->mirrored;
    localHdr->timestamp = htons(hdr->timestamp);
    localHdr->rotation = htons(hdr->rotation);
    localHdr->width = htons(destWidth);
    localHdr->height = htons(destHeight);
    localHdr->left = htons(0);
    localHdr->right = htons(0);
    localHdr->top = htons(0);
    localHdr->bottom = htons(0);

    I420Scale((const uint8*)y, info->stride0, (const uint8*)u, info->strideU, (const uint8*)v, info->strideV, info->width, info->height, 
        (uint8*)videoInfo.m_ybuffer, destWidth, (uint8*)videoInfo.m_ubuffer, destWidth / 2, (uint8*)videoInfo.m_vbuffer, destWidth / 2, destWidth, destHeight, kFilterNone);

    videoInfo.m_count = 0;
    videoInfo.m_needUpdate = true;
    return 0;
}

int NodeVideoFrameTransporter::deliverFrame_I420(NodeRenderType type, agora::rtc::uid_t uid, std::string channelId, const agora::media::IVideoFrame& videoFrame, int rotation, bool mirrored)
{
    if (!init)
        return -1;
    int srcStride = videoFrame.stride(IVideoFrame::Y_PLANE);
    int stride = srcStride;
    if (stride & 0xf) {
        stride = (((stride + 15) >> 4) << 4);
    }
    rotation = rotation < 0 ? rotation + 360 : rotation;
    std::lock_guard<std::mutex> lck(m_lock);
    VideoFrameInfo& info = getVideoFrameInfo(type, uid, channelId);
    if (info.m_videoSizeChanged) {
        return 0;
    }
    int destStride = info.m_destWidth ? info.m_destWidth : stride;
    int destWidth = info.m_destWidth ? info.m_destWidth : videoFrame.width();
    int destHeight = info.m_destHeight ? info.m_destHeight : videoFrame.height();
    if (destStride != info.m_videoStride || destWidth != info.m_videoWidth || destHeight != info.m_videoHeight) {
        info.m_videoSizeChanged = true;
        info.m_videoStride = destStride;
        info.m_videoWidth = destWidth;
        info.m_videoHeight = destHeight;
        info.m_needUpdate = true;
        return 0;
    }

    if (info.m_ybuffer == nullptr || info.m_ubuffer == nullptr || info.m_vbuffer == nullptr || info.m_headbuffer == nullptr) {
        return 0;
    }

    image_header_type* hdr = reinterpret_cast<image_header_type*>(info.m_headbuffer);
    hdr->mirrored = mirrored ? 1 : 0;
    hdr->rotation = htons(rotation);
    setupFrameHeader(hdr, destStride, destWidth, destHeight);
    
    copyFrame(videoFrame, info);
    info.m_count = 0;
    info.m_needUpdate = true;
    return 0;
    
}

void NodeVideoFrameTransporter::setupFrameHeader(image_header_type*header, int stride, int width, int height)
{
    int left = (stride - width) / 2;
    int top = 0;
    header->format = 0;
    header->width = htons(width);
    header->height = htons(height);
    header->left = htons((uint16_t)left);
    header->top = htons((uint16_t)top);
    header->right = htons((uint16_t)(stride - width - left));
    header->bottom = htons((uint16_t)0);
    header->timestamp = 0;
}

void NodeVideoFrameTransporter::copyFrame(const agora::media::IVideoFrame& videoFrame, VideoFrameInfo& info)
{
    int width2 = info.m_videoStride / 2, heigh2 = info.m_videoHeight / 2;
    int strideY = videoFrame.stride(IVideoFrame::Y_PLANE);
    int strideU = videoFrame.stride(IVideoFrame::U_PLANE);
    int strideV = videoFrame.stride(IVideoFrame::V_PLANE);

    const unsigned char* planeY = videoFrame.buffer(IVideoFrame::Y_PLANE);
    const unsigned char* planeU = videoFrame.buffer(IVideoFrame::U_PLANE);
    const unsigned char* planeV = videoFrame.buffer(IVideoFrame::V_PLANE);

    if (videoFrame.width() == info.m_videoWidth && videoFrame.height() == info.m_videoHeight)
    {
        copyAndCentreYuv(planeY, planeU, planeV, videoFrame.width(), videoFrame.height(), strideY, info.m_ybuffer, info.m_ubuffer, info.m_vbuffer, info.m_videoStride);
    }
    else
    {
        I420Scale(planeY, strideY, planeU, strideU, planeV, strideV, videoFrame.width(), videoFrame.height(), 
        (uint8*)info.m_ybuffer, info.m_videoStride, (uint8*)info.m_ubuffer, info.m_videoStride/2, (uint8*)info.m_vbuffer, info.m_videoStride/2, info.m_videoWidth, info.m_videoHeight, kFilterNone);
    }
}

void NodeVideoFrameTransporter::copyAndCentreYuv(const unsigned char* srcYPlane, const unsigned char* srcUPlane, const unsigned char* srcVPlane, int width, int height, int srcStride,
unsigned char* dstYPlane, unsigned char* dstUPlane, unsigned char* dstVPlane, int dstStride)
{
    if (srcStride == width && dstStride == width)
    {
        memcpy(dstYPlane, srcYPlane, width * height);
        memcpy(dstUPlane, srcUPlane, width * height / 4);
        memcpy(dstVPlane, srcVPlane, width * height / 4);
        return;
    }

    int dstDiff = dstStride - width;
    //RGB(0,0,0) to YUV(0,128,128)
    memset(dstYPlane, 0, dstStride * height);
    memset(dstUPlane, 128, dstStride * height / 4);
    memset(dstVPlane, 128, dstStride * height / 4);

    for (int i = 0; i < height; ++i)
    {
        memcpy(dstYPlane + (dstDiff >> 1), srcYPlane, width);
        srcYPlane += srcStride;
        dstYPlane += dstStride;

        if (i % 2 == 0)
        {
            memcpy(dstUPlane + (dstDiff >> 2), srcUPlane, width >> 1);
            srcUPlane += srcStride >> 1;
            dstUPlane += dstStride >> 1;

            memcpy(dstVPlane + (dstDiff >> 2), srcVPlane, width >> 1);
            srcVPlane += srcStride >> 1;
            dstVPlane += dstStride >> 1;
        }
    }
}

void NodeVideoFrameTransporter::addToHighVideo(agora::rtc::uid_t uid, std::string channelId)
{

}

void NodeVideoFrameTransporter::removeFromeHighVideo(agora::rtc::uid_t uid, std::string channelId)
{

}

void NodeVideoFrameTransporter::setHighFPS(uint32_t fps)
{

}

void NodeVideoFrameTransporter::setFPS(uint32_t fps)
{
    if(fps == 0)
        return;
    std::lock_guard<std::mutex> lck(m_lock);
    m_FPS = fps;
}

#define NODE_SET_OBJ_PROP_UINT32(obj, name, val) \
    { \
        Local<Value> propName = String::NewFromUtf8(isolate, name, NewStringType::kInternalized).ToLocalChecked(); \
        Local<Value> propVal = napi_create_uint32_(isolate, val); \
        v8::Maybe<bool> ret = obj->Set(isolate->GetCurrentContext(), propName, propVal); \
        if(!ret.IsNothing()) { \
            if(!ret.ToChecked()) { \
                break; \
            } \
        } \
    }
#define NODE_SET_OBJ_PROP_BOOL(obj, name, val) \
    { \
        Local<Value> propName = String::NewFromUtf8(isolate, name, NewStringType::kInternalized).ToLocalChecked(); \
        Local<Value> propVal = napi_create_bool_(isolate, val); \
        v8::Maybe<bool> ret = obj->Set(isolate->GetCurrentContext(), propName, propVal); \
        if(!ret.IsNothing()) { \
            if(!ret.ToChecked()) { \
                break; \
            } \
        } \
    }
#define NODE_SET_OBJ_PROP_STRING(obj, name, val) \
    { \
        Local<Value> propName = String::NewFromUtf8(isolate, name, NewStringType::kInternalized).ToLocalChecked(); \
        Local<Value> propVal = napi_create_string_(isolate, val); \
        v8::Maybe<bool> ret = obj->Set(isolate->GetCurrentContext(), propName, propVal); \
        if(!ret.IsNothing()) { \
            if(!ret.ToChecked()) { \
                break; \
            } \
        } \
    }
#define NODE_SET_OBJ_PROP_HEADER(obj, it) \
    { \
        Local<Value> propName = String::NewFromUtf8(isolate, "header", NewStringType::kInternalized).ToLocalChecked(); \
        Local<v8::ArrayBuffer> buff = v8::ArrayBuffer::New(isolate, (it)->buffer, (it)->length); \
        v8::Maybe<bool> ret = obj->Set(isolate->GetCurrentContext(), propName, buff); \
        if(!ret.IsNothing()) { \
            if(!ret.ToChecked()) { \
                break; \
            } \
        } \
    }

#define NODE_SET_OBJ_PROP_DATA(obj, name, it) \
    { \
        Local<Value> propName = String::NewFromUtf8(isolate, name, NewStringType::kInternalized).ToLocalChecked(); \
        Local<v8::ArrayBuffer> buff = v8::ArrayBuffer::New(isolate, (it)->buffer, (it)->length); \
        Local<v8::Uint8Array> dataarray = v8::Uint8Array::New(buff, 0, it->length);\
        v8::Maybe<bool> ret = obj->Set(isolate->GetCurrentContext(), propName, dataarray); \
        if(!ret.IsNothing()) { \
            if(!ret.ToChecked()) { \
                break; \
            } \
        } \
    }

bool AddObj(Isolate* isolate, Local<v8::Array>& infos, int index, VideoFrameInfo& info)
{
    if (!info.m_needUpdate)
        return false;
    info.m_needUpdate = false;
    bool result = false;
    do {
        Local<v8::Object> obj = Object::New(isolate);
        NODE_SET_OBJ_PROP_UINT32(obj, "type", info.m_renderType);
        NODE_SET_OBJ_PROP_UINT32(obj, "uid", info.m_uid);
        NODE_SET_OBJ_PROP_UINT32(obj, "width", info.m_videoWidth);
        NODE_SET_OBJ_PROP_UINT32(obj, "height", info.m_videoHeight);
        NODE_SET_OBJ_PROP_UINT32(obj, "stride", info.m_videoStride);
        NODE_SET_OBJ_PROP_BOOL(obj, "videoSizeChanged", info.m_videoSizeChanged);
        NODE_SET_OBJ_PROP_STRING(obj, "channelId", info.m_channelId.c_str());

        result = infos->Set(isolate->GetCurrentContext(), index, obj).FromJust();
    }while(false);
    return result;
}

void NodeVideoFrameTransporter::FlushVideo()
{
    while (!m_stopFlag) {
        {
            std::unique_lock<std::mutex> lck(m_lock);
            for (auto cit = m_remoteVideoFrames.begin(); cit != m_remoteVideoFrames.end();) {
                for (auto it = cit->second.begin(); it != cit->second.end();) {
                    if (it->second.m_count > MAX_MISS_COUNT)
                        it = cit->second.erase(it);
                    else
                        ++it;
                }
                ++cit;
            }

            if (m_devTestVideoFrame.get() && m_localVideoFrame && m_localVideoFrame->m_count > MAX_MISS_COUNT) {
                m_devTestVideoFrame.reset();
            }
            lck.unlock();

            agora::rtc::node_async_call::async_call([this]() {
                Isolate *isolate = env;
                HandleScope scope(isolate);
                std::lock_guard<std::mutex> lock(m_lock);
                Local<v8::Array> infos = v8::Array::New(isolate);

                uint32_t i = 0;
                for (auto& cit : m_remoteVideoFrames) {
                    for (auto& it : cit.second) {
                        if (AddObj(isolate, infos, i, it.second))
                            ++i;
                        else {
                            ++it.second.m_count;
                        }
                    }
                }
                if (m_localVideoFrame.get()) {
                    if (AddObj(isolate, infos, i, *m_localVideoFrame.get()))
                        ++i;
                    else {
                        ++m_localVideoFrame->m_count;
                    }
                }

                if (m_videoSourceVideoFrame.get()) {
                    if (AddObj(isolate, infos, i, *m_videoSourceVideoFrame.get()))
                        ++i;
                    else {
                        ++m_videoSourceVideoFrame->m_count;
                    }
                }

                if (m_devTestVideoFrame.get()) {
                    if (AddObj(isolate, infos, i, *m_devTestVideoFrame.get()))
                        ++i;
                    else {
                        ++m_devTestVideoFrame->m_count;
                    }
                }
                if (i > 0) {
                    Local<v8::Value> args[1] = { infos };
                    callback.Get(isolate)->Call(isolate->GetCurrentContext(), js_this.Get(isolate), 1, args);
                }
            });
            std::this_thread::sleep_for(std::chrono::milliseconds(1000 / m_FPS));
        }
    }
}

int napi_get_value_string_utf8_(const Local<Value>& str, char *buffer, uint32_t len)
{
    Isolate* isolate = Isolate::GetCurrent();
    if(!isolate) {
        return 0;
    }
    Local<Context> context = isolate->GetCurrentContext();
    if (!str->IsString())
        return 0;
    if (!buffer) {
#if NODE_MAJOR_VERSION <= 10
        return str.As<String>()->Utf8Length();
#else
        return str.As<String>()->Utf8Length(isolate);
#endif
    }
    else {
#if NODE_MAJOR_VERSION <= 10
        int copied = str.As<String>()->WriteUtf8(buffer, len - 1, nullptr, String::REPLACE_INVALID_UTF8 | String::NO_NULL_TERMINATION);
#else
        int copied = str.As<String>()->WriteUtf8(isolate, buffer, len - 1, nullptr, String::REPLACE_INVALID_UTF8 | String::NO_NULL_TERMINATION);
#endif
        
        buffer[copied] = '\0';
        return copied;
    }
}

napi_status napi_get_value_uid_t_(const Local<Value>& value, agora::rtc::uid_t& result)
{
    return agora::rtc::NodeUid::getUidFromNodeValue(value, result);
}

napi_status napi_get_value_uint32_(const Local<Value>& value, uint32_t& result)
{
    if (!value->IsUint32())
        return napi_invalid_arg;
    result = Nan::To<v8::Uint32>(value).ToLocalChecked()->Value();
    return napi_ok;
}

napi_status napi_get_value_bool_(const Local<Value>& value, bool& result)
{
    if(!value->IsBoolean())
        return napi_invalid_arg;
    
    result = Nan::To<v8::Boolean>(value).ToLocalChecked()->Value();
    return napi_ok;
}
	
napi_status napi_get_value_int32_(const Local<Value>& value, int32_t& result)
{
    if (!value->IsInt32())
        return napi_invalid_arg;

    result = Nan::To<v8::Int32>(value).ToLocalChecked()->Value();
    return napi_ok;
}

napi_status napi_get_value_double_(const Local<Value>& value, double &result)
{
    if (!value->IsNumber())
        return napi_invalid_arg;

    result = Nan::To<v8::Number>(value).ToLocalChecked()->Value();
    return napi_ok;
}

napi_status napi_get_value_int64_(const Local<Value>& value, int64_t& result)
{
    int32_t tmp;
    napi_status status = napi_get_value_int32_(value, tmp);
    result = tmp;
    return status;
}

napi_status napi_get_value_nodestring_(const Local<Value>& str, NodeString& nodechar)
{
    napi_status status = napi_ok;
    do {
        int len = napi_get_value_string_utf8_(str, nullptr, 0);
        if (len == 0) {
            break;
        }
        char *outstr = NodeString::alloc_buf(len + 1);
        if (!outstr) {
            status = napi_generic_failure;
            break;
        }
        len = napi_get_value_string_utf8_(str, outstr, len + 1);

        if (status != napi_ok) {
            break;
        }
        nodechar.setBuf(outstr);
    } while (false);
    return status;
}

napi_status napi_get_value_object_(Isolate* isolate, const Local<Value>& value, Local<Object>& object)
{
    if(!value->IsObject()) {
        return napi_invalid_arg;
    }

    object = value->ToObject(isolate->GetCurrentContext()).ToLocalChecked();
    return napi_ok;
}

Local<Value> napi_create_uint32_(Isolate *isolate, const uint32_t& value)
{
    return v8::Number::New(isolate, value);
}

Local<Value> napi_create_bool_(Isolate *isolate, const bool& value)
{
    return v8::Boolean::New(isolate, value);
}

Local<Value> napi_create_string_(Isolate *isolate, const char* value)
{
    return String::NewFromUtf8(isolate, value ? value : "", NewStringType::kInternalized).ToLocalChecked();
}

Local<Value> napi_create_double_(Isolate *isolate, const double &value)
{
    return v8::Number::New(isolate, value);
}

Local<Value> napi_create_uint64_(Isolate *isolate, const uint64_t& value)
{
    return v8::Number::New(isolate, (double)value);
}

Local<Value> napi_create_int32_(Isolate *isolate, const int32_t& value)
{
    return v8::Int32::New(isolate, value);
}

Local<Value> napi_create_uint16_(Isolate *isolate, const uint16_t& value)
{
    return v8::Uint32::New(isolate, value);
}

Local<Value> napi_create_uid_(Isolate *isolate, const agora::rtc::uid_t& uid)
{
    return agora::rtc::NodeUid::getNodeValue(isolate, uid);
}

static Local<Value> napi_get_object_property_value(Isolate* isolate, const Local<Object>& obj, const std::string& propName)
{
    Local<Name> keyName = Nan::New<String>(propName).ToLocalChecked();
    return obj->Get(isolate->GetCurrentContext(), keyName).ToLocalChecked();
}

/**
* get uint32 property from V8 object.
*/
napi_status napi_get_object_property_uint32_(Isolate* isolate, const Local<Object>& obj, const std::string& propName, uint32_t& result)
{
    Local<Value> value = napi_get_object_property_value(isolate, obj, propName);
    return napi_get_value_uint32_(value, result);
}

/**
* get bool property from V8 object.
*/
napi_status napi_get_object_property_bool_(Isolate* isolate, const Local<Object>& obj, const std::string& propName, bool& result)
{
    Local<Value> value = napi_get_object_property_value(isolate, obj, propName);
    return napi_get_value_bool_(value, result);
}

/**
* get int32 property from V8 object.
*/
napi_status napi_get_object_property_int32_(Isolate* isolate, const Local<Object>& obj, const std::string& propName, int32_t& result)
{
    Local<Value> value = napi_get_object_property_value(isolate, obj, propName);
    return napi_get_value_int32_(value, result);
}

/**
* get double property from V8 object.
*/
napi_status napi_get_object_property_double_(Isolate* isolate, const Local<Object>& obj, const std::string& propName, double &result)
{
    Local<Value> value = napi_get_object_property_value(isolate, obj, propName);
    return napi_get_value_double_(value, result);
}

/**
* get int64 property from V8 object.
*/
napi_status napi_get_object_property_int64_(Isolate* isolate, const Local<Object>& obj, const std::string& propName, int64_t& result)
{
    Local<Value> value = napi_get_object_property_value(isolate, obj, propName);
    return napi_get_value_int64_(value, result);
}

/**
* get nodestring property from V8 object.
*/
napi_status napi_get_object_property_nodestring_(Isolate* isolate, const Local<Object>& obj, const std::string& propName, NodeString& nodechar)
{
    Local<Value> value = napi_get_object_property_value(isolate, obj, propName);
    return napi_get_value_nodestring_(value, nodechar);
}

/**
* get nodestring property from V8 object.
*/
napi_status napi_get_object_property_uid_(Isolate* isolate, const Local<Object>& obj, const std::string& propName, agora::rtc::uid_t& uid)
{
    Local<Value> value = napi_get_object_property_value(isolate, obj, propName);
    return agora::rtc::NodeUid::getUidFromNodeValue(value, uid);
}

/**
* get object property from V8 object.
*/
napi_status napi_get_object_property_object_(Isolate* isolate, const Local<Object>& obj, const std::string& propName, Local<Object>& childobj)
{
    Local<Value> value = napi_get_object_property_value(isolate, obj, propName);
    return napi_get_value_object_(isolate, value, childobj);
}

const char* nullable( char const* s)
{
    return (s ? s : "");
}

#ifdef _WIN32
char* U2G(const char* srcstr)
{
    int len = MultiByteToWideChar(CP_UTF8, 0, srcstr, -1, NULL, 0);
    wchar_t* wstr = new wchar_t[len + 1];
    memset(wstr, 0, len + 1);
    MultiByteToWideChar(CP_UTF8, 0, srcstr, -1, wstr, len);
    len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, NULL, 0, NULL, NULL);
    char* deststr = new char[len + 1];
    memset(deststr, 0, len+1);
    WideCharToMultiByte(CP_ACP, 0, wstr, -1, deststr, len, NULL, NULL);
    if(wstr) delete[] wstr;
    return deststr;
}
#endif
