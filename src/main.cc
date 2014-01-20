#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>
#include <sstream>
#include <map>
#include <vector>
#include <algorithm>
#include <memory>

#include <v8.h>
#include <cppa/cppa.hpp>
#include <cppa/scheduler.hpp>
#include <thread>

using namespace v8;
using namespace std;
using namespace cppa;

void log(const FunctionCallbackInfo<Value> &args) {
    bool first = true;
    for (int i = 0; i < args.Length(); i++) {
        HandleScope handle_scope(args.GetIsolate());
        if (first) 
            first = false;
        else 
            aout << " ";
        String::Utf8Value utf8(args[i]);
        aout << *utf8;
    }
    aout << endl;
}

string file_source =  "log(\"Hello, world\");";

__thread Isolate* isolate;
__thread map<string, Persistent<Script>*>* script_map; 
__thread Persistent<Context>* context;

void test_actor(const string &file) {
    bool thread_initialized = (isolate != NULL);
    if (!thread_initialized) {
        isolate = Isolate::New();
        script_map = new map<string, Persistent<Script>*>();
    }
    Locker locker(isolate);
    Isolate::Scope isolate_scope(isolate);        
    HandleScope handle_scope(isolate);
    if (!thread_initialized) {
        context = new Persistent<Context>();
        auto global = ObjectTemplate::New();
        global->Set(String::New("log"), FunctionTemplate::New(log));
        Handle<Context> context_ = Context::New(isolate, NULL, global);
        context->Reset(isolate, context_);
    }
    Handle<Context> local_context = Local<Context>::New(isolate, *context);
    {
        Context::Scope context_scope(local_context); 
        if (!thread_initialized) {
            (*script_map)[file] = new Persistent<Script>();
            (*script_map)[file]->Reset(isolate, Script::New(String::New(file_source.c_str(), file_source.size()), String::New(file.c_str(), file.size())));
        }
        Handle<Script> script = Local<Script>::New(isolate, *(*script_map)[file]);
        Handle<Value> result = script->Run();
        if (!result->IsUndefined()) {
            String::Utf8Value utf8(result);
            aout << *utf8 << endl;
        }
    }
    //isolate->Dispose();
}

int main(int argc, char* argv[]) {
    string file = "test.js";
    set_default_scheduler(4);
    for(int i = 0; i < 1000; i++) spawn(test_actor, file);
    await_all_others_done();
    shutdown(); 

    return 0;
}
