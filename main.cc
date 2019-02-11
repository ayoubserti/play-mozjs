
#include <jsapi.h>
#include <js/Initialization.h>

#include "websocket.h"
#include "utils.h"
#include "debugger-wrap.h"
#include "scopeJs.h"

using namespace std;

//minimalist scope_guard
struct scope_exit {
    std::function<void()> f_;
    explicit scope_exit(std::function<void()> f) noexcept : f_(std::move(f)) {}
    ~scope_exit() { if (f_) f_(); }
};

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



MainJSSope*  MainJSSope::sInstance_ = nullptr;

MainJSSope::MainJSSope()
:websocket_(nullptr)
,debugger_(nullptr)
,runtime_(nullptr)
,context_(nullptr)
,global_()
,interruptStat_(eUnknown)
{
    
}


MainJSSope& MainJSSope::Get()
{
    if ( sInstance_ == nullptr){
        sInstance_ = new MainJSSope();
        sInstance_->_Init();
    }
    return *sInstance_;
}

void MainJSSope::_Init(){
    
    JS_Init();
    runtime_ = JS_NewRuntime(8L * 1024 * 1024);
    context_ = JS_NewContext(runtime_, 8192);
    JS_SetInterruptCallback(runtime_,&MainJSSope::_interruptCallback);
    
    websocket_ = new WebSocketWrap();
    
    JS::CompartmentOptions options;
    
    global_ =  JS_NewGlobalObject(context_,&global_class,nullptr,JS::FireOnNewGlobalHook,options);
    if ( initGlobal(context_, global_))
    {
        JSAutoCompartment ac(context_,global_);
        
        
        debugger_ = new JSDebuggerObject(context_);
        JSObject* websockwrap =  websocket_->wrap(context_);
        debugger_->install(global_);
        JS::RootedObject gWrapper(context_, websockwrap);
        JS_WrapObject(context_, &gWrapper);
        JS::RootedValue v(context_, JS::ObjectValue(*gWrapper));
        debugger_->setProperty("WebSocket", gWrapper );
        
        debugger_->executeFile("debugger-script.js");
    }
    
}

void MainJSSope::Start(){
    
    JSAutoCompartment ac(context_, global_);
    Utils::ExecuteFile(context_,"script.js");
}

MainJSSope::~MainJSSope(){
    delete  debugger_;
    JS_DestroyContext(context_);
    JS_DestroyRuntime(runtime_);
    JS_ShutDown();
}

bool MainJSSope::_interruptCallback(JSContext* ctx){
    
    
    MainJSSope& jsScope = MainJSSope::Get();
    JS_SetInterruptCallback(jsScope.runtime_, nullptr); // uninstall callback
    
    scope_exit guard([&](){
        JS_SetInterruptCallback(jsScope.runtime_, _interruptCallback);});
    
    std::lock_guard<std::mutex> lock(jsScope.mutex_);
    
    if(jsScope.interruptStat_ == eForDebugging)
    {
        jsScope.debugger_->eval("test()");
        jsScope.interruptStat_ = eUnknown;
    }
    
    
    return true;
}


void MainJSSope::Interrupt(InterruptionStat stat)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (stat== eForDebugging)
        interruptStat_ = eForDebugging;
    JS_RequestInterruptCallback(runtime_); //request an interrupt
}




int main(int argc, const char *argv[])
{
    MainJSSope& jsScope = MainJSSope::Get();
    jsScope.Start();
    return 0;
}
