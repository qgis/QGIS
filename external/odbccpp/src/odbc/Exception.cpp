#include <sstream>
#include <odbc/Exception.h>
#include <odbc/internal/Odbc.h>
//------------------------------------------------------------------------------
using namespace std;
//------------------------------------------------------------------------------
namespace {
//------------------------------------------------------------------------------
bool appendRecord(short handleType, void* handle, SQLSMALLINT recNumber,
    ostringstream& out)
{
    SQLCHAR sqlState[6];
    SQLINTEGER nativeError;
    SQLCHAR messageText[2048];
    SQLSMALLINT textLength;
    SQLRETURN rc = SQLGetDiagRecA(handleType, handle, recNumber, sqlState,
        &nativeError, messageText, sizeof(messageText)/sizeof(SQLCHAR),
        &textLength);
    switch (rc)
    {
    case SQL_SUCCESS:
    case SQL_SUCCESS_WITH_INFO:
        if (recNumber > 1)
            out << endl;
        out << "ERROR: " << nativeError << ": " << sqlState << " : "
            << messageText << endl;
        return true;
    case SQL_ERROR:
        if (recNumber > 1)
            out << endl;
        out << "An error occurred while calling SQLGetDiagRec" << endl;
        return false;
    case SQL_INVALID_HANDLE:
        if (recNumber > 1)
            out << endl;
        out << "The handle passed to SQLGetDiagRec is not valid" << endl;
        return false;
    case SQL_NO_DATA:
        return false;
    default:
        if (recNumber > 1)
            out << endl;
        out << "An unknown return code was returned by SQLGetDiagRec" << endl;
        return false;
    }
}
//------------------------------------------------------------------------------
}
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
void Exception::checkForError(short rc, short handleType, void* handle)
{
    if ((rc != SQL_SUCCESS) && (rc != SQL_SUCCESS_WITH_INFO))
        throw create(handleType, handle);
}
//------------------------------------------------------------------------------
Exception Exception::create(short handleType, void* handle)
{
    ostringstream out;
    SQLSMALLINT recNumber = 1;
    while (appendRecord(handleType, handle, recNumber, out))
    {
        ++recNumber;
    }
    return Exception(out.str());
}
//------------------------------------------------------------------------------
Exception::Exception(const char* s) : msg_(s)
{
}
//------------------------------------------------------------------------------
Exception::Exception(const std::string& s) : msg_(s)
{
}
//------------------------------------------------------------------------------
const char* Exception::what() const noexcept
{
    return msg_.c_str();
}
//------------------------------------------------------------------------------
NS_ODBC_END
