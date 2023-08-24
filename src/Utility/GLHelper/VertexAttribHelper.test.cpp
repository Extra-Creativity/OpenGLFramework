#include "VertexAttribHelper.h"
#include "glm/glm.hpp"
#include <vector>

struct TestVertAttrib
{
    glm::vec3 normal;
    glm::vec3 texCoord;
};

BEGIN_REFLECT(TestVertAttrib, 2);
REFLECT(0, float, normal);
REFLECT(1, float, texCoord);
END_REFLECT;

using namespace OpenGLFramework::GLHelper;

int main()
{
    IVertexAttribContainer c = std::vector<TestVertAttrib>{};
    return 0;
}