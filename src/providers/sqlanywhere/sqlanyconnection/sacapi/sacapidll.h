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

#ifndef SACAPIDLL_H
#define SACAPIDLL_H

#include "sacapi.h"

/** \file sacapidll.h
 * \brief Header file for stub that can dynamically load the main API DLL.
 * The user will need to include sacapidll.h in their source files and compile in sacapidll.c
 */


#if defined( __cplusplus )
extern "C"
{
#endif
  typedef sacapi_bool( *sqlany_init_func )( const char * app_name, sacapi_u32 api_version, sacapi_u32 * max_version );
  typedef void ( *sqlany_fini_func )();
  typedef a_sqlany_connection * ( *sqlany_new_connection_func )( );
  typedef void ( *sqlany_free_connection_func )( a_sqlany_connection *sqlany_conn );
  typedef a_sqlany_connection * ( *sqlany_make_connection_func )( void * arg );
  typedef sacapi_bool( *sqlany_connect_func )( a_sqlany_connection * sqlany_conn, const char * str );
  typedef sacapi_bool( *sqlany_disconnect_func )( a_sqlany_connection * sqlany_conn );
  typedef sacapi_bool( *sqlany_execute_immediate_func )( a_sqlany_connection * sqlany_conn, const char * sql );
  typedef a_sqlany_stmt * ( *sqlany_prepare_func )( a_sqlany_connection * sqlany_conn, const char * sql_str );
  typedef void ( *sqlany_free_stmt_func )( a_sqlany_stmt * sqlany_stmt );
  typedef sacapi_i32( *sqlany_num_params_func )( a_sqlany_stmt * sqlany_stmt );
  typedef sacapi_bool( *sqlany_describe_bind_param_func )( a_sqlany_stmt * sqlany_stmt, sacapi_u32 index, a_sqlany_bind_param * params );
  typedef sacapi_bool( *sqlany_bind_param_func )( a_sqlany_stmt * sqlany_stmt, sacapi_u32 index, a_sqlany_bind_param * params );
  typedef sacapi_bool( *sqlany_send_param_data_func )( a_sqlany_stmt * sqlany_stmt, sacapi_u32 index, char * buffer, size_t size );
  typedef sacapi_bool( *sqlany_reset_func )( a_sqlany_stmt * sqlany_stmt );
  typedef sacapi_bool( *sqlany_get_bind_param_info_func )( a_sqlany_stmt * sqlany_stmt, sacapi_u32 index, a_sqlany_bind_param_info * info );
  typedef sacapi_bool( *sqlany_execute_func )( a_sqlany_stmt * sqlany_stmt );
  typedef a_sqlany_stmt * ( *sqlany_execute_direct_func )( a_sqlany_connection * sqlany_conn, const char * sql_str );
  typedef sacapi_bool( *sqlany_fetch_absolute_func )( a_sqlany_stmt * sqlany_result, sacapi_i32 row_num );
  typedef sacapi_bool( *sqlany_fetch_next_func )( a_sqlany_stmt * sqlany_stmt );
  typedef sacapi_bool( *sqlany_get_next_result_func )( a_sqlany_stmt * sqlany_stmt );
  typedef sacapi_i32( *sqlany_affected_rows_func )( a_sqlany_stmt * sqlany_stmt );
  typedef sacapi_i32( *sqlany_num_cols_func )( a_sqlany_stmt * sqlany_stmt );
  typedef sacapi_i32( *sqlany_num_rows_func )( a_sqlany_stmt * sqlany_stmt );
  typedef sacapi_bool( *sqlany_get_column_func )( a_sqlany_stmt * sqlany_stmt, sacapi_u32 col_index, a_sqlany_data_value * buffer );
  typedef sacapi_i32( *sqlany_get_data_func )( a_sqlany_stmt * sqlany_stmt, sacapi_u32 col_index, size_t offset, void * buffer, size_t size );
  typedef sacapi_bool( *sqlany_get_data_info_func )( a_sqlany_stmt * sqlany_stmt, sacapi_u32 col_index, a_sqlany_data_info * buffer );
  typedef sacapi_bool( *sqlany_get_column_info_func )( a_sqlany_stmt * sqlany_stmt, sacapi_u32 col_index, a_sqlany_column_info * buffer );
  typedef sacapi_bool( *sqlany_commit_func )( a_sqlany_connection * sqlany_conn );
  typedef sacapi_bool( *sqlany_rollback_func )( a_sqlany_connection * sqlany_conn );
  typedef sacapi_bool( *sqlany_client_version_func )( char * buffer, size_t len );
  typedef sacapi_i32( *sqlany_error_func )( a_sqlany_connection * sqlany_conn, char * buffer, size_t size );
  typedef size_t ( *sqlany_sqlstate_func )( a_sqlany_connection * sqlany_conn, char * buffer, size_t size );
  typedef void ( *sqlany_clear_error_func )( a_sqlany_connection * sqlany_conn );
#if _SACAPI_VERSION+0 >= 2
  typedef a_sqlany_interface_context *( *sqlany_init_ex_func )( const char *app_name, sacapi_u32 api_version, sacapi_u32 *max_version );
  typedef void ( *sqlany_fini_ex_func )( a_sqlany_interface_context *context );
  typedef a_sqlany_connection *( *sqlany_new_connection_ex_func )( a_sqlany_interface_context *context );
  typedef a_sqlany_connection *( *sqlany_make_connection_ex_func )( a_sqlany_interface_context *context, void *arg );
  typedef sacapi_bool( *sqlany_client_version_ex_func )( a_sqlany_interface_context *context, char *buffer, size_t len );
  typedef void ( *sqlany_cancel_func )( a_sqlany_connection * sqlany_conn );
#endif

#if defined( __cplusplus )
}
#endif

