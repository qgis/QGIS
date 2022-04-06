#ifndef ODBC_ENVIRONMENT_H_INCLUDED
#define ODBC_ENVIRONMENT_H_INCLUDED
//------------------------------------------------------------------------------
#include <string>
#include <vector>
#include <odbc/Config.h>
#include <odbc/Forwards.h>
#include <odbc/Types.h>
//------------------------------------------------------------------------------
NS_ODBC_START
//------------------------------------------------------------------------------
struct DataSourceInformation;
struct DriverInformation;
//------------------------------------------------------------------------------
/**
 * ODBC Environment Wrapper.
 *
 * Represents an environment context that manages connection creations.
 */
class ODBC_EXPORT Environment : public RefCounted
{
public:
    /**
     * Creates a new Environment object.
     *
     * @return  Returns a reference to the newly created Environment object.
     */
    static EnvironmentRef create();

private:
    Environment();
    ~Environment();

public:
    /**
     * Creates a new Connection object.
     *
     * @return  Returns a reference to the newly created Connection object.
     */
    ConnectionRef createConnection();

    /**
     * Retrieves information about available data sources.
     *
     * @return  Returns a full list of DataSourceInformation objects.
     */
    std::vector<DataSourceInformation> getDataSources();

    /**
     * Retrieves information about available data sources of the specified type.
     *
     * @param dsnType  Specifies the type of the returned ODBC DSNs.
     * @return         Returns a list of DataSourceInformation objects.
     */
    std::vector<DataSourceInformation> getDataSources(DSNType dsnType);

    /**
     * Retrieves information about available drivers.
     *
     * @return  Returns a list of DriverInformation objects.
     */
    std::vector<DriverInformation> getDrivers();

    /**
     * Gets a value indicating whether the given driver is installed or not.
     *
     * @param name  The driver name.
     * @return      Returns true if the driver is installed, otherwise false.
     */
    bool isDriverInstalled(const char* name);

private:
    void* henv_;
};
//------------------------------------------------------------------------------
/**
 * Stores the name and description of a data source.
 */
struct DataSourceInformation
{
    /**
     * The data source name.
     *
     * For example, dBASE Files or SQL Server.
     */
    std::string name;
    /**
     * The description of the driver associated with the data source.
     *
     * For example, Microsoft Access dBASE Driver (*.dbf, *.ndx, *.mdx).
     */
    std::string description;
};
//------------------------------------------------------------------------------
/**
 * Stores the description and additional information about the driver.
 */
struct DriverInformation
{
    /**
     * Stores the name and the value of a driver attribute.
     *
     * For example, DriverODBCVer=03.51.
     */
    struct Attribute
    {
        /**
         * The attribute's name.
         */
        std::string name;
        /**
         * The attribute's value.
         */
        std::string value;
    };

    /**
     * The driver description.
     *
     * For example, PostgreSQL Unicode(x64) or SQL Server.
     */
    std::string description;

    /**
     * The list of driver attributes.
     */
    std::vector<Attribute> attributes;
};
//------------------------------------------------------------------------------
NS_ODBC_END
//------------------------------------------------------------------------------
#endif
