#ifndef ODBC_INTERNAL_PARAMETERDATA_H_INCLUDED
#define ODBC_INTERNAL_PARAMETERDATA_H_INCLUDED
//------------------------------------------------------------------------------
#include <cstddef>
#include <cinttypes>
#include <odbc/Config.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
/**
 * Class that holds the data of a parameter.
 *
 * The data of a parameter consists of its type and its value, which can be
 * NULL.
 *
 * This class has a small inplace buffer that can hold numeric, date/time-
 * related types and short strings without the need for additional memory
 * allocation from the heap. Larger values are allocated from the heap.
 *
 * The data of this class is intended to be transferrable to a block with batch
 * data. Values in the inplace buffer are just copied over. If a value is
 * located in a buffer allocated from the heap, the ownership of the heap buffer
 * is transferred to the batch block.
 *
 * As the data in the buffer can be re-added (i.e. without the user setting a
 * new value for the next row), we have to keep the pointer to the data, but
 * have to mark that we don't own the heap buffer anymore.
 *
 * Ownership of the buffer can also be transferred back to a ParameterData
 * object. This happens if a batch is executed/cleared and a ParameterData
 * object holds a pointer to the memory buffer in the batch.
 */
class ParameterData
{
public:
    // Size of the inplace buffer
    static constexpr std::size_t INPLACE_BYTES = 32;

    // Required load factor so that no re-allocation occurs if a buffer
    // allocated from the heap receives new content.
    static constexpr double LOAD_FACTOR = 0.75;

public:
    /**
     * Creates an uninitialized instance.
     */
    ParameterData();

    ParameterData(const ParameterData& other) = delete;

    /**
     * Creates an instance and moves the data of another instance over to this
     * instance.
     *
     * @param other  Another instance.
     */
    ParameterData(ParameterData&& other);

    /**
     * Destructor.
     */
    ~ParameterData();

    ParameterData& operator=(const ParameterData& other) = delete;

    /**
     * Moves the parameter data of another instance to this instance.
     *
     * @param other  Another instance.
     * @return       Returns a reference to this instance.
     */
    ParameterData& operator=(ParameterData&& other);

public:
    /**
     * Sets the type and value of the parameter.
     *
     * @param type       The ODBC-C-type of the value.
     * @param value      Pointer to the value.
     * @param size       Size of the value in bytes.
     */
    void setValue(std::int16_t type, const void* value, std::size_t size);

    /**
     * Sets the type of this parameter and sets the value to NULL.
     *
     * @param type  The ODBC-C-type of the value.
     */
    void setNull(std::int16_t type);

    /**
     * Clears the parameter.
     *
     * The instance will be uninitialized after this method has been called.
     */
    void clear();

    /**
     * Checks whether the value has been initialized.
     *
     * @return  True if the value has been initialized, false otherwise.
     */
    bool isInitialized() const { return state_ != UNINITIALIZED; }

    /**
     * Returns the ODBC-C-type of the value.
     *
     * @return  The ODBC-C-type of the value.
     */
    std::int16_t getValueType() const { return valueType_; }

    /**
     * Returns the size of the parameter. If the parameter type is SQL_DECIMAL,
     * SQL_NUMERIC, SQL_FLOAT, SQL_REAL or SQL_DOUBLE the returned value
     * represents the maximum precision of the corrresponding parameter. If the
     * parameter type is SQL_CHAR, SQL_VARCHAR, SQL_LONGVARCHAR, SQL_BINARY,
     * SQL_VARBINARY, SQL_LONGVARBINARY, SQL_TYPE_DATE, SQL_TYPE_TIME or
     * SQL_TYPE_TIMESTAMP the returned value represents the maximum length of
     * the corresponding parameter in bytes.
     *
     * @return  The column size.
     */
    std::size_t getColumnSize() const { return columnSize_; }

    /**
     * Sets the column size of this parameter. If the parameter type is
     * SQL_DECIMAL, SQL_NUMERIC, SQL_FLOAT, SQL_REAL or SQL_DOUBLE the value
     * represents the maximum precision of the corrresponding parameter. If the
     * parameter type is SQL_CHAR, SQL_VARCHAR, SQL_LONGVARCHAR, SQL_BINARY,
     * SQL_VARBINARY, SQL_LONGVARBINARY, SQL_TYPE_DATE, SQL_TYPE_TIME or
     * SQL_TYPE_TIMESTAMP the value represents the maximum length of the
     * corresponding parameter in bytes.
     *
     * @param value  The column size.
     */
    void setColumnSize(std::size_t value) { columnSize_ = value; }

