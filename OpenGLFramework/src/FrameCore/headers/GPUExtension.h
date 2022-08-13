#pragma once
#ifndef _OPENGLFRAMEWORK_GPUEXTENSION_H_
#define _OPENGLFRAMEWORK_GPUEXTENSION_H_

#include <array>
#include "Model.h"
// #include <span>

namespace GPUExtension
{
    template<typename T>
    class GPUvector
    {
        // friend GPU -> CPU()
    public:
        // TODO : If in the future nvcc supports C++20, std::span<T> is a better choice.
        GPUvector(T* init_CPUptr, size_t init_size);
        GPUvector(size_t init_size);

        size_t size() { return m_size; }
        T* dataSymbol() { return m_GPUptr; }

        void ToCPUVector(T* CPUptr);
        ~GPUvector();
    private:
        T* m_GPUptr;
        size_t m_size;
    };
    void GPUSynchronize();

    struct GPUExecuteConfig
    {
    public:
        GPUExecuteConfig(std::array<unsigned int, 3>&& init_blocksPerGrid, bool init_needSynchronization) :
            m_blocksPerGrid{ init_blocksPerGrid }, m_needSynchronization{ init_needSynchronization } {};

        GPUExecuteConfig(std::array<unsigned int, 3>&& init_blocksPerGrid, std::array<unsigned int, 3>&& init_threadsPerBlock,
            bool init_needSynchronization) : m_blocksPerGrid{ init_blocksPerGrid }, 
            m_threadsPerBlock{ init_threadsPerBlock }, m_needSynchronization{ init_needSynchronization } {};

        std::array<unsigned int, 3> m_threadsPerBlock{ 1, 1, 1};
        std::array<unsigned int, 3> m_blocksPerGrid{ 1, 1, 1};
        bool m_needSynchronization{ true };
    };

    namespace Mesh
    {
        void LoadVertices(GPUExecuteConfig& config, GPUvector<Vertex>& dstVerts, 
            GPUvector<aiVector3D>& srcVerts, GPUvector<aiVector3D>& srcNorms, GPUvector<aiVector3D>& srcTextures);
    }
}
#endif // !_OPENGLFRAMEWORK_GPUEXTENSION_H_