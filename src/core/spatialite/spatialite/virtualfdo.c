/*

 virtualfdo.c -- SQLite3 extension [VIRTUAL TABLE accessing FDO-OGR tables]

 version 2.3, 2008 October 13

 Author: Sandro Furieri a.furieri@lqt.it

 -----------------------------------------------------------------------------

 Version: MPL 1.1/GPL 2.0/LGPL 2.1

 The contents of this file are subject to the Mozilla Public License Version
 1.1 (the "License"); you may not use this file except in compliance with
 the License. You may obtain a copy of the License at
 http://www.mozilla.org/MPL/

Software distributed under the License is distributed on an "AS IS" basis,
WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
for the specific language governing rights and limitations under the
License.

The Original Code is the SpatiaLite library

The Initial Developer of the Original Code is Alessandro Furieri

Portions created by the Initial Developer are Copyright (C) 2008
the Initial Developer. All Rights Reserved.

Contributor(s):

Alternatively, the contents of this file may be used under the terms of
either the GNU General Public License Version 2 or later (the "GPL"), or
the GNU Lesser General Public License Version 2.1 or later (the "LGPL"),
in which case the provisions of the GPL or the LGPL are applicable instead
of those above. If you wish to allow use of your version of this file only
under the terms of either the GPL or the LGPL, and not to allow others to
use your version of this file under the terms of the MPL, indicate your
decision by deleting the provisions above and replace them with the notice
and other provisions required by the GPL or the LGPL. If you do not delete
the provisions above, a recipient may use your version of this file under
the terms of any one of the MPL, the GPL or the LGPL.

*/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <spatialite/sqlite3ext.h>
#include <spatialite/spatialite.h>
#include <spatialite/gaiaaux.h>
#include <spatialite/gaiageo.h>

#ifdef _MSC_VER
#define strcasecmp(a,b) _stricmp(a,b)
#endif

/* constants definining FDO-OGR Geometry formats */
#define FDO_OGR_NONE 0
#define FDO_OGR_WKT  1
#define FDO_OGR_WKB  2
#define FDO_OGR_FGF  3

static SQLITE_EXTENSION_INIT1 static struct sqlite3_module my_module;

typedef struct SqliteValue
{
  /* a multitype storing a column value */
  int Type;
  sqlite3_int64 IntValue;
  double DoubleValue;
  char *Text;
  unsigned char *Blob;
  int Size;
} SqliteValue;
typedef SqliteValue *SqliteValuePtr;

typedef struct VirtualFDOStruct
{
  /* extends the sqlite3_vtab struct */
  const sqlite3_module *pModule; /* ptr to sqlite module: USED INTERNALLY BY SQLITE */
  int nRef;   /* # references: USED INTERNALLY BY SQLITE */
  char *zErrMsg;  /* error message: USE INTERNALLY BY SQLITE */
  sqlite3 *db;  /* the sqlite db holding the virtual table */
  char *table;  /* the real-table name */
  int nColumns;  /* the # columns into the table */
  char **Column;  /* the name for each column */
  char **Type;  /* the type for each column */
  int *NotNull;  /* NotNull clause for each column */
  SqliteValuePtr *Value; /* the current-row value for each column */
  int nGeometries;  /* # Geometry columns into the table */
  char **GeoColumn;  /* the name for each Geometry column */
  int *Srid;   /* the SRID for each Geometry column */
  int *GeoType;  /* the Type for each Geometry column */
  int *Format;  /* the Format for each Geometry column */
  int *CoordDimensions; /* # Dimensions for each Geometry column */
} VirtualFDO;
typedef VirtualFDO *VirtualFDOPtr;

typedef struct VirtualFDOCursorStruct
{
  /* extends the sqlite3_vtab_cursor struct */
  VirtualFDOPtr pVtab; /* Virtual table of this cursor */
  sqlite3_int64 current_row; /* the current row ID */
  int eof;   /* the EOF marker */
} VirtualFDOCursor;
typedef VirtualFDOCursor *VirtualFDOCursorPtr;

static SqliteValuePtr
value_alloc()
{
  /* allocates and initialites a Value multitype */
  SqliteValuePtr p = malloc( sizeof( SqliteValue ) );
  p->Type = SQLITE_NULL;
  p->Text = NULL;
  p->Blob = NULL;
  return p;
}

static void
value_free( SqliteValuePtr p )
{
  /* freeing a Value multitype */
  if ( !p )
    return;
  if ( p->Text )
    free( p->Text );
  if ( p->Blob )
    free( p->Blob );
  free( p );
}

static void
value_set_null( SqliteValuePtr p )
{
  /* setting a NULL value to the multitype */
  if ( !p )
    return;
  p->Type = SQLITE_NULL;
  if ( p->Text )
    free( p->Text );
  if ( p->Blob )
    free( p->Blob );
  p->Text = NULL;
  p->Blob = NULL;
}

static void
value_set_int( SqliteValuePtr p, sqlite3_int64 value )
{
  /* setting an INT value to the multitype */
  if ( !p )
    return;
  p->Type = SQLITE_INTEGER;
  if ( p->Text )
    free( p->Text );
  if ( p->Blob )
    free( p->Blob );
  p->Text = NULL;
  p->Blob = NULL;
  p->IntValue = value;
}

