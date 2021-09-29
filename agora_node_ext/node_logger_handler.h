//
//  node_logger_handler.hpp
//  binding
//
//  Created by Jerry-Luo on 2021/9/29.
//

#ifndef node_logger_handler_hpp
#define node_logger_handler_hpp

#include <stdio.h>
#include "IAgoraRtcEngine.h"
#include "agora_node_ext.h"
#include <string>
#include "IAgoraLog.h"
#include "node_event_handler.h"

namespace agora {
namespace rtc {


class NodeRtcEngine;
class NodeUid;
class NodeLogWriterHandler : public agora::commons::ILogWriter
{
    
public:
    NodeLogWriterHandler(NodeEventHandler* nodeEventHandler);
    ~NodeLogWriterHandler();
    virtual int32_t writeLog(const char* message, uint16_t length) override;
private:
    NodeEventHandler* m_nodeEventHandler;
};


}//agora
}//rtc

#endif /* node_logger_handler_hpp */
