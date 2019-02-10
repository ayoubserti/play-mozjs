
#include <js/CharacterEncoding.h>
#include <jsapi.h>
#include <js/Initialization.h>

#include "mozilla/ArrayUtils.h"
#include "mozilla/TypeTraits.h"

#include "websocket.h"

using namespace std;

void FinalizeWebSocketWrap (JSFreeOp* fop, JSObject* obj)
{
    if ( obj)
    {
        WebSocketWrap* websocket = reinterpret_cast<WebSocketWrap*>( JS_GetPrivate(obj) );
        if ( websocket)
        {
            delete websocket;
        }
    }
}

//WebServer Javascript class
static JSClass JSR_WebSocketGlobal = {
    "WebSocket",
    JSCLASS_GLOBAL_FLAGS,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    FinalizeWebSocketWrap,  //the finalizer is needed for this class to prevent leaks. elsewhere app will crash at JS_DestroyContext
    nullptr,
    nullptr,
    nullptr,
    JS_GlobalObjectTraceHook};

JSFunctionSpec WebSocketWrap::JS_TestGlobalFuntions[] = {
    { "run",  {runwrap , 0}, JSPROP_PERMANENT | JSPROP_ENUMERATE },
    { "pop_event",  {pop_event_wrap, 0}, JSPROP_PERMANENT | JSPROP_ENUMERATE },
    { "send_event",  {send_event_wrap, 0}, JSPROP_PERMANENT | JSPROP_ENUMERATE },
    JS_FS_END
};
//WebSocketWrap

WebSocketWrap::WebSocketWrap()
:thd_(nullptr)
,connection_(nullptr)
{
    
    server_.config.port = 8083;
    
    auto& debugger = server_.endpoint["^/debugger/?$"];
    debugger.on_message = [this](shared_ptr<WsServer::Connection> connection, shared_ptr<WsServer::InMessage> in_message) {
        auto out_message = in_message->string();
        if ( (connection_.get() == nullptr))
        {
            connection_ = connection;
            //JS_RequestInterruptCallback(<#JSRuntime *rt#>)
        }
        
        EventMsg event_msg;
        event_msg.msg_ = out_message;
        event_mutex_.lock();
        event_queue_.push(event_msg);
        event_mutex_.unlock();
        
    };
}

void WebSocketWrap::run()
{
    thd_.reset(new thread([this]{
        this->server_.start();
    }));
    //wait 1s for server to start
    this_thread::sleep_for(chrono::seconds(1));
}
bool WebSocketWrap::runwrap(JSContext *context, unsigned int argc,
             JS::Value *vp )
{
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    JS::Value val = args.computeThis(context);
    if ( val.isObject()){
        JSObject* obj = val.toObjectOrNull();
        WebSocketWrap* websocket = reinterpret_cast< WebSocketWrap*>(JS_GetPrivate(obj));
        if ( websocket){
            websocket->run();
        }
    }
    
    return true;
}

bool WebSocketWrap::pop_event_wrap(JSContext *context, unsigned int argc,
                    JS::Value *vp ){
    
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    JS::Value val = args.computeThis(context);
    if ( val.isObject()){
        JSObject* obj = val.toObjectOrNull();
        WebSocketWrap* websocket = reinterpret_cast< WebSocketWrap*>(JS_GetPrivate(obj));
        if ( websocket){
            JS::RootedValue ret(context);
            websocket->pop_event(context,&ret);
            args.rval().set(ret);
        }
    }
    
    return true;
}
bool WebSocketWrap::send_event_wrap(JSContext *context, unsigned int argc,
                                   JS::Value *vp ){
    
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    JS::Value val = args.computeThis(context);
    if ( val.isObject()){
        JSObject* obj = val.toObjectOrNull();
        WebSocketWrap* websocket = reinterpret_cast< WebSocketWrap*>(JS_GetPrivate(obj));
        if ( websocket){
            
            websocket->send_event(context,args.get(0));
            
        }
    }
    
    
    return true;
}

JSObject* WebSocketWrap::wrap(JSContext* context){
    
    JSObject*   obj = JS_NewGlobalObject(context, &JSR_WebSocketGlobal, nullptr, JS::DontFireOnNewGlobalHook);
    JS::RootedObject rootedObj(context,obj);
    JSAutoCompartment a2(context, rootedObj);
    JS_InitStandardClasses(context, rootedObj);
    if ( !JS_DefineFunctions( context, rootedObj, &JS_TestGlobalFuntions[0] ) ) {
        throw std::runtime_error( "Cannot register global functions for WebSocket object." );
    }
    JS_SetPrivate(obj, this);
    return obj;
}

void WebSocketWrap::pop_event(JSContext* context, JS::MutableHandleValue vp)
{
    event_mutex_.lock();
    if(event_queue_.empty()) {
        event_mutex_.unlock();
        return;
    }
    EventMsg event = event_queue_.front();
    
    std::string msg = event.msg_;
    JSString* jsstr = JS_NewStringCopyN(context, msg.c_str(), msg.size());
    vp.setString(jsstr);
    
    event_queue_.pop();
    event_mutex_.unlock();
}

void WebSocketWrap::send_event(JSContext* context, JS::HandleValue vp)
{
   if (vp.isString())
   {
       
       JSString* jsstr =  vp.get().toString();
       if ( !jsstr) return;
       JSFlatString* flat = JS_FlattenString(context, jsstr);
       
       size_t length = JS::GetDeflatedUTF8StringLength(flat);
       if ( length == 0) return;
       char* out = new char[length+1];
       JS::DeflateStringToUTF8Buffer(flat, mozilla::RangedPtr<char>(out, length));
       out[length] = '\0';
       
       string msg=out;
       delete[] out;
       if ( !connection_) return;
       try{
           connection_->send(msg);
       }
       catch(...)
       {
           //Maybe throw a js exception
       }
       
   }
    
}


WebSocketWrap::~WebSocketWrap()
{
    server_.stop();
    thd_->join();  //wait websocket thread to terminate.
}
