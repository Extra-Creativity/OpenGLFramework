#pragma once

namespace OpenGLFramework::Core
{

class ContextManager
{
public:
    static ContextManager& GetInstance();
private:
    ContextManager();
    ~ContextManager();
    static void InitImGuiContext_();
    static void InitGLFWContext_();
    static void EndAllContext_();
};

}
