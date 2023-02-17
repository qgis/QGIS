#include <cstring>
#include <odbc/Connection.h>
#include <odbc/Environment.h>
#include <odbc/Exception.h>
#include <odbc/internal/Macros.h>
#include <odbc/internal/Odbc.h>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
EnvironmentRef Environment::create()
{
    return EnvironmentRef(new Environment);
}
//------------------------------------------------------------------------------
Environment::Environment()
{
    SQLRETURN rc;
    rc = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &henv_);
    if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO))
        throw Exception("Could not allocate environment");

    EXEC_ENV(SQLSetEnvAttr, henv_, SQL_ATTR_ODBC_VERSION,
        (SQLPOINTER)SQL_OV_ODBC3, 0);
}
//------------------------------------------------------------------------------
Environment::~Environment()
{
    SQLFreeHandle(SQL_HANDLE_ENV, henv_);
}
//------------------------------------------------------------------------------
ConnectionRef Environment::createConnection()
{
    SQLHANDLE hdbc;
    ConnectionRef ret(new Connection(this));
    SQLRETURN rc = SQLAllocHandle(SQL_HANDLE_DBC, henv_, &hdbc);
    Exception::checkForError(rc, SQL_HANDLE_ENV, henv_);
    ret->setHandle(hdbc);
    return ret;
}
//------------------------------------------------------------------------------
vector<DataSourceInformation> Environment::getDataSources()
{
    return getDataSources(DSNType::ALL);
}
//------------------------------------------------------------------------------
vector<DataSourceInformation> Environment::getDataSources(DSNType dsnType)
{
    vector<DataSourceInformation> ret;

    SQLCHAR nameBuf[SQL_MAX_DSN_LENGTH + 1];
    vector<SQLCHAR> descBuf;
    descBuf.resize(256);
    SQLSMALLINT nameLen;
    SQLSMALLINT descLen;
    SQLUSMALLINT direction;

    switch (dsnType)
    {
    case DSNType::ALL:
        direction = SQL_FETCH_FIRST;
        break;
    case DSNType::SYSTEM:
        direction = SQL_FETCH_FIRST_SYSTEM;
        break;
    case DSNType::USER:
        direction = SQL_FETCH_FIRST_USER;
        break;
    default:
        ODBC_FAIL("Unknown DSN type.");
    }

    for (;;)
    {
        for (;;)
        {
            SQLRETURN rc = SQLDataSourcesA(henv_, direction,
                nameBuf, sizeof(nameBuf), &nameLen,
                descBuf.data(), (SQLSMALLINT)descBuf.size(), &descLen);
            if (rc == SQL_NO_DATA)
                return ret;
            Exception::checkForError(rc, SQL_HANDLE_ENV, henv_);
            if (descLen < (SQLSMALLINT)descBuf.size())
                break;
            descBuf.resize(descLen + 1);
        }

        ret.push_back({ string((const char*)nameBuf, nameLen),
          string((const char*)descBuf.data(), descLen) });
        direction = SQL_FETCH_NEXT;
    }

    return ret;
}
//------------------------------------------------------------------------------
vector<DriverInformation> Environment::getDrivers()
{
    vector<DriverInformation> ret;

    vector<SQLCHAR> descBuf;
    descBuf.resize(256);
    vector<SQLCHAR> attrBuf;
    attrBuf.resize(256);
    SQLSMALLINT descLen;
    SQLSMALLINT attrLen;
    SQLUSMALLINT direction = SQL_FETCH_FIRST;

    for (;;)
    {
        for (;;)
        {
            SQLRETURN rc = SQLDriversA(henv_, direction,
                descBuf.data(), (SQLSMALLINT)descBuf.size(), &descLen,
                attrBuf.data(), (SQLSMALLINT)attrBuf.size(), &attrLen);
            if (rc == SQL_NO_DATA)
                return ret;
            Exception::checkForError(rc, SQL_HANDLE_ENV, henv_);
            if (descLen < (SQLSMALLINT)descBuf.size() &&
                attrLen < (SQLSMALLINT)attrBuf.size())
                break;
            if (descLen >= (SQLSMALLINT)descBuf.size())
                descBuf.resize(descLen + 1);
            if (attrLen >= (SQLSMALLINT)attrBuf.size())
                attrBuf.resize(attrLen + 1);
        }

        DriverInformation driverInfo;
        driverInfo.description = string((const char*)descBuf.data(), descLen);
        if (attrLen > 0)
        {
            // Parse keyword-value pairs given in the following format
            // FileUsage=1\0FileExtns=*.dbf\0\0
            const char* start = (const char*)attrBuf.data();
            size_t attrBufLen = (size_t)attrLen;
            size_t totalLen = 0;
            while (totalLen < attrBufLen)
            {
                const char* end = strchr(start, '\0');
                if (end == nullptr)
                    throw Exception("Unable to parse driver attribute value.");
                size_t len = end - start;
                const char* sep = strchr(start, '=');
                if (sep == nullptr)
                    throw Exception("Unable to parse driver attribute value.");
                size_t pos = sep - start;
                driverInfo.attributes.push_back(
                    { string(start,  pos),
                      string(start + pos + 1, len - pos - 1) });
                totalLen += len + 1;
                start = end + 1;
            }
        }
        ret.push_back(move(driverInfo));

        direction = SQL_FETCH_NEXT;
    }

    return ret;
}
//------------------------------------------------------------------------------
bool Environment::isDriverInstalled(const char* name)
{
    vector<SQLCHAR> descBuf;
    descBuf.resize(256);
    SQLSMALLINT descLen;
    SQLSMALLINT attrLen;
    SQLUSMALLINT direction = SQL_FETCH_FIRST;

    for (;;)
    {
        for (;;)
        {
            SQLRETURN rc = SQLDriversA(henv_, direction,
                descBuf.data(), (SQLSMALLINT)descBuf.size(), &descLen,
                nullptr, 0, &attrLen);
            if (rc == SQL_NO_DATA)
                return false;
            Exception::checkForError(rc, SQL_HANDLE_ENV, henv_);
            if (descLen < (SQLSMALLINT)descBuf.size())
                break;
            descBuf.resize(descLen + 1);
        }

        if (strcmp(name, (const char*)descBuf.data()) == 0)
            return true;

        direction = SQL_FETCH_NEXT;
    }

    return false;
}
//------------------------------------------------------------------------------
NS_ODBC_END
