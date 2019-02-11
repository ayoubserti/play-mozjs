#ifndef __UTILS_H_
#define __UTILS_H_

class Utils
{
public:
    static std::string ReadFile( const std::string& filename);
    
    static bool  ExecuteFile( JSContext* ctx,const std::string& filename);
    
};

class JSUtils{
    
public:
    static bool JS_common_fn_print( JSContext *context, unsigned int argc, JS::Value *vp );
    static bool JS_common_fn_readline( JSContext *context, unsigned int argc, JS::Value *vp );
};



#endif //__UTILS_H_
