#include "PrepareContext.h"
#include "MainWindow.h"
#include "Shader.h"
#include <fstream>

using namespace OpenGLFramework::Core;

int main()
{
    InitContext();
    MainWindow window{ 800, 600, "Test" };

    EndContext();
    return 0;
}