static void
value_set_double( SqliteValuePtr p, double value )
{
  /* setting a DOUBLE value to the multitype */
  if ( !p )
    return;
  p->Type = SQLITE_FLOAT;
  if ( p->Text )
    free( p->Text );
  if ( p->Blob )
    free( p->Blob );
  p->Text = NULL;
  p->Blob = NULL;
  p->DoubleValue = value;
}

static void
value_set_text( SqliteValuePtr p, const char *value, int size )
{
  /* setting a TEXT value to the multitype */
  if ( !p )
    return;
  p->Type = SQLITE_TEXT;
  if ( p->Text )
    free( p->Text );
  if ( p->Blob )
    free( p->Blob );
  p->Blob = NULL;
  p->Text = malloc( size );
  memcpy( p->Text, value, size );
  p->Size = size;
}

static void
value_set_blob( SqliteValuePtr p, const unsigned char *value, int size )
{
  /* setting a BLOB value to the multitype */
  if ( !p )
    return;
  p->Type = SQLITE_BLOB;
  if ( p->Text )
    free( p->Text );
  if ( p->Blob )
    free( p->Blob );
  p->Text = NULL;
  p->Blob = malloc( size );
  memcpy( p->Blob, value, size );
  p->Size = size;
}

static void
free_table( VirtualFDOPtr p_vt )
{
  /* memory cleanup; freeing the virtual table struct */
  int i;
  if ( !p_vt )
    return;
  if ( p_vt->Column )
  {
    for ( i = 0; i < p_vt->nColumns; i++ )
    {
      if ( *( p_vt->Column + i ) )
        sqlite3_free( *( p_vt->Column + i ) );
    }
    sqlite3_free( p_vt->Column );
  }
  if ( p_vt->Type )
  {
    for ( i = 0; i < p_vt->nColumns; i++ )
    {
      if ( *( p_vt->Type + i ) )
        sqlite3_free( *( p_vt->Type + i ) );
    }
    sqlite3_free( p_vt->Type );
  }
  if ( p_vt->NotNull )
    sqlite3_free( p_vt->NotNull );
  if ( p_vt->Value )
  {
    for ( i = 0; i < p_vt->nColumns; i++ )
    {
      if ( *( p_vt->Value + i ) )
        value_free( *( p_vt->Value + i ) );
    }
    sqlite3_free( p_vt->Value );
  }
  if ( p_vt->GeoColumn )
  {
    for ( i = 0; i < p_vt->nGeometries; i++ )
    {
      if ( *( p_vt->GeoColumn + i ) )
        sqlite3_free( *( p_vt->GeoColumn + i ) );
    }
    sqlite3_free( p_vt->GeoColumn );
  }
  if ( p_vt->Srid )
    sqlite3_free( p_vt->Srid );
  if ( p_vt->GeoType )
    sqlite3_free( p_vt->GeoType );
  if ( p_vt->Format )
    sqlite3_free( p_vt->Format );
  if ( p_vt->CoordDimensions )
    sqlite3_free( p_vt->CoordDimensions );
  sqlite3_free( p_vt );
}

