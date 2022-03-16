#include <algorithm>
#include <string>
#include <utility>
#include <odbc/Exception.h>
#include <odbc/ResultSet.h>
#include <odbc/ResultSetMetaData.h>
#include <odbc/ResultSetMetaDataUnicode.h>
#include <odbc/Statement.h>
#include <odbc/internal/Macros.h>
#include <odbc/internal/Odbc.h>
#include <odbc/internal/UtilInternal.h>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
ResultSet::ResultSet(StatementBase* parent)
: parent_(parent, true)
{
}
//------------------------------------------------------------------------------
ResultSet::~ResultSet()
{
}
//------------------------------------------------------------------------------
bool ResultSet::next()
{
    SQLRETURN rc = SQLFetch(parent_->hstmt_);
    if (rc == SQL_NO_DATA)
        return false;
    Exception::checkForError(rc, SQL_HANDLE_STMT, parent_->hstmt_);
    return true;
}
//------------------------------------------------------------------------------
void ResultSet::close()
{
    EXEC_STMT(SQLFreeStmt, parent_->hstmt_, SQL_CLOSE);
}
//------------------------------------------------------------------------------
ResultSetMetaDataRef ResultSet::getMetaData()
{
    ResultSetMetaDataRef ret(new ResultSetMetaData(parent_.get()));
    return ret;
}
//------------------------------------------------------------------------------
ResultSetMetaDataUnicodeRef ResultSet::getMetaDataUnicode()
{
    ResultSetMetaDataUnicodeRef ret(
        new ResultSetMetaDataUnicode(parent_.get()));
    return ret;
}
//------------------------------------------------------------------------------
Boolean ResultSet::getBoolean(unsigned short columnIndex)
{
    bool val;
    SQLLEN ind;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex, SQL_C_BIT, &val,
        sizeof(val), &ind);
    if (ind == SQL_NULL_DATA)
        return Boolean();
    return Boolean(val);
}
//------------------------------------------------------------------------------
Byte ResultSet::getByte(unsigned short columnIndex)
{
    int8_t val;
    SQLLEN ind;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex, SQL_C_STINYINT, &val,
        sizeof(val), &ind);
    if (ind == SQL_NULL_DATA)
        return Byte();
    return Byte(val);
}
//------------------------------------------------------------------------------
UByte ResultSet::getUByte(unsigned short columnIndex)
{
    uint8_t val;
    SQLLEN ind;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex, SQL_C_UTINYINT, &val,
        sizeof(val), &ind);
    if (ind == SQL_NULL_DATA)
        return UByte();
    return UByte(val);
}
//------------------------------------------------------------------------------
Short ResultSet::getShort(unsigned short columnIndex)
{
    int16_t val;
    SQLLEN ind;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex, SQL_C_SSHORT, &val,
        sizeof(val), &ind);
    if (ind == SQL_NULL_DATA)
        return Short();
    return Short(val);
}
//------------------------------------------------------------------------------
UShort ResultSet::getUShort(unsigned short columnIndex)
{
    uint16_t val;
    SQLLEN ind;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex, SQL_C_USHORT, &val,
        sizeof(val), &ind);
    if (ind == SQL_NULL_DATA)
        return UShort();
    return UShort(val);
}
//------------------------------------------------------------------------------
Int ResultSet::getInt(unsigned short columnIndex)
{
    int32_t val;
    SQLLEN ind;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex, SQL_C_SLONG, &val,
        sizeof(val), &ind);
    if (ind == SQL_NULL_DATA)
        return Int();
    return Int(val);
}
//------------------------------------------------------------------------------
UInt ResultSet::getUInt(unsigned short columnIndex)
{
    uint32_t val;
    SQLLEN ind;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex, SQL_C_ULONG, &val,
        sizeof(val), &ind);
    if (ind == SQL_NULL_DATA)
        return UInt();
    return UInt(val);
}
//------------------------------------------------------------------------------
Long ResultSet::getLong(unsigned short columnIndex)
{
    int64_t val;
    SQLLEN ind;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex, SQL_C_SBIGINT, &val,
        sizeof(val), &ind);
    if (ind == SQL_NULL_DATA)
        return Long();
    return Long(val);
}
//------------------------------------------------------------------------------
ULong ResultSet::getULong(unsigned short columnIndex)
{
    uint64_t val;
    SQLLEN ind;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex, SQL_C_UBIGINT, &val,
        sizeof(val), &ind);
    if (ind == SQL_NULL_DATA)
        return ULong();
    return ULong(val);
}
//------------------------------------------------------------------------------
Decimal ResultSet::getDecimal(unsigned short columnIndex)
{
    SQL_NUMERIC_STRUCT num;
    SQLLEN ind;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex, SQL_C_NUMERIC, &num,
        sizeof(num), &ind);
    if (ind == SQL_NULL_DATA)
        return Decimal();
    char str[64];
    UtilInternal::numericToString(num, str);
    return Decimal(decimal(str, num.precision, num.scale));
}
//------------------------------------------------------------------------------
Float ResultSet::getFloat(unsigned short columnIndex)
{
    float val;
    SQLLEN ind;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex, SQL_C_FLOAT, &val,
        sizeof(val), &ind);
    if (ind == SQL_NULL_DATA)
        return Float();
    return Float(val);
}
//------------------------------------------------------------------------------
Double ResultSet::getDouble(unsigned short columnIndex)
{
    double val;
    SQLLEN ind;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex, SQL_C_DOUBLE, &val,
        sizeof(val), &ind);
    if (ind == SQL_NULL_DATA)
        return Double();
    return Double(val);
}
//------------------------------------------------------------------------------
Date ResultSet::getDate(unsigned short columnIndex)
{
    DATE_STRUCT ds;
    SQLLEN ind;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex, SQL_C_TYPE_DATE, &ds,
        sizeof(ds), &ind);
    if (ind == SQL_NULL_DATA)
        return Date();
    return makeNullable<date>(ds.year, ds.month, ds.day);
}
//------------------------------------------------------------------------------
Time ResultSet::getTime(unsigned short columnIndex)
{
    TIME_STRUCT ts;
    SQLLEN ind;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex, SQL_C_TYPE_TIME, &ts,
        sizeof(ts), &ind);
    if (ind == SQL_NULL_DATA)
        return Time();
    return makeNullable<time>(ts.hour, ts.minute, ts.second);
}
//------------------------------------------------------------------------------
Timestamp ResultSet::getTimestamp(unsigned short columnIndex)
{
    TIMESTAMP_STRUCT ts;
    SQLLEN ind;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex, SQL_C_TYPE_TIMESTAMP,
        &ts, sizeof(ts), &ind);
    if (ind == SQL_NULL_DATA)
        return Timestamp();
    return makeNullable<timestamp>(ts.year, ts.month, ts.day, ts.hour,
        ts.minute, ts.second, ts.fraction / 1000000);
}
//------------------------------------------------------------------------------
String ResultSet::getString(unsigned short columnIndex)
{
    SQLRETURN rc;
    SQLLEN ind;
    char dummy;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex,
        SQL_C_CHAR, &dummy, sizeof(dummy), &ind);
    if (ind == SQL_NULL_DATA)
        return String();
    if (ind == 0)
        return String("");

    string ret;
    if (ind == SQL_NO_TOTAL)
    {
        char buffer[1024];
        for (;;)
        {
            rc = SQLGetData(parent_->hstmt_, columnIndex, SQL_C_CHAR,
                buffer, sizeof(buffer), &ind);
            Exception::checkForError(rc, SQL_HANDLE_STMT, parent_->hstmt_);
            size_t len = (ind == SQL_NO_TOTAL)
                ? sizeof(buffer) - 1
                : min<size_t>(sizeof(buffer) - 1, ind);
            ret.append(buffer, len);
            if (rc == SQL_SUCCESS)
                break;
        }
    }
    else
    {
        ret.resize(ind + 1);
        EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex,
            SQL_C_CHAR, &ret[0], ret.size(), &ind);
        ret.resize(ind);
    }
    return String(move(ret));
}
//------------------------------------------------------------------------------
NString ResultSet::getNString(unsigned short columnIndex)
{
    SQLRETURN rc;
    SQLLEN ind;
    char16_t dummy;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex,
        SQL_C_WCHAR, &dummy, sizeof(dummy), &ind);
    if (ind == SQL_NULL_DATA)
        return NString();
    if (ind == 0)
        return NString(u"");

    u16string ret;
    if (ind == SQL_NO_TOTAL)
    {
        char16_t buffer[1024];
        for (;;)
        {
            rc = SQLGetData(parent_->hstmt_, columnIndex, SQL_C_WCHAR,
                buffer, sizeof(buffer), &ind);
            Exception::checkForError(rc, SQL_HANDLE_STMT, parent_->hstmt_);
            size_t len = (ind == SQL_NO_TOTAL)
                ? sizeof(buffer)/sizeof(char16_t) - 1
                : min<size_t>(sizeof(buffer)/sizeof(char16_t) - 1, ind/2);
            ret.append(buffer, len);
            if (rc == SQL_SUCCESS)
                break;
        }
    }
    else
    {
        ret.resize(ind/2 + 1);
        EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex,
            SQL_C_WCHAR, &ret[0], ret.size()*sizeof(char16_t), &ind);
        ret.resize(ind/2);
    }
    return NString(move(ret));
}
//------------------------------------------------------------------------------
Binary ResultSet::getBinary(unsigned short columnIndex)
{
    SQLRETURN rc;
    SQLLEN ind;
    char dummy;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex,
        SQL_C_BINARY, &dummy, 0, &ind);
    if (ind == SQL_NULL_DATA)
        return Binary();
    if (ind == 0)
        return Binary(vector<char>());

    vector<char> ret;
    if (ind == SQL_NO_TOTAL)
    {
        char buffer[1024];
        for (;;)
        {
            rc = SQLGetData(parent_->hstmt_, columnIndex, SQL_C_BINARY,
                buffer, sizeof(buffer), &ind);
            Exception::checkForError(rc, SQL_HANDLE_STMT, parent_->hstmt_);
            if (ind == SQL_NO_TOTAL)
                ret.insert(ret.end(), buffer, buffer + sizeof(buffer));
            else
                ret.insert(ret.end(), buffer, buffer + ind);
            if (rc == SQL_SUCCESS)
                break;
        }
    }
    else
    {
        ret.resize(ind);
        EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex,
            SQL_C_BINARY, ret.data(), ret.size(), &ind);
    }
    return Binary(move(ret));
}
//------------------------------------------------------------------------------
static size_t convertLength(SQLLEN ind)
{
    switch (ind)
    {
    case SQL_NULL_DATA:
        return ResultSet::NULL_DATA;
    case SQL_NO_TOTAL:
        return ResultSet::UNKNOWN_LENGTH;
    }
    return ind;
}
//------------------------------------------------------------------------------
size_t ResultSet::getBinaryLength(unsigned short columnIndex)
{
    SQLLEN ind;
    char dummy;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex,
        SQL_C_BINARY, &dummy, 0, &ind);
    return convertLength(ind);
}
//------------------------------------------------------------------------------
void ResultSet::getBinaryData(unsigned short columnIndex, void* data,
    size_t size)
{
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex,
        SQL_C_BINARY, (SQLPOINTER)data, size, NULL);
}
//------------------------------------------------------------------------------
size_t ResultSet::getStringLength(unsigned short columnIndex)
{
    SQLLEN ind;
    char dummy;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex,
        SQL_C_CHAR, &dummy, sizeof(dummy), &ind);
    return convertLength(ind);
}
//------------------------------------------------------------------------------
void ResultSet::getStringData(unsigned short columnIndex, void* data,
    size_t size)
{
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex,
        SQL_C_CHAR, (SQLPOINTER)data, size, NULL);
}
//------------------------------------------------------------------------------
size_t ResultSet::getNStringLength(unsigned short columnIndex)
{
    SQLLEN ind;
    char16_t dummy;
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex,
        SQL_C_WCHAR, &dummy, sizeof(dummy), &ind);
    size_t ret = convertLength(ind);
    if (ret == ResultSet::NULL_DATA || ret == ResultSet::UNKNOWN_LENGTH)
        return ret;
    return ret / sizeof(char16_t);
}
//------------------------------------------------------------------------------
void ResultSet::getNStringData(unsigned short columnIndex, void* data,
    size_t size)
{
    EXEC_STMT(SQLGetData, parent_->hstmt_, columnIndex,
        SQL_C_WCHAR, (SQLPOINTER)data, size * sizeof(char16_t), NULL);
}
//------------------------------------------------------------------------------
NS_ODBC_END
