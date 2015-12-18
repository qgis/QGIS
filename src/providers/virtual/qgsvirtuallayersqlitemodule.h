/***************************************************************************
             sqlite_vlayer_module.h : SQLite module for QGIS virtual layers
begin                : Nov 2015
copyright            : (C) 2015 Hugo Mercier, Oslandia
email                : hugo dot mercier at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVIRTUAL_SQLITE_LAYER_MODULE_H
#define QGSVIRTUAL_SQLITE_LAYER_MODULE_H

#ifdef __cplusplus
extern "C"
{
#endif
  int vtable_create( sqlite3* sql, void* aux, int argc, const char* const* argv, sqlite3_vtab **out_vtab, char** out_err );
  int vtable_connect( sqlite3* sql, void* aux, int argc, const char* const* argv, sqlite3_vtab **out_vtab, char** out_err );
  int vtable_rename( sqlite3_vtab *vtab, const char *new_name );
  int vtable_bestindex( sqlite3_vtab *vtab, sqlite3_index_info* );
  int vtable_disconnect( sqlite3_vtab *vtab );
  int vtable_destroy( sqlite3_vtab *vtab );

  int vtable_open( sqlite3_vtab *vtab, sqlite3_vtab_cursor **out_cursor );
  int vtable_close( sqlite3_vtab_cursor * );
  int vtable_filter( sqlite3_vtab_cursor * cursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv );

  int vtable_next( sqlite3_vtab_cursor *cursor );
  int vtable_eof( sqlite3_vtab_cursor *cursor );
  int vtable_column( sqlite3_vtab_cursor *cursor, sqlite3_context*, int );
  int vtable_rowid( sqlite3_vtab_cursor *cursor, sqlite3_int64 *out_rowid );

  int vtable_findfunction( sqlite3_vtab *pVtab,
                           int nArg,
                           const char *zName,
                           void ( **pxFunc )( sqlite3_context*, int, sqlite3_value** ),
                           void **ppArg );


  int qgsvlayer_module_init( sqlite3 *db,
                             char **pzErrMsg,
                             void * unused /*const sqlite3_api_routines *pApi*/ );

#ifdef __cplusplus
}

#include <qgsgeometry.h>

/**
 * Convert a spatialite geometry blob to a QgsGeometry
 *
 * \param blob Pointer to the raw blob memory
 * \param size Size in bytes of the blob
 * \returns a QgsGeometry (that are implicitly shared)
 */
QgsGeometry spatialite_blob_to_qgsgeometry( const unsigned char* blob, size_t size );

/**
 * Init the SQLite file with proper metadata tables
 */
void initVirtualLayerMetadata( sqlite3* db );

#endif

#define VIRTUAL_LAYER_VERSION 1

#endif