static int
vfdo_insert_row( VirtualFDOPtr p_vt, sqlite3_int64 * rowid, int argc,
                 sqlite3_value ** argv )
{
  /* trying to insert a row into FDO-OGR real-table */
  sqlite3_stmt *stmt;
  int ret;
  int i;
  int ic;
  int ig;
  int geom_done;
  int err_geom = 0;
  int geom_constraint_err = 0;
  char prefix;
  const char *text;
  const unsigned char *blob;
  char *text_wkt;
  unsigned char *blob_wkb;
  int size;
  char sql[4096];
  char buf[256];
  gaiaGeomCollPtr geom;
  sprintf( sql, "INSERT INTO %s ", p_vt->table );
  for ( ic = 0; ic < p_vt->nColumns; ic++ )
  {
    if ( ic == 0 )
      prefix = '(';
    else
      prefix = ',';
    sprintf( buf, "%c%s", prefix, *( p_vt->Column + ic ) );
    strcat( sql, buf );
  }
  strcat( sql, ") VALUES " );
  for ( ic = 0; ic < p_vt->nColumns; ic++ )
  {
    if ( ic == 0 )
      prefix = '(';
    else
      prefix = ',';
    sprintf( buf, "%c?", prefix );
    strcat( sql, buf );
  }
  strcat( sql, ")" );
  ret = sqlite3_prepare_v2( p_vt->db, sql, strlen( sql ), &stmt, NULL );
  if ( ret != SQLITE_OK )
    return SQLITE_ERROR;
  for ( i = 2; i < argc; i++ )
  {
    geom_done = 0;
    for ( ig = 0; ig < p_vt->nGeometries; ig++ )
    {
      if ( strcasecmp
           ( *( p_vt->Column + i - 2 ), *( p_vt->GeoColumn + ig ) ) == 0 )
      {
        /* this one is a Geometry column */
        if ( sqlite3_value_type( argv[i] ) == SQLITE_BLOB )
        {
          blob = sqlite3_value_blob( argv[i] );
          size = sqlite3_value_bytes( argv[i] );
          geom = gaiaFromSpatiaLiteBlobWkb( blob, size );
          if ( geom )
          {
            if ( geom->Srid != *( p_vt->Srid + ig ) )
            {
              /* SRID constraint violation */
              geom_constraint_err = 1;
              goto error;
            }
            /* checking for TYPE constraint violation */
            if ( gaiaGeometryType( geom ) !=
                 *( p_vt->GeoType + ig ) )
            {
              /* Geometry TYPE constraint violation */
              geom_constraint_err = 1;
              goto error;
            }
            switch ( *( p_vt->Format + ig ) )
            {
              case FDO_OGR_WKT:
                gaiaOutWkt( geom, &text_wkt );
                if ( text_wkt )
                  sqlite3_bind_text( stmt, i - 1,
                                     text_wkt,
                                     strlen
                                     ( text_wkt ),
                                     free );
                else
                {
                  err_geom = 1;
                  goto error;
                }
                break;
              case FDO_OGR_WKB:
                gaiaToWkb( geom, &blob_wkb, &size );
                if ( blob_wkb )
                  sqlite3_bind_blob( stmt, i - 1,
                                     blob_wkb, size,
                                     free );
                else
                {
                  err_geom = 1;
                  goto error;
                }
                break;
              case FDO_OGR_FGF:
                gaiaToFgf( geom, &blob_wkb, &size,
                           *( p_vt->CoordDimensions +
                              ig ) );
                if ( blob_wkb )
                  sqlite3_bind_blob( stmt, i - 1,
                                     blob_wkb, size,
                                     free );
                else
                {
                  err_geom = 1;
                  goto error;
                }
                break;
              default:
                err_geom = 1;
                goto error;
                break;
            };
          }
          else
          {
            err_geom = 1;
            goto error;
          }
        }
        else if ( sqlite3_value_type( argv[i] ) == SQLITE_NULL )
          sqlite3_bind_null( stmt, i - 1 );
        else
        {
          err_geom = 1;
          goto error;
        }
        geom_done = 1;
      }
    }
    if ( geom_done )
      continue;
    switch ( sqlite3_value_type( argv[i] ) )
    {
      case SQLITE_INTEGER:
        sqlite3_bind_int64( stmt, i - 1, sqlite3_value_int64( argv[i] ) );
        break;
      case SQLITE_FLOAT:
        sqlite3_bind_double( stmt, i - 1,
                             sqlite3_value_double( argv[i] ) );
        break;
      case SQLITE_TEXT:
        text = ( char * ) sqlite3_value_text( argv[i] );
        size = sqlite3_value_bytes( argv[i] );
        sqlite3_bind_text( stmt, i - 1, text, size, SQLITE_STATIC );
        break;
      case SQLITE_BLOB:
        blob = sqlite3_value_blob( argv[i] );
        size = sqlite3_value_bytes( argv[i] );
        sqlite3_bind_blob( stmt, i - 1, blob, size, SQLITE_STATIC );
        break;
      case SQLITE_NULL:
      default:
        sqlite3_bind_null( stmt, i - 1 );
        break;
    };
  }
error:
  if ( err_geom || geom_constraint_err )
  {
    sqlite3_finalize( stmt );
    return SQLITE_CONSTRAINT;
  }
  ret = sqlite3_step( stmt );
  if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
    ;
  else
  {
    sqlite3_finalize( stmt );
    return ret;
  }
  sqlite3_finalize( stmt );
  *rowid = sqlite3_last_insert_rowid( p_vt->db );
  return SQLITE_OK;
}

