#include <odbc/RefCounted.h>
#include <cstddef>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
RefCounted::RefCounted() : refcount_(1)
{
}
//------------------------------------------------------------------------------
RefCounted::~RefCounted()
{
}
//------------------------------------------------------------------------------
void RefCounted::incRef() const
{
    refcount_.fetch_add(1, std::memory_order_relaxed);
}
//------------------------------------------------------------------------------
void RefCounted::decRef() const
{
    if (refcount_.fetch_sub(1, std::memory_order_release) == 1)
    {
        std::atomic_thread_fence(std::memory_order_acquire);
        delete this;
    }
}
//------------------------------------------------------------------------------
NS_ODBC_END
