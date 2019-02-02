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

bool exec(JSContext *cx, const char *bytes, const char *filename, int lineno)
{
    JS::RootedValue v(cx);
    JS::CompileOptions opts(cx);
    opts.setFileAndLine(filename, lineno);
    return JS::Evaluate(cx, opts, bytes, strlen(bytes), &v);
}

bool testIndirectEval(JSContext *cx, JS::HandleObject scope, const char *code)
{
    {
        JSAutoCompartment ae(cx, scope);
        JSString *codestr = JS_NewStringCopyZ(cx, code);
        
        JS::RootedValue arg(cx, JS::StringValue(codestr));
        JS::RootedValue v(cx);
        JS_CallFunctionName(cx, scope, "eval", JS::HandleValueArray(arg), &v);
    }
    return true;
}

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

const char* script = nullptr;


static std::string ReadFile( const std::string& filename){
    std::ifstream t(filename.c_str());
    std::string str;
    
    t.seekg(0, std::ios::end);
    
    str.resize(t.tellg());
    t.seekg(0, std::ios::beg);
    str.assign((std::istreambuf_iterator<char>(t)),
               std::istreambuf_iterator<char>());
    return str;
}



int main(int argc, const char *argv[])
{
    JS_Init();
    
    JSRuntime *rt = JS_NewRuntime(8L * 1024 * 1024);
    if (!rt)
        return 1;
    
    JSContext *cx = JS_NewContext(rt, 8192);
    if (!cx)
        return 1;
    
    JSContext* dbgCx = cx; //JS_NewContext(rt, 8192);
    std::string str =ReadFile("script.js");
    
    
    { // Scope for our various stack objects (JSAutoRequest, RootedObject), so they all go
        // out of scope before we JS_DestroyContext.
        
        //JSAutoRequest ar(cx); // In practice, you would want to exit this any
        // time you're spinning the event loop
        JS::CompartmentOptions options;
        
        
        JS::RootedObject global(cx, JS_NewGlobalObject(cx, &global_class, nullptr, JS::FireOnNewGlobalHook,options));
        if (!global)
            return 1;
        {
            {
                JSAutoCompartment a1(cx, global);
                JS_InitStandardClasses(cx, global);
                // Register global functions for test script.
                if ( !JS_DefineFunctions( cx, global, &JS_TestGlobalFuntions[0] ) ) {
                    throw std::runtime_error( "Cannot register global functions for test script." );
                }
            }
            
            {
                
            JS::RootedObject debugger(dbgCx, JS_NewGlobalObject(dbgCx, &JSR_DebuggerEngineGlobalGlass, nullptr, JS::FireOnNewGlobalHook,options));
            if ( !debugger ) return 2;
            {
                JS_SetErrorReporter(rt, [](JSContext* cx, const char* message, JSErrorReport* report){
                    std::cout << message << std::endl;
                });
                JSAutoCompartment a2(dbgCx, debugger);
                JS_InitStandardClasses(dbgCx, debugger);
                if ( !JS_DefineFunctions( dbgCx, debugger, &JS_TestGlobalFuntions[0] ) ) {
                    throw std::runtime_error( "Cannot register global functions for test script." );
                }
                if (!JS_DefineDebuggerObject(dbgCx, debugger))
                 {
                 return 1;
                 }
                JS::RootedObject gWrapper(cx, global);
                JS_WrapObject(cx, &gWrapper);
                JS::RootedValue v(cx, JS::ObjectValue(*gWrapper));
                JS_SetProperty(cx, debugger, "g", v);
                
                testIndirectEval(dbgCx,debugger,str.c_str());
            }
            }
            //
            
            testIndirectEval(cx,global,"var a = 'me'; debugger; print('aa')");
            
        }
        
    }
    
    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);
    JS_ShutDown();
    return 0;
}