static int
vfdo_update_row( VirtualFDOPtr p_vt, sqlite3_int64 rowid, int argc,
                 sqlite3_value ** argv )
{
  /* trying to update a row in FDO-OGR real-table */
  sqlite3_stmt *stmt;
  int ret;
  int i;
  int ic;
  int ig;
  int geom_done;
  int err_geom = 0;
  int geom_constraint_err = 0;
  char prefix;
  const char *text;
  const unsigned char *blob;
  char *text_wkt;
  unsigned char *blob_wkb;
  int size;
  char sql[4096];
  char buf[256];
  gaiaGeomCollPtr geom;
  sprintf( sql, "UPDATE %s SET", p_vt->table );
  for ( ic = 0; ic < p_vt->nColumns; ic++ )
  {
    if ( ic == 0 )
      prefix = ' ';
    else
      prefix = ',';
    sprintf( buf, "%c%s = ?", prefix, *( p_vt->Column + ic ) );
    strcat( sql, buf );
  }
  sprintf( buf, " WHERE ROWID = %lld", rowid );
  strcat( sql, buf );
  ret = sqlite3_prepare_v2( p_vt->db, sql, strlen( sql ), &stmt, NULL );
  if ( ret != SQLITE_OK )
    return SQLITE_ERROR;
  for ( i = 2; i < argc; i++ )
  {
    geom_done = 0;
    for ( ig = 0; ig < p_vt->nGeometries; ig++ )
    {
      if ( strcasecmp
           ( *( p_vt->Column + i - 2 ), *( p_vt->GeoColumn + ig ) ) == 0 )
      {
        /* this one is a Geometry column */
        if ( sqlite3_value_type( argv[i] ) == SQLITE_BLOB )
        {
          blob = sqlite3_value_blob( argv[i] );
          size = sqlite3_value_bytes( argv[i] );
          geom = gaiaFromSpatiaLiteBlobWkb( blob, size );
          if ( geom )
          {
            if ( geom->Srid != *( p_vt->Srid + ig ) )
            {
              /* SRID constraint violation */
              geom_constraint_err = 1;
              goto error;
            }
            /* checking for TYPE constraint violation */
            if ( gaiaGeometryType( geom ) !=
                 *( p_vt->GeoType + ig ) )
            {
              /* Geometry TYPE constraint violation */
              geom_constraint_err = 1;
              goto error;
            }
            switch ( *( p_vt->Format + ig ) )
            {
              case FDO_OGR_WKT:
                gaiaOutWkt( geom, &text_wkt );
                if ( text_wkt )
                  sqlite3_bind_text( stmt, i - 1,
                                     text_wkt,
                                     strlen
                                     ( text_wkt ),
                                     free );
                else
                {
                  err_geom = 1;
                  goto error;
                }
                break;
              case FDO_OGR_WKB:
                gaiaToWkb( geom, &blob_wkb, &size );
                if ( blob_wkb )
                  sqlite3_bind_blob( stmt, i - 1,
                                     blob_wkb, size,
                                     free );
                else
                {
                  err_geom = 1;
                  goto error;
                }
                break;
              case FDO_OGR_FGF:
                gaiaToFgf( geom, &blob_wkb, &size,
                           *( p_vt->CoordDimensions +
                              ig ) );
                if ( blob_wkb )
                  sqlite3_bind_blob( stmt, i - 1,
                                     blob_wkb, size,
                                     free );
                else
                {
                  err_geom = 1;
                  goto error;
                }
                break;
              default:
                err_geom = 1;
                goto error;
                break;
            };
          }
          else
          {
            err_geom = 1;
            goto error;
          }
        }
        else if ( sqlite3_value_type( argv[i] ) == SQLITE_NULL )
          sqlite3_bind_null( stmt, i - 1 );
        else
        {
          err_geom = 1;
          goto error;
        }
        geom_done = 1;
      }
    }
    if ( geom_done )
      continue;
    switch ( sqlite3_value_type( argv[i] ) )
    {
      case SQLITE_INTEGER:
        sqlite3_bind_int64( stmt, i - 1, sqlite3_value_int64( argv[i] ) );
        break;
      case SQLITE_FLOAT:
        sqlite3_bind_double( stmt, i - 1,
                             sqlite3_value_double( argv[i] ) );
        break;
      case SQLITE_TEXT:
        text = ( char * ) sqlite3_value_text( argv[i] );
        size = sqlite3_value_bytes( argv[i] );
        sqlite3_bind_text( stmt, i - 1, text, size, SQLITE_STATIC );
        break;
      case SQLITE_BLOB:
        blob = sqlite3_value_blob( argv[i] );
        size = sqlite3_value_bytes( argv[i] );
        sqlite3_bind_blob( stmt, i - 1, blob, size, SQLITE_STATIC );
        break;
      case SQLITE_NULL:
      default:
        sqlite3_bind_null( stmt, i - 1 );
        break;
    };
  }
error:
  if ( err_geom || geom_constraint_err )
  {
    sqlite3_finalize( stmt );
    return SQLITE_CONSTRAINT;
  }
  ret = sqlite3_step( stmt );
  if ( ret == SQLITE_DONE || ret == SQLITE_ROW )
    ;
  else
  {
    sqlite3_finalize( stmt );
    return ret;
  }
  sqlite3_finalize( stmt );
  return SQLITE_OK;
}

static int
vfdo_delete_row( VirtualFDOPtr p_vt, sqlite3_int64 rowid )
{
  /* trying to delete a row from FDO-OGR real-table */
  char sql[1024];
  int ret;
  sprintf( sql, "DELETE FROM %s WHERE ROWID = %lld", p_vt->table, rowid );
  ret = sqlite3_exec( p_vt->db, sql, NULL, NULL, NULL );
  return ret;
}

