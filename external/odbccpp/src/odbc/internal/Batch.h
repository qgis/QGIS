#ifndef ODBC_INTERNAL_BATCH_H_INCLUDED
#define ODBC_INTERNAL_BATCH_H_INCLUDED
//------------------------------------------------------------------------------
#include <cstddef>
#include <cstdint>
#include <vector>
#include <odbc/Config.h>
#include <odbc/Forwards.h>
#include <odbc/RefCounted.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
class ParameterData;
//------------------------------------------------------------------------------
/**
 * A batch of rows consisting of parameter values.
 *
 * A batch consists of blocks that are made up by rows.
 *
 * A row contains a length/indicator field and a value field for each parameter.
 * The size of the value field depends on the value type used for the parameter.
 * The value type must be the same for all parameter values in the same column.
 *
 * In case of fixed size value types, the fixed size is used as size for the
 * value field. In case of variable size value types, the value of
 * ParameterData::INPLACE_BYTES is used as size.
 *
 * When a row is added to the batch, fixed size values and variable size values
 * with a size of at most ParameterData::INPLACE_BYTES are copied over to the
 * row. The batch will take over the heap buffer ownership of parameter values
 * that are larger than ParameterData::INPLACE_BYTES.
 */
class Batch : public RefCounted
{
    /// The maximum size of a batch block in bytes.
    constexpr static std::size_t BLOCK_SIZE = 256 * 1024;

    /// Minimum number of rows per batch block, even if that results in blocks
    /// larger than BLOCK_SIZE.
    constexpr static std::size_t MIN_NUM_ROWS = 128;

    class NextRowInfo;

public:
    /**
     * Constructor.
     *
     * @param parameters  Reference to the PreparedStatement's parameters
     *                    vector.
     */
    Batch(std::vector<ParameterData>& parameters) : parameters_(parameters) { }

    /**
     * Destructor.
     */
    ~Batch() { clear(); }

public:
    /**
     * Adds a row using the current PreparedStatement's parameter values.
     *
     * For all but the first row this method will verify that the value types
     * of the parameters matches the value types in the batch. If there is a
     * mismatch, an exception will be thrown.
     */
    void addRow();

    /**
     * Clears the batch.
     */
    void clear();

    /**
     * Executes the batch.
     *
     * The batch will be cleared after execution, even if an error occurred.
     *
     * @param hstmt  The ODBC statement andle.
     */
    void execute(void* hstmt);

    /**
     * Retrieves the number of bytes required by the batch.
     *
     * @return  Returns the number of bytes required by the batch.
     */
    std::size_t getDataSize() const;

private:
    void writeParameter(char* dest, ParameterData& pd);
    void writeVariableSizeParameter(char* dest, ParameterData& pd);
    void writeFixedSizeParameter(char* dest, ParameterData& pd);

    void clearBatchParameter(std::size_t index);
    const void* clearBatchParameterBlock(
        char* data,
        std::size_t numRows,
        const void* last,
        const void* preserve);

    void bindBlockParameters(
        const char* blockData,
        std::size_t numRows,
        void* hstmt);
    void executeBlockBatch(
        const char* blockData,
        std::size_t numRows,
        NextRowInfo& nextRowInfo,
        void* hstmt);
    size_t findNextVarSizeRow(
        const char* paramDataFirstRow,
        size_t startRow,
        size_t numRows);

    void initialize();
    void checkAndCompleteValueTypes();

private:
    /**
     * A block of memory.
     *
     * Could be replaced by vector<char> with the desired size. However,
     * vector<char> has the disadvantage that it unncecessarily initializes
     * the allocated memory.
     */
    class Block
    {
    public:
        Block(std::size_t size);
        Block(Block&& other);
        ~Block();

        Block(const Block& other) = delete;
        Block& operator=(const Block& other) = delete;

    public:
        char* getData() const { return data_; }

    private:
        char* data_;
    };

    /**
     * Helper class that ensures that the batch is cleared.
     */
    class Clearer
    {
    public:
        Clearer(Batch& parent) : parent_(parent) { }
        ~Clearer() { parent_.clear(); }
    private:
        Batch& parent_;
    };

    /**
     * Helper class storing the next row that contains a value in a heap buffer
     * and requires SQLPutData() (for each column).
     *
     * Implementing this is a bit tricky. When binding parameters we have to
     * give the pointer to the data in the first row because this pointer will
     * be used by the ODBC driver to consume the inplace values.
     *
     * This pointer will also be returned by SQLParamData() when we need to send
     * the data of a value in a heap buffer. We can easily compute the row
     * offset, but finding the index of the parameter would require a lookup
     * in some offset-to-index map. Instead we just allocate memory for a
     * whole row and store the information on the next row at the offset we can
     * easily compute.
     */
    class NextRowInfo
    {
    public:
        NextRowInfo(std::size_t rowLength) : row_(rowLength) { }

    public:
        void setNextRow(std::size_t offset, std::size_t nextRow);
        std::size_t getNextRow(std::size_t offset) const;

    private:
        Block row_;
    };

    /**
     * Helper class for storing parameter value information.
     */
    struct ValueTypeInfo
    {
        std::int16_t type;
        std::size_t columnSize;
        std::int16_t decimalDigits;
    };

private:
    /// Reference to the PreparedStatement's parameters
    std::vector<ParameterData>& parameters_;

    /// Parameter value information of the columns. Initialized when the first
    /// row is added to the batch.
    std::vector<ValueTypeInfo> valueTypeInfos_;

    /// Offsets of the length/indicator fields in a row, which is immediately
    /// followed by the value field. Initialized when the first row is added to
    /// the batch.
    std::vector<std::size_t> paramDataOffsets_;

    /// The length of a row. Initialized when the first row is added to the
    /// batch.
    std::size_t rowLength_;

    /// Number of rows per block. Calculated when the first row is added to the
    /// batch.
    std::size_t rowsPerBlock_;

    /// Vector with batch blocks. Will grow on demand.
    std::vector<Block> batchBlocks_;

    /// The number of rows in the last block (in batchBlocks_.back()).
    std::size_t blockRow_;

    /// The number of bytes required for the batch.
    std::size_t dataSize_{0};
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
