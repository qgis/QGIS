#ifndef ODBC_REFCOUNTED_H_INCLUDED
#define ODBC_REFCOUNTED_H_INCLUDED
//------------------------------------------------------------------------------
#include <atomic>
#include <odbc/Config.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
template<typename T>
class Reference;
//------------------------------------------------------------------------------
/**
 * Base class for reference-counted objects.
 *
 * Reference count management is done by the Reference class.
 */
class ODBC_EXPORT RefCounted
{
    template<typename T>
    friend class Reference;

public:
    /**
     * Constructor initializing the reference-count to 1.
     */
    RefCounted();
    RefCounted(const RefCounted& other) = delete;
    RefCounted& operator=(const RefCounted& other) = delete;

protected:
    /**
     * Destructor.
     */
    virtual ~RefCounted();

private:
    void incRef() const;
    void decRef() const;

private:
    mutable std::atomic_ptrdiff_t refcount_;
};
//------------------------------------------------------------------------------
/**
 * Manager for reference-counted objects.
 *
 * Takes care of properly incrementing and decrementing reference counts of the
 * managed object.
 *
 * @tparam T  Type offering methods incRef() and decRef().
 */
template<typename T>
class Reference
{
public:
    /**
     * Constructs a NULL reference.
     */
    Reference() : ptr_(0) {}

    /**
     * Constructs a reference to a given reference-counted object.
     *
     * @param ptr     A pointer to a reference-counted object.
     * @param incref  Increases the reference-count of the passed object if set
     *                to true.
     */
    Reference(T* ptr, bool incref = false) : ptr_(ptr)
    {
        if (ptr && incref)
            ptr->incRef();
    }

    /**
     * Copy constructs a reference.
     *
     * The reference-count of the managed object is increased by 1.
     *
     * @param other  Another reference.
     */
    Reference(const Reference<T>& other) { set_(other.ptr_); }

    /**
     * Move constructor moving an existing reference.
     *
     * @param other  Another reference
     */
    Reference(Reference<T>&& other) noexcept : ptr_(other.ptr_)
    {
        other.ptr_ = nullptr;
    }

    /**
     * Destructor decreasing the reference-count of the managed object.
     */
    ~Reference() { free_(); }

public:
    /**
     * Assigns another reference to this reference.
     *
     * The reference-count of the currently managed object is decremented and
     * the reference-count of the newly managed object is incremented.
     *
     * @param other  Another reference.
     * @return       Returns a reference to this object.
     */
    Reference& operator=(const Reference<T>& other)
    {
        reset_(other.ptr_);
        return *this;
    }

    /**
     * Move-assigns another reference to this reference.
     *
     * The reference-count of the currently held object is decreased.
     *
     * @param other  Another reference.
     * @return       Returns a reference to this object.
     */
    Reference& operator=(Reference<T>&& other) noexcept
    {
        free_();
        ptr_ = other.ptr_;
        other.ptr_ = nullptr;
        return *this;
    }

    /**
     * Checks whether this reference manages an object.
     *
     * @return  Returns true if this reference manages an object, false
     *          otherwise.
     */
    bool isNull() const { return ptr_ == 0; }

    /**
     * Returns a reference to the managed object.
     *
     * @return  Returns a reference to the managed object.
     */
    T& operator*() { return *ptr_; }

    /**
     * Returns a constant reference to the managed object.
     *
     * @return  Returns a constant reference to the managed object.
     */
    const T& operator*() const { return *ptr_; }

    /**
     * Returns a pointer to the managed object.
     *
     * @return  Returns a pointer to the managed object.
     */
    T* operator->() { return ptr_; }

    /**
     * Returns a constant pointer to the managed object.
     *
     * @return  Returns a constant pointer to the managed object.
     */
    const T* operator->() const { return ptr_; }

    /**
     * Returns a pointer to the managed object.
     *
     * @return  Returns a pointer to the managed object.
     */
    T* get() { return ptr_; }

    /**
     * Returns a constant pointer to the managed object.
     *
     * @return  Returns a constant pointer to the managed object.
     */
    const T* get() const { return ptr_; }

    /**
     * Releases the currently managed object.
     *
     * Decreases the reference-count of the managed object and does not
     * reference it anymore.
     */
    void reset() { reset_(0); }

    /**
     * Releases the currently managed object and manages a new object.
     *
     * Decreases the reference-count of the managed object and increases the
     * reference-count of the newly managed object.
     *
     * @param newPtr  The newly managed object.
     */
    void reset(T* newPtr) { reset_(newPtr); }

    /**
     * Checks whether the managed object is less than the object managed by
     * another reference.
     *
     * NULL references are never considered less than another reference.
     *
     * @param other  Another reference.
     * @return       Returns true if the managed object is less than the object
     *               manages by the other reference, false otherwise.
     */
    bool operator<(const Reference<T>& other) const
    {
        if (ptr_)
        {
            if (other.ptr_)
                return *ptr_ < *other.ptr_;
            return true;
        }
        return false;
    }

private:
    void reset_(T* newPtr)
    {
        if (newPtr != ptr_)
        {
            free_();
            set_(newPtr);
        }
    }
    void free_()
    {
        if (ptr_)
            ptr_->decRef();
    }
    void set_(T* ptr)
    {
        ptr_ = ptr;
        if (ptr_)
            ptr_->incRef();
    }

private:
    T* ptr_;
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
