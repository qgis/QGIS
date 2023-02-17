#include <odbc/Exception.h>
#include <odbc/internal/Batch.h>
#include <odbc/internal/Macros.h>
#include <odbc/internal/Odbc.h>
#include <odbc/internal/ParameterData.h>
#include <odbc/internal/TypeInfo.h>
#include <algorithm>
#include <cassert>
#include <cstdlib>
#include <cstring>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
// Batch class
//------------------------------------------------------------------------------
void Batch::addRow()
{
    if (valueTypeInfos_.empty())
        initialize();
    else
        checkAndCompleteValueTypes();

    if (batchBlocks_.empty() || (blockRow_ == rowsPerBlock_))
    {
        // We might fetch the blocks from a Batch-object specific pool in a more
        // sophisticated version.
        batchBlocks_.emplace_back(rowsPerBlock_ * rowLength_);
        blockRow_ = 0;
    }

    dataSize_ += rowLength_;
    Block& block = batchBlocks_.back();
    char* dest = block.getData() + rowLength_ * blockRow_;
    for (size_t i = 0; i < parameters_.size(); ++i)
        writeParameter(dest + paramDataOffsets_[i], parameters_[i]);
    ++blockRow_;
}
//------------------------------------------------------------------------------
void Batch::clear()
{
    if (batchBlocks_.empty())
        return;

    // Before we can clear the vector with batch blocks, we have to deal with
    // the buffers on the heap. For the last row, we have to return ownership
    // to the parameter data. For previous rows, we have to delete the heap
    // buffer.
    for (size_t i = 0; i < parameters_.size(); ++i)
        clearBatchParameter(i);

    // We might want to put the blocks into a Batch-object specific block pool
    // instead in a more sophisticated version.
    batchBlocks_.clear();
    blockRow_ = 0;
    dataSize_ = 0;
}
//------------------------------------------------------------------------------
void Batch::execute(void* hstmt)
{
    if (batchBlocks_.empty())
        return;

    // Ensure that the batch is cleared, even if an exception occurs.
    Clearer clearer(*this);

    NextRowInfo nextRowInfo(rowLength_);
    for (size_t i = 0; i < (batchBlocks_.size() - 1); ++i)
    {
        bindBlockParameters(batchBlocks_[i].getData(), rowsPerBlock_, hstmt);
        executeBlockBatch(batchBlocks_[i].getData(), rowsPerBlock_,
            nextRowInfo, hstmt);
    }
    bindBlockParameters(batchBlocks_.back().getData(), blockRow_, hstmt);
    executeBlockBatch(batchBlocks_.back().getData(), blockRow_,
        nextRowInfo, hstmt);
}
//------------------------------------------------------------------------------
size_t Batch::getDataSize() const
{
    return dataSize_;
}
//------------------------------------------------------------------------------
void Batch::writeParameter(char* dest, ParameterData& pd)
{
    int16_t valueType = pd.getValueType();
    size_t valueSize = TypeInfo::getSizeOfValueFromValueType(valueType);
    if (valueSize == 0)
        writeVariableSizeParameter(dest, pd);
    else
        writeFixedSizeParameter(dest, pd);
}
//------------------------------------------------------------------------------
void Batch::writeVariableSizeParameter(char* dest, ParameterData& pd)
{
    if (pd.isNull())
    {
        memcpy(dest, pd.getLenIndPtr(), sizeof(SQLLEN));
    }
    else if (pd.getSize() <= ParameterData::INPLACE_BYTES)
    {
        memcpy(dest, pd.getLenIndPtr(), sizeof(SQLLEN));
        dest += sizeof(SQLLEN);
        memcpy(dest, pd.getData(), pd.getSize());
    }
    else
    {
        // We take over the ownership of the heap buffer and store a pointer
        // to the data on the heap in the data field of the row
        SQLLEN ind = SQL_LEN_DATA_AT_EXEC((SQLLEN)pd.getSize());
        memcpy(dest, &ind, sizeof(SQLLEN));
        dest += sizeof(SQLLEN);
        const void* data = pd.getData();
        memcpy(dest, &data, sizeof(data));
        if (pd.ownsHeapBuffer())
            pd.releaseHeapBufferOwnership();
        dataSize_ += pd.getSize();
    }
}
//------------------------------------------------------------------------------
void Batch::writeFixedSizeParameter(char* dest, ParameterData& pd)
{
    memcpy(dest, pd.getLenIndPtr(), sizeof(SQLLEN));
    if (!pd.isNull())
    {
        dest += sizeof(SQLLEN);
        memcpy(dest, pd.getData(), pd.getSize());
    }
}
//------------------------------------------------------------------------------
void Batch::clearBatchParameter(size_t index)
{
    // We don't need to do anything for fixed size parameters
    if (TypeInfo::getSizeOfValueFromValueType(valueTypeInfos_[index].type) != 0)
        return;

    ParameterData& pd = parameters_[index];

    // If the current parameter uses a heap buffer and has transferred
    // ownership to this batch, we must transfer back the ownership. Afterwards,
    // we have to delete all other buffers on the heap. The same buffer might
    // be used in several rows, so we have to take care to delete it only once.
    // We use the fact that the same buffer can only be used in consecutive
    // rows, so we can just remember the last one.
    const void* preserve = nullptr;
    const void* last = nullptr;
    if (pd.usesHeapBuffer() && !pd.ownsHeapBuffer())
    {
        pd.restoreHeapBufferOwnership();
        preserve = pd.getData();
    }

    size_t offset = paramDataOffsets_[index];
    for (size_t i = 0; i < (batchBlocks_.size() - 1); ++i)
    {
        char* data = batchBlocks_[i].getData() + offset;
        last = clearBatchParameterBlock(data, rowsPerBlock_, last, preserve);
    }
    char* data = batchBlocks_.back().getData() + offset;
    clearBatchParameterBlock(data, blockRow_, last, preserve);
}
//------------------------------------------------------------------------------
const void* Batch::clearBatchParameterBlock(char* data, size_t numRows,
    const void* last, const void* preserve)
{
    for (size_t i = 0; i < numRows; ++i, data += rowLength_)
    {
        SQLLEN len;
        memcpy(&len, data, sizeof(SQLLEN));
        if ((len != SQL_NULL_DATA) && (len < 0))
        {
            void* buffer;
            memcpy(&buffer, data + sizeof(SQLLEN), sizeof(void*));
            if ((buffer != last) && (buffer != preserve))
            {
                free(buffer);
                last = buffer;
            }
        }
    }
    return last;
}
//------------------------------------------------------------------------------
void Batch::bindBlockParameters(const char* blockData, size_t numRows,
    void* hstmt)
{
    EXEC_STMT(SQLFreeStmt, hstmt, SQL_UNBIND);
    EXEC_STMT(SQLFreeStmt, hstmt, SQL_RESET_PARAMS);
    EXEC_STMT(SQLSetStmtAttr, hstmt, SQL_ATTR_PARAM_BIND_TYPE,
        (SQLPOINTER)rowLength_, SQL_IS_UINTEGER);
    EXEC_STMT(SQLSetStmtAttr, hstmt, SQL_ATTR_PARAMSET_SIZE,
        (SQLPOINTER)numRows, SQL_IS_UINTEGER);

    for (size_t i = 0; i < valueTypeInfos_.size(); ++i)
    {
        const ValueTypeInfo& vti = valueTypeInfos_[i];
        SQLLEN* lenInd = (SQLLEN*)(blockData + paramDataOffsets_[i]);
        EXEC_STMT(SQLBindParameter, hstmt, (SQLUSMALLINT)(i + 1),
            SQL_PARAM_INPUT, vti.type,
            TypeInfo::getParamTypeForValueType(vti.type),
            vti.columnSize, vti.decimalDigits,
            (SQLPOINTER)(lenInd + 1), 0, lenInd);
    }
}
//------------------------------------------------------------------------------
void Batch::executeBlockBatch(const char* blockData, size_t numRows,
    NextRowInfo& nextRowInfo, void* hstmt)
{
    SQLRETURN rc = SQLExecute(hstmt);
    char* paramData = nullptr;
    if (rc == SQL_NEED_DATA)
    {
        // For variable row size parameters, determine the first variable size
        // value of the column.
        for (size_t i = 0; i < valueTypeInfos_.size(); ++i)
        {
            size_t valueSize = TypeInfo::getSizeOfValueFromValueType(
                valueTypeInfos_[i].type);
            if (valueSize != 0)
                continue;
            size_t nextRow = findNextVarSizeRow(
                blockData + paramDataOffsets_[i], 0, numRows);
            nextRowInfo.setNextRow(paramDataOffsets_[i], nextRow);
        }
        rc = SQLParamData(hstmt, (SQLPOINTER*)&paramData);
    }
    while (rc == SQL_NEED_DATA)
    {
        // Compute the offset of the length/indicator field in the row
        size_t offset = (paramData - sizeof(SQLLEN)) - blockData;

        // Get the row to send
        size_t nextRow = nextRowInfo.getNextRow(offset);

        // Compute the pointer to the field value. This field contains the
        // pointer to the buffer on the heap containing the actual data.
        char* dataPtr = paramData + nextRow * rowLength_;
        char* data;
        memcpy(&data, dataPtr, sizeof(char*));

        // Use the length/indicator field to compute the size of the data
        SQLLEN len;
        memcpy(&len, dataPtr - sizeof(SQLLEN), sizeof(SQLLEN));
        len = SQL_LEN_DATA_AT_EXEC(len);

        EXEC_STMT(SQLPutData, hstmt, data, len);

        // Advance to the next row with a value on the heap in the current
        // column
        nextRow = findNextVarSizeRow(
            blockData + offset, nextRow + 1, numRows);
        nextRowInfo.setNextRow(offset, nextRow);

        rc = SQLParamData(hstmt, (SQLPOINTER*)&paramData);
    }
    Exception::checkForError(rc, SQL_HANDLE_STMT, hstmt);
}
//------------------------------------------------------------------------------
size_t Batch::findNextVarSizeRow(const char* paramDataFirstRow, size_t startRow,
    size_t numRows)
{
    const char* data = paramDataFirstRow + startRow * rowLength_;
    for (size_t row = startRow; row < numRows; ++row, data += rowLength_)
    {
        SQLLEN ind;
        memcpy(&ind, data, sizeof(SQLLEN));
        if ((ind != SQL_NULL_DATA) && (ind < 0))
            return row;
    }
    return numRows;
}
//------------------------------------------------------------------------------
void Batch::initialize()
{
    assert(!parameters_.empty());
    valueTypeInfos_.resize(parameters_.size());
    paramDataOffsets_.resize(parameters_.size());
    dataSize_ = 0;
    rowLength_ = 0;
    for (size_t i = 0; i < parameters_.size(); ++i)
    {
        const ParameterData& param = parameters_[i];
        assert(param.isInitialized());
        valueTypeInfos_[i] = { param.getValueType(),
            param.getColumnSize(), param.getDecimalDigits() };
        paramDataOffsets_[i] = rowLength_;
        rowLength_ += sizeof(SQLLEN);
        size_t valueSize =
            TypeInfo::getSizeOfValueFromValueType(param.getValueType());
        rowLength_ +=
            (valueSize == 0) ? ParameterData::INPLACE_BYTES : valueSize;
    }
    rowsPerBlock_ = BLOCK_SIZE / rowLength_;
    if (rowsPerBlock_ < MIN_NUM_ROWS)
        rowsPerBlock_ = MIN_NUM_ROWS;
}
//------------------------------------------------------------------------------
void Batch::checkAndCompleteValueTypes()
{
    assert(parameters_.size() == valueTypeInfos_.size());
    for (size_t i = 0; i < parameters_.size(); ++i)
    {
        const ParameterData& param = parameters_[i];
        assert(param.isInitialized());
        ValueTypeInfo& valTypeInfo = valueTypeInfos_[i];
        ODBC_CHECK(
            param.getValueType() == valTypeInfo.type,
            "Value type of parameter " << (i + 1) << " does not match the "
            "previous value type used in the batch. Before it was "
            << TypeInfo::getValueTypeName(valTypeInfo.type) << ", now it is "
            << TypeInfo::getValueTypeName(
                param.getValueType()) << ".");

        if (param.getValueType() == SQL_C_NUMERIC)
        {
            // columnSize and decimalDigits might not be set during the first
            // call of addRow method. Therefore, we set their values here.
            if (valTypeInfo.columnSize == 0)
            {
                valTypeInfo.columnSize = param.getColumnSize();
                valTypeInfo.decimalDigits = param.getDecimalDigits();
            }

            ODBC_CHECK(
                param.getColumnSize() == valTypeInfo.columnSize &&
                param.getDecimalDigits() == valTypeInfo.decimalDigits,
                "Precision and scale values of parameter " << (i + 1) << " do "
                "not match the previous values used in the batch. Before it "
                "was numeric(" << valTypeInfo.columnSize << "," <<
                valTypeInfo.decimalDigits << "), now it is numeric(" <<
                param.getColumnSize() << ", " << param.getDecimalDigits() <<
                ").");
        }

        // Update column size for types with variable size
        if (TypeInfo::getSizeOfValueFromValueType(param.getValueType()) == 0)
        {
            valTypeInfo.columnSize =
                max(valTypeInfo.columnSize, param.getColumnSize());
        }
    }
}
//------------------------------------------------------------------------------
// Batch::Block class
//------------------------------------------------------------------------------
Batch::Block::Block(size_t size)
{
    data_ = (char*)malloc(size);
    if (data_ == nullptr)
        throw bad_alloc();
}
//------------------------------------------------------------------------------
Batch::Block::Block(Batch::Block&& other) : data_(other.data_)
{
    other.data_ = nullptr;
}
//------------------------------------------------------------------------------
Batch::Block::~Block()
{
    free(data_);
}
//------------------------------------------------------------------------------
// Batch::NextRowInfo class
//------------------------------------------------------------------------------
void Batch::NextRowInfo::setNextRow(size_t offset, size_t nextRow)
{
    char* data = row_.getData() + offset;
    memcpy(data, &nextRow, sizeof(size_t));
}
//------------------------------------------------------------------------------
size_t Batch::NextRowInfo::getNextRow(size_t offset) const
{
    size_t ret;
    const char* data = row_.getData() + offset;
    memcpy(&ret, data, sizeof(size_t));
    return ret;
}
//------------------------------------------------------------------------------
NS_ODBC_END
