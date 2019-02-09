#include <string>
#include <iostream>
#include <fstream>
#include <js/CharacterEncoding.h>
#include <jsapi.h>
#include <js/Initialization.h>

#include "mozilla/ArrayUtils.h"
#include "mozilla/TypeTraits.h"
#include "utils.h"


std::string Utils::ReadFile( const std::string& filename){
    
    std::string str("");
    std::ifstream ifs(filename.c_str());
    
    if ( ifs.is_open()){
        
        ifs.seekg(0, std::ios::end);
        
        str.resize(ifs.tellg());
        ifs.seekg(0, std::ios::beg);
        str.assign((std::istreambuf_iterator<char>(ifs)),
                   std::istreambuf_iterator<char>());
    }
    return str;
}

bool  Utils::ExecuteFile( JSContext* ctx,const std::string& filename){
    
    std::string scriptStr = Utils::ReadFile(filename);
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


bool JSUtils::JS_common_fn_print( JSContext *context, unsigned int argc, JS::Value *vp )
{
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


bool JSUtils::JS_common_fn_readline( JSContext *context, unsigned int argc, JS::Value *vp )
{
    
    std::string line;
    
    std::getline(std::cin, line);
    
    
    JS::CallArgs args = CallArgsFromVp(argc, vp);
    
    JSString* jsline = JS_NewStringCopyN(context, line.c_str(), line.size());
    
    args.rval().setString(jsline);
    
    return true;
}
