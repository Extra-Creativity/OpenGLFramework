#define CUDA_KERNEL(func, ...) func<<<__VA_ARGS__>>>

#include <iostream>
#include "config.h"
#include "GPUExtension.h"

namespace GPUExtension
{
    template class GPUvector<aiVector3D>;
    template class GPUvector<Vertex>;

    // TODO : if in the future nvcc supports C++20, std::source_location, std::format and [[likely]] are better.
    static void CheckError(cudaError_t error, const char* file, int line)
    {
        if (error == cudaSuccess)
            return;
        std::cerr << "Cuda Error at file " << file << ", line : " << 
            line << " :" << cudaGetErrorString(error);
        exit(EXIT_FAILURE);
    }

#define CHECK_ERROR(error) (CheckError((error), __FILE__, __LINE__))


    template<typename T>
    GPUvector<T>::GPUvector(T* init_CPUptr, size_t init_size): m_size(init_size), m_GPUptr(nullptr)
    {
        if (init_size == 0 || init_CPUptr == nullptr)
            return;
        size_t byteNum = m_size * sizeof(T);
        CHECK_ERROR(cudaMalloc(&m_GPUptr, byteNum));
        CHECK_ERROR(cudaMemcpy(m_GPUptr, init_CPUptr, byteNum, cudaMemcpyHostToDevice));
        return;
    };
    
    template<typename T>
    GPUvector<T>::GPUvector(size_t init_size) : m_size(init_size), m_GPUptr(nullptr)
    {
        if (init_size == 0)
            return;
        size_t byteNum = m_size * sizeof(T);
        CHECK_ERROR(cudaMalloc(&m_GPUptr, byteNum));
        return;
    };


    template<typename T>
    void GPUvector<T>::ToCPUVector(T* CPUptr)
    {
        CHECK_ERROR(cudaMemcpy(CPUptr, m_GPUptr, m_size * sizeof(T), cudaMemcpyDeviceToHost));
        return;
    }

    template<typename T>
    GPUvector<T>::~GPUvector()
    {
        CHECK_ERROR(cudaFree(m_GPUptr));
        return;
    }

    void GPUSynchronize()
    {
        cudaDeviceSynchronize();
        return;
    };

    namespace Mesh
    {
        __global__ void GPULoadVertices(size_t size, Vertex* dstVerts, aiVector3D* srcVerts, 
            aiVector3D* srcNorms, aiVector3D* srcTextures)
        {
            // size_t blockID = blockIdx.x + blockIdx.y * gridDim.x + blockIdx.z  * gridDim.y * gridDim.x;
            // size_t threadID = threadIdx.x + threadIdx.y * blockDim.x + threadIdx.z * blockDim.y * blockDim.x;
            // size_t id = blockID * blockDim.x * blockDim.y + threadID;
            size_t id = threadIdx.x + blockDim.x * blockIdx.x;
            if (id > size)
                return;
            Vertex& currVert = dstVerts[id];
            currVert.position.x = srcVerts[id].x;
            currVert.position.y = srcVerts[id].y;
            currVert.position.z = srcVerts[id].z;

            currVert.normalCoord.x = srcNorms[id].x;
            currVert.normalCoord.y = srcNorms[id].y;
            currVert.normalCoord.z = srcNorms[id].z;

            currVert.textureCoord.x = srcTextures[id].x;
            currVert.textureCoord.y = srcTextures[id].y;
            return;
        }

        __global__ void GPULoadVertices_OnlyVerts(size_t size, Vertex* dstVerts, aiVector3D* srcVerts)
        {
            // size_t blockID = blockIdx.x + blockIdx.y * gridDim.x + blockIdx.z  * gridDim.y * gridDim.x;
            // size_t threadID = threadIdx.x + threadIdx.y * blockDim.x + threadIdx.z * blockDim.y * blockDim.x;
            // size_t id = blockID * blockDim.x * blockDim.y + threadID;            
            size_t id = threadIdx.x + blockDim.x * blockIdx.x;
            if (id > size)
                return;
            Vertex& currVert = dstVerts[id];
            currVert.position.x = srcVerts[id].x;
            currVert.position.y = srcVerts[id].y;
            currVert.position.z = srcVerts[id].z;
            
            currVert.normalCoord.x = 0.0f;
            currVert.normalCoord.y = 0.0f;
            currVert.normalCoord.z = 0.0f;

            currVert.textureCoord.x = 0.0f;
            currVert.textureCoord.y = 0.0f;
            return;
        }