/// @internal
#define function( x )  x ## _func x

/** The SQL Anywhere C API interface structure.
 *
 * Only one instance of this structure is required in your application environment.  This structure
 * is initialized by the sqlany_initialize_interface method.   It attempts to load the SQL Anywhere C
 * API DLL or shared object dynamically and looks up all the entry points of the DLL.  The fields in
 * the SQLAnywhereInterface structure is populated to point to the corresponding functions in the DLL.
 * \sa sqlany_initialize_interface()
 */
typedef struct SQLAnywhereInterface
{
  /** DLL handle.
   */
  void * dll_handle;

  /** Flag to know if initialized or not.
   */
  int    initialized;

  /** Pointer to ::sqlany_init() function.
   */
  function( sqlany_init );

  /** Pointer to ::sqlany_fini() function.
   */
  function( sqlany_fini );

  /** Pointer to ::sqlany_new_connection() function.
   */
  function( sqlany_new_connection );

  /** Pointer to ::sqlany_free_connection() function.
   */
  function( sqlany_free_connection );

  /** Pointer to ::sqlany_make_connection() function.
   */
  function( sqlany_make_connection );

  /** Pointer to ::sqlany_connect() function.
   */
  function( sqlany_connect );

  /** Pointer to ::sqlany_disconnect() function.
   */
  function( sqlany_disconnect );

  /** Pointer to ::sqlany_execute_immediate() function.
   */
  function( sqlany_execute_immediate );

  /** Pointer to ::sqlany_prepare() function.
   */
  function( sqlany_prepare );

  /** Pointer to ::sqlany_free_stmt() function.
   */
  function( sqlany_free_stmt );

  /** Pointer to ::sqlany_num_params() function.
   */
  function( sqlany_num_params );

  /** Pointer to ::sqlany_describe_bind_param() function.
   */
  function( sqlany_describe_bind_param );

  /** Pointer to ::sqlany_bind_param() function.
   */
  function( sqlany_bind_param );

  /** Pointer to ::sqlany_send_param_data() function.
   */
  function( sqlany_send_param_data );

  /** Pointer to ::sqlany_reset() function.
   */
  function( sqlany_reset );

  /** Pointer to ::sqlany_get_bind_param_info() function.
   */
  function( sqlany_get_bind_param_info );

  /** Pointer to ::sqlany_execute() function.
   */
  function( sqlany_execute );

  /** Pointer to ::sqlany_execute_direct() function.
   */
  function( sqlany_execute_direct );

  /** Pointer to ::sqlany_fetch_absolute() function.
   */
  function( sqlany_fetch_absolute );

  /** Pointer to ::sqlany_fetch_next() function.
   */
  function( sqlany_fetch_next );

  /** Pointer to ::sqlany_get_next_result() function.
   */
  function( sqlany_get_next_result );

  /** Pointer to ::sqlany_affected_rows() function.
   */
  function( sqlany_affected_rows );

  /** Pointer to ::sqlany_num_cols() function.
   */
  function( sqlany_num_cols );

  /** Pointer to ::sqlany_num_rows() function.
   */
  function( sqlany_num_rows );

  /** Pointer to ::sqlany_get_column() function.
   */
  function( sqlany_get_column );

  /** Pointer to ::sqlany_get_data() function.
   */
  function( sqlany_get_data );

  /** Pointer to ::sqlany_get_data_info() function.
   */
  function( sqlany_get_data_info );

  /** Pointer to ::sqlany_get_column_info() function.
   */
  function( sqlany_get_column_info );

  /** Pointer to ::sqlany_commit() function.
   */
  function( sqlany_commit );

  /** Pointer to ::sqlany_rollback() function.
   */
  function( sqlany_rollback );

  /** Pointer to ::sqlany_client_version() function.
   */
  function( sqlany_client_version );

  /** Pointer to ::sqlany_error() function.
   */
  function( sqlany_error );

  /** Pointer to ::sqlany_sqlstate() function.
   */
  function( sqlany_sqlstate );

  /** Pointer to ::sqlany_clear_error() function.
   */
  function( sqlany_clear_error );

#if _SACAPI_VERSION+0 >= 2
  /** Pointer to ::sqlany_init_ex() function.
   */
  function( sqlany_init_ex );

  /** Pointer to ::sqlany_fini_ex() function.
   */
  function( sqlany_fini_ex );

  /** Pointer to ::sqlany_new_connection_ex() function.
   */
  function( sqlany_new_connection_ex );

  /** Pointer to ::sqlany_make_connection_ex() function.
   */
  function( sqlany_make_connection_ex );

  /** Pointer to ::sqlany_client_version_ex() function.
   */
  function( sqlany_client_version_ex );

  /** Pointer to ::sqlany_cancel() function.
   */
  function( sqlany_cancel );
#endif

} SQLAnywhereInterface;
#undef function

