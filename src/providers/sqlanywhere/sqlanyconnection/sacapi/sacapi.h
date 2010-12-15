/* ====================================================
 *
 *       Copyright 2008-2010 iAnywhere Solutions, Inc.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 *
 *    While not a requirement of the license, if you do modify this file, we
 *    would appreciate hearing about it. Please email
 *    sqlany_interfaces@sybase.com
 *
 * ====================================================
 */

#ifndef SACAPI_H
#define SACAPI_H

/** \mainpage SQL Anywhere C API
 *
 * \section intro_sec Introduction
 * The purpose of this API is to simplify creating C/C++ wrapper drivers for various interpreted languages
 * such as PHP, Perl, Python, Ruby, and others. This API layer sits on top of DBLIB and was implemented
 * using Embedded SQL. This API is not a replacement of DBLIB. It is just a way to simplify creating applications
 * from C/C++ code without having to learn all the details of Embedded SQL.
 *
 * \section distribution Distribution of the API
 * The API is built as a DLL (shared object on UNIX). The name of the DLL is \b dbcapi.dll
 * ( \b libdbcapi.so on UNIX). This DLL is linked statically to DBLIB of the SQL Anywhere version that it was
 * built against. When dbcapi.dll is loaded, the corresponding dblibX.dll is loaded by the OS. Applications that
 * interface with dbcapi.dll can either link directly to it or load it dynamically. For dynamic
 * loading of the DLL, please refer to the Dynamic Loading section below.
 *
 * The file sacapi.h is the main header file for the API. It describes all the data types and the entry
 * points of the API.
 *
 * \section dynamic_loading Dynamically Loading the DLL
 * The header file sacapidll.h contains the proper code to dynamically load the DLL. An application would
 * need to include the sacapidll.h header file and compile in sacapidll.c with their source.
 * sqlany_initialize_interface() can be used to dynamically load the DLL and lookup all the entry points.
 *
 * \section threading_support Threading Support
 * The C API library is thread-unaware, meaning that the library does not perform any tasks that require
 * mutual exclusion. In order to allow the library to work in threaded applications, there is only one rule to
 * follow: <b>no more than one request is allowed on a single connection </b>. With this rule, the application
 * is responsible for doing mutual exclusion when accessing any connection-specific resource. This includes
 * connection handles, prepared statements, and result set objects.
 *
 * \version 2.0
 */

/** \file sacapi.h
 * Main API header file.
 * This file describes all the data types and entry points of the API.
 */

/** Defines to indicate the API versions
 */
#define SQLANY_API_VERSION_1  1

/** Version 2 introduced the "_ex" functions and the ability to cancel requests.
 */
#define SQLANY_API_VERSION_2  2

/** Returns the minimal error buffer size.
 */
#define SACAPI_ERROR_SIZE  256

