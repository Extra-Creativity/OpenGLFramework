#include "../Model.h"

namespace OpenGLFramework::Core
{

class Cube
{
public:
    static BasicTriMesh GetBasicTriMesh();
    static BasicTriRenderMesh GetBasicTriRenderMesh();
    static BasicTriModel GetBasicTriModel();
    static BasicTriRenderModel GetBasicTriRenderModel();
    Cube() = delete;
};
}