#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>

#include <v8.h>
#include <cppa/cppa.hpp>
#include <cppa/scheduler.hpp>

using namespace v8;
using namespace std;
using namespace cppa;

void veta_actor(string file) {
    auto isolate = Isolate::New();
    {
        Locker locker(isolate);
        Isolate::Scope isolate_scope(isolate);        
        HandleScope handle_scope(isolate);
        //Handle<ObjectTemplate> object_template = Local<ObjectTemplate>::New(isolate, global);
        Handle<Context> context = Context::New(isolate);
        {
            Context::Scope context_scope(context);
    
            if (file.substr(file.size() - 3) != ".js") {
                file += ".js";
            }

            ifstream file_stream(file.c_str());
            if (!file_stream) {
                aout << "Error: Unable to open file: " << file << endl;
                return;
            }
            string file_source;    
            file_stream.seekg(0, std::ios::end);   
            file_source.reserve(file_stream.tellg());
            file_stream.seekg(0, std::ios::beg);
            file_source.assign((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>()); 

            auto source = String::NewFromUtf8(isolate, file_source.c_str());
            auto script = Script::Compile(source);

            Handle<Value> result = script->Run();

            if (!result.IsEmpty()) {
                String::Utf8Value utf8(result);
                aout << *utf8 << endl;
            }
        }
    }
    isolate->Dispose();
}

int main(int argc, char* argv[]) {
    std::string file_arg = "";
    if (argc > 1) { 
        file_arg = argv[1]; 
    }
    if (file_arg == "") {
        return 0;
    }
    
    set_default_scheduler(4);
    for(int i = 0; i < 100; i++) spawn(veta_actor, file_arg);
    await_all_others_done();
    shutdown(); 

    return 0;
}
