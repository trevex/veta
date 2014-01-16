#include <v8.h>
#include <string>
#include <iostream>
#include "cppa/cppa.hpp"

using namespace v8;
using namespace std;
using namespace cppa;

void mirror() {
    // wait for messages
    become (
        // invoke this lambda expression if we receive a string
        on_arg_match >> [](const string& what) -> string {
            // prints "Hello World!" via aout (thread-safe cout wrapper)
            aout << what << endl;
            // terminates this actor ('become' otherwise loops forever)
            self->quit();
            // reply "!dlroW olleH"
            return string(what.rbegin(), what.rend());
        }
    );
}

void hello_world(const actor_ptr& buddy) {
    // send "Hello World!" to our buddy ...
    sync_send(buddy, "Hello World!").then(
        // ... and wait for a response
        on_arg_match >> [](const string& what) {
            // prints "!dlroW olleH"
            aout << what << endl;
        }
    );
}

int main(int argc, char* argv[]) {
    // Get the default Isolate created at startup.
    Isolate* isolate = Isolate::GetCurrent();

    // Create a stack-allocated handle scope.
    HandleScope handle_scope(isolate);

    // Create a new context.
    Handle<Context> context = Context::New(isolate);

    // Enter the context for compiling and running the hello world script.
    Context::Scope context_scope(context);

    // Create a string containing the JavaScript source code.
    Handle<String> source = String::NewFromUtf8(isolate, "'Hello' + ', World!'");
    
    // Compile the source code.
    Handle<Script> script = Script::Compile(source);
    
    // Run the script to get the result.
    Handle<Value> result = script->Run();
    
    // Convert the result to an UTF8 string and print it.
    String::Utf8Value utf8(result);
    printf("%s\n", *utf8);

    // create a new actor that calls 'mirror()'
    auto mirror_actor = spawn(mirror);
    // create another actor that calls 'hello_world(mirror_actor)'
    spawn(hello_world, mirror_actor);
    // wait until all other actors we have spawned are done
    await_all_others_done();
    // run cleanup code before exiting main
    shutdown();

    return 0;
}
