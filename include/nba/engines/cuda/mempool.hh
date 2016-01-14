#ifndef __NBA_CUDA_MEMPOOL_HH__
#define __NBA_CUDA_MEMPOOL_HH__

#include <nba/engines/cuda/utils.hh>
#include <nba/core/mempool.hh>
#include <cstdint>
#include <cassert>
#include <cuda.h>

namespace nba {

class CUDAMemoryPool : public MemoryPool
{
public:
    CUDAMemoryPool() : MemoryPool(), base(NULL)
    {
    }

    virtual ~CUDAMemoryPool()
    {
        destroy();
    }

    virtual bool init(size_t max_size)
    {
        this->max_size = max_size;
        cutilSafeCall(cudaMalloc((void **) &base, max_size));
        return true;
    }

    void *alloc(size_t size)
    {
        size_t offset;
        int ret = _alloc(size, &offset);
        if (ret == 0)
            return (void *) ((uintptr_t) base + offset);
        return NULL;
    }

    void destroy()
    {
        if (base != NULL)
            cudaFree(base);
    }

    void *get_base_ptr() const
    {
        return base;
    }

private:
    void *base;
};

class CPUMemoryPool : public MemoryPool
{
public:
    CPUMemoryPool(int cuda_flags = 0) : MemoryPool(), base(NULL), flags(cuda_flags), use_external(false)
    {
    }

    virtual ~CPUMemoryPool()
    {
        destroy();
    }

    virtual bool init(unsigned long size)
    {
        this->max_size = size;
        cutilSafeCall(cudaHostAlloc((void **) &base, size,
                      this->flags));
        return true;
    }

    bool init_with_flags(unsigned long size, void *ext_ptr, int flags)
    {
        this->max_size = size;
        if (ext_ptr != nullptr) {
            base = ext_ptr;
            use_external = true;
        } else {
            cutilSafeCall(cudaHostAlloc((void **) &base, size,
                          flags));
        }
        return true;
    }

    void *alloc(size_t size)
    {
        size_t offset;
        int ret = _alloc(size, &offset);
        if (ret == 0)
            return (void *) ((uintptr_t) base + offset);
        return NULL;
    }

    void destroy()
    {
        if (base != NULL && !use_external)
            cudaFreeHost(base);
    }

    void *get_base_ptr() const
    {
        return base;
    }

protected:
    void *base;
    int flags;
    bool use_external;
};

}
#endif

// vim: ts=8 sts=4 sw=4 et
