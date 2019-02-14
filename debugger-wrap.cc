#include <js/CharacterEncoding.h>
#include <jsapi.h>
#include <js/Initialization.h>

#include "mozilla/ArrayUtils.h"
#include "mozilla/TypeTraits.h"

#include <string>
#include "debugger-wrap.h"
#include "utils.h"



JSFunctionSpec JSDebuggerObject::JS_GlobalObjectFunc[]={
    { "print",  {JSUtils::JS_common_fn_print , 0}, JSPROP_PERMANENT | JSPROP_ENUMERATE },
    { "readline",  {JSUtils::JS_common_fn_readline , 0}, JSPROP_PERMANENT | JSPROP_ENUMERATE },
    JS_FS_END
};


JSDebuggerObject::JSDebuggerObject(JSContext* context)
:context_(context)
,object_()
,compartment_(nullptr)
,class_({"JSDebuggerGlobal",
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
    JS_GlobalObjectTraceHook})
{}


void    JSDebuggerObject::install(JS::HandleObject global)
{
    //the global here to be set as the debuggee
    
    //create the debugger global object
    JS::RootedObject debugger(context_, JS_NewGlobalObject(context_, &class_, nullptr, JS::DontFireOnNewGlobalHook));
    if ( !debugger)
    {
        MOZ_ASSERT("Error creating debugger global Object");
        return;
    }
    compartment_ = JS_EnterCompartment(context_, debugger);
    JS_InitStandardClasses(context_, debugger);
    if ( !JS_DefineFunctions( context_, debugger, &JS_GlobalObjectFunc[0] ) ) {
        //throw std::runtime_error( "Cannot register global functions for test script." );
    }
    if ( JS_DefineDebuggerObject(context_, debugger)){
        
        JS::RootedObject gWrapper(context_, global);
        JS_WrapObject(context_, &gWrapper);
        JS::RootedValue v(context_, JS::ObjectValue(*gWrapper));
        JS_SetProperty(context_, debugger, "__globalObjectProxy", v);
        
    }
    
    JS_LeaveCompartment(context_, compartment_);
    
    object_ = debugger;
}


void   JSDebuggerObject::setProperty(const char* field,  JS::HandleObject obj)
{
    JS::RootedObject gWrapper(context_, obj);
    JS_WrapObject(context_, &gWrapper);
    JS::RootedValue v(context_, JS::ObjectValue(*gWrapper));
    JS_SetProperty(context_, object_, field, v);
}

void JSDebuggerObject::executeFile(const std::string &filepath)
{
    auto comp = JS_EnterCompartment(context_, object_.get());
    if ( !Utils::ExecuteFile(context_, filepath))
    {
        //throw std::exception("cannot execute debugger script.");
    }
    JS_LeaveCompartment(context_, comp);
}

void JSDebuggerObject::eval(const std::string &src)
{
    auto comp = JS_EnterCompartment(context_, object_.get());
    JSString* codestr = JS_NewStringCopyZ(context_, src.c_str());
    
    JS::RootedValue arg(context_, JS::StringValue(codestr));
    JS::RootedValue v(context_);
    JS_CallFunctionName(context_, object_, "eval", JS::HandleValueArray(arg), &v);
    JS_LeaveCompartment(context_, comp);
}


JSDebuggerObject::~JSDebuggerObject()
{
    if ( compartment_ !=nullptr)
    {
        JS_LeaveCompartment(context_, compartment_);
    }
    object_.reset(); //reset the persistan object
    
}