static void
vfdo_read_row( VirtualFDOCursorPtr cursor )
{
  /* trying to read a row from FDO-OGR real-table */
  sqlite3_stmt *stmt;
  int ret;
  char sql[4096];
  char buf[256];
  int ic;
  int ig;
  const unsigned char *wkt;
  const char *text;
  const unsigned char *blob;
  unsigned char *xblob;
  int size;
  sqlite3_int64 pk;
  int geom_done;
  gaiaGeomCollPtr geom;
  strcpy( sql, "SELECT ROWID" );
  for ( ic = 0; ic < cursor->pVtab->nColumns; ic++ )
  {
    sprintf( buf, ",%s", *( cursor->pVtab->Column + ic ) );
    strcat( sql, buf );
  }
  sprintf( buf, " FROM %s WHERE ROWID >= %lld", cursor->pVtab->table,
           cursor->current_row );
  strcat( sql, buf );
  ret =
    sqlite3_prepare_v2( cursor->pVtab->db, sql, strlen( sql ), &stmt, NULL );
  if ( ret != SQLITE_OK )
  {
    /* an error occurred */
    cursor->eof = 1;
    return;
  }
  ret = sqlite3_step( stmt );
  if ( ret == SQLITE_ROW )
  {
    pk = sqlite3_column_int64( stmt, 0 );
    for ( ic = 0; ic < cursor->pVtab->nColumns; ic++ )
    {
      /* fetching column values */
      geom_done = 0;
      for ( ig = 0; ig < cursor->pVtab->nGeometries; ig++ )
      {
        if ( strcasecmp
             ( *( cursor->pVtab->Column + ic ),
               *( cursor->pVtab->GeoColumn + ig ) ) == 0 )
        {
          /* this one is a Geometry column */
          switch ( *( cursor->pVtab->Format + ig ) )
          {
            case FDO_OGR_WKT:
              if ( sqlite3_column_type( stmt, ic + 1 ) ==
                   SQLITE_TEXT )
              {
                /* trying to parse a WKT Geometry */
                wkt =
                  sqlite3_column_text( stmt, ic + 1 );
                geom = gaiaParseWkt( wkt, -1 );
                if ( !geom )
                  value_set_null( *
                                  ( cursor->pVtab->
                                    Value + ic ) );
                else
                {
                  geom->Srid =
                    *( cursor->pVtab->Srid + ig );
                  gaiaToSpatiaLiteBlobWkb( geom,
                                           &xblob,
                                           &size );
                  if ( xblob )
                    value_set_blob( *
                                    ( cursor->
                                      pVtab->
                                      Value + ic ),
                                    xblob, size );
                  else
                    value_set_null( *
                                    ( cursor->
                                      pVtab->
                                      Value + ic ) );
                  gaiaFreeGeomColl( geom );
                }
              }
              else
                value_set_null( *
                                ( cursor->pVtab->Value +
                                  ic ) );
              break;
            case FDO_OGR_WKB:
              if ( sqlite3_column_type( stmt, ic + 1 ) ==
                   SQLITE_BLOB )
              {
                /* trying to parse a WKB Geometry */
                blob =
                  sqlite3_column_blob( stmt, ic + 1 );
                size =
                  sqlite3_column_bytes( stmt, ic + 1 );
                geom = gaiaFromWkb( blob, size );
                if ( !geom )
                  value_set_null( *
                                  ( cursor->pVtab->
                                    Value + ic ) );
                else
                {
                  geom->Srid =
                    *( cursor->pVtab->Srid + ig );
                  gaiaToSpatiaLiteBlobWkb( geom,
                                           &xblob,
                                           &size );
                  if ( xblob )
                    value_set_blob( *
                                    ( cursor->
                                      pVtab->
                                      Value + ic ),
                                    xblob, size );
                  else
                    value_set_null( *
                                    ( cursor->
                                      pVtab->
                                      Value + ic ) );
                  gaiaFreeGeomColl( geom );
                }
              }
              else
                value_set_null( *
                                ( cursor->pVtab->Value +
                                  ic ) );
              break;
            case FDO_OGR_FGF:
              if ( sqlite3_column_type( stmt, ic + 1 ) ==
                   SQLITE_BLOB )
              {
                /* trying to parse an FGF Geometry */
                blob =
                  sqlite3_column_blob( stmt, ic + 1 );
                size =
                  sqlite3_column_bytes( stmt, ic + 1 );
                geom = gaiaFromFgf( blob, size );
                if ( !geom )
                  value_set_null( *
                                  ( cursor->pVtab->
                                    Value + ic ) );
                else
                {
                  geom->Srid =
                    *( cursor->pVtab->Srid + ig );
                  gaiaToSpatiaLiteBlobWkb( geom,
                                           &xblob,
                                           &size );
                  if ( xblob )
                    value_set_blob( *
                                    ( cursor->
                                      pVtab->
                                      Value + ic ),
                                    xblob, size );
                  else
                    value_set_null( *
                                    ( cursor->
                                      pVtab->
                                      Value + ic ) );
                  gaiaFreeGeomColl( geom );
                }
              }
              else
                value_set_null( *
                                ( cursor->pVtab->Value +
                                  ic ) );
              break;
            default:
              value_set_null( *( cursor->pVtab->Value + ic ) );
              break;
          };
          geom_done = 1;
        }
      }
      if ( geom_done )
        continue;
      switch ( sqlite3_column_type( stmt, ic + 1 ) )
      {
        case SQLITE_INTEGER:
          value_set_int( *( cursor->pVtab->Value + ic ),
                         sqlite3_column_int64( stmt, ic + 1 ) );
          break;
        case SQLITE_FLOAT:
          value_set_double( *( cursor->pVtab->Value + ic ),
                            sqlite3_column_double( stmt, ic + 1 ) );
          break;
        case SQLITE_TEXT:
          text = ( char * ) sqlite3_column_text( stmt, ic + 1 );
          size = sqlite3_column_bytes( stmt, ic + 1 );
          value_set_text( *( cursor->pVtab->Value + ic ), text, size );
          break;
        case SQLITE_BLOB:
          blob = sqlite3_column_blob( stmt, ic + 1 );
          size = sqlite3_column_bytes( stmt, ic + 1 );
          value_set_blob( *( cursor->pVtab->Value + ic ), blob, size );
          break;
        case SQLITE_NULL:
        default:
          value_set_null( *( cursor->pVtab->Value + ic ) );
          break;
      };
    }
  }
  else
  {
    /* an error occurred */
    sqlite3_finalize( stmt );
    cursor->eof = 1;
    return;
  }
  sqlite3_finalize( stmt );
  cursor->eof = 0;
  cursor->current_row = pk;
}

