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

static JSClass JSR_DebuggerEngineGlobalGlass = {
    "JSRDebuggerGlobal",
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

static bool JS_common_fn_print( JSContext *context, unsigned int argc,
                               JS::Value *vp ) {
    
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    
    std::string val;
    
    JS::HandleValue jsVal= args.get(0);
    
    JSString* str = jsVal.toString();
    if ( !str) return false;
    JSFlatString* flat = JS_FlattenString(context, str);
    
    size_t length = JS::GetDeflatedUTF8StringLength(flat);
    if ( length == 0) return false;
    char* out = new char[length+1];
    JS::DeflateStringToUTF8Buffer(flat, mozilla::RangedPtr<char>(out, length));
    out[length] = '\0';
    
    std::cout << out << std::endl;
    
    delete[] out;
    args.rval().setNull();
    
    return true;
}

static bool JS_common_fn_readline( JSContext *context, unsigned int argc,
                                  JS::Value *vp ) {
    
    
    std::string line;
    
    std::getline(std::cin, line);
    
    
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    
    JSString* jsline = JS_NewStringCopyN(context, line.c_str(), line.size());
    
    args.rval().setString(jsline);
    
    return true;
}
static JSFunctionSpec JS_TestGlobalFuntions[] = {
    { "print",  {JS_common_fn_print , 0}, JSPROP_PERMANENT | JSPROP_ENUMERATE },
    { "readline",  {JS_common_fn_readline , 0}, JSPROP_PERMANENT | JSPROP_ENUMERATE },
    JS_FS_END
};

static std::string ReadFile( const std::string& filename){
    
    std::string str("");
    std::ifstream t(filename.c_str());
    
    if ( t.is_open()){
        
        t.seekg(0, std::ios::end);
        
        str.resize(t.tellg());
        t.seekg(0, std::ios::beg);
        str.assign((std::istreambuf_iterator<char>(t)),
                   std::istreambuf_iterator<char>());
    }
    return str;
}


static bool  ExecuteFile( JSContext* ctx,const std::string& filename){
    
    std::string scriptStr = ReadFile(filename);
    if(!scriptStr.empty()){
        
        JS::CompileOptions co(ctx);
        co.setUTF8(true);
        co.setFileAndLine(filename.c_str(), 1);
        
        JS::RootedScript _script(ctx);
        
        if ( JS::Compile(ctx, co,scriptStr.c_str() ,scriptStr.size(), &_script) ){
            return JS_ExecuteScript(ctx, _script);
        }
    }
    
    return false;
}



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
                
                JS::RootedObject debugger(context, JS_NewGlobalObject(context, &JSR_DebuggerEngineGlobalGlass, nullptr, JS::DontFireOnNewGlobalHook,options));
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
                    ExecuteFile(context,"debugger-script.js");
                    
                }
            }
            ExecuteFile(context,"script.js");
            
        }
   
        
    }
    
    JS_DestroyContext(context);
    JS_DestroyRuntime(rt);
    JS_ShutDown();
    return 0;
}
