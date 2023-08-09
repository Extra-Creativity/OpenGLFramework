#include "../Model.h"
#include <stdexcept>

namespace OpenGLFramework::Core
{
    
enum class SpecialModelTag { Cube, Quad };

template<SpecialModelTag>
class SpecialModel
{
public:
    static BasicTriMesh GetBasicTriMesh();
    static BasicTriRenderMesh GetBasicTriRenderMesh();
    static BasicTriModel GetBasicTriModel();
    static BasicTriRenderModel GetBasicTriRenderModel();
    SpecialModel() = delete;
};

using Cube = SpecialModel<SpecialModelTag::Cube>;
using Quad = SpecialModel<SpecialModelTag::Quad>;

}