static int
vfdo_create( sqlite3 * db, void *pAux, int argc, const char *const *argv,
             sqlite3_vtab ** ppVTab, char **pzErr )
{
  /* creates the virtual table connected to some FDO-OGR table */
  const char *vtable;
  const char *table;
  int ret;
  int i;
  int len;
  int n_rows;
  int n_columns;
  const char *col_name;
  const char *col_type;
  const char *format;
  int coord_dimension;
  int not_null;
  int srid;
  int type;
  char **results;
  char sql[4096];
  char buf[256];
  char prefix;
  VirtualFDOPtr p_vt = NULL;
  /* checking for table_name */
  if ( argc == 4 )
  {
    vtable = argv[2];
    table = argv[3];
  }
  else
  {
    *pzErr =
      sqlite3_mprintf
      ( "[VirtualFDO module] CREATE VIRTUAL: illegal arg list {table_name}\n" );
    return SQLITE_ERROR;
  }
  /* retrieving the base table columns */
  sprintf( sql, "PRAGMA table_info(%s)", table );
  ret = sqlite3_get_table( db, sql, &results, &n_rows, &n_columns, NULL );
  if ( ret != SQLITE_OK )
    goto illegal;
  if ( n_rows >= 1 )
  {
    p_vt = ( VirtualFDOPtr ) sqlite3_malloc( sizeof( VirtualFDO ) );
    if ( !p_vt )
      return SQLITE_NOMEM;
    p_vt->db = db;
    p_vt->nRef = 0;
    p_vt->zErrMsg = NULL;
    len = strlen( table );
    p_vt->table = sqlite3_malloc( len + 1 );
    strcpy( p_vt->table, table );
    p_vt->nColumns = n_rows;
    p_vt->Column = sqlite3_malloc( sizeof( char * ) * n_rows );
    p_vt->Type = sqlite3_malloc( sizeof( char * ) * n_rows );
    p_vt->NotNull = sqlite3_malloc( sizeof( int ) * n_rows );
    p_vt->Value = sqlite3_malloc( sizeof( SqliteValuePtr ) * n_rows );
    for ( i = 0; i < n_rows; i++ )
    {
      *( p_vt->Column + i ) = NULL;
      *( p_vt->Type + i ) = NULL;
      *( p_vt->NotNull + i ) = -1;
      *( p_vt->Value + i ) = value_alloc();
    }
    p_vt->nGeometries = 0;
    p_vt->GeoColumn = NULL;
    p_vt->Srid = NULL;
    p_vt->GeoType = NULL;
    p_vt->Format = NULL;
    p_vt->CoordDimensions = NULL;
    for ( i = 1; i <= n_rows; i++ )
    {
      col_name = results[( i * n_columns ) + 1];
      col_type = results[( i * n_columns ) + 2];
      if ( atoi( results[( i * n_columns ) + 3] ) == 0 )
        not_null = 0;
      else
        not_null = 1;
      len = strlen( col_name );
      *( p_vt->Column + ( i - 1 ) ) = sqlite3_malloc( len + 1 );
      strcpy( *( p_vt->Column + ( i - 1 ) ), col_name );
      len = strlen( col_type );
      *( p_vt->Type + ( i - 1 ) ) = sqlite3_malloc( len + 1 );
      strcpy( *( p_vt->Type + ( i - 1 ) ), col_type );
      *( p_vt->NotNull + ( i - 1 ) ) = not_null;
    }
    sqlite3_free_table( results );
  }
  else
    goto illegal;
  /* retrieving the base table columns */
  strcpy( sql,
          "SELECT f_geometry_column, geometry_type, srid, geometry_format, coord_dimension\n" );
  strcat( sql, "FROM geometry_columns WHERE f_table_name LIKE '" );
  strcat( sql, table );
  strcat( sql, "'" );
  ret = sqlite3_get_table( db, sql, &results, &n_rows, &n_columns, NULL );
  if ( ret != SQLITE_OK )
    goto illegal;
  if ( n_rows >= 1 )
  {
    p_vt->nGeometries = n_rows;
    p_vt->GeoColumn = sqlite3_malloc( sizeof( char * ) * n_rows );
    p_vt->Srid = sqlite3_malloc( sizeof( char * ) * n_rows );
    p_vt->GeoType = sqlite3_malloc( sizeof( int ) * n_rows );
    p_vt->Format = sqlite3_malloc( sizeof( int ) * n_rows );
    p_vt->CoordDimensions = sqlite3_malloc( sizeof( int ) * n_rows );
    for ( i = 0; i < n_rows; i++ )
    {
      *( p_vt->GeoColumn + i ) = NULL;
      *( p_vt->Srid + i ) = -1;
      *( p_vt->GeoType + i ) = -1;
      *( p_vt->Format + i ) = FDO_OGR_NONE;
      *( p_vt->CoordDimensions + i ) = GAIA_XY;
    }
    for ( i = 1; i <= n_rows; i++ )
    {
      col_name = results[( i * n_columns ) + 0];
      type = atoi( results[( i * n_columns ) + 1] );
      srid = atoi( results[( i * n_columns ) + 2] );
      format = results[( i * n_columns ) + 3];
      coord_dimension = atoi( results[( i * n_columns ) + 4] );
      len = strlen( col_name );
      *( p_vt->GeoColumn + ( i - 1 ) ) = sqlite3_malloc( len + 1 );
      strcpy( *( p_vt->GeoColumn + ( i - 1 ) ), col_name );
      *( p_vt->GeoType + ( i - 1 ) ) = type;
      *( p_vt->Srid + ( i - 1 ) ) = srid;
      if ( strcasecmp( format, "WKT" ) == 0 )
        *( p_vt->Format + ( i - 1 ) ) = FDO_OGR_WKT;
      if ( strcasecmp( format, "WKB" ) == 0 )
        *( p_vt->Format + ( i - 1 ) ) = FDO_OGR_WKB;
      if ( strcasecmp( format, "FGF" ) == 0 )
        *( p_vt->Format + ( i - 1 ) ) = FDO_OGR_FGF;
      if ( coord_dimension == 3 )
        *( p_vt->CoordDimensions + ( i - 1 ) ) = GAIA_XY_Z;
      else if ( coord_dimension == 4 )
        *( p_vt->CoordDimensions + ( i - 1 ) ) = GAIA_XY_Z_M;
      else
        *( p_vt->CoordDimensions + ( i - 1 ) ) = GAIA_XY;
    }
    sqlite3_free_table( results );
  }
  else
    goto illegal;
  /* preparing the COLUMNs for this VIRTUAL TABLE */
  strcpy( sql, "CREATE TABLE " );
  strcat( sql, vtable );
  for ( i = 0; i < p_vt->nColumns; i++ )
  {
    if ( i == 0 )
      prefix = '(';
    else
      prefix = ',';
    sprintf( buf, "%c%s %s", prefix, *( p_vt->Column + i ),
             *( p_vt->Type + i ) );
    if ( *( p_vt->NotNull + i ) )
      strcat( buf, " NOT NULL" );
    strcat( sql, buf );
  }
  strcat( sql, ")" );
  if ( sqlite3_declare_vtab( db, sql ) != SQLITE_OK )
  {
    *pzErr =
      sqlite3_mprintf
      ( "[VirtualFDO module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
        sql );
    return SQLITE_ERROR;
  }
  *ppVTab = ( sqlite3_vtab * ) p_vt;
  return SQLITE_OK;
illegal:
  /* something is going the wrong way */
  if ( p_vt )
    free_table( p_vt );
  *pzErr =
    sqlite3_mprintf
    ( "[VirtualFDO module] '%s' isn't a valid FDO-OGR Geometry table\n",
      table );
  return SQLITE_ERROR;
}

