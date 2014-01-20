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

using namespace v8;
using namespace std;
using namespace cppa;

void log(const FunctionCallbackInfo<Value> &args) {
    bool first = true;
    for (int i = 0; i < args.Length(); i++) {
        HandleScope handle_scope(args.GetIsolate());
        if (first) {
            first = false;
        } else {
            aout << " ";
        }
        String::Utf8Value utf8(args[i]);
        aout << *utf8;
    }
    aout << endl;
}

map<string, Extension*> extension_map;

Handle<Context> create_veta_context(Isolate* isolate, const string &file) {
    Handle<ObjectTemplate> global = ObjectTemplate::New();
    global->Set(String::New("log"), FunctionTemplate::New(log));
    const char* extensions[] = { file.c_str() };
    return Context::New(isolate, new ExtensionConfiguration(1, extensions), global);
}

int preload_file(string file) { // , bool base_checked = false) {
    if (file.substr(file.size() - 3) != ".js") {
        file += ".js";
    }
    if (extension_map.find(file) == extension_map.end()) {
        ifstream file_stream(file.c_str());
        if (!file_stream) {
            // if (!base_checked) {
            //     file_stream = check_other_includes(file); 
            //     if (!file_stream)
            //         return -2;
            //
            // } else {
            cout << "Error: Unable to open file: " << file << endl;
            return -1;
            // }           
        }
        string file_source;
        file_stream.seekg(0, ios::end);   
        file_source.reserve(file_stream.tellg());
        file_stream.seekg(0, ios::beg);
        file_source.assign((istreambuf_iterator<char>(file_stream)), istreambuf_iterator<char>());
        // int include_error = check_source_includes(file_source);
        // if (include_error != 0)
        //     return include_error;
        // NOTE: additional check in map possible because of includes
        Extension* extension = new Extension(file.c_str(), file_source.c_str(), 0, 0, file_source.size());
        extension_map[file] = extension;
        RegisterExtension(extension); 
        cout << "Extension registered!" << endl;
    }
    return 0;
}

void veta_actor(const string &file) {
    auto isolate = Isolate::New();
    {
        Locker locker(isolate);
        Isolate::Scope isolate_scope(isolate);        
        HandleScope handle_scope(isolate);
        Handle<Context> context = create_veta_context(isolate, file);
        {
            Context::Scope context_scope(context); 
            auto script = Script::Compile(String::NewFromUtf8(isolate, "init();"));
            Handle<Value> result = script->Run();
            if (!result->IsUndefined()) {
                String::Utf8Value utf8(result);
                aout << *utf8 << endl;
            }
        }
    }
    isolate->Dispose();
}

int argv_to_int(char* arg) {
    stringstream ss;
    ss << arg;
    int result;
    ss >> result;
    return result;
}

int main(int argc, char* argv[]) {
    std::string file_arg = "";
    int num_actors = 1;
    if (argc > 1) for (int i = 1; i < argc; i++) { 
        if (strcmp(argv[i], "-n") == 0 && (i+1) < argc) {
            i += 1;
            num_actors = argv_to_int(argv[i]);            
        } else {
            string temp(argv[i]);
            if (temp.size() > 3 && temp.substr(temp.size() - 3) == ".js") {
                file_arg = temp;
            }
        }
    }
    if (file_arg == "") {
        return 0;
    }
   
    if (preload_file(file_arg) != 0) {
        return 0;
    }
 
    set_default_scheduler(4);
    for(int i = 0; i < num_actors; i++) spawn(veta_actor, file_arg);
    await_all_others_done();
    shutdown(); 

    return 0;
}