        __global__ void GPULoadVertices_NoTextures(size_t size, Vertex* dstVerts, aiVector3D* srcVerts, aiVector3D* srcNorms)
        {
            // size_t blockID = blockIdx.x + blockIdx.y * gridDim.x + blockIdx.z  * gridDim.y * gridDim.x;
            // size_t threadID = threadIdx.x + threadIdx.y * blockDim.x + threadIdx.z * blockDim.y * blockDim.x;
            // size_t id = blockID * blockDim.x * blockDim.y + threadID;      
            size_t id = threadIdx.x + blockDim.x * blockIdx.x;
            if (id > size)
                return;
            Vertex& currVert = dstVerts[id];
            currVert.position.x = srcVerts[id].x;
            currVert.position.y = srcVerts[id].y;
            currVert.position.z = srcVerts[id].z;

            currVert.normalCoord.x = srcNorms[id].x;
            currVert.normalCoord.y = srcNorms[id].y;
            currVert.normalCoord.z = srcNorms[id].z;

            currVert.textureCoord.x = 0.0f;
            currVert.textureCoord.y = 0.0f;
            return;
        }

        __global__ void GPULoadVertices_NoNorms(size_t size, Vertex* dstVerts, aiVector3D* srcVerts, aiVector3D* srcTextures)
        {
            // size_t blockID = blockIdx.x + blockIdx.y * gridDim.x + blockIdx.z  * gridDim.y * gridDim.x;
            // size_t threadID = threadIdx.x + threadIdx.y * blockDim.x + threadIdx.z * blockDim.y * blockDim.x;
            // size_t id = blockID * blockDim.x * blockDim.y + threadID;  
            size_t id = threadIdx.x + blockDim.x * blockIdx.x;
            if (id > size)
                return;
            Vertex& currVert = dstVerts[id];
            currVert.position.x = srcVerts[id].x;
            currVert.position.y = srcVerts[id].y;
            currVert.position.z = srcVerts[id].z;

            currVert.normalCoord.x = 0.0f;
            currVert.normalCoord.y = 0.0f;
            currVert.normalCoord.z = 0.0f;

            currVert.textureCoord.x = srcTextures[id].x;
            currVert.textureCoord.y = srcTextures[id].y;
            return;
        }

        void LoadVertices(GPUExecuteConfig& config, GPUvector<Vertex>& dstVerts,
            GPUvector<aiVector3D>& srcVerts, GPUvector<aiVector3D>& srcNorms, GPUvector<aiVector3D>& srcTextures)
        {
            dim3 threadsPerBlock{ config.m_threadsPerBlock[0], config.m_threadsPerBlock[1], config.m_threadsPerBlock[2] };
            dim3 blocksPerGrid{ config.m_blocksPerGrid[0], config.m_blocksPerGrid[1], config.m_blocksPerGrid[2] };

            bool hasNorms = srcNorms.size() != 0, hasTextures = srcTextures.size() != 0;
            if (hasNorms && hasTextures)
            {
                CUDA_KERNEL(GPULoadVertices, threadsPerBlock, blocksPerGrid)(dstVerts.size(), dstVerts.dataSymbol(),
                    srcVerts.dataSymbol(), srcNorms.dataSymbol(), srcTextures.dataSymbol());
            }
            else if (hasNorms && !hasTextures)
            {
                CUDA_KERNEL(GPULoadVertices_NoTextures, threadsPerBlock, blocksPerGrid)(dstVerts.size(), 
                    dstVerts.dataSymbol(), srcVerts.dataSymbol(), srcNorms.dataSymbol());
            }
            else if (!hasNorms && hasTextures)
            {
                CUDA_KERNEL(GPULoadVertices_NoNorms, threadsPerBlock, blocksPerGrid)(dstVerts.size(),
                    dstVerts.dataSymbol(), srcVerts.dataSymbol(), srcTextures.dataSymbol());
            }
            else
            {
                CUDA_KERNEL(GPULoadVertices_OnlyVerts, threadsPerBlock, blocksPerGrid)(dstVerts.size(),
                    dstVerts.dataSymbol(), srcVerts.dataSymbol());
            }

            if(config.m_needSynchronization)
            {
                GPUSynchronize();
            }
        }
    }
}