static int
vfdo_connect( sqlite3 * db, void *pAux, int argc, const char *const *argv,
              sqlite3_vtab ** ppVTab, char **pzErr )
{
  /* connects the virtual table to some shapefile - simply aliases vfdo_create() */
  return vfdo_create( db, pAux, argc, argv, ppVTab, pzErr );
}

static int
vfdo_best_index( sqlite3_vtab * pVTab, sqlite3_index_info * pIndex )
{
  /* best index selection */
  return SQLITE_OK;
}

static int
vfdo_disconnect( sqlite3_vtab * pVTab )
{
  /* disconnects the virtual table */
  VirtualFDOPtr p_vt = ( VirtualFDOPtr ) pVTab;
  free_table( p_vt );
  return SQLITE_OK;
}

static int
vfdo_destroy( sqlite3_vtab * pVTab )
{
  /* destroys the virtual table - simply aliases vfdo_disconnect() */
  return vfdo_disconnect( pVTab );
}

static int
vfdo_open( sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor )
{
  /* opening a new cursor */
  VirtualFDOCursorPtr cursor = ( VirtualFDOCursorPtr )
                               sqlite3_malloc( sizeof( VirtualFDOCursor ) );
  if ( cursor == NULL )
    return SQLITE_ERROR;
  cursor->pVtab = ( VirtualFDOPtr ) pVTab;
  cursor->current_row = min_rowid();
  cursor->eof = 0;
  *ppCursor = ( sqlite3_vtab_cursor * ) cursor;
  vfdo_read_row( cursor );
  return SQLITE_OK;
}

static int
vfdo_close( sqlite3_vtab_cursor * pCursor )
{
  /* closing the cursor */
  sqlite3_free( pCursor );
  return SQLITE_OK;
}

