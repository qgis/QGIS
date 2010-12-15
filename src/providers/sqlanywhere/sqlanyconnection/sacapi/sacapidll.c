//====================================================
//
//      Copyright 2008-2010 iAnywhere Solutions, Inc.
//
//   Licensed under the Apache License, Version 2.0 (the "License");
//   you may not use this file except in compliance with the License.
//   You may obtain a copy of the License at
//
//   http://www.apache.org/licenses/LICENSE-2.0
//
//   Unless required by applicable law or agreed to in writing, software
//   distributed under the License is distributed on an "AS IS" BASIS,
//   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//
//   See the License for the specific language governing permissions and
//   limitations under the License.
//
//   While not a requirement of the license, if you do modify this file, we
//   would appreciate hearing about it. Please email
//   sqlany_interfaces@sybase.com
//
//====================================================
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#if defined( _WIN32 )
    #include <windows.h>
    #define DEFAULT_LIBRARY_NAME "dbcapi.dll"
#else
    #include <dlfcn.h>
    /* assume we are running on a UNIX platform */
    #if defined( __APPLE__ )
	#define LIB_EXT "dylib"
    #else
	#define LIB_EXT "so"
    #endif
    #if defined( _REENTRANT ) || defined( _THREAD_SAFE ) \
	|| defined( __USE_REENTRANT )
    /* if building a thread-safe library, we need to load 
       the thread-safe dbcapi library */
	#define DEFAULT_LIBRARY_NAME "libdbcapi_r." LIB_EXT
    #else
	#define DEFAULT_LIBRARY_NAME "libdbcapi." LIB_EXT
    #endif
#endif

#include "sacapidll.h"

static 
void * loadLibrary( const char * name )
/*************************************/
{
    void * handle;
#if defined( _WIN32 )
    handle = LoadLibrary( name );
#else
    handle = dlopen( name, RTLD_LAZY );
#endif
    return handle;
}

static 
void unloadLibrary( void * handle )
/**********************************/
{
#if defined( _WIN32 )
    FreeLibrary( handle );
#else
    dlclose( handle );
#endif
}

static 
void * findSymbol( void * dll_handle, char * name )
/**************************************************/
{
#if defined( _WIN32 )
    return GetProcAddress( dll_handle, name );
#else
    return dlsym( dll_handle, name );
#endif
}

#define LookupSymbol( api, sym )					\
	api->sym = (sym ## _func)findSymbol( api->dll_handle, #sym );

#define LookupSymbolAndCheck( api, sym )				\
	api->sym = (sym ## _func)findSymbol( api->dll_handle, #sym );	\
	if( api->sym == NULL ) {					\
	    unloadLibrary( api->dll_handle );				\
	    return 0;							\
	}
	
int sqlany_initialize_interface( SQLAnywhereInterface * api, const char * path )
/*******************************************************************************/
{
    char * env;
    memset( api, 0, sizeof(*api));

    if( path != NULL ) {
	api->dll_handle = loadLibrary( path );
	if( api->dll_handle != NULL ) {
	    goto loaded;
	}
    }
    env = getenv( "SQLANY_API_DLL" );
    if( env != NULL ) {
	api->dll_handle = loadLibrary( env );
	if( api->dll_handle != NULL ) {
	    goto loaded;
	}
    }
    api->dll_handle = loadLibrary( DEFAULT_LIBRARY_NAME );
    if( api->dll_handle != NULL ) {
	goto loaded;
    }
    return 0;

loaded:
    LookupSymbolAndCheck( api, sqlany_init );
    LookupSymbolAndCheck( api, sqlany_fini );
    LookupSymbolAndCheck( api, sqlany_new_connection );
    LookupSymbolAndCheck( api, sqlany_free_connection );
    LookupSymbolAndCheck( api, sqlany_make_connection );
    LookupSymbolAndCheck( api, sqlany_connect );
    LookupSymbolAndCheck( api, sqlany_disconnect );
    LookupSymbolAndCheck( api, sqlany_execute_immediate );
    LookupSymbolAndCheck( api, sqlany_prepare );
    LookupSymbolAndCheck( api, sqlany_free_stmt );
    LookupSymbolAndCheck( api, sqlany_num_params );
    LookupSymbolAndCheck( api, sqlany_describe_bind_param );
    LookupSymbolAndCheck( api, sqlany_bind_param );
    LookupSymbolAndCheck( api, sqlany_send_param_data );
    LookupSymbolAndCheck( api, sqlany_reset );
    LookupSymbolAndCheck( api, sqlany_get_bind_param_info );
    LookupSymbolAndCheck( api, sqlany_execute );
    LookupSymbolAndCheck( api, sqlany_execute_direct );
    LookupSymbolAndCheck( api, sqlany_fetch_absolute );
    LookupSymbolAndCheck( api, sqlany_fetch_next );
    LookupSymbolAndCheck( api, sqlany_get_next_result );
    LookupSymbolAndCheck( api, sqlany_affected_rows );
    LookupSymbolAndCheck( api, sqlany_num_cols );
    LookupSymbolAndCheck( api, sqlany_num_rows );
    LookupSymbolAndCheck( api, sqlany_get_column );
    LookupSymbolAndCheck( api, sqlany_get_data );
    LookupSymbolAndCheck( api, sqlany_get_data_info );
    LookupSymbolAndCheck( api, sqlany_get_column_info );
    LookupSymbolAndCheck( api, sqlany_commit );
    LookupSymbolAndCheck( api, sqlany_rollback );
    LookupSymbolAndCheck( api, sqlany_client_version );
    LookupSymbolAndCheck( api, sqlany_error );
    LookupSymbolAndCheck( api, sqlany_sqlstate );
    LookupSymbolAndCheck( api, sqlany_clear_error );

#if _SACAPI_VERSION+0 >= 2
    /* We don't report an error if we don't find the v2 entry points.
       That allows the calling app to revert to v1 */
    LookupSymbol( api, sqlany_init_ex );
    LookupSymbol( api, sqlany_fini_ex );
    LookupSymbol( api, sqlany_new_connection_ex );
    LookupSymbol( api, sqlany_make_connection_ex );
    LookupSymbol( api, sqlany_client_version_ex );
    LookupSymbolAndCheck( api, sqlany_cancel );
#endif

    api->initialized = 1;
    return 1;
}
#undef LookupSymbolAndCheck

void sqlany_finalize_interface( SQLAnywhereInterface * api )
/***********************************************************/
{
    if( !api->initialized ) {
	return;
    }
    unloadLibrary( api->dll_handle );
    memset( api, 0, sizeof(*api));
}


