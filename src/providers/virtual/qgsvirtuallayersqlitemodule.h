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
  int vtableCreate( sqlite3* sql, void* aux, int argc, const char* const* argv, sqlite3_vtab **out_vtab, char** out_err );
  int vtableConnect( sqlite3* sql, void* aux, int argc, const char* const* argv, sqlite3_vtab **out_vtab, char** out_err );
  int vtableRename( sqlite3_vtab *vtab, const char *new_name );
  int vtableBestIndex( sqlite3_vtab *vtab, sqlite3_index_info* );
  int vtableDisconnect( sqlite3_vtab *vtab );
  int vtableDestroy( sqlite3_vtab *vtab );

  int vtableOpen( sqlite3_vtab *vtab, sqlite3_vtab_cursor **out_cursor );
  int vtableClose( sqlite3_vtab_cursor * );
  int vtableFilter( sqlite3_vtab_cursor * cursor, int idxNum, const char *idxStr, int argc, sqlite3_value **argv );

  int vtableNext( sqlite3_vtab_cursor *cursor );
  int vtableEof( sqlite3_vtab_cursor *cursor );
  int vtableColumn( sqlite3_vtab_cursor *cursor, sqlite3_context*, int );
  int vtableRowId( sqlite3_vtab_cursor *cursor, sqlite3_int64 *out_rowid );

  int qgsvlayerModuleInit( sqlite3 *db,
                           char **pzErrMsg,
                           void * unused /*const sqlite3_api_routines *pApi*/ );

#ifdef __cplusplus
}

#include <qgsgeometry.h>

/**
 * Init the SQLite file with proper metadata tables
 */
void initVirtualLayerMetadata( sqlite3* db );

#endif

#define VIRTUAL_LAYER_VERSION 1

#endif
