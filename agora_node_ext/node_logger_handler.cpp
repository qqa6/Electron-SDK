//
//  node_logger_handler.cpp
//  binding
//
//  Created by Jerry-Luo on 2021/9/29.
//

#include "node_logger_handler.h"



#include "node_event_handler.h"
#include "node_log.h"
#include <stdio.h>
#include "node_uid.h"
#include "agora_rtc_engine.h"
#include "uv.h"
#include "node_async_queue.h"
namespace agora {
namespace rtc {

#define FUNC_TRACE

NodeLogWriterHandler::NodeLogWriterHandler(NodeEventHandler* nodeEventHandler)
: m_nodeEventHandler(nodeEventHandler)
{
    
}

NodeLogWriterHandler::~NodeLogWriterHandler()
{
    m_nodeEventHandler = nullptr;
}



int32_t NodeLogWriterHandler::writeLog(const char* message, uint16_t length)
{
    m_nodeEventHandler->writeLog(message, length);
    return 0;
}


}//rtc
}//agora
