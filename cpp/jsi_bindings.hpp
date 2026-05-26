#pragma once

#include <jsi/jsi.h>
#include "engine.hpp"

using namespace facebook;

class GhostSyncHostObject : public jsi::HostObject {
public:
    GhostSyncHostObject();
    ~GhostSyncHostObject();

    // The 'get' function intercepts JavaScript calls like `ghostSyncEngine.pushFrame`
    jsi::Value get(jsi::Runtime& rt, const jsi::PropNameID& name) override;
    
    // The 'set' function handles assignments like `ghostSyncEngine.prop = value`
    // We keep our engine read-only, so we won't implement much here.
    void set(jsi::Runtime& rt, const jsi::PropNameID& name, const jsi::Value& value) override;

private:
    GhostSyncEngine engine_;
};

// The global installation function called when the app starts
void installGhostSyncJSI(jsi::Runtime& jsiRuntime);