    /**
     * Returns the number of decimal digits of this parameter. If the parameter
     * type is SQL_DECIMAL or SQL_NUMERIC the value represents the scale of the
     * corresponding parameter. If the parameter type is SQL_TYPE_TIME or
     * SQL_TYPE_TIMESTAMP the value represents the precision of the
     * corresponding parameter.
     *
     * @return  The number of decimal digits.
     */
    std::int16_t getDecimalDigits() const { return decimalDigits_; }

    /**
     * Sets the number of decimal digits of this parameter. If the parameter
     * type is SQL_DECIMAL or SQL_NUMERIC the value represents the scale of the
     * corresponding parameter. If the parameter type is SQL_TYPE_TIME or
     * SQL_TYPE_TIMESTAMP the value represents the precision of the
     * corresponding parameter.
     *
     * @param value  The number of decimal digits.
     */
    void setDecimalDigits(std::int16_t value) { decimalDigits_ = value; }

    /**
     * Checks whether the value is NULL.
     *
     * @return  True if the value is NULL, false otherwise.
     */
    bool isNull() const { return state_ == IS_NULL; }

    /**
     * Returns a pointer to the data.
     *
     * The result is undefined if the value is either uninitialized or NULL.
     *
     * @return  Returns a pointer to the data.
     */
    const void* getData() const;

    /**
     * Returns the size of the data in bytes.
     *
     * The result is undefined if the value is either uninitialized or NULL.
     *
     * @return  The size of the data in bytes.
     */
    std::size_t getSize() const { return size_; }

    /**
     * Returns a pointer to the value's length or NULL indicator.
     *
     * The result is undefined if the value is uninitialized.
     *
     * @return  A pointer to the value's length or NULL indicator.
     */
    const void* getLenIndPtr() const { return &size_; }

    /**
     * Checks whether the data is located in a buffer on the heap.
     *
     * @return  True if the data is located in a buffer on the heap, false
     *          if the data is in the inplace buffer or if the value is NULL.
     */
    bool usesHeapBuffer() const;

    /**
     * Checks whether this instance owns a buffer on the heap.
     *
     * @return  True if this instance owns a buffer on the heap, false
     *          otherwise.
     */
    bool ownsHeapBuffer() const { return state_ == NORMAL_HEAP_OWNING; }

    /**
     * Returns the capacity of the buffer on the heap.
     *
     * This method must only be called if usesHeapBuffer() returns true.
     *
     * @return  Returns the capacity of the buffer on the heap.
     */
    std::size_t getHeapBufferCapacity() const { return capacity_; }

    /**
     * Releases the heap buffer ownership.
     *
     * This method must only be called if ownsHeapBuffer() returns true.
     * Afterwards the caller is responsible for the buffer on the heap. This
     * means the caller has either to free that memory or he has to return
     * ownership to this instance via restoreHeapBufferOwnership().
     *
     * The pointer to the buffer can be retrieved via getData().
     */
    void releaseHeapBufferOwnership();

    /**
     * Restores the heap buffer ownership.
     *
     * This method must only be called if usesHeapBuffer() return true and
     * ownsHeapBuffer() returns false.
     *
     * After calling this method, this instance will manage the lifetime of the
     * heap buffer again.
     */
    void restoreHeapBufferOwnership();

private:
    void setValueInplace(const void* value, std::size_t size);
    void setValueOnHeap(const void* value, std::size_t size);

private:
    enum State : std::uint8_t
    {
        /// Uninitialized
        UNINITIALIZED,
        /// Value is NULL
        IS_NULL,
        /// Normal value stored in-place
        NORMAL_INPLACE,
        /// Normal value stored in buffer on heap that is ownerd by this object
        NORMAL_HEAP_OWNING,
        /// Normal value stored in buffer on heap whose ownership has been
        /// transferred
        NORMAL_HEAP_NOT_OWNING,
    };

private:
    // We have the following invariants:
    //  - inplaceData_ is the active union member iff state_ is NORMAL_INPLACE
    //  - the structure is the active union member iff state_ is
    //    NORMAL_HEAP_OWNING or NORMAL_HEAP_NOT_OWNING
    State state_;
    std::int16_t valueType_;
    std::size_t columnSize_;
    std::int16_t decimalDigits_;
    std::size_t size_;
    union
    {
        char inplaceData_[INPLACE_BYTES];
        struct
        {
            std::size_t capacity_;
            void* heapData_;
        };
    };
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