/** Initializes the SQLAnywhereInterface object and loads the DLL dynamically.
 *
 * Use the following statement to include the function prototype:
 *
 * <pre>
 * \#include "sacapidll.h"
 * </pre>
 *
 * This function attempts to load the SQL Anywhere C API DLL dynamically and looks up all
 * the entry points of the DLL. The fields in the SQLAnywhereInterface structure are
 * populated to point to the corresponding functions in the DLL. If the optional path argument
 * is NULL, the environment variable SQLANY_DLL_PATH is checked. If the variable is set,
 * the library attempts to load the DLL specified by the environment variable. If that fails,
 * the interface attempts to load the DLL directly (this relies on the environment being
 * setup correctly).
 *
 * To view examples of the sqlany_initialize_interface method in use, see the following topics:
 *
 * <ul>
 * <li>\salink{connecting.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-connecting-cpp.html", "programming", "pg-c-api-connecting-cpp"}
 * <li>\salink{dbcapi_isql.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-dbcapi-isql-cpp.html", "programming", "pg-c-api-dbcapi-isql-cpp"}
 * <li>\salink{fetching_a_result_set.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-fetching-a-result-set-cpp.html", "programming", "pg-c-api-fetching-a-result-set-cpp"}
 * <li>\salink{fetching_multiple_from_sp.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-fetching-multiple-from-sp-cpp.html", "programming", "pg-c-api-fetching-multiple-from-sp-cpp"}
 * <li>\salink{preparing_statements.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-preparing-statements-cpp.html", "programming", "pg-c-api-preparing-statements-cpp"}
 * <li>\salink{send_retrieve_full_blob.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-send-retrieve-full-blob-cpp.html", "programming", "pg-c-api-send-retrieve-full-blob-cpp"}
 * <li>\salink{send_retrieve_part_blob.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-send-retrieve-part-blob-cpp.html", "programming", "pg-c-api-send-retrieve-part-blob-cpp"}
 * </ul>
 *
 * \param api An API structure to initialize.
 * \param optional_path_to_dll An optional argument that specifies a path to the SQL Anywhere C API DLL.
 * \return 1 on successful initialization, and 0 on failure.
 */
int sqlany_initialize_interface( SQLAnywhereInterface * api, const char * optional_path_to_dll );

/** Unloads the C API DLL library and resets the SQLAnywhereInterface structure.
 *
 * Use the following statement to include the function prototype:
 *
 * <pre>
 * \#include "sacapidll.h"
 * </pre>
 *
 * Use this method to finalize and free resources associated with the SQL Anywhere C API DLL.
 *
 * To view examples of the sqlany_finalize_interface method in use, see the following topics:
 *
 * <ul>
 * <li>\salink{connecting.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-connecting-cpp.html", "programming", "pg-c-api-connecting-cpp"}
 * <li>\salink{dbcapi_isql.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-dbcapi-isql-cpp.html", "programming", "pg-c-api-dbcapi-isql-cpp"}
 * <li>\salink{fetching_a_result_set.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-fetching-a-result-set-cpp.html", "programming", "pg-c-api-fetching-a-result-set-cpp"}
 * <li>\salink{fetching_multiple_from_sp.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-fetching-multiple-from-sp-cpp.html", "programming", "pg-c-api-fetching-multiple-from-sp-cpp"}
 * <li>\salink{preparing_statements.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-preparing-statements-cpp.html", "programming", "pg-c-api-preparing-statements-cpp"}
 * <li>\salink{send_retrieve_full_blob.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-send-retrieve-full-blob-cpp.html", "programming", "pg-c-api-send-retrieve-full-blob-cpp"}
 * <li>\salink{send_retrieve_part_blob.cpp, "http://dcx.sybase.com/1200en/dbprogramming/pg-c-api-send-retrieve-part-blob-cpp.html", "programming", "pg-c-api-send-retrieve-part-blob-cpp"}
 * </ul>
 *
 * \param api An initialized structure to finalize.
 */

void sqlany_finalize_interface( SQLAnywhereInterface * api );

#endif
