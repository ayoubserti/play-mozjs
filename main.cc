// following code might be needed in some case
//#define __STDC_LIMIT_MACROS
//#include <stdint.h>
#include <js/CharacterEncoding.h>
#include <jsapi.h>
#include <js/Initialization.h>

#include "mozilla/ArrayUtils.h"
#include "mozilla/TypeTraits.h"
#include <iostream>
#include <string>
#include <stdexcept>
#include <locale>
#include <codecvt>
#include <string>
#include <fstream>

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


int main(int argc, const char *argv[])
{
    JS_Init();
    
    JSRuntime *rt = JS_NewRuntime(8L * 1024 * 1024);
    if (!rt)
        return 1;
    
    JSContext *context = JS_NewContext(rt, 8192);
    if (!context)
        return 1;
    
    {
        JS::CompartmentOptions options;
        JS::RootedObject global(context, JS_NewGlobalObject(context, &global_class, nullptr, JS::FireOnNewGlobalHook,options));
        if (!global)
            return 1;
        {
            
                JSAutoCompartment a1(context, global);
                JS_InitStandardClasses(context, global);
                // Register global functions for test script.
                if ( !JS_DefineFunctions( context, global, &JS_TestGlobalFuntions[0] ) ) {
                    throw std::runtime_error( "Cannot register global functions for test script." );
                }
            
            {
                
                /*JS::RootedObject debugger(context, JS_NewGlobalObject(context, &JSR_DebuggerEngineGlobalGlass, nullptr, JS::DontFireOnNewGlobalHook,options));
                if ( !debugger ) return 2;
                {
                    JS_SetErrorReporter(rt, [](JSContext* context, const char* message, JSErrorReport* report){
                        std::cout << message << std::endl;
                    });
                    
                    JSAutoCompartment a2(context, debugger);
                    JS_InitStandardClasses(context, debugger);
                    if ( !JS_DefineFunctions( context, debugger, &JS_TestGlobalFuntions[0] ) ) {
                        throw std::runtime_error( "Cannot register global functions for test script." );
                    }
                    if (!JS_DefineDebuggerObject(context, debugger))
                    {
                        return 1;
                    }
                    
                    
                    JS::RootedObject gWrapper(context, global);
                    JS_WrapObject(context, &gWrapper);
                    JS::RootedValue v(context, JS::ObjectValue(*gWrapper));
                    JS_SetProperty(context, debugger, "g", v);
                    {
                        auto websocket = new WebSocketWrap();
                        
                        JSObject* websockwrap =  websocket->wrap(context);
                        JS::RootedObject gWrapper(context, websockwrap);
                        JS_WrapObject(context, &gWrapper);
                        JS::RootedValue v(context, JS::ObjectValue(*gWrapper));
                        JS_SetProperty(context, debugger, "WebSocket", v);
                    }
                    Utils::ExecuteFile(context,"debugger-script.js");
                    
                }*/
                
                JSDebuggerObject* debuggerObj = new JSDebuggerObject(context);
                auto websocket = new WebSocketWrap();
                JSObject* websockwrap =  websocket->wrap(context);
                debuggerObj->install(global);
                JS::RootedObject gWrapper(context, websockwrap);
                JS_WrapObject(context, &gWrapper);
                JS::RootedValue v(context, JS::ObjectValue(*gWrapper));
                debuggerObj->setProperty("WebSocket", gWrapper );
            
                debuggerObj->ExecuteFile("debugger-script.js");
            }
            Utils::ExecuteFile(context,"script.js");
            
        }
   
        
    }
    
    JS_DestroyContext(context);
    JS_DestroyRuntime(rt);
    JS_ShutDown();
    return 0;
}
