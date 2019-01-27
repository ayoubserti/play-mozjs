// following code might be needed in some case
//#define __STDC_LIMIT_MACROS
//#include <stdint.h>
#include <jsapi.h>
#include <js/Initialization.h>

#include "mozilla/ArrayUtils.h"
#include "mozilla/TypeTraits.h"


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
    JS_GlobalObjectTraceHook
};


bool exec(JSContext* cx,const char* bytes, const char* filename, int lineno)
{
    JS::RootedValue v(cx);
    JS::CompileOptions opts(cx);
    opts.setFileAndLine(filename, lineno);
    return JS::Evaluate(cx, opts, bytes, strlen(bytes), &v) ;
}

bool testIndirectEval(JSContext* cx,JS::HandleObject scope, const char* code)
{
    exec(cx,"hits = 0;","internal.js",1);

    {
        JSAutoCompartment ae(cx, scope);
        JSString* codestr = JS_NewStringCopyZ(cx, code);
        
        JS::RootedValue arg(cx, JS::StringValue(codestr));
        JS::RootedValue v(cx);
        JS_CallFunctionName(cx, scope, "eval", JS::HandleValueArray(arg), &v);
    }

    JS::RootedValue hitsv(cx);
    JS::CompileOptions opts(cx);
    opts.setFileAndLine(__FILE__, __LINE__);
    JS::Evaluate(cx, opts, code, strlen(code), &hitsv);
   
   uint32_t a = hitsv.toInt32();
   printf("%d\n", a);
    return hitsv.isInt32(1);
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

    { // Scope for our various stack objects (JSAutoRequest, RootedObject), so they all go
      // out of scope before we JS_DestroyContext.

      JSAutoRequest ar(cx); // In practice, you would want to exit this any
                            // time you're spinning the event loop
      
       

      JS::RootedObject global(cx, JS_NewGlobalObject(cx, &global_class, nullptr, JS::FireOnNewGlobalHook));
      if (!global)
          return 1;
       {
       JSAutoCompartment a1(cx, global); 
         
        if ( !JS_DefineDebuggerObject(cx, global))
        {
            return 1;
        }
       }
        JS::CompartmentOptions options;
       JS::RootedObject g(cx, JS_NewGlobalObject(cx, &global_class, nullptr,
                                              JS::FireOnNewGlobalHook, options));
        if (g)
        {
             JSAutoCompartment ae(cx, g);
            JS_InitStandardClasses(cx, g);
        }
        
        {
         JSAutoCompartment ae(cx, g);
        if (!JS_InitStandardClasses(cx, g)) return 1;
        
        exec(cx,"var dbg = Debugger(g);\n"
         " hits = 0;\n"
         "dbg.onNewScript = function (s) {\n"
         "    hits += Number(s instanceof Debugger.Script);\n"
         "};\n","my_debug.js",1);
        
      JS::RootedValue rval(cx);
        testIndirectEval(cx,g,"Math.abs(0)");
        }
     /* { // Scope for JSAutoCompartment
        JSAutoCompartment ac(cx, global);
        JS_InitStandardClasses(cx, global);

        const char *script = "'hello'+'world, it is '+new Date()";
        const char *filename = "noname";
        int lineno = 1;
        JS::CompileOptions opts(cx);
        opts.setFileAndLine(filename, lineno);
        bool ok = JS::Evaluate(cx, opts, script, strlen(script), &rval);
        if (!ok)
          return 1;
      }
*/
     
    }

    JS_DestroyContext(cx);
    JS_DestroyRuntime(rt);
    JS_ShutDown();
    return 0;
}