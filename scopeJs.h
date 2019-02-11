#ifndef __MAIN_JS_SCOPE__
#define __MAIN_JS_SCOPE__



enum InterruptionStat
{
    eUnknown  =0,
    eForDebugging
};

class MainJSSope{
    
    WebSocketWrap* websocket_;
    JSDebuggerObject*   debugger_;
    JSRuntime*          runtime_;
    JSContext*          context_;
    JS::PersistentRootedObject    global_;
    InterruptionStat    interruptStat_;
    std::mutex          mutex_;
    
    static MainJSSope*         sInstance_;
    
    MainJSSope();
    
    void    _Init();
    
    static bool    _interruptCallback(JSContext* ctx);
    
    ~MainJSSope();
    
public:
    
    static MainJSSope& Get();
    
    void   Start();
    
    void    Interrupt(InterruptionStat stat);
};



#endif //__MAIN_JS_SCOPE__
