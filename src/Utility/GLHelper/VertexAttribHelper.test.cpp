#include "VertexAttribHelper.h"
#include <glm/glm.hpp>
#include <vector>

struct TestVertAttrib
{
    glm::vec3 normal;
    glm::vec3 texCoord;
};

BEGIN_REFLECT(TestVertAttrib);
REFLECT(1, float, normal);
REFLECT(2, float, texCoord);
END_REFLECT(2);

VERTEX_ATTRIB_SPECIALIZE_COPY(std::vector<TestVertAttrib>& v,
    const aiMesh* mesh) {}

using namespace OpenGLFramework::GLHelper;

int main()
{
    IVertexAttribContainer c = std::vector<TestVertAttrib>{};
    return 0;
}