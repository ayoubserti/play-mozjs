
#include <jsapi.h>
#include <js/Initialization.h>

#include "websocket.h"
#include "utils.h"
#include "debugger-wrap.h"

using namespace std;

/* The class of the global object. */
static JSClass global_class = {
    "global",
    JSCLASS_GLOBAL_FLAGS,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    nullptr,
    JS_GlobalObjectTraceHook};


static JSFunctionSpec JS_TestGlobalFuntions[] = {
    { "print",  {JSUtils::JS_common_fn_print , 0}, JSPROP_PERMANENT | JSPROP_ENUMERATE },
    { "readline",  {JSUtils::JS_common_fn_readline , 0}, JSPROP_PERMANENT | JSPROP_ENUMERATE },
    JS_FS_END
};


static bool initGlobal(JSContext* context, JS::HandleObject global)
{
    JSAutoCompartment ac(context, global);
    if ( !JS_InitStandardClasses(context, global)) return false;
    // Register global functions for test script.
    if ( !JS_DefineFunctions( context, global, &JS_TestGlobalFuntions[0] ) ) {
        throw std::runtime_error( "Cannot register global functions for test script." );
    }
    
    return true;
}


int main(int argc, const char *argv[])
{
    JS_Init();
    
    auto websocket = new WebSocketWrap();
    JSDebuggerObject* debuggerObj  = nullptr;
    
    JSRuntime *rt = JS_NewRuntime(8L * 1024 * 1024);
    if (!rt)
        return 1;
    
    JSContext *context = JS_NewContext(rt, 8192);
    
    if(context!=nullptr)
    {
        JS::CompartmentOptions options;
        
        JS::RootedObject global(context, JS_NewGlobalObject(context,&global_class,nullptr,JS::FireOnNewGlobalHook,options));
        if ( initGlobal(context, global))
        {
            JSAutoCompartment ac(context,global);
            
            debuggerObj = new JSDebuggerObject(context);
            JSObject* websockwrap =  websocket->wrap(context);
            debuggerObj->install(global);
            JS::RootedObject gWrapper(context, websockwrap);
            JS_WrapObject(context, &gWrapper);
            JS::RootedValue v(context, JS::ObjectValue(*gWrapper));
            debuggerObj->setProperty("WebSocket", gWrapper );
            
            debuggerObj->executeFile("debugger-script.js");
            
            Utils::ExecuteFile(context,"script.js");
        }
        
    }
    
    delete debuggerObj;
    JS_DestroyContext(context);
    JS_DestroyRuntime(rt);
    JS_ShutDown();
    return 0;
}
