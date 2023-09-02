#include "VertexAttribHelper.h"
#include <glm/glm.hpp>
#include <vector>
#include <memory_resource>

using namespace OpenGLFramework::GLHelper;

struct TestVertAttrib
{
    glm::vec3 normal;
    glm::vec3 texCoord;
};

class TestVertAttribContainer : public IVertexAttribContainer
{
public:
    void AllocateAndBind(size_t posSize, size_t vertexNum) final {
        IVertexAttribContainer::AllocateAttributes_(container, posSize, vertexNum);
        BEGIN_BIND(TestVertAttrib, posSize, vertexNum);
        BIND_VERTEX_ATTRIB(1, float, normal);
        BIND_VERTEX_ATTRIB(2, float, texCoord);
        END_BIND;
    };

    void CopyFromMesh(const aiMesh*) final { }
private:
    std::vector<TestVertAttrib> container;
};

//We may introduce customized deleter in the future.
//class MemoryOptimizedVACFactory : public IVertexAttibContainerFactory
//{
//    std::pmr::monotonic_buffer_resource buffer;
//public:
//    MemoryOptimizedVACFactory(size_t maxSize) : buffer{ maxSize } { }
//    std::unique_ptr<IVertexAttribContainer> Create() override {
//    }
//};

int main()
{
    NaiveVACFactory<TestVertAttribContainer> fac;
    return 0;
}