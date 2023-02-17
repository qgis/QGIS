#include <string>
#include <odbc/Connection.h>
#include <odbc/Exception.h>
#include <odbc/ParameterMetaData.h>
#include <odbc/PreparedStatement.h>
#include <odbc/ResultSet.h>
#include <odbc/ResultSetMetaData.h>
#include <odbc/ResultSetMetaDataUnicode.h>
#include <odbc/internal/Batch.h>
#include <odbc/internal/Macros.h>
#include <odbc/internal/Odbc.h>
#include <odbc/internal/ParameterData.h>
#include <odbc/internal/TypeInfo.h>
#include <odbc/internal/UtilInternal.h>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
PreparedStatement::PreparedStatement(Connection* parent) : StatementBase(parent)
{
}
//------------------------------------------------------------------------------
PreparedStatement::~PreparedStatement()
{
}
//------------------------------------------------------------------------------
void PreparedStatement::setHandleAndQuery(void* hstmt, const char* query)
{
    hstmt_ = hstmt;

    EXEC_STMT(SQLPrepareA, hstmt, (SQLCHAR*)query, SQL_NTS);

    SQLSMALLINT count;
    EXEC_STMT(SQLNumParams, hstmt_, &count);
    parameterData_.resize(count);

    batch_.reset(new Batch(parameterData_));
}
//------------------------------------------------------------------------------
void PreparedStatement::setHandleAndQuery(void* hstmt, const char16_t* query)
{
    hstmt_ = hstmt;

    EXEC_STMT(SQLPrepareW, hstmt, (SQLWCHAR*)query, SQL_NTS);

    SQLSMALLINT count;
    EXEC_STMT(SQLNumParams, hstmt_, &count);
    parameterData_.resize(count);

    batch_.reset(new Batch(parameterData_));
}
//------------------------------------------------------------------------------
ParameterMetaDataRef PreparedStatement::getParameterMetaData()
{
    ParameterMetaDataRef ret(new ParameterMetaData(this));
    return ret;
}
//------------------------------------------------------------------------------
void PreparedStatement::setBoolean(unsigned short paramIndex,
    const Boolean& value)
{
    setFixedSizeData(paramIndex, value);
}
//------------------------------------------------------------------------------
void PreparedStatement::setByte(unsigned short paramIndex, const Byte& value)
{
    setFixedSizeData(paramIndex, value);
}
//------------------------------------------------------------------------------
void PreparedStatement::setUByte(unsigned short paramIndex, const UByte& value)
{
    setFixedSizeData(paramIndex, value);
}
//------------------------------------------------------------------------------
void PreparedStatement::setShort(unsigned short paramIndex, const Short& value)
{
    setFixedSizeData(paramIndex, value);
}
//------------------------------------------------------------------------------
void PreparedStatement::setUShort(unsigned short paramIndex,
    const UShort& value)
{
    setFixedSizeData(paramIndex, value);
}
//------------------------------------------------------------------------------
void PreparedStatement::setInt(unsigned short paramIndex, const Int& value)
{
    setFixedSizeData(paramIndex, value);
}
//------------------------------------------------------------------------------
void PreparedStatement::setUInt(unsigned short paramIndex, const UInt& value)
{
    setFixedSizeData(paramIndex, value);
}
//------------------------------------------------------------------------------
void PreparedStatement::setLong(unsigned short paramIndex, const Long& value)
{
    setFixedSizeData(paramIndex, value);
}
//------------------------------------------------------------------------------
void PreparedStatement::setULong(unsigned short paramIndex, const ULong& value)
{
    setFixedSizeData(paramIndex, value);
}
//------------------------------------------------------------------------------
void PreparedStatement::setDecimal(unsigned short paramIndex,
    const Decimal& value)
{
    if (value.isNull())
    {
        parameterData_[paramIndex - 1].setNull(TypeToOdbc<decimal>::VALUETYPE);
    }
    else
    {
        SQL_NUMERIC_STRUCT ns;
        UtilInternal::decimalToNumeric(*value, ns);
        ParameterData& pd = parameterData_[paramIndex - 1];
        pd.setValue(TypeToOdbc<decimal>::VALUETYPE, &ns, sizeof(ns));
        pd.setColumnSize(ns.precision);
        pd.setDecimalDigits(ns.scale);
    }
}
//------------------------------------------------------------------------------
void PreparedStatement::setFloat(unsigned short paramIndex, const Float& value)
{
    setFixedSizeData(paramIndex, value);
}
//------------------------------------------------------------------------------
void PreparedStatement::setDouble(unsigned short paramIndex,
    const Double& value)
{
    setFixedSizeData(paramIndex, value);
}
//------------------------------------------------------------------------------
void PreparedStatement::setString(unsigned short paramIndex,
    const String& value)
{
    if (value.isNull())
        setCString(paramIndex, nullptr, 0);
    else
        setCString(paramIndex, value->c_str(), value->length());
}
//------------------------------------------------------------------------------
void PreparedStatement::setCString(unsigned short paramIndex, const char* s)
{
    if (s == nullptr)
        setCString(paramIndex, nullptr, 0);
    else
        setCString(paramIndex, s, char_traits<char>::length(s));
}
//------------------------------------------------------------------------------
void PreparedStatement::setCString(unsigned short paramIndex, const char* s,
    std::size_t len)
{
    verifyValidParamIndex(paramIndex);
    ParameterData& pd = parameterData_[paramIndex - 1];
    if (s == nullptr)
    {
        pd.setNull(SQL_C_CHAR);
    }
    else
    {
        pd.setValue(SQL_C_CHAR, s, len);
        pd.setColumnSize(len);
    }
}
//------------------------------------------------------------------------------
void PreparedStatement::setNString(unsigned short paramIndex,
    const NString& value)
{
    if (value.isNull())
        setNCString(paramIndex, nullptr, 0);
    else
        setNCString(paramIndex, value->c_str(), value->length());
}
//------------------------------------------------------------------------------
void PreparedStatement::setNCString(unsigned short paramIndex,
    const char16_t* s)
{
    if (s == nullptr)
        setNCString(paramIndex, nullptr, 0);
    else
        setNCString(paramIndex, s, char_traits<char16_t>::length(s));
}
//------------------------------------------------------------------------------
void PreparedStatement::setNCString(unsigned short paramIndex,
    const char16_t* s, std::size_t len)
{
    verifyValidParamIndex(paramIndex);
    ParameterData& pd = parameterData_[paramIndex - 1];
    if (s == nullptr)
    {
        pd.setNull(SQL_C_WCHAR);
    }
    else
    {
        pd.setValue(SQL_C_WCHAR, s, len*sizeof(char16_t));
        pd.setColumnSize(len);
    }
}
//------------------------------------------------------------------------------
void PreparedStatement::setBinary(unsigned short paramIndex,
    const Binary& value)
{
    if (value.isNull())
        setBytes(paramIndex, nullptr, 0);
    else
        setBytes(paramIndex, value->data(), value->size());
}
//------------------------------------------------------------------------------
void PreparedStatement::setBytes(unsigned short paramIndex, const void* data,
    size_t size)
{
    verifyValidParamIndex(paramIndex);
    ParameterData& pd = parameterData_[paramIndex - 1];
    if (data == nullptr)
    {
        pd.setNull(SQL_C_BINARY);
    }
    else
    {
        pd.setValue(SQL_C_BINARY, data, size);
        pd.setColumnSize(size);
    }
}
//------------------------------------------------------------------------------
void PreparedStatement::setDate(unsigned short paramIndex, const Date& value)
{
    verifyValidParamIndex(paramIndex);
    if (value.isNull())
    {
        parameterData_[paramIndex - 1].setNull(TypeToOdbc<date>::VALUETYPE);
    }
    else
    {
        SQL_DATE_STRUCT ds = {
            (SQLSMALLINT)value->year(),
            (SQLUSMALLINT)value->month(),
            (SQLUSMALLINT)value->day()
        };
        parameterData_[paramIndex - 1].setValue(
            TypeToOdbc<date>::VALUETYPE, &ds, sizeof(ds));
    }
}
//------------------------------------------------------------------------------
void PreparedStatement::setTime(unsigned short paramIndex, const Time& value)
{
    verifyValidParamIndex(paramIndex);
    if (value.isNull())
    {
        parameterData_[paramIndex - 1].setNull(TypeToOdbc<time>::VALUETYPE);
    }
    else
    {
        SQL_TIME_STRUCT ts = {
            (SQLUSMALLINT)value->hour(),
            (SQLUSMALLINT)value->minute(),
            (SQLUSMALLINT)value->second()
        };
        parameterData_[paramIndex - 1].setValue(
            TypeToOdbc<time>::VALUETYPE, &ts, sizeof(ts));
    }
}
//------------------------------------------------------------------------------
void PreparedStatement::setTimestamp(unsigned short paramIndex,
    const Timestamp& value)
{
    verifyValidParamIndex(paramIndex);
    if (value.isNull())
    {
        parameterData_[paramIndex - 1].setNull(
            TypeToOdbc<timestamp>::VALUETYPE);
    }
    else
    {
        SQL_TIMESTAMP_STRUCT tss = {
            (SQLSMALLINT)value->year(),
            (SQLUSMALLINT)value->month(),
            (SQLUSMALLINT)value->day(),
            (SQLUSMALLINT)value->hour(),
            (SQLUSMALLINT)value->minute(),
            (SQLUSMALLINT)value->second(),
            (SQLUINTEGER)value->milliseconds() * 1000000
        };
        parameterData_[paramIndex - 1].setValue(
            TypeToOdbc<timestamp>::VALUETYPE, &tss, sizeof(tss));
    }
}
//------------------------------------------------------------------------------
void PreparedStatement::clearParameters()
{
    for (ParameterData& pd : parameterData_)
        pd.clear();
}
//------------------------------------------------------------------------------
ResultSetMetaDataRef PreparedStatement::getMetaData()
{
    ResultSetMetaDataRef ret(new ResultSetMetaData(this));
    return ret;
}
//------------------------------------------------------------------------------
ResultSetMetaDataUnicodeRef PreparedStatement::getMetaDataUnicode()
{
    ResultSetMetaDataUnicodeRef ret(new ResultSetMetaDataUnicode(this));
    return ret;
}
//------------------------------------------------------------------------------
ResultSetRef PreparedStatement::executeQuery()
{
    ResultSetRef ret(new ResultSet(this));
    EXEC_STMT(SQLFreeStmt, hstmt_, SQL_CLOSE);
    bindParameters();
    EXEC_STMT(SQLExecute, hstmt_);
    return ret;
}
//------------------------------------------------------------------------------
size_t PreparedStatement::executeUpdate()
{
    EXEC_STMT(SQLFreeStmt, hstmt_, SQL_CLOSE);
    bindParameters();
    SQLRETURN rc = SQLExecute(hstmt_);
    if (rc == SQL_NO_DATA)
        return 0;
    Exception::checkForError(rc, SQL_HANDLE_STMT, hstmt_);
    SQLLEN numRows;
    EXEC_STMT(SQLRowCount, hstmt_, &numRows);
    return numRows;
}
//------------------------------------------------------------------------------
void PreparedStatement::addBatch()
{
    verifyAllParametersValid();
    batch_->addRow();
}
//------------------------------------------------------------------------------
void PreparedStatement::executeBatch()
{
    batch_->execute(hstmt_);
}
//------------------------------------------------------------------------------
void PreparedStatement::clearBatch()
{
    batch_->clear();
}
//------------------------------------------------------------------------------
size_t PreparedStatement::getBatchDataSize() const
{
    return batch_->getDataSize();
}
//------------------------------------------------------------------------------
void PreparedStatement::bindParameters()
{
    verifyAllParametersValid();
    for (size_t i = 1; i <= parameterData_.size(); ++i)
    {
        const ParameterData& pd = parameterData_[i - 1];
        if (pd.isNull())
        {
            EXEC_STMT(SQLBindParameter, hstmt_, (SQLUSMALLINT)i,
                SQL_PARAM_INPUT, pd.getValueType(),
                TypeInfo::getParamTypeForValueType(pd.getValueType()),
                0, 0, 0, 0, (SQLLEN*)pd.getLenIndPtr());
        }
        else
        {
            EXEC_STMT(SQLBindParameter, hstmt_, (SQLUSMALLINT)i,
                SQL_PARAM_INPUT, pd.getValueType(),
                TypeInfo::getParamTypeForValueType(pd.getValueType()),
                pd.getColumnSize(), pd.getDecimalDigits(),
                (SQLPOINTER)pd.getData(), pd.getSize(),
                (SQLLEN*)pd.getLenIndPtr());
        }
    }
}
//------------------------------------------------------------------------------
template<typename T>
void PreparedStatement::setFixedSizeData(
    unsigned short paramIndex, const Nullable<T>& value)
{
    verifyValidParamIndex(paramIndex);
    if (value.isNull())
        parameterData_[paramIndex - 1].setNull(TypeToOdbc<T>::VALUETYPE);
    else
        parameterData_[paramIndex - 1].setValue(
            TypeToOdbc<T>::VALUETYPE, &(*value), sizeof(T));
}
//------------------------------------------------------------------------------
void PreparedStatement::verifyValidParamIndex(unsigned short paramIndex) const
{
    ODBC_CHECK(
        (paramIndex >= 1) && ((size_t)paramIndex <= parameterData_.size()),
        "Invalid parameter index (" << paramIndex << ")");
}
//------------------------------------------------------------------------------
void PreparedStatement::verifyAllParametersValid() const
{
    for (size_t i = 0; i < parameterData_.size(); ++i)
    {
        ODBC_CHECK(parameterData_[i].isInitialized(),
            "Parameter " << (i+1) << " has not been set");
    }
}
//------------------------------------------------------------------------------
NS_ODBC_END
