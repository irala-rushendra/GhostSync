#include "jsi_bindings.hpp"
#include <iostream>

GhostSyncHostObject::GhostSyncHostObject() {
    // The GhostSyncEngine is automatically instantiated here via the member initializer
}

GhostSyncHostObject::~GhostSyncHostObject() {}

jsi::Value GhostSyncHostObject::get(jsi::Runtime& rt, const jsi::PropNameID& name) {
    auto propName = name.utf8(rt);

    // ---------------------------------------------------------
    // BINDING 1: ghostSyncEngine.pushFrame(arrayBuffer, width, height)
    // ---------------------------------------------------------
    if (propName == "pushFrame") {
        return jsi::Function::createFromHostFunction(
            rt, name, 3, // Expecting 3 arguments
            [this](jsi::Runtime& rt, const jsi::Value& thisVal, const jsi::Value* args, size_t count) -> jsi::Value {
                
                if (count < 3 || !args[0].isObject() || !args[0].getObject(rt).isArrayBuffer(rt)) {
                    throw jsi::JSError(rt, "[GhostSync] pushFrame requires (ArrayBuffer, Number, Number)");
                }

                // Zero-copy memory access: Grab the raw pointer to the JS ArrayBuffer
                auto arrayBuffer = args[0].getObject(rt).getArrayBuffer(rt);
                uint8_t* dataPtr = arrayBuffer.data(rt);
                size_t size = arrayBuffer.size(rt);
                
                int width = args[1].asNumber();
                int height = args[2].asNumber();

                // Convert to std::vector and blast it into our engine's drop-buffer
                std::vector<uint8_t> frameData(dataPtr, dataPtr + size);
                engine_.pushFrame(frameData, width, height);
                
                return jsi::Value::undefined();
            }
        );
    }

    // ---------------------------------------------------------
    // BINDING 2: ghostSyncEngine.getLatestResult()
    // ---------------------------------------------------------
    if (propName == "getLatestResult") {
        return jsi::Function::createFromHostFunction(
            rt, name, 0, // Expecting 0 arguments
            [this](jsi::Runtime& rt, const jsi::Value& thisVal, const jsi::Value* args, size_t count) -> jsi::Value {
                
                // Grab the thread-safe result from the engine
                AuthResult result = engine_.getLatestResult();
                
                // Pack it into a standard JavaScript Object
                jsi::Object jsResult(rt);
                jsResult.setProperty(rt, "isLive", jsi::Value(result.isLive));
                jsResult.setProperty(rt, "confidenceScore", jsi::Value((double)result.confidenceScore));
                jsResult.setProperty(rt, "userId", jsi::String::createFromUtf8(rt, result.userId));
                jsResult.setProperty(rt, "processingTimeMs", jsi::Value((double)result.processingTimeMs));

                return jsResult;
            }
        );
    }

    return jsi::Value::undefined();
}

void GhostSyncHostObject::set(jsi::Runtime& rt, const jsi::PropNameID& name, const jsi::Value& value) {
    // Prevent JS from overwriting engine functions
    throw jsi::JSError(rt, "GhostSyncEngine is read-only.");
}

// ---------------------------------------------------------
// THE INSTALLER: Injects the C++ object into the JS global space
// ---------------------------------------------------------
void installGhostSyncJSI(jsi::Runtime& jsiRuntime) {
    std::cout << "[GhostSync] Installing JSI Bindings..." << std::endl;
    
    auto ghostSyncObj = std::make_shared<GhostSyncHostObject>();
    auto jsObject = jsi::Object::createFromHostObject(jsiRuntime, ghostSyncObj);
    
    jsiRuntime.global().setProperty(jsiRuntime, "ghostSyncEngine", jsObject);
}