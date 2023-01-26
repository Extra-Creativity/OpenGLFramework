#pragma once
#ifndef OPENGLFRAMEWORK_CORE_CONTEXTMANAGER_H_
#define OPENGLFRAMEWORK_CORE_CONTEXTMANAGER_H_

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
#endif // !OPENGLFRAMEWORK_CORE_CONTEXTMANAGER_H_