static int
vfdo_filter( sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
             int argc, sqlite3_value ** argv )
{
  /* setting up a cursor filter */
  return SQLITE_OK;
}

static int
vfdo_next( sqlite3_vtab_cursor * pCursor )
{
  /* fetching next row from cursor */
  VirtualFDOCursorPtr cursor = ( VirtualFDOCursorPtr ) pCursor;
  ( cursor->current_row )++;
  vfdo_read_row( cursor );
  return SQLITE_OK;
}

static int
vfdo_eof( sqlite3_vtab_cursor * pCursor )
{
  /* cursor EOF */
  VirtualFDOCursorPtr cursor = ( VirtualFDOCursorPtr ) pCursor;
  return cursor->eof;
}

static int
vfdo_column( sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
             int column )
{
  /* fetching value for the Nth column */
  VirtualFDOCursorPtr cursor = ( VirtualFDOCursorPtr ) pCursor;
  SqliteValuePtr value;
  if ( column >= 0 && column < cursor->pVtab->nColumns )
  {
    value = *( cursor->pVtab->Value + column );
    switch ( value->Type )
    {
      case SQLITE_INTEGER:
        sqlite3_result_int64( pContext, value->IntValue );
        break;
      case SQLITE_FLOAT:
        sqlite3_result_double( pContext, value->DoubleValue );
        break;
      case SQLITE_TEXT:
        sqlite3_result_text( pContext, value->Text, value->Size,
                             SQLITE_STATIC );
        break;
      case SQLITE_BLOB:
        sqlite3_result_blob( pContext, value->Blob, value->Size,
                             SQLITE_STATIC );
        break;
      default:
        sqlite3_result_null( pContext );
        break;
    };
  }
  else
    sqlite3_result_null( pContext );
  return SQLITE_OK;
}

static int
vfdo_rowid( sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid )
{
  /* fetching the ROWID */
  VirtualFDOCursorPtr cursor = ( VirtualFDOCursorPtr ) pCursor;
  *pRowid = cursor->current_row;
  return SQLITE_OK;
}

static int
vfdo_update( sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
             sqlite_int64 * pRowid )
{
  /* generic update [INSERT / UPDATE / DELETE */
  sqlite3_int64 rowid;
  int ret;
  VirtualFDOPtr p_vt = ( VirtualFDOPtr ) pVTab;
  if ( argc == 1 )
  {
    /* performing a DELETE */
    if ( sqlite3_value_type( argv[0] ) == SQLITE_INTEGER )
    {
      rowid = sqlite3_value_int64( argv[0] );
      ret = vfdo_delete_row( p_vt, rowid );
    }
    else
      ret = SQLITE_MISMATCH;
  }
  else
  {
    if ( sqlite3_value_type( argv[0] ) == SQLITE_NULL )
    {
      /* performing an INSERT */
      ret = vfdo_insert_row( p_vt, &rowid, argc, argv );
      if ( ret == SQLITE_OK )
        *pRowid = rowid;
    }
    else
    {
      /* performing an UPDATE */
      rowid = sqlite3_value_int64( argv[0] );
      ret = vfdo_update_row( p_vt, rowid, argc, argv );
    }
  }
  return ret;
}

static int
vfdo_begin( sqlite3_vtab * pVTab )
{
  /* BEGIN TRANSACTION */
  return SQLITE_OK;
}

static int
vfdo_sync( sqlite3_vtab * pVTab )
{
  /* BEGIN TRANSACTION */
  return SQLITE_OK;
}

static int
vfdo_commit( sqlite3_vtab * pVTab )
{
  /* BEGIN TRANSACTION */
  return SQLITE_OK;
}

static int
vfdo_rollback( sqlite3_vtab * pVTab )
{
  /* BEGIN TRANSACTION */
  return SQLITE_OK;
}

int
sqlite3VirtualFDOInit( sqlite3 * db )
{
  int rc = SQLITE_OK;
  my_module.iVersion = 1;
  my_module.xCreate = &vfdo_create;
  my_module.xConnect = &vfdo_connect;
  my_module.xBestIndex = &vfdo_best_index;
  my_module.xDisconnect = &vfdo_disconnect;
  my_module.xDestroy = &vfdo_destroy;
  my_module.xOpen = &vfdo_open;
  my_module.xClose = &vfdo_close;
  my_module.xFilter = &vfdo_filter;
  my_module.xNext = &vfdo_next;
  my_module.xEof = &vfdo_eof;
  my_module.xColumn = &vfdo_column;
  my_module.xRowid = &vfdo_rowid;
  my_module.xUpdate = &vfdo_update;
  my_module.xBegin = &vfdo_begin;
  my_module.xSync = &vfdo_sync;
  my_module.xCommit = &vfdo_commit;
  my_module.xRollback = &vfdo_rollback;
  my_module.xFindFunction = NULL;
  sqlite3_create_module_v2( db, "VirtualFDO", &my_module, NULL, 0 );
  return rc;
}

int
virtualfdo_extension_init( sqlite3 * db, const sqlite3_api_routines * pApi )
{
  SQLITE_EXTENSION_INIT2( pApi ) return sqlite3VirtualFDOInit( db );
}
