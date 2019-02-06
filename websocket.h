#ifndef __WEB_SOCKET_H__
#define __WEB_SOCKET_H__

#define USE_STANDALONE_ASIO
#define ASIO_STANDALONE

#include <memory>
#include <thread>
#include "server_ws.hpp"
#include <queue>
#include <string>


using WsServer = SimpleWeb::SocketServer<SimpleWeb::WS>;

class EventMsg
{
public:
    std::string msg_;
    
};

class WebSocketWrap
{
    WsServer server_;
    std::thread* thd_;
    std::queue<EventMsg>    event_queue_;
    std::mutex              event_mutex_;
    std::shared_ptr<WsServer::Connection>    connection_;
    
    static JSFunctionSpec JS_TestGlobalFuntions[];
    static bool runwrap(JSContext *context, unsigned int argc,
                        JS::Value *vp );
    static bool pop_event_wrap(JSContext *context, unsigned int argc,
                        JS::Value *vp );

    static bool send_event_wrap(JSContext *context, unsigned int argc,
                               JS::Value *vp );

    
public:
    void run();
    WebSocketWrap();
    
    JSObject* wrap(JSContext* context);
    void pop_event(JSContext* context, JS::MutableHandleValue vp);
    void send_event(JSContext* context, JS::HandleValue vp);
   
};





#endif
