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

map<string, shared_ptr<Extension>> extension_map;

unique_ptr<ExtensionConfiguration> get_extension_configuration(const string &file) {
    if (extension_map.find(file) == extension_map.end()) { // TODO: possible memory issue
        ifstream file_stream(file.c_str());
        if (!file_stream) {
            aout << "Error: Unable to open file: " << file << endl;
            return NULL;
        }
        string file_source;    
        file_stream.seekg(0, std::ios::end);   
        file_source.reserve(file_stream.tellg());
        file_stream.seekg(0, std::ios::beg);
        file_source.assign((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>()); 
        
        shared_ptr<Extension> extension(new Extension(file.c_str(), file_source.c_str(), 0, 0, file_source.size()));
        extension_map[file] = extension;       
        RegisterExtension(extension.get()); 
        aout << "Extension registered!" << endl;
    }
    // aout << v8::RegisteredExtension::first_extension()->extension->name();
    const char* extensions[] = { file.c_str() };
    //vector<char*> extension_vector;
    //transform(begin(extensions), end(extensions), back_inserter(extension_vector), [](string& s){ s.push_back(0); return &s[0]; });
    //extension_vector.push_back(nullptr);
    return unique_ptr<ExtensionConfiguration>(new ExtensionConfiguration(1, extensions));
}

Handle<Context> create_veta_context(Isolate* isolate, const string &file) {
    Handle<ObjectTemplate> global = ObjectTemplate::New();
    global->Set(String::New("log"), FunctionTemplate::New(log));
    return Context::New(isolate, get_extension_configuration(file).get(), global);
}

void veta_actor(string file) {
    auto isolate = Isolate::New();
    {
        Locker locker(isolate);
        Isolate::Scope isolate_scope(isolate);        
        HandleScope handle_scope(isolate);
        if (file.substr(file.size() - 3) != ".js") {
            file += ".js";
        }
        Handle<Context> context = create_veta_context(isolate, file);
        {
            Context::Scope context_scope(context);
    
            //ifstream file_stream(file.c_str());
            //if (!file_stream) {
            //    aout << "Error: Unable to open file: " << file << endl;
            //    return;
            //}
            //string file_source;    
            //file_stream.seekg(0, std::ios::end);   
            //file_source.reserve(file_stream.tellg());
            //file_stream.seekg(0, std::ios::beg);
            //file_source.assign((std::istreambuf_iterator<char>(file_stream)), std::istreambuf_iterator<char>()); 

            //auto source = String::NewFromUtf8(isolate, file_source.c_str());
            auto script = Script::Compile(String::NewFromUtf8(isolate, ""));
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
    
    set_default_scheduler(4);
    for(int i = 0; i < num_actors; i++) spawn(veta_actor, file_arg);
    await_all_others_done();
    shutdown(); 

    return 0;
}
