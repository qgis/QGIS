/*
** alias MACROs to avoid any potential collision
** for linker symbols declared into the sqlite3 code
** internally embedded into SpatiaLite
*/
#define sqlite3_version SPLite3_version
#define sqlite3_libversion SPLite3_libversion
#define sqlite3_sourceid SPLite3_sourceid
#define sqlite3_libversion_number SPLite3_libversion_number
#define sqlite3_compileoption_used SPLite3_compileoption_used
#define sqlite3_compileoption_get SPLite3_compileoption_get
#define sqlite3_threadsafe SPLite3_threadsafe
#define sqlite3_close SPLite3_close
#define sqlite3_exec SPLite3_exec
#define sqlite3_initialize SPLite3_initialize
#define sqlite3_shutdown SPLite3_shutdown
#define sqlite3_os_init SPLite3_os_init
#define sqlite3_os_end SPLite3_os_end
#define sqlite3_config SPLite3_config
#define sqlite3_db_config SPLite3_db_config
#define sqlite3_extended_result_codes SPLite3_extended_result_codes
#define sqlite3_last_insert_rowid SPLite3_last_insert_rowid
#define sqlite3_changes SPLite3_changes
#define sqlite3_total_changes SPLite3_total_changes
#define sqlite3_interrupt SPLite3_interrupt
#define sqlite3_complete SPLite3_complete
#define sqlite3_complete16 SPLite3_complete16
#define sqlite3_busy_handler SPLite3_busy_handler
#define sqlite3_busy_timeout SPLite3_busy_timeout
#define sqlite3_get_table SPLite3_get_table
#define sqlite3_free_table SPLite3_free_table
#define sqlite3_mprintf SPLite3_mprintf
#define sqlite3_vmprintf SPLite3_vmprintf
#define sqlite3_snprintf SPLite3_snprintf
#define sqlite3_malloc SPLite3_malloc
#define sqlite3_realloc SPLite3_realloc
#define sqlite3_free SPLite3_free
#define sqlite3_memory_used SPLite3_memory_used
#define sqlite3_memory_highwater SPLite3_memory_highwater
#define sqlite3_randomness SPLite3_randomness
#define sqlite3_set_authorizer SPLite3_set_authorizer
#define sqlite3_trace SPLite3_trace
#define sqlite3_progress_handler SPLite3_progress_handler
#define sqlite3_open SPLite3_open
#define sqlite3_open16 SPLite3_open16
#define sqlite3_open_v2 SPLite3_open_v2
#define sqlite3_errcode SPLite3_errcode
#define sqlite3_extended_errcode SPLite3_extended_errcode
#define sqlite3_errmsg SPLite3_errmsg
#define sqlite3_errmsg16 SPLite3_errmsg16
#define sqlite3_limit SPLite3_limit
#define sqlite3_prepare SPLite3_prepare
#define sqlite3_prepare_v2 SPLite3_prepare_v2
#define sqlite3_prepare16 SPLite3_prepare16
#define sqlite3_prepare16_v2 SPLite3_prepare16_v2
#define sqlite3_sql SPLite3_sql
#define sqlite3_bind_blob SPLite3_bind_blob
#define sqlite3_bind_double SPLite3_bind_double
#define sqlite3_bind_int SPLite3_bind_int
#define sqlite3_bind_int64 SPLite3_bind_int64
#define sqlite3_bind_null SPLite3_bind_null
#define sqlite3_bind_text SPLite3_bind_text
#define sqlite3_bind_text16 SPLite3_bind_text16
#define sqlite3_bind_value SPLite3_bind_value
#define sqlite3_bind_zeroblob SPLite3_bind_zeroblob
#define sqlite3_bind_parameter_count SPLite3_bind_parameter_count
#define sqlite3_bind_parameter_name SPLite3_bind_parameter_name
#define sqlite3_bind_parameter_index SPLite3_bind_parameter_index
#define sqlite3_clear_bindings SPLite3_clear_bindings
#define sqlite3_column_count SPLite3_column_count
#define sqlite3_column_name SPLite3_column_name
#define sqlite3_column_name16 SPLite3_column_name16
#define sqlite3_column_database_name SPLite3_column_database_name
#define sqlite3_column_database_name16 SPLite3_column_database_name16
#define sqlite3_column_table_name SPLite3_column_table_name
#define sqlite3_column_table_name16 SPLite3_column_table_name16
#define sqlite3_column_origin_name SPLite3_column_origin_name
#define sqlite3_column_origin_name16 SPLite3_column_origin_name16
#define sqlite3_column_decltype SPLite3_column_decltype
#define sqlite3_column_decltype16 SPLite3_column_decltype16
#define sqlite3_step SPLite3_step
#define sqlite3_data_count SPLite3_data_count
#define sqlite3_column_blob SPLite3_column_blob
#define sqlite3_column_bytes SPLite3_column_bytes
#define sqlite3_column_bytes16 SPLite3_column_bytes16
#define sqlite3_column_double SPLite3_column_double
#define sqlite3_column_int SPLite3_column_int
#define sqlite3_column_int64 SPLite3_column_int64
#define sqlite3_column_text SPLite3_column_text
#define sqlite3_column_text16 SPLite3_column_text16
#define sqlite3_column_type SPLite3_column_type
#define sqlite3_column_value SPLite3_column_value
#define sqlite3_finalize SPLite3_finalize
#define sqlite3_reset SPLite3_reset
#define sqlite3_create_function SPLite3_create_function
#define sqlite3_create_function16 SPLite3_create_function16
#define sqlite3_create_function_v2 SPLite3_create_function_v2
#define sqlite3_value_blob SPLite3_value_blob
#define sqlite3_value_bytes SPLite3_value_bytes
#define sqlite3_value_bytes16 SPLite3_value_bytes16
#define sqlite3_value_double SPLite3_value_double
#define sqlite3_value_int SPLite3_value_int
#define sqlite3_value_int64 SPLite3_value_int64
#define sqlite3_value_text SPLite3_value_text
#define sqlite3_value_text16 SPLite3_value_text16
#define sqlite3_value_text16le SPLite3_value_text16le
#define sqlite3_value_text16be SPLite3_value_text16be
#define sqlite3_value_type SPLite3_value_type
#define sqlite3_value_numeric_type SPLite3_value_numeric_type
#define sqlite3_aggregate_context SPLite3_aggregate_context
#define sqlite3_user_data SPLite3_user_data
#define sqlite3_context_db_handle SPLite3_context_db_handle
#define sqlite3_get_auxdata SPLite3_get_auxdata
#define sqlite3_set_auxdata SPLite3_set_auxdata
#define sqlite3_result_blob SPLite3_result_blob
#define sqlite3_result_double SPLite3_result_double
#define sqlite3_result_error SPLite3_result_error
#define sqlite3_result_error16 SPLite3_result_error16
#define sqlite3_result_error_toobig SPLite3_result_error_toobig
#define sqlite3_result_error_nomem SPLite3_result_error_nomem
#define sqlite3_result_error_code SPLite3_result_error_code
#define sqlite3_result_int SPLite3_result_int
#define sqlite3_result_int64 SPLite3_result_int64
#define sqlite3_result_null SPLite3_result_null
#define sqlite3_result_text SPLite3_result_text
#define sqlite3_result_text16 SPLite3_result_text16
#define sqlite3_result_text16le SPLite3_result_text16le
#define sqlite3_result_text16be SPLite3_result_text16be
#define sqlite3_result_value SPLite3_result_value
#define sqlite3_result_zeroblob SPLite3_result_zeroblob
#define sqlite3_create_collation SPLite3_create_collation
#define sqlite3_create_collation_v2 SPLite3_create_collation_v2
#define sqlite3_create_collation16 SPLite3_create_collation16
#define sqlite3_collation_needed SPLite3_collation_needed
#define sqlite3_collation_needed16 SPLite3_collation_needed16
#define sqlite3_key SPLite3_key
#define sqlite3_rekey SPLite3_rekey
#define sqlite3_activate_see SPLite3_activate_see
#define sqlite3_activate_cerod SPLite3_activate_cerod
#define sqlite3_sleep SPLite3_sleep
#define sqlite3_temp_directory SPLite3_temp_directory
#define sqlite3_get_autocommit SPLite3_get_autocommit
#define sqlite3_db_handle SPLite3_db_handle
#define sqlite3_next_stmt SPLite3_next_stmt
#define sqlite3_commit_hook SPLite3_commit_hook
#define sqlite3_rollback_hook SPLite3_rollback_hook
#define sqlite3_update_hook SPLite3_update_hook
#define sqlite3_enable_shared_cache SPLite3_enable_shared_cache
#define sqlite3_release_memory SPLite3_release_memory
#define sqlite3_soft_heap_limit64 SPLite3_soft_heap_limit64
#define sqlite3_table_column_metadata SPLite3_table_column_metadata
#define sqlite3_load_extension SPLite3_load_extension
#define sqlite3_enable_load_extension SPLite3_enable_load_extension
#define sqlite3_auto_extension SPLite3_auto_extension
#define sqlite3_reset_auto_extension SPLite3_reset_auto_extension
#define sqlite3_create_module SPLite3_create_module
#define sqlite3_create_module_v2 SPLite3_create_module_v2
#define sqlite3_declare_vtab SPLite3_declare_vtab
#define sqlite3_overload_function SPLite3_overload_function
#define sqlite3_blob_open SPLite3_blob_open
#define sqlite3_blob_close SPLite3_blob_close
#define sqlite3_blob_bytes SPLite3_blob_bytes
#define sqlite3_blob_read SPLite3_blob_read
#define sqlite3_blob_write SPLite3_blob_write
#define sqlite3_vfs_find SPLite3_vfs_find
#define sqlite3_vfs_register SPLite3_vfs_register
#define sqlite3_vfs_unregister SPLite3_vfs_unregister
#define sqlite3_mutex_alloc SPLite3_mutex_alloc
#define sqlite3_mutex_free SPLite3_mutex_free
#define sqlite3_mutex_enter SPLite3_mutex_enter
#define sqlite3_mutex_try SPLite3_mutex_try
#define sqlite3_mutex_leave SPLite3_mutex_leave
#define sqlite3_mutex_held SPLite3_mutex_held
#define sqlite3_mutex_notheld SPLite3_mutex_notheld
#define sqlite3_db_mutex SPLite3_db_mutex
#define sqlite3_file_control SPLite3_file_control
#define sqlite3_test_control SPLite3_test_control
#define sqlite3_status SPLite3_status
#define sqlite3_db_status SPLite3_db_status
#define sqlite3_stmt_status SPLite3_stmt_status
#define sqlite3_backup_init SPLite3_backup_init
#define sqlite3_backup_step SPLite3_backup_step
#define sqlite3_backup_finish SPLite3_backup_finish
#define sqlite3_backup_remaining SPLite3_backup_remaining
#define sqlite3_backup_pagecount SPLite3_backup_pagecount
#define sqlite3_unlock_notify SPLite3_unlock_notify
#define sqlite3_strnicmp SPLite3_strnicmp
#define sqlite3_log SPLite3_log
#define sqlite3_wal_hook SPLite3_wal_hook
#define sqlite3_wal_autocheckpoint SPLite3_wal_autocheckpoint
#define sqlite3_wal_checkpoint SPLite3_wal_checkpoint
#define sqlite3_rtree_geometry_callback SPLite3_rtree_geometry_callback
#define sqlite3_memdebug_vfs_oom_test SPLite3_memdebug_vfs_oom_test
#define sqlite3_memory_alarm SPLite3_memory_alarm
#define sqlite3_soft_heap_limit SPLite3_soft_heap_limit
#define sqlite3_io_error_hit SPLite3_io_error_hit
#define sqlite3_io_error_hardhit SPLite3_io_error_hardhit
#define sqlite3_io_error_pending SPLite3_io_error_pending
#define sqlite3_io_error_persist SPLite3_io_error_persist
#define sqlite3_io_error_benign SPLite3_io_error_benign
#define sqlite3_diskfull_pending SPLite3_diskfull_pending
#define sqlite3_diskfull SPLite3_diskfull
#define sqlite3_open_file_count SPLite3_open_file_count
#define sqlite3_sync_count SPLite3_sync_count
#define sqlite3_fullsync_count SPLite3_fullsync_count
#define sqlite3_current_time SPLite3_current_time
#define sqlite3_hostid_num SPLite3_hostid_num
#define sqlite3_os_type SPLite3_os_type
#define sqlite3_win32_mbcs_to_utf8 SPLite3_win32_mbcs_to_utf8
#define sqlite3_pager_readdb_count SPLite3_pager_readdb_count
#define sqlite3_pager_writedb_count SPLite3_pager_writedb_count
#define sqlite3_pager_writej_count SPLite3_pager_writej_count
#define sqlite3_opentemp_count SPLite3_opentemp_count
#define sqlite3_expired SPLite3_expired
#define sqlite3_aggregate_count SPLite3_aggregate_count
#define sqlite3_transfer_bindings SPLite3_transfer_bindings
#define sqlite3_search_count SPLite3_search_count
#define sqlite3_interrupt_count SPLite3_interrupt_count
#define sqlite3_sort_count SPLite3_sort_count
#define sqlite3_max_blobsize SPLite3_max_blobsize
#define sqlite3_found_count SPLite3_found_count
#define sqlite3_like_count SPLite3_like_count
#define sqlite3_xferopt_count SPLite3_xferopt_count
#define sqlite3_profile SPLite3_profile
#define sqlite3_global_recover SPLite3_global_recover
#define sqlite3_thread_cleanup SPLite3_thread_cleanup
#define sqlite3_fts3_enable_parentheses SPLite3_fts3_enable_parentheses
/* end SpatiaLite/sqlite3 alias macros */

