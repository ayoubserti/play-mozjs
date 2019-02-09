#ifndef __DEBUGGER_WRAP_JS__
#define __DEBUGGER_WRAP_JS__

class JSDebuggerObject
{
    JSContext* context_;
    JS::PersistentRootedObject  object_;
    JSCompartment*   compartment_;
    JSClass    class_;
    
    
    static JSFunctionSpec JS_GlobalObjectFunc[];
    
public:
    
    JSDebuggerObject(JSContext* context);
    
    ~JSDebuggerObject();
    
    void    install(JS::HandleObject global);
    
    void    setProperty(const char* field,  JS::HandleObject obj);
    
    void    ExecuteFile(const std::string& filename);
    
};



#endif //__DEBUGGER_WRAP_JS__