#if defined(__cplusplus)
extern "C"
{
#endif

  /** A handle to an interface context
   */
  typedef struct a_sqlany_interface_context a_sqlany_interface_context;

  /** A handle to a connection object
   */
  typedef struct a_sqlany_connection a_sqlany_connection;

  /** A handle to a statement object
   */
  typedef struct a_sqlany_stmt    a_sqlany_stmt;

#ifdef WIN32
  /** A portable 32-bit signed value */
  typedef __int32    sacapi_i32;
  /** A portable 32-bit unsigned value */
  typedef unsigned __int32  sacapi_u32;
  /** A portable boolean value */
  typedef sacapi_i32   sacapi_bool;
#else
  /** A portable 32-bit signed value */
  typedef signed int    sacapi_i32;
  /** A portable 32-bit unsigned value */
  typedef unsigned int  sacapi_u32;
  /** A portable boolean value */
  typedef sacapi_i32  sacapi_bool;
#endif

// TODO:Character set issues


  /** Specifies the data type being passed in or retrieved.
   */
  typedef enum a_sqlany_data_type
  {
    /// Invalid data type.
    A_INVALID_TYPE,
    /// Binary data.  Binary data is treated as-is and no character set conversion is performed.
    A_BINARY,
    /// String data.  The data where character set conversion is performed.
    A_STRING,
    /// Double data.  Includes float values.
    A_DOUBLE,
    /// 64-bit integer.
    A_VAL64,
    /// 64-bit unsigned integer.
    A_UVAL64,
    /// 32-bit integer.
    A_VAL32,
    /// 32-bit unsigned integer.
    A_UVAL32,
    /// 16-bit integer.
    A_VAL16,
    /// 16-bit unsigned integer.
    A_UVAL16,
    /// 8-bit integer.
    A_VAL8,
    /// 8-bit unsigned integer.
    A_UVAL8
  } a_sqlany_data_type;

  /** Returns a description of the attributes of a data value.
   *
   * To view examples of the a_sqlany_data_value structure in use, see the following topics:
   *
   * <ul>
   * <li>\salink{dbcapi_isql.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-dbcapi-isql-cpp.html", "programming", "pg-c-api-dbcapi-isql-cpp"}
   * <li>\salink{fetching_a_result_set.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-fetching-a-result-set-cpp.html", "programming", "pg-c-api-fetching-a-result-set-cpp"}
   * <li>\salink{send_retrieve_full_blob.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-send-retrieve-full-blob-cpp.html", "programming", "pg-c-api-send-retrieve-full-blob-cpp"}
   * <li>\salink{preparing_statements.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-preparing-statements-cpp.html", "programming", "pg-c-api-preparing-statements-cpp"}
   * </ul>
   */
  typedef struct a_sqlany_data_value
  {
    /// A pointer to user supplied buffer of data.
    char *   buffer;
    /// The size of the buffer.
    size_t  buffer_size;
    /// A pointer to the number of valid bytes in the buffer.  This value must be less than buffer_size.
    size_t *  length;
    /// The type of the data
    a_sqlany_data_type type;
    /// A pointer to indicate whether the last fetched data is NULL.
    sacapi_bool * is_null;
  } a_sqlany_data_value;

  /** A data direction enumeration.
   */
  typedef enum a_sqlany_data_direction
  {
    /// Invalid data direction.
    DD_INVALID       = 0x0,
    /// Input-only host variables.
    DD_INPUT         = 0x1,
    /// Output-only host variables.
    DD_OUTPUT        = 0x2,
    /// Input and output host variables.
    DD_INPUT_OUTPUT  = 0x3
  } a_sqlany_data_direction;

  /** A bind parameter structure used to bind parameter and prepared statements.
   *
   * To view examples of the a_sqlany_bind_param structure in use, see the following topics:
   *
   * <ul>
   * <li>\salink{preparing_statements.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-preparing-statements-cpp.html", "programming", "pg-c-api-preparing-statements-cpp"}
   * <li>\salink{send_retrieve_full_blob.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-send-retrieve-full-blob-cpp.html", "programming", "pg-c-api-send-retrieve-full-blob-cpp"}
   * <li>\salink{send_retrieve_part_blob.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-send-retrieve-part-blob-cpp.html", "programming", "pg-c-api-send-retrieve-part-blob-cpp"}
   * </ul>
   * \sa sqlany_execute()
   */
  typedef struct a_sqlany_bind_param
  {
    /// The direction of the data. (input, output, input_output)
    a_sqlany_data_direction direction;
    /// The actual value of the data.
    a_sqlany_data_value  value;
    /// Name of the bind parameter. This is only used by sqlany_describe_bind_param().
    char   *name;
  } a_sqlany_bind_param;

  /** An enumeration of the native types of values as described by the server.
   *
   * The value types correspond to the embedded SQL data types.
   * For more information on embedded SQL data types, see
   * \salink{Embedded SQL data types, "http://dcx.sybase.com/1200en/dbprogramming/esqlvar.html", "programming", "esqlvar"}.
   *
   * \hideinitializers
   * \sa sqlany_get_column_info(), a_sqlany_column_info
   */
  typedef enum a_sqlany_native_type
  {
    /// No data type.
    DT_NOTYPE        = 0,
    /// Null-terminated character string that is a valid date.
    DT_DATE          = 384,
    /// Null-terminated character string that is a valid time.
    DT_TIME          = 388,
    /// Null-terminated character string that is a valid timestamp.
    DT_TIMESTAMP     = 392,
    /// Varying length character string, in the CHAR character set, with a two-byte length field. The maximum length is 32765 bytes . When sending data, you must set the length field. When fetching data, the database server sets the length field. The data is not null-terminated or blank-padded.
    DT_VARCHAR       = 448,
    /// Fixed-length blank-padded character string, in the CHAR character set. The maximum length, specified in bytes, is 32767. The data is not null-terminated.
    DT_FIXCHAR       = 452,
    /// Long varying length character string, in the CHAR character set.
    DT_LONGVARCHAR   = 456,
    /// Null-terminated character string, in the CHAR character set. The string is blank-padded if the database is initialized with blank-padded strings.
    DT_STRING        = 460,
    /// 8-byte floating-point number.
    DT_DOUBLE        = 480,
    /// 4-byte floating-point number.
    DT_FLOAT         = 482,
    /// Packed decimal number (proprietary format).
    DT_DECIMAL       = 484,
    /// 32-bit signed integer.
    DT_INT           = 496,
    /// 16-bit signed integer.
    DT_SMALLINT      = 500,
    /// Varying length binary data with a two-byte length field. The maximum length is 32765 bytes. When supplying information to the database server, you must set the length field. When fetching information from the database server, the server sets the length field.
    DT_BINARY        = 524,
    /// Long binary data.
    DT_LONGBINARY    = 528,
    /// 8-bit signed integer.
    DT_TINYINT       = 604,
    /// 64-bit signed integer.
    DT_BIGINT        = 608,
    /// 32-bit unsigned integer.
    DT_UNSINT        = 612,
    /// 16-bit unsigned integer.
    DT_UNSSMALLINT   = 616,
    /// 64-bit unsigned integer.
    DT_UNSBIGINT     = 620,
    /// 8-bit signed integer.
    DT_BIT           = 624,
    /// Long varying length character string, in the NCHAR character set.
    DT_LONGNVARCHAR  = 640
  } a_sqlany_native_type;

  /** Returns column metadata information.
   *
   * sqlany_get_column_info() can be used to populate this structure.
   *
   * To view an example of the a_sqlany_column_info structure in use, see
   * \salink{dbcapi_isql.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-dbcapi-isql-cpp.html", "programming", "pg-c-api-dbcapi-isql-cpp"}.
   */
  typedef struct a_sqlany_column_info
  {
    /// The name of the column (null-terminated).
    /// The string can be referenced as long as the result set object is not freed.
    char *   name;
    /// The column data type.
    a_sqlany_data_type  type;
    /// The native type of the column in the database.
    a_sqlany_native_type native_type;
    /// The precision.
    unsigned short  precision;
    /// The scale.
    unsigned short  scale;
    /// The maximum size a data value in this column can take.
    size_t    max_size;
    /// Indicates whether a value in the column can be null.
    sacapi_bool   nullable;
  } a_sqlany_column_info;

  /** Gets information about the currently bound parameters.
   *
   * sqlany_get_bind_param_info() can be used to populate this structure.
   *
   * To view examples of the a_sqlany_bind_param_info structure in use, see the following topics:
   *
   * <ul>
   * <li>\salink{preparing_statements.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-preparing-statements-cpp.html", "programming", "pg-c-api-preparing-statements-cpp"}
   * <li>\salink{send_retrieve_full_blob.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-send-retrieve-full-blob-cpp.html", "programming", "pg-c-api-send-retrieve-full-blob-cpp"}
   * <li>\salink{send_retrieve_part_blob.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-send-retrieve-part-blob-cpp.html", "programming", "pg-c-api-send-retrieve-part-blob-cpp"}
   * </ul>
   * \sa sqlany_execute()
   */
  typedef struct a_sqlany_bind_param_info
  {
    /// A pointer to the name of the parameter.
    char *       name;
    /// The direction of the parameter.
    a_sqlany_data_direction  direction;
    /// Information about the bound input value.
    a_sqlany_data_value  input_value;
    /// Information about the bound output value.
    a_sqlany_data_value  output_value;
  } a_sqlany_bind_param_info;

  /** Returns metadata information about a column value in a result set.
   *
   * sqlany_get_data_info() can be used
   * to populate this structure with information about what was last retrieved by a fetch operation.
   *
   * To view an example of the a_sqlany_data_data_info structure in use, see
   * \salink{send_retrieve_part_blob.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-send-retrieve-part-blob-cpp.html", "programming", "pg-c-api-send-retrieve-part-blob-cpp"}.
   * \sa sqlany_get_data_info()
   */
  typedef struct a_sqlany_data_info
  {
    /// The type of the data in the column.
    a_sqlany_data_type type;
    /// Indicates whether the last fetched data is NULL.
    /// This field is only valid after a successful fetch operation.
    sacapi_bool  is_null;
    /// The total number of bytes available to be fetched.
    /// This field is only valid after a successful fetch operation.
    size_t   data_size;
  } a_sqlany_data_info;

  /** Initializes the interface.
   *
   * The following example demonstrates how to initialize the SQL Anywhere C API DLL:
   *
   * <pre>
   * sacapi_u32 api_version;
   * if( sqlany_init( "PHP", SQLANY_API_VERSION_1, &api_version ) ) {
   *     printf( "Interface initialized successfully!\n" );
   * } else {
   *     printf( "Failed to initialize the interface! Supported version=%d\n", api_version );
   * }
   * </pre>
   *
   * \param app_name A string that names the application that is using the API.  For example, "PHP", "PERL", or "RUBY".
   * \param api_version The version of the compiled application.
   * \param version_available An optional argument to return the maximum supported API version.
   * \return 1 on success, 0 otherwise
   * \sa sqlany_fini()
   */
  sacapi_bool sqlany_init( const char * app_name, sacapi_u32 api_version, sacapi_u32 * version_available );

#if _SACAPI_VERSION+0 >= 2
  /** Initializes the interface using a context.
   *
   * \param app_name A string that names the API used, for example "PHP", "PERL", or "RUBY".
   * \param api_version The current API version that the application is using.
   * This should normally be one of the SQLANY_API_VERSION_* macros
   * \param version_available An optional argument to return the maximum API version that is supported.
   * \return a context object on success and NULL on failure.
   * \sa sqlany_fini_ex()
   */
  a_sqlany_interface_context *sqlany_init_ex( const char * app_name, sacapi_u32 api_version, sacapi_u32 * version_available );
#endif

  /** Finalizes the interface.
   *
   * Frees any resources allocated by the API.
   *
   * \sa sqlany_init()
  */
  void sqlany_fini();
#if _SACAPI_VERSION+0 >= 2
  /** Finalize the interface that was created using the specified context.
   * Frees any resources allocated by the API.
   * \param context A context object that was returned from sqlany_init_ex()
   * \sa sqlany_init_ex()
   */
  void sqlany_fini_ex( a_sqlany_interface_context *context );
#endif

  /** Creates a connection object.
   *
   * You must create an API connection object before establishing a database connection. Errors can be retrieved
   * from the connection object. Only one request can be processed on a connection at a time. In addition,
   * not more than one thread is allowed to access a connection object at a time. Undefined behavior or a failure
   * occurs when multiple threads attempt to access a connection object simultaneously.
   *
   * \return A connection object
   * \sa sqlany_connect(), sqlany_disconnect()
   */
  a_sqlany_connection * sqlany_new_connection( void );
#if _SACAPI_VERSION+0 >= 2
  /** Creates a connection object using a context.
   * An API connection object needs to be created before a database connection is established. Errors can be retrieved
   * from the connection object. Only one request can be processed on a connection at a time. In addition,
   * not more than one thread is allowed to access a connection object at a time. If multiple threads attempt
   * to access a connection object simultaneously, then undefined behaviour/crashes will occur.
   * \param context A context object that was returned from sqlany_init_ex()
   * \return A connection object
   * \sa sqlany_connect(), sqlany_disconnect(), sqlany_init_ex()
   */
  a_sqlany_connection *sqlany_new_connection_ex( a_sqlany_interface_context *context );
#endif

  /** Frees the resources associated with a connection object.
   *
   * \param sqlany_conn A connection object created with sqlany_new_connection().
   * \sa sqlany_new_connection()
   */
  void sqlany_free_connection( a_sqlany_connection *sqlany_conn );

  /** Creates a connection object based on a supplied DBLIB SQLCA pointer.
   *
   * \param arg A void * pointer to a DBLIB SQLCA object.
   * \return A connection object.
   * \sa sqlany_new_connection(), sqlany_execute(), sqlany_execute_direct(), sqlany_execute_immediate(), sqlany_prepare()
   */
  a_sqlany_connection * sqlany_make_connection( void * arg );
#if _SACAPI_VERSION+0 >= 2
  /** Creates a connection object based on a supplied DBLIB SQLCA pointer and context.
   * \param context A valid context object that was created by sqlany_init_ex()
   * \param arg A void * pointer to a DBLIB SQLCA object.
   * \return A connection object.
   * \sa sqlany_init_ex(), sqlany_execute(), sqlany_execute_direct(), sqlany_execute_immediate(), sqlany_prepare()
   */
  a_sqlany_connection *sqlany_make_connection_ex( a_sqlany_interface_context *context, void *arg );
#endif

  /** Creates a connection to a SQL Anywhere database server using the supplied connection object and connection string.
   *
   * The supplied connection object must first be allocated using sqlany_new_connection().
   *
   * The following example demonstrates how to retrieve the error code of a failed connection attempt:
   *
   * <pre>
   * a_sqlany_connection * sqlany_conn;
   * sqlany_conn = sqlany_new_connection();
   * if( !sqlany_connect( sqlany_conn, "uid=dba;pwd=sql" ) ) {
   *     char reason[SACAPI_ERROR_SIZE];
   *     sacapi_i32 code;
   *     code = sqlany_error( sqlany_conn, reason, sizeof(reason) );
   *     printf( "Connection failed. Code: %d Reason: %s\n", code, reason );
   * } else {
   *     printf( "Connected successfully!\n" );
   *     sqlany_disconnect( sqlany_conn );
   * }
   * sqlany_free_connection( sqlany_conn );
   * </pre>
   *
   * For more information on connecting to a SQL Anywhere database server, see
   * \salink{Connection parameters, "http://dcx.sybase.com/1200en/dbadmin/da-conparm.html", "dbadmin", "da-conparm"} and
   * \salink{SQL Anywhere database connections, "http://dcx.sybase.com/1200en/dbadmin/da-dbconnections.html", "dbadmin", "da-dbconnections"}.
   *
   * \param sqlany_conn A connection object created by sqlany_new_connection().
   * \param str A SQL Anywhere connection string.
   * \return 1 if the connection is established successfully or 0 when the connection fails. Use sqlany_error() to
   * retrieve the error code and message.
   * \sa sqlany_new_connection(), sqlany_error()
   */
  sacapi_bool sqlany_connect( a_sqlany_connection * sqlany_conn, const char * str );

  /** Disconnects an already established SQL Anywhere connection.
   *
   * All uncommitted transactions are rolled back.
   *
   * \param sqlany_conn A connection object with a connection established using sqlany_connect().
   * \return 1 when successful or 0 when unsuccessful.
   * \sa sqlany_connect(), sqlany_new_connection()
   */
  sacapi_bool sqlany_disconnect( a_sqlany_connection * sqlany_conn );

#if _SACAPI_VERSION+0 >= 2
  /** Cancel an outstanding request on a connection
   * This function can be used to cancel an outstanding request on a specific connection.
   * \param sqlany_conn A connection object with a connection established using sqlany_connect().
   */
  void sqlany_cancel( a_sqlany_connection * sqlany_conn );
#endif

  /** Executes the supplied SQL statement immediately without returning a result set.
   *
   * This function is useful for SQL statements that do not return a result set.
   *
   * \param sqlany_conn A connection object with a connection established using sqlany_connect().
   * \param sql A string representing the SQL statement to be executed.
   * \return 1 on success or 0 on failure.
   */
  sacapi_bool sqlany_execute_immediate( a_sqlany_connection * sqlany_conn, const char * sql );

  /** Prepares a supplied SQL string.
   *
   * Execution does not happen until sqlany_execute() is
   * called. The returned statement object should be freed using sqlany_free_stmt().
   *
  * The following statement demonstrates how to prepare a SELECT SQL string:
   *
   * <pre>
   * char * str;
   * a_sqlany_stmt * stmt;
   *
   * str = "select * from employees where salary >= ?";
   * stmt = sqlany_prepare( sqlany_conn, str );
   * if( stmt == NULL ) {
   *     // Failed to prepare statement, call sqlany_error() for more info
   * }
   * </pre>
   *
   * \param sqlany_conn A connection object with a connection established using sqlany_connect().
   * \param sql_str The SQL statement to be prepared.
   * \return A handle to a SQL Anywhere statement object. The statement object can be used by sqlany_execute()
   * to execute the statement.
   * \sa sqlany_free_stmt(), sqlany_connect(), sqlany_execute(), sqlany_num_params(), sqlany_describe_bind_param(), sqlany_bind_param()
   */
  a_sqlany_stmt * sqlany_prepare( a_sqlany_connection * sqlany_conn, const char * sql_str );

  /** Frees resources associated with a prepared statement object.
   *
   * \param sqlany_stmt A statement object returned by the successful execution of sqlany_prepare() or sqlany_execute_direct().
   * \sa sqlany_prepare(), sqlany_execute_direct()
   */
  void sqlany_free_stmt( a_sqlany_stmt * sqlany_stmt );

  /** Returns the number of parameters expected for a prepared statement.
   *
   * \param sqlany_stmt A statement object returned by the successful execution of sqlany_prepare().
   * \return The expected number of parameters, or -1 if the statement object is not valid.
   * \sa sqlany_prepare()
   */
  sacapi_i32 sqlany_num_params( a_sqlany_stmt * sqlany_stmt );

  /** Describes the bind parameters of a prepared statement.
   *
   * This function allows the caller to determine information about prepared statement parameters.  The type of prepared
   * statement, stored procedured or a DML, determines the amount of information provided.  The direction of the parameters
   * (input, output, or input-output) are always provided.
   *
   * \param sqlany_stmt A statement prepared successfully using sqlany_prepare().
   * \param index The index of the parameter. This number must be between 0 and sqlany_num_params() - 1.
   * \param param A a_sqlany_bind_param structure that is populated with information.
   * \return 1 when successful or 0 when unsuccessful.
   * \sa sqlany_bind_param(), sqlany_prepare()
   */
  sacapi_bool sqlany_describe_bind_param( a_sqlany_stmt * sqlany_stmt, sacapi_u32 index, a_sqlany_bind_param * param );

  /** Bind a user-supplied buffer as a parameter to the prepared statement.
   *
   * \param sqlany_stmt A statement prepared successfully using sqlany_prepare().
   * \param index The index of the parameter. This number must be between 0 and sqlany_num_params() - 1.
   * \param param A a_sqlany_bind_param structure description of the parameter to be bound.
   * \return 1 on success or 0 on unsuccessful.
   * \sa sqlany_describe_bind_param()
   */
  sacapi_bool sqlany_bind_param( a_sqlany_stmt * sqlany_stmt, sacapi_u32 index, a_sqlany_bind_param * param );

  /** Sends data as part of a bound parameter.
   *
   * This method can be used to send a large amount of data for a bound parameter in chunks.
   *
   * \param sqlany_stmt A statement prepared successfully using sqlany_prepare().
   * \param index The index of the parameter. This should be a number between 0 and sqlany_num_params() - 1.
   * \param buffer The data to be sent.
   * \param size The number of bytes to send.
   * \return 1 on success or 0 on failure.
   * \sa sqlany_prepare()
   */
  sacapi_bool sqlany_send_param_data( a_sqlany_stmt * sqlany_stmt, sacapi_u32 index, char * buffer, size_t size );

  /** Resets a statement to its prepared state condition.
   *
   * \param sqlany_stmt A statement prepared successfully using sqlany_prepare().
   * \return 1 on success, 0 on failure.
   * \sa sqlany_prepare()
   */
  sacapi_bool sqlany_reset( a_sqlany_stmt * sqlany_stmt );

  /** Retrieves information about the parameters that were bound using sqlany_bind_param().
   *
   * \param sqlany_stmt A statement prepared successfully using sqlany_prepare().
   * \param index The index of the parameter. This number should be between 0 and sqlany_num_params() - 1.
   * \param info A sqlany_bind_param_info buffer to be populated with the bound parameter's information.
   * \return 1 on success or 0 on failure.
   * \sa sqlany_bind_param(), sqlany_describe_bind_param(), sqlany_prepare()
   */
  sacapi_bool sqlany_get_bind_param_info( a_sqlany_stmt * sqlany_stmt, sacapi_u32 index, a_sqlany_bind_param_info * info );

  /** Executes a prepared statement.
   *
   * You can use sqlany_num_cols() to verify if the executed statement returned a result set.
   *
   * The following example shows how to execute a statement that does not return a result set:
   *
   * <pre>
   * a_sqlany_stmt *   stmt;
   * int         i;
   * a_sqlany_bind_param   param;
   *
   * stmt = sqlany_prepare( sqlany_conn, "insert into moe(id,value) values( ?,? )" );
   * if( stmt ) {
   *     sqlany_describe_bind_param( stmt, 0, &param );
   *     param.value.buffer = (char *)\&i;
   *     param.value.type   = A_VAL32;
   *     sqlany_bind_param( stmt, 0, &param );
   *
   *     sqlany_describe_bind_param( stmt, 1, &param );
   *     param.value.buffer = (char *)\&i;
   *     param.value.type   = A_VAL32;
   *     sqlany_bind_param( stmt, 1, &param );
   *
   *     for( i = 0; i < 10; i++ ) {
   *         if( !sqlany_execute( stmt ) ) {
   *            // call sqlany_error()
   *        }
   *     }
   *     sqlany_free_stmt( stmt );
   * }
   * </pre>
   *
   * \param sqlany_stmt A statement prepared successfully using sqlany_prepare().
   * \return 1 if the statement is executed successfully or 0 on failure.
   * \sa sqlany_prepare()
   */
  sacapi_bool sqlany_execute( a_sqlany_stmt * sqlany_stmt );

  /** Executes the SQL statement specified by the string argument and possibly returns a result set.
   *
   * Use this method if you want to prepare and execute a statement,
   * or instead of calling sqlany_prepare() followed by sqlany_execute().
   *
   * The following example shows how to execute a statement that returns a result set:
   *
   * <pre>
   * stmt = sqlany_execute_direct( sqlany_conn, "select * from employees" ) ) {
   * if( stmt && sqlany_num_cols( stmt ) > 0 ) {
   *     while( sqlany_fetch_next( stmt ) ) {
   *         int i;
   *        for( i = 0; i < sqlany_num_cols( stmt ); i++ ) {
   *             // Get column i data
   *         }
   *     }
   *     sqlany_free_stmt( stmt  );
   * }
   * </pre>
   *
   * <em>Note:</em> This function can not be used for executing a SQL statement with parameters.
   *
   * \param sqlany_conn A connection object with a connection established using sqlany_connect().
   * \param sql_str A SQL string. The SQL string should not have parameters such as ?.
   * \return A statement handle if the function executes successfully, NULL when the function executes unsuccessfully.
   * \sa sqlany_fetch_absolute(), sqlany_fetch_next(), sqlany_num_cols(), sqlany_get_column()
   */
  a_sqlany_stmt * sqlany_execute_direct( a_sqlany_connection * sqlany_conn, const char * sql_str );

  /** Moves the current row in the result set to the row number specified and then fetches the data at that row.
   *
   * \param sqlany_stmt A statement object that was executed by
   *     sqlany_execute() or sqlany_execute_direct().
   * \param row_num The row number to be fetched. The first row is 1, the last row is -1.
   * \return 1 if the fetch was successfully, 0 when the fetch is unsuccessful.
   * \sa sqlany_execute_direct(), sqlany_execute(), sqlany_error(), sqlany_fetch_next()
   */
  sacapi_bool sqlany_fetch_absolute( a_sqlany_stmt * sqlany_stmt, sacapi_i32 row_num );

  /** Returns the next row from the result set.
   *
   * This function fetches the next row from the result set.
   * When the result object is first created, the current row
   * pointer is set to before the first row (i.e. row 0).
   * This function first advances the row pointer and then fetches
   * the data at the new row.
   *
   * \param sqlany_stmt A statement object that was executed by
   *     sqlany_execute() or sqlany_execute_direct().
   * \return 1 if the fetch was successfully, 0 when the fetch is unsuccessful.
   * \sa sqlany_fetch_absolute(), sqlany_execute_direct(), sqlany_execute(), sqlany_error()
   */
  sacapi_bool sqlany_fetch_next( a_sqlany_stmt * sqlany_stmt );

  /** Advances to the next result set in a multiple result set query.
   *
   * If a query (such as a call to a stored procedure) returns multiple result sets, then this function
   * advances from the current result set to the next.
   *
  * The following example demonstrates how to advance to the next result set in a multiple result set query:
   *
   * <pre>
   * stmt = sqlany_execute_direct( sqlany_conn, "call my_multiple_results_procedure()" );
   * if( result ) {
   *     do {
   *         while( sqlany_fetch_next( stmt ) ) {
   *            // get column data
   *         }
   *     } while( sqlany_get_next_result( stmt ) );
   *     sqlany_free_stmt( stmt );
   * }
   * </pre>
   *
   * \param sqlany_stmt A statement object executed by
   *     sqlany_execute() or sqlany_execute_direct().
   * \return 1 if the statement successfully advances to the next result set, 0 otherwise.
   * \sa sqlany_execute_direct(), sqlany_execute()
   */
  sacapi_bool sqlany_get_next_result( a_sqlany_stmt * sqlany_stmt );

  /** Returns the number of rows affected by execution of the prepared statement.
   *
   * \param sqlany_stmt A statement that was prepared and executed successfully with no result set returned.
   *                    For example, an INSERT, UPDATE or DELETE statement was executed.
   * \return The number of rows affected or -1 on failure.
   * \sa sqlany_execute(), sqlany_execute_direct()
   */
  sacapi_i32 sqlany_affected_rows( a_sqlany_stmt * sqlany_stmt );

  /** Returns number of columns in the result set.
   *
   * \param sqlany_stmt A statement object created by sqlany_prepare() or sqlany_execute_direct().
   * \return The number of columns in the result set or -1 on a failure.
   * \sa sqlany_execute(), sqlany_execute_direct(), sqlany_prepare()
   */
  sacapi_i32 sqlany_num_cols( a_sqlany_stmt * sqlany_stmt );

  /** Returns the number of rows in the result set.
   *
   * By default this function only returns an estimate. To return an exact count, set the row_counts option
   * on the connection. For more information on the row_counts option, see
   * \salink{row_counts option [database], "http://dcx.sybase.com/1200en/dbadmin/row-counts-option.html", "dbadmin", "row-counts-option"}.
   *
   * \param sqlany_stmt A statement object that was executed by
   *     sqlany_execute() or sqlany_execute_direct().
   * \return The number rows in the result set. If the number of rows is an estimate, the number returned is
   * negative and the estimate is the absolute value of the returned integer. The value returned is positive
   * if the number of rows is exact.
   * \sa sqlany_execute_direct(), sqlany_execute()
   */
  sacapi_i32 sqlany_num_rows( a_sqlany_stmt * sqlany_stmt );

  /** Fills the supplied buffer with the value fetched for the specified column.
   *
   * For A_BINARY and A_STRING * data types,
   * value->buffer points to an internal buffer associated with the result set.
   * Do not rely upon or alter the content of the pointer buffer as it changes when a
   * new row is fetched or when the result set object is feed.  Users should copy the
   * data out of those pointers into their own buffers.
   *
   * The value->length field indicates the number of valid characters that
   * value->buffer points to. The data returned in value->buffer is not
   * null-terminated. This function fetches all the returned values from the SQL
   * Anywhere database server.  For example, if the column contains
   * a 2GB blob, this function attempts to allocate enough memory to hold that value.
   * If you do not want to allocate memory, use sqlany_get_data() instead.
   *
   * \param sqlany_stmt A statement object executed by
   *     sqlany_execute() or sqlany_execute_direct().
   * \param col_index The number of the column to be retrieved.
   * A column number is between 0 and sqlany_num_cols() - 1.
   * \param buffer A a_sqlany_data_value object to be filled with the data fetched for column col_index.
   * \return 1 on success or 0 for failure. A failure can happen if any of the parameters are invalid or if there is
   * not enough memory to retrieve the full value from the SQL Anywhere database server.
   * \sa sqlany_execute_direct(), sqlany_execute(), sqlany_fetch_absolute(), sqlany_fetch_next()
   */
  sacapi_bool sqlany_get_column( a_sqlany_stmt * sqlany_stmt, sacapi_u32 col_index, a_sqlany_data_value * buffer );

  /** Retrieves the data fetched for the specified column into the supplied buffer memory.
   *
   * \param sqlany_stmt A statement object executed by
   *     sqlany_execute() or sqlany_execute_direct().
   * \param col_index The number of the column to be retrieved.
   * A column number is between 0 and sqlany_num_cols() - 1.
   * \param offset The starting offset of the data to get.
   * \param buffer A buffer to be filled with the contents of the column. The buffer pointer must be aligned correctly
   * for the data type copied into it.
   * \param size The size of the buffer in bytes. The function fails
   * if you specify a size greater than 2GB.
   * \return The number of bytes successfully copied into the supplied buffer.
   * This number must not exceed 2GB.
   * 0 indicates that no data remains to be copied.  -1 indicates a failure.
   * \sa sqlany_execute(), sqlany_execute_direct(), sqlany_fetch_absolute(), sqlany_fetch_next()
   */
  sacapi_i32 sqlany_get_data( a_sqlany_stmt * sqlany_stmt, sacapi_u32 col_index, size_t offset, void * buffer, size_t size );

  /** Retrieves information about the data that was fetched by the last fetch operation.
   *
   * \param sqlany_stmt A statement object executed by
   *     sqlany_execute() or sqlany_execute_direct().
   * \param col_index The column number between 0 and sqlany_num_cols() - 1.
   * \param buffer A data info buffer to be filled with the metadata about the data fetched.
   * \return 1 on success, and 0 on failure. Failure is returned when any of the supplied parameters are invalid.
   * \sa sqlany_execute(), sqlany_execute_direct(), sqlany_fetch_absolute(), sqlany_fetch_next()
   */
  sacapi_bool sqlany_get_data_info( a_sqlany_stmt * sqlany_stmt, sacapi_u32 col_index, a_sqlany_data_info * buffer );

  /** Retrieves column metadata information and fills the a_sqlany_column_info structure with information about the column.
   *
   * \param sqlany_stmt A statement object created by sqlany_prepare() or sqlany_execute_direct().
   * \param col_index The column number between 0 and sqlany_num_cols() - 1.
   * \param buffer A column info structure to be filled with column information.
   * \return 1 on success or 0 if the column index is out of range,
   * or if the statement does not return a result set.
   * \sa sqlany_execute(), sqlany_execute_direct(), sqlany_prepare()
   */
  sacapi_bool sqlany_get_column_info( a_sqlany_stmt * sqlany_stmt, sacapi_u32 col_index, a_sqlany_column_info * buffer );

  /** Commits the current transaction.
   *
   * \param sqlany_conn The connection object on which the commit operation is performed.
   * \return 1 when successful or 0 when unsuccessful.
   * \sa sqlany_rollback()
   */
  sacapi_bool sqlany_commit( a_sqlany_connection * sqlany_conn );

  /** Rolls back the current transaction.
   *
   * \param sqlany_conn The connection object on which the rollback operation is to be performed.
   * \return 1 on success, 0 otherwise.
   * \sa sqlany_commit()
   */
  sacapi_bool sqlany_rollback( a_sqlany_connection * sqlany_conn );

  /** Returns the current client version.
   *
   * This method fills the buffer passed with the major, minor, patch, and build number of the client library.
   * The buffer will be null-terminated.
   *
   * \param buffer The buffer to be filled with the client version string.
   * \param len The length of the buffer supplied.
   * \return 1 when successful or 0 when unsuccessful.
   */
  sacapi_bool sqlany_client_version( char * buffer, size_t len );
#if _SACAPI_VERSION+0 >= 2
  /** Returns the current client version.
   *
  * This method fills the buffer passed with the major, minor, patch, and build number of the client library.
   * The buffer will be null-terminated.
   *
   * \param context object that was create with sqlany_init_ex()
   * \param buffer The buffer to be filled with the client version string.
   * \param len The length of the buffer supplied.
   * \return 1 when successful or 0 when unsuccessful.
   * \sa sqlany_init_ex()
   */
  sacapi_bool sqlany_client_version_ex( a_sqlany_interface_context *context, char *buffer, size_t len );
#endif

  /** Retrieves the last error code and message stored in the connection object.
   *
   * For more information on SQLCODE error messages, see
   * \salink{SQL Anywhere error messages sorted by SQLCODE, "http://dcx.sybase.com/1200en/saerrors/sa-errors-by-sqlcode.html", "errors", "sa-errors-by-sqlcode"}.
   *
   * \param sqlany_conn A connection object returned from sqlany_new_connection().
   * \param buffer A buffer to be filled with the error message.
   * \param size The size of the supplied buffer.
   * \return The last error code. Positive values are warnings, negative values are errors, and 0 indicates success.
   * \sa sqlany_connect()
   */
  sacapi_i32 sqlany_error( a_sqlany_connection * sqlany_conn, char * buffer, size_t size );

  /** Retrieves the current SQLSTATE.
   *
   * For more information on SQLSTATE error messages, see
   * \salink{SQL Anywhere error messages sorted by SQLSTATE, "http://dcx.sybase.com/1200en/saerrors/sa-errors-by-sqlstate.html", "errors", "sa-errors-by-sqlstate"}.
   *
   * \param sqlany_conn A connection object returned from sqlany_new_connection().
   * \param buffer A buffer to be filled with the current 5-character SQLSTATE.
   * \param size The buffer size.
   * \return The number of bytes copied into the buffer.
   * \sa sqlany_error()
   */
  size_t sqlany_sqlstate( a_sqlany_connection * sqlany_conn, char * buffer, size_t size );

  /** Clears the last stored error code
   *
   * \param sqlany_conn A connection object returned from sqlany_new_connection().
   * \sa sqlany_new_connection()
   */
  void sqlany_clear_error( a_sqlany_connection * sqlany_conn );

#if defined(__cplusplus)
}
#endif

/** \example connecting.cpp
 * This is an example of how to create a connection object and connect with it to SQL Anywhere.
 */

/** \example fetching_a_result_set.cpp
 * This example shows how to fetch data from a result set.
 */

/** \example preparing_statements.cpp
 * This example shows how to prepare and execute a statement.
 */

/** \example fetching_multiple_from_sp.cpp
 * This example shows how to fetch multiple result sets from a stored procedure.
 */

/** \example send_retrieve_part_blob.cpp
 * This example shows how to insert a blob in chunks and retrieve it in chunks too.
 */

/** \example send_retrieve_full_blob.cpp
 * This example shows how to insert and retrieve a blob in one chunk .
 */

/** \example dbcapi_isql.cpp
 * This example shows how to write an ISQL application using dbcapi.
 */

#endif