/*
** 2006 June 7
**
** The author disclaims copyright to this source code.  In place of
** a legal notice, here is a blessing:
**
**    May you do good and not evil.
**    May you find forgiveness for yourself and forgive others.
**    May you share freely, never taking more than you give.
**
*************************************************************************
** This header file defines the SQLite interface for use by
** shared libraries that want to be imported as extensions into
** an SQLite instance.  Shared libraries that intend to be loaded
** as extensions by SQLite should #include this file instead of 
** sqlite3.h.
*/
#ifndef _SQLITE3EXT_H_
#define _SQLITE3EXT_H_
#include "sqlite3.h"

typedef struct sqlite3_api_routines sqlite3_api_routines;

/*
** The following structure holds pointers to all of the SQLite API
** routines.
**
** WARNING:  In order to maintain backwards compatibility, add new
** interfaces to the end of this structure only.  If you insert new
** interfaces in the middle of this structure, then older different
** versions of SQLite will not be able to load each others' shared
** libraries!
*/
struct sqlite3_api_routines {
  void * (*aggregate_context)(sqlite3_context*,int nBytes);
  int  (*aggregate_count)(sqlite3_context*);
  int  (*bind_blob)(sqlite3_stmt*,int,const void*,int n,void(*)(void*));
  int  (*bind_double)(sqlite3_stmt*,int,double);
  int  (*bind_int)(sqlite3_stmt*,int,int);
  int  (*bind_int64)(sqlite3_stmt*,int,sqlite_int64);
  int  (*bind_null)(sqlite3_stmt*,int);
  int  (*bind_parameter_count)(sqlite3_stmt*);
  int  (*bind_parameter_index)(sqlite3_stmt*,const char*zName);
  const char * (*bind_parameter_name)(sqlite3_stmt*,int);
  int  (*bind_text)(sqlite3_stmt*,int,const char*,int n,void(*)(void*));
  int  (*bind_text16)(sqlite3_stmt*,int,const void*,int,void(*)(void*));
  int  (*bind_value)(sqlite3_stmt*,int,const sqlite3_value*);
  int  (*busy_handler)(sqlite3*,int(*)(void*,int),void*);
  int  (*busy_timeout)(sqlite3*,int ms);
  int  (*changes)(sqlite3*);
  int  (*close)(sqlite3*);
  int  (*collation_needed)(sqlite3*,void*,void(*)(void*,sqlite3*,int eTextRep,const char*));
  int  (*collation_needed16)(sqlite3*,void*,void(*)(void*,sqlite3*,int eTextRep,const void*));
  const void * (*column_blob)(sqlite3_stmt*,int iCol);
  int  (*column_bytes)(sqlite3_stmt*,int iCol);
  int  (*column_bytes16)(sqlite3_stmt*,int iCol);
  int  (*column_count)(sqlite3_stmt*pStmt);
  const char * (*column_database_name)(sqlite3_stmt*,int);
  const void * (*column_database_name16)(sqlite3_stmt*,int);
  const char * (*column_decltype)(sqlite3_stmt*,int i);
  const void * (*column_decltype16)(sqlite3_stmt*,int);
  double  (*column_double)(sqlite3_stmt*,int iCol);
  int  (*column_int)(sqlite3_stmt*,int iCol);
  sqlite_int64  (*column_int64)(sqlite3_stmt*,int iCol);
  const char * (*column_name)(sqlite3_stmt*,int);
  const void * (*column_name16)(sqlite3_stmt*,int);
  const char * (*column_origin_name)(sqlite3_stmt*,int);
  const void * (*column_origin_name16)(sqlite3_stmt*,int);
  const char * (*column_table_name)(sqlite3_stmt*,int);
  const void * (*column_table_name16)(sqlite3_stmt*,int);
  const unsigned char * (*column_text)(sqlite3_stmt*,int iCol);
  const void * (*column_text16)(sqlite3_stmt*,int iCol);
  int  (*column_type)(sqlite3_stmt*,int iCol);
  sqlite3_value* (*column_value)(sqlite3_stmt*,int iCol);
  void * (*commit_hook)(sqlite3*,int(*)(void*),void*);
  int  (*complete)(const char*sql);
  int  (*complete16)(const void*sql);
  int  (*create_collation)(sqlite3*,const char*,int,void*,int(*)(void*,int,const void*,int,const void*));
  int  (*create_collation16)(sqlite3*,const void*,int,void*,int(*)(void*,int,const void*,int,const void*));
  int  (*create_function)(sqlite3*,const char*,int,int,void*,void (*xFunc)(sqlite3_context*,int,sqlite3_value**),void (*xStep)(sqlite3_context*,int,sqlite3_value**),void (*xFinal)(sqlite3_context*));
  int  (*create_function16)(sqlite3*,const void*,int,int,void*,void (*xFunc)(sqlite3_context*,int,sqlite3_value**),void (*xStep)(sqlite3_context*,int,sqlite3_value**),void (*xFinal)(sqlite3_context*));
  int (*create_module)(sqlite3*,const char*,const sqlite3_module*,void*);
  int  (*data_count)(sqlite3_stmt*pStmt);
  sqlite3 * (*db_handle)(sqlite3_stmt*);
  int (*declare_vtab)(sqlite3*,const char*);
  int  (*enable_shared_cache)(int);
  int  (*errcode)(sqlite3*db);
  const char * (*errmsg)(sqlite3*);
  const void * (*errmsg16)(sqlite3*);
  int  (*exec)(sqlite3*,const char*,sqlite3_callback,void*,char**);
  int  (*expired)(sqlite3_stmt*);
  int  (*finalize)(sqlite3_stmt*pStmt);
  void  (*free)(void*);
  void  (*free_table)(char**result);
  int  (*get_autocommit)(sqlite3*);
  void * (*get_auxdata)(sqlite3_context*,int);
  int  (*get_table)(sqlite3*,const char*,char***,int*,int*,char**);
  int  (*global_recover)(void);
  void  (*interruptx)(sqlite3*);
  sqlite_int64  (*last_insert_rowid)(sqlite3*);
  const char * (*libversion)(void);
  int  (*libversion_number)(void);
  void *(*malloc)(int);
  char * (*mprintf)(const char*,...);
  int  (*open)(const char*,sqlite3**);
  int  (*open16)(const void*,sqlite3**);
  int  (*prepare)(sqlite3*,const char*,int,sqlite3_stmt**,const char**);
  int  (*prepare16)(sqlite3*,const void*,int,sqlite3_stmt**,const void**);
  void * (*profile)(sqlite3*,void(*)(void*,const char*,sqlite_uint64),void*);
  void  (*progress_handler)(sqlite3*,int,int(*)(void*),void*);
  void *(*realloc)(void*,int);
  int  (*reset)(sqlite3_stmt*pStmt);
  void  (*result_blob)(sqlite3_context*,const void*,int,void(*)(void*));
  void  (*result_double)(sqlite3_context*,double);
  void  (*result_error)(sqlite3_context*,const char*,int);
  void  (*result_error16)(sqlite3_context*,const void*,int);
  void  (*result_int)(sqlite3_context*,int);
  void  (*result_int64)(sqlite3_context*,sqlite_int64);
  void  (*result_null)(sqlite3_context*);
  void  (*result_text)(sqlite3_context*,const char*,int,void(*)(void*));
  void  (*result_text16)(sqlite3_context*,const void*,int,void(*)(void*));
  void  (*result_text16be)(sqlite3_context*,const void*,int,void(*)(void*));
  void  (*result_text16le)(sqlite3_context*,const void*,int,void(*)(void*));
  void  (*result_value)(sqlite3_context*,sqlite3_value*);
  void * (*rollback_hook)(sqlite3*,void(*)(void*),void*);
  int  (*set_authorizer)(sqlite3*,int(*)(void*,int,const char*,const char*,const char*,const char*),void*);
  void  (*set_auxdata)(sqlite3_context*,int,void*,void (*)(void*));
  char * (*snprintf)(int,char*,const char*,...);
  int  (*step)(sqlite3_stmt*);
  int  (*table_column_metadata)(sqlite3*,const char*,const char*,const char*,char const**,char const**,int*,int*,int*);
  void  (*thread_cleanup)(void);
  int  (*total_changes)(sqlite3*);
  void * (*trace)(sqlite3*,void(*xTrace)(void*,const char*),void*);
  int  (*transfer_bindings)(sqlite3_stmt*,sqlite3_stmt*);
  void * (*update_hook)(sqlite3*,void(*)(void*,int ,char const*,char const*,sqlite_int64),void*);
  void * (*user_data)(sqlite3_context*);
  const void * (*value_blob)(sqlite3_value*);
  int  (*value_bytes)(sqlite3_value*);
  int  (*value_bytes16)(sqlite3_value*);
  double  (*value_double)(sqlite3_value*);
  int  (*value_int)(sqlite3_value*);
  sqlite_int64  (*value_int64)(sqlite3_value*);
  int  (*value_numeric_type)(sqlite3_value*);
  const unsigned char * (*value_text)(sqlite3_value*);
  const void * (*value_text16)(sqlite3_value*);
  const void * (*value_text16be)(sqlite3_value*);
  const void * (*value_text16le)(sqlite3_value*);
  int  (*value_type)(sqlite3_value*);
  char *(*vmprintf)(const char*,va_list);
  /* Added ??? */
  int (*overload_function)(sqlite3*, const char *zFuncName, int nArg);
  /* Added by 3.3.13 */
  int (*prepare_v2)(sqlite3*,const char*,int,sqlite3_stmt**,const char**);
  int (*prepare16_v2)(sqlite3*,const void*,int,sqlite3_stmt**,const void**);
  int (*clear_bindings)(sqlite3_stmt*);
  /* Added by 3.4.1 */
  int (*create_module_v2)(sqlite3*,const char*,const sqlite3_module*,void*,void (*xDestroy)(void *));
  /* Added by 3.5.0 */
  int (*bind_zeroblob)(sqlite3_stmt*,int,int);
  int (*blob_bytes)(sqlite3_blob*);
  int (*blob_close)(sqlite3_blob*);
  int (*blob_open)(sqlite3*,const char*,const char*,const char*,sqlite3_int64,int,sqlite3_blob**);
  int (*blob_read)(sqlite3_blob*,void*,int,int);
  int (*blob_write)(sqlite3_blob*,const void*,int,int);
  int (*create_collation_v2)(sqlite3*,const char*,int,void*,int(*)(void*,int,const void*,int,const void*),void(*)(void*));
  int (*file_control)(sqlite3*,const char*,int,void*);
  sqlite3_int64 (*memory_highwater)(int);
  sqlite3_int64 (*memory_used)(void);
  sqlite3_mutex *(*mutex_alloc)(int);
  void (*mutex_enter)(sqlite3_mutex*);
  void (*mutex_free)(sqlite3_mutex*);
  void (*mutex_leave)(sqlite3_mutex*);
  int (*mutex_try)(sqlite3_mutex*);
  int (*open_v2)(const char*,sqlite3**,int,const char*);
  int (*release_memory)(int);
  void (*result_error_nomem)(sqlite3_context*);
  void (*result_error_toobig)(sqlite3_context*);
  int (*sleep)(int);
  void (*soft_heap_limit)(int);
  sqlite3_vfs *(*vfs_find)(const char*);
  int (*vfs_register)(sqlite3_vfs*,int);
  int (*vfs_unregister)(sqlite3_vfs*);
  int (*xthreadsafe)(void);
  void (*result_zeroblob)(sqlite3_context*,int);
  void (*result_error_code)(sqlite3_context*,int);
  int (*test_control)(int, ...);
  void (*randomness)(int,void*);
  sqlite3 *(*context_db_handle)(sqlite3_context*);
  int (*extended_result_codes)(sqlite3*,int);
  int (*limit)(sqlite3*,int,int);
  sqlite3_stmt *(*next_stmt)(sqlite3*,sqlite3_stmt*);
  const char *(*sql)(sqlite3_stmt*);
  int (*status)(int,int*,int*,int);
};

/*
** The following macros redefine the API routines so that they are
** redirected throught the global sqlite3_api structure.
**
** This header file is also used by the loadext.c source file
** (part of the main SQLite library - not an extension) so that
** it can get access to the sqlite3_api_routines structure
** definition.  But the main library does not want to redefine
** the API.  So the redefinition macros are only valid if the
** SQLITE_CORE macros is undefined.
*/
#ifndef SQLITE_CORE
#define sqlite3_aggregate_context      sqlite3_api->aggregate_context
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_aggregate_count        sqlite3_api->aggregate_count
#endif
#define sqlite3_bind_blob              sqlite3_api->bind_blob
#define sqlite3_bind_double            sqlite3_api->bind_double
#define sqlite3_bind_int               sqlite3_api->bind_int
#define sqlite3_bind_int64             sqlite3_api->bind_int64
#define sqlite3_bind_null              sqlite3_api->bind_null
#define sqlite3_bind_parameter_count   sqlite3_api->bind_parameter_count
#define sqlite3_bind_parameter_index   sqlite3_api->bind_parameter_index
#define sqlite3_bind_parameter_name    sqlite3_api->bind_parameter_name
#define sqlite3_bind_text              sqlite3_api->bind_text
#define sqlite3_bind_text16            sqlite3_api->bind_text16
#define sqlite3_bind_value             sqlite3_api->bind_value
#define sqlite3_busy_handler           sqlite3_api->busy_handler
#define sqlite3_busy_timeout           sqlite3_api->busy_timeout
#define sqlite3_changes                sqlite3_api->changes
#define sqlite3_close                  sqlite3_api->close
#define sqlite3_collation_needed       sqlite3_api->collation_needed
#define sqlite3_collation_needed16     sqlite3_api->collation_needed16
#define sqlite3_column_blob            sqlite3_api->column_blob
#define sqlite3_column_bytes           sqlite3_api->column_bytes
#define sqlite3_column_bytes16         sqlite3_api->column_bytes16
#define sqlite3_column_count           sqlite3_api->column_count
#define sqlite3_column_database_name   sqlite3_api->column_database_name
#define sqlite3_column_database_name16 sqlite3_api->column_database_name16
#define sqlite3_column_decltype        sqlite3_api->column_decltype
#define sqlite3_column_decltype16      sqlite3_api->column_decltype16
#define sqlite3_column_double          sqlite3_api->column_double
#define sqlite3_column_int             sqlite3_api->column_int
#define sqlite3_column_int64           sqlite3_api->column_int64
#define sqlite3_column_name            sqlite3_api->column_name
#define sqlite3_column_name16          sqlite3_api->column_name16
#define sqlite3_column_origin_name     sqlite3_api->column_origin_name
#define sqlite3_column_origin_name16   sqlite3_api->column_origin_name16
#define sqlite3_column_table_name      sqlite3_api->column_table_name
#define sqlite3_column_table_name16    sqlite3_api->column_table_name16
#define sqlite3_column_text            sqlite3_api->column_text
#define sqlite3_column_text16          sqlite3_api->column_text16
#define sqlite3_column_type            sqlite3_api->column_type
#define sqlite3_column_value           sqlite3_api->column_value
#define sqlite3_commit_hook            sqlite3_api->commit_hook
#define sqlite3_complete               sqlite3_api->complete
#define sqlite3_complete16             sqlite3_api->complete16
#define sqlite3_create_collation       sqlite3_api->create_collation
#define sqlite3_create_collation16     sqlite3_api->create_collation16
#define sqlite3_create_function        sqlite3_api->create_function
#define sqlite3_create_function16      sqlite3_api->create_function16
#define sqlite3_create_module          sqlite3_api->create_module
#define sqlite3_create_module_v2       sqlite3_api->create_module_v2
#define sqlite3_data_count             sqlite3_api->data_count
#define sqlite3_db_handle              sqlite3_api->db_handle
#define sqlite3_declare_vtab           sqlite3_api->declare_vtab
#define sqlite3_enable_shared_cache    sqlite3_api->enable_shared_cache
#define sqlite3_errcode                sqlite3_api->errcode
#define sqlite3_errmsg                 sqlite3_api->errmsg
#define sqlite3_errmsg16               sqlite3_api->errmsg16
#define sqlite3_exec                   sqlite3_api->exec
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_expired                sqlite3_api->expired
#endif
#define sqlite3_finalize               sqlite3_api->finalize
#define sqlite3_free                   sqlite3_api->free
#define sqlite3_free_table             sqlite3_api->free_table
#define sqlite3_get_autocommit         sqlite3_api->get_autocommit
#define sqlite3_get_auxdata            sqlite3_api->get_auxdata
#define sqlite3_get_table              sqlite3_api->get_table
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_global_recover         sqlite3_api->global_recover
#endif
#define sqlite3_interrupt              sqlite3_api->interruptx
#define sqlite3_last_insert_rowid      sqlite3_api->last_insert_rowid
#define sqlite3_libversion             sqlite3_api->libversion
#define sqlite3_libversion_number      sqlite3_api->libversion_number
#define sqlite3_malloc                 sqlite3_api->malloc
#define sqlite3_mprintf                sqlite3_api->mprintf
#define sqlite3_open                   sqlite3_api->open
#define sqlite3_open16                 sqlite3_api->open16
#define sqlite3_prepare                sqlite3_api->prepare
#define sqlite3_prepare16              sqlite3_api->prepare16
#define sqlite3_prepare_v2             sqlite3_api->prepare_v2
#define sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#define sqlite3_profile                sqlite3_api->profile
#define sqlite3_progress_handler       sqlite3_api->progress_handler
#define sqlite3_realloc                sqlite3_api->realloc
#define sqlite3_reset                  sqlite3_api->reset
#define sqlite3_result_blob            sqlite3_api->result_blob
#define sqlite3_result_double          sqlite3_api->result_double
#define sqlite3_result_error           sqlite3_api->result_error
#define sqlite3_result_error16         sqlite3_api->result_error16
#define sqlite3_result_int             sqlite3_api->result_int
#define sqlite3_result_int64           sqlite3_api->result_int64
#define sqlite3_result_null            sqlite3_api->result_null
#define sqlite3_result_text            sqlite3_api->result_text
#define sqlite3_result_text16          sqlite3_api->result_text16
#define sqlite3_result_text16be        sqlite3_api->result_text16be
#define sqlite3_result_text16le        sqlite3_api->result_text16le
#define sqlite3_result_value           sqlite3_api->result_value
#define sqlite3_rollback_hook          sqlite3_api->rollback_hook
#define sqlite3_set_authorizer         sqlite3_api->set_authorizer
#define sqlite3_set_auxdata            sqlite3_api->set_auxdata
#define sqlite3_snprintf               sqlite3_api->snprintf
#define sqlite3_step                   sqlite3_api->step
#define sqlite3_table_column_metadata  sqlite3_api->table_column_metadata
#define sqlite3_thread_cleanup         sqlite3_api->thread_cleanup
#define sqlite3_total_changes          sqlite3_api->total_changes
#define sqlite3_trace                  sqlite3_api->trace
#ifndef SQLITE_OMIT_DEPRECATED
#define sqlite3_transfer_bindings      sqlite3_api->transfer_bindings
#endif
#define sqlite3_update_hook            sqlite3_api->update_hook
#define sqlite3_user_data              sqlite3_api->user_data
#define sqlite3_value_blob             sqlite3_api->value_blob
#define sqlite3_value_bytes            sqlite3_api->value_bytes
#define sqlite3_value_bytes16          sqlite3_api->value_bytes16
#define sqlite3_value_double           sqlite3_api->value_double
#define sqlite3_value_int              sqlite3_api->value_int
#define sqlite3_value_int64            sqlite3_api->value_int64
#define sqlite3_value_numeric_type     sqlite3_api->value_numeric_type
#define sqlite3_value_text             sqlite3_api->value_text
#define sqlite3_value_text16           sqlite3_api->value_text16
#define sqlite3_value_text16be         sqlite3_api->value_text16be
#define sqlite3_value_text16le         sqlite3_api->value_text16le
#define sqlite3_value_type             sqlite3_api->value_type
#define sqlite3_vmprintf               sqlite3_api->vmprintf
#define sqlite3_overload_function      sqlite3_api->overload_function
#define sqlite3_prepare_v2             sqlite3_api->prepare_v2
#define sqlite3_prepare16_v2           sqlite3_api->prepare16_v2
#define sqlite3_clear_bindings         sqlite3_api->clear_bindings
#define sqlite3_bind_zeroblob          sqlite3_api->bind_zeroblob
#define sqlite3_blob_bytes             sqlite3_api->blob_bytes
#define sqlite3_blob_close             sqlite3_api->blob_close
#define sqlite3_blob_open              sqlite3_api->blob_open
#define sqlite3_blob_read              sqlite3_api->blob_read
#define sqlite3_blob_write             sqlite3_api->blob_write
#define sqlite3_create_collation_v2    sqlite3_api->create_collation_v2
#define sqlite3_file_control           sqlite3_api->file_control
#define sqlite3_memory_highwater       sqlite3_api->memory_highwater
#define sqlite3_memory_used            sqlite3_api->memory_used
#define sqlite3_mutex_alloc            sqlite3_api->mutex_alloc
#define sqlite3_mutex_enter            sqlite3_api->mutex_enter
#define sqlite3_mutex_free             sqlite3_api->mutex_free
#define sqlite3_mutex_leave            sqlite3_api->mutex_leave
#define sqlite3_mutex_try              sqlite3_api->mutex_try
#define sqlite3_open_v2                sqlite3_api->open_v2
#define sqlite3_release_memory         sqlite3_api->release_memory
#define sqlite3_result_error_nomem     sqlite3_api->result_error_nomem
#define sqlite3_result_error_toobig    sqlite3_api->result_error_toobig
#define sqlite3_sleep                  sqlite3_api->sleep
#define sqlite3_soft_heap_limit        sqlite3_api->soft_heap_limit
#define sqlite3_vfs_find               sqlite3_api->vfs_find
#define sqlite3_vfs_register           sqlite3_api->vfs_register
#define sqlite3_vfs_unregister         sqlite3_api->vfs_unregister
#define sqlite3_threadsafe             sqlite3_api->xthreadsafe
#define sqlite3_result_zeroblob        sqlite3_api->result_zeroblob
#define sqlite3_result_error_code      sqlite3_api->result_error_code
#define sqlite3_test_control           sqlite3_api->test_control
#define sqlite3_randomness             sqlite3_api->randomness
#define sqlite3_context_db_handle      sqlite3_api->context_db_handle
#define sqlite3_extended_result_codes  sqlite3_api->extended_result_codes
#define sqlite3_limit                  sqlite3_api->limit
#define sqlite3_next_stmt              sqlite3_api->next_stmt
#define sqlite3_sql                    sqlite3_api->sql
#define sqlite3_status                 sqlite3_api->status
#endif /* SQLITE_CORE */

#define SQLITE_EXTENSION_INIT1     const sqlite3_api_routines *sqlite3_api = 0;
#define SQLITE_EXTENSION_INIT2(v)  sqlite3_api = v;

#endif /* _SQLITE3EXT_H_ */
