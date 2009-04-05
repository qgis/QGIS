/*

 virtualtext.c -- SQLite3 extension [VIRTUAL TABLE accessing CSV/TXT]

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
#include <spatialite/sqlite3ext.h>
#include <spatialite/spatialite.h>
#include <spatialite/gaiaaux.h>

#ifdef _MSC_VER
#define strcasecmp(a,b) _stricmp(a,b)
#endif

#define VRTTXT_TEXT  1
#define VRTTXT_INTEGER 2
#define VRTTXT_DOUBLE 3

static SQLITE_EXTENSION_INIT1 static struct sqlite3_module my_module;

struct row_buffer
{
  /* a complete row */
  int n_cells;  /* how many cells are stored into this line */
  char **cells;  /* the cells array */
  struct row_buffer *next; /* pointer for linked list */
};

struct text_buffer
{
  int max_n_cells;  /* the maximun cell index */
  char **titles;  /* the column titles array */
  char *types;  /* the column types array */
  int n_rows;   /* the number of rows */
  struct row_buffer **rows; /* the rows array */
  struct row_buffer *first; /* pointers to build a linked list of rows */
  struct row_buffer *last;
};

typedef struct VirtualTextStruct
{
  /* extends the sqlite3_vtab struct */
  const sqlite3_module *pModule; /* ptr to sqlite module: USED INTERNALLY BY SQLITE */
  int nRef;   /* # references: USED INTERNALLY BY SQLITE */
  char *zErrMsg;  /* error message: USED INTERNALLY BY SQLITE */
  sqlite3 *db;  /* the sqlite db holding the virtual table */
  struct text_buffer *buffer; /* the in-memory buffer storing text */
} VirtualText;
typedef VirtualText *VirtualTextPtr;

typedef struct VirtualTextCursortStruct
{
  /* extends the sqlite3_vtab_cursor struct */
  VirtualTextPtr pVtab; /* Virtual table of this cursor */
  long current_row;  /* the current row ID */
  int eof;   /* the EOF marker */
} VirtualTextCursor;
typedef VirtualTextCursor *VirtualTextCursorPtr;

static void
text_insert_row( struct text_buffer *text, char **fields, int max_cell )
{
  /* inserting a row into the text buffer struct */
  int i;
  struct row_buffer *row = malloc( sizeof( struct row_buffer ) );
  row->n_cells = max_cell + 1;
  if ( max_cell < 0 )
    row->cells = NULL;
  else
  {
    row->cells = malloc( sizeof( char * ) * ( max_cell + 1 ) );
    for ( i = 0; i < row->n_cells; i++ )
    {
      /* setting cell values */
      *( row->cells + i ) = *( fields + i );
    }
  }
  row->next = NULL;
  /* inserting the row into the linked list */
  if ( !( text->first ) )
    text->first = row;
  if ( text->last )
    text->last->next = row;
  text->last = row;
}

static struct text_buffer *
      text_buffer_alloc()
{
  /* allocating and initializing the text buffer struct */
  struct text_buffer *text = malloc( sizeof( struct text_buffer ) );
  text->max_n_cells = 0;
  text->titles = NULL;
  text->types = NULL;
  text->n_rows = 0;
  text->rows = NULL;
  text->first = NULL;
  text->last = NULL;
  return text;
}

static void
text_buffer_free( struct text_buffer *text )
{
  /* memory cleanup - freeing the text buffer */
  int i;
  struct row_buffer *row;
  if ( !text )
    return;
  row = text->first;
  while ( row )
  {
    for ( i = 0; i < row->n_cells; i++ )
    {
      if ( *( row->cells + i ) )
        free( *( row->cells + i ) );
    }
    row = row->next;
  }
  if ( text->types )
    free( text->types );
  free( text );
}

static int
text_is_integer( char *value )
{
  /* checking if this value can be an INTEGER */
  int invalids = 0;
  int digits = 0;
  int signs = 0;
  char last = '\0';
  char *p = value;
  while ( *p != '\0' )
  {
    last = *p;
    if ( *p >= '0' && *p <= '9' )
      digits++;
    else if ( *p == '+' || *p == '-' )
      signs++;
    else
      signs++;
    p++;
  }
  if ( invalids )
    return 0;
  if ( signs > 1 )
    return 0;
  if ( signs )
  {
    if ( *value == '+' || *value == '-' || last == '+' || last == '-' )
      ;
    else
      return 0;
  }
  return 1;
}

static int
text_is_double( char *value, char decimal_separator )
{
  /* checking if this value can be a DOUBLE */
  int invalids = 0;
  int digits = 0;
  int signs = 0;
  int points = 0;
  char last = '\0';
  char *p = value;
  while ( *p != '\0' )
  {
    last = *p;
    if ( *p >= '0' && *p <= '9' )
      digits++;
    else if ( *p == '+' || *p == '-' )
      points++;
    else
    {
      if ( decimal_separator == ',' )
      {
        if ( *p == ',' )
          points++;
        else
          invalids++;
      }
      else
      {
        if ( *p == '.' )
          points++;
        else
          invalids++;
      }
    }
    p++;
  }
  if ( invalids )
    return 0;
  if ( points > 1 )
    return 0;
  if ( signs > 1 )
    return 0;
  if ( signs )
  {
    if ( *value == '+' || *value == '-' || last == '+' || last == '-' )
      ;
    else
      return 0;
  }
  return 1;
}

static void
text_clean_integer( char *value )
{
  /* cleaning an integer value */
  char last;
  char buffer[35536];
  int len = strlen( value );
  last = value[len - 1];
  if ( last == '-' || last == '+' )
  {
    /* trailing sign; transforming into a leading sign */
    *buffer = last;
    strcpy( buffer + 1, value );
    buffer[len - 1] = '\0';
    strcpy( value, buffer );
  }
}

static void
text_clean_double( char *value )
{
  /* cleaning an integer value */
  char *p;
  char last;
  char buffer[35536];
  int len = strlen( value );
  last = value[len - 1];
  if ( last == '-' || last == '+' )
  {
    /* trailing sign; transforming into a leading sign */
    *buffer = last;
    strcpy( buffer + 1, value );
    buffer[len - 1] = '\0';
    strcpy( value, buffer );
  }
  p = value;
  while ( *p != '\0' )
  {
    /* transforming COMMAs into POINTs */
    if ( *p == ',' )
      *p = '.';
    p++;
  }
}

static int
text_clean_text( char **value, void *toUtf8 )
{
  /* cleaning a TEXT value and converting to UTF-8 */
  char *text = *value;
  char *utf8text;
  int err;
  int i;
  int oldlen = strlen( text );
  int newlen;
  for ( i = oldlen - 1; i > 0; i++ )
  {
    /* cleaning up trailing spaces */
    if ( text[i] == ' ' )
      text[i] = '\0';
    else
      break;
  }
  utf8text = gaiaConvertToUTF8( toUtf8, text, oldlen, &err );
  if ( err )
    return 1;
  newlen = strlen( utf8text );
  if ( newlen <= oldlen )
    strcpy( *value, utf8text );
  else
  {
    free( *value );
    *value = malloc( newlen + 1 );
    strcpy( *value, utf8text );
  }
  return 0;
}

static struct text_buffer *
      text_parse( char *path, char *encoding, char first_line_titles,
                  char field_separator, char text_separator, char decimal_separator )
{
  /* trying to open and parse the text file */
  int c;
  int fld;
  int len;
  int max_cell;
  int is_string = 0;
  char last = '\0';
  char *fields[4096];
  char buffer[35536];
  char *p = buffer;
  struct text_buffer *text;
  int nrows;
  int ncols;
  int errs;
  struct row_buffer *row;
  void *toUtf8;
  int encoding_errors;
  int ir;
  char title[64];
  char *first_valid_row;
  int i;
  char *name;
  FILE *in;

  for ( fld = 0; fld < 4096; fld++ )
  {
    /* preparing an empty row */
    fields[fld] = NULL;
  }
  /* trying to open the text file */
  in = fopen( path, "rb" );
  if ( !in )
    return NULL;
  text = text_buffer_alloc();
  fld = 0;
  while (( c = getc( in ) ) != EOF )
  {
    /* parsing the file, one char at each time */
    if ( c == '\r' && !is_string )
    {
      last = c;
      continue;
    }
    if ( c == field_separator && !is_string )
    {
      /* insering a field into the fields tmp array */
      last = c;
      *p = '\0';
      len = strlen( buffer );
      if ( len )
      {
        fields[fld] = malloc( len + 1 );
        strcpy( fields[fld], buffer );
      }
      fld++;
      p = buffer;
      *p = '\0';
      continue;
    }
    if ( c == text_separator )
    {
      /* found a text separator */
      if ( is_string )
      {
        is_string = 0;
        last = c;
      }
      else
      {
        if ( last == text_separator )
          *p++ = text_separator;
        is_string = 1;
      }
      continue;
    }
    last = c;
    if ( c == '\n' && !is_string )
    {
      /* inserting the row into the text buffer */
      *p = '\0';
      len = strlen( buffer );
      if ( len )
      {
        fields[fld] = malloc( len + 1 );
        strcpy( fields[fld], buffer );
      }
      fld++;
      p = buffer;
      *p = '\0';
      max_cell = -1;
      for ( fld = 0; fld < 4096; fld++ )
      {
        if ( fields[fld] )
          max_cell = fld;
      }
      text_insert_row( text, fields, max_cell );
      for ( fld = 0; fld < 4096; fld++ )
      {
        /* resetting an empty row */
        fields[fld] = NULL;
      }
      fld = 0;
      continue;
    }
    *p++ = c;
  }
  fclose( in );
  /* checking if the text file really seems to contain a table */
  nrows = 0;
  ncols = 0;
  errs = 0;
  row = text->first;
  while ( row )
  {
    if ( first_line_titles && row == text->first )
    {
      /* skipping first line */
      row = row->next;
      continue;
    }
    nrows++;
    if ( row->n_cells > ncols )
      ncols = row->n_cells;
    row = row->next;
  }
  if ( nrows == 0 && ncols == 0 )
  {
    text_buffer_free( text );
    return NULL;
  }
  text->n_rows = nrows;
  /* going to check the column types */
  text->max_n_cells = ncols;
  text->types = malloc( sizeof( char ) * text->max_n_cells );
  first_valid_row = malloc( sizeof( char ) * text->max_n_cells );
  for ( fld = 0; fld < text->max_n_cells; fld++ )
  {
    /* initally assuming any cell contains TEXT */
    *( text->types + fld ) = VRTTXT_TEXT;
    *( first_valid_row + fld ) = 1;
  }
  row = text->first;
  while ( row )
  {
    if ( first_line_titles && row == text->first )
    {
      /* skipping first line */
      row = row->next;
      continue;
    }
    for ( fld = 0; fld < row->n_cells; fld++ )
    {
      if ( *( row->cells + fld ) )
      {
        if ( text_is_integer( *( row->cells + fld ) ) )
        {
          if ( *( first_valid_row + fld ) )
          {
            *( text->types + fld ) = VRTTXT_INTEGER;
            *( first_valid_row + fld ) = 0;
          }
        }
        else if ( text_is_double
                  ( *( row->cells + fld ), decimal_separator ) )
        {
          if ( *( first_valid_row + fld ) )
          {
            *( text->types + fld ) = VRTTXT_DOUBLE;
            *( first_valid_row + fld ) = 0;
          }
          else
          {
            /* promoting an INTEGER column to be of the DOUBLE type */
            if ( *( text->types + fld ) == VRTTXT_INTEGER )
              *( text->types + fld ) = VRTTXT_DOUBLE;
          }
        }
        else
        {
          /* this column is anyway of the TEXT type */
          *( text->types + fld ) = VRTTXT_TEXT;
          if ( *( first_valid_row + fld ) )
            *( first_valid_row + fld ) = 0;
        }
      }
    }
    row = row->next;
  }
  free( first_valid_row );
  /* preparing the column names */
  text->titles = malloc( sizeof( char * ) * text->max_n_cells );
  if ( first_line_titles )
  {
    for ( fld = 0; fld < text->max_n_cells; fld++ )
    {
      if ( fld >= text->first->n_cells )
      {
        /* this column name is NULL; setting a default name */
        sprintf( title, "COL%03d", fld + 1 );
        len = strlen( title );
        *( text->titles + fld ) = malloc( len + 1 );
        strcpy( *( text->titles + fld ), title );
      }
      else
      {
        if ( *( text->first->cells + fld ) )
        {
          len = strlen( *( text->first->cells + fld ) );
          *( text->titles + fld ) = malloc( len + 1 );
          strcpy( *( text->titles + fld ),
                  *( text->first->cells + fld ) );
          name = *( text->titles + fld );
          for ( i = 0; i < len; i++ )
          {
            /* masking any space in the column name */
            if ( *( name + i ) == ' ' )
              *( name + i ) = '_';
          }
        }
        else
        {
          /* this column name is NULL; setting a default name */
          sprintf( title, "COL%03d", fld + 1 );
          len = strlen( title );
          *( text->titles + fld ) = malloc( len + 1 );
          strcpy( *( text->titles + fld ), title );
        }
      }
    }
  }
  else
  {
    for ( fld = 0; fld < text->max_n_cells; fld++ )
    {
      sprintf( title, "COL%03d", fld + 1 );
      len = strlen( title );
      *( text->titles + fld ) = malloc( len + 1 );
      strcpy( *( text->titles + fld ), title );
    }
  }
  /* cleaning cell values when needed */
  toUtf8 = gaiaCreateUTF8Converter( encoding );
  if ( !toUtf8 )
  {
    text_buffer_free( text );
    return NULL;
  }
  encoding_errors = 0;
  row = text->first;
  while ( row )
  {
    if ( first_line_titles && row == text->first )
    {
      /* skipping first line */
      row = row->next;
      continue;
    }
    for ( fld = 0; fld < row->n_cells; fld++ )
    {
      if ( *( row->cells + fld ) )
      {
        if ( *( text->types + fld ) == VRTTXT_INTEGER )
          text_clean_integer( *( row->cells + fld ) );
        else if ( *( text->types + fld ) == VRTTXT_DOUBLE )
          text_clean_double( *( row->cells + fld ) );
        else
          encoding_errors +=
            text_clean_text( row->cells + fld, toUtf8 );
      }
    }
    row = row->next;
  }
  gaiaFreeUTF8Converter( toUtf8 );
  if ( encoding_errors )
  {
    text_buffer_free( text );
    return NULL;
  }
  /* ok, we can now go to prepare the rows array */
  text->rows = malloc( sizeof( struct text_row * ) * text->n_rows );
  ir = 0;
  row = text->first;
  while ( row )
  {
    if ( first_line_titles && row == text->first )
    {
      /* skipping first line */
      row = row->next;
      continue;
    }
    *( text->rows + ir++ ) = row;
    row = row->next;
  }
  return text;
}

static int
vtxt_create( sqlite3 * db, void *pAux, int argc, const char *const *argv,
             sqlite3_vtab ** ppVTab, char **pzErr )
{
  /* creates the virtual table connected to some TEXT file */
  char path[2048];
  char encoding[128];
  const char *vtable;
  const char *pEncoding = NULL;
  int len;
  struct text_buffer *text = NULL;
  const char *pPath = NULL;
  char field_separator = '\t';
  char text_separator = '"';
  char decimal_separator = '.';
  char first_line_titles = 1;
  int i;
  char sql[4096];
  int seed;
  int dup;
  int idup;
  char dummyName[4096];
  char **col_name = NULL;
  VirtualTextPtr p_vt;
  /* checking for TEXTfile PATH */
  if ( argc >= 5 && argc <= 9 )
  {
    vtable = argv[1];
    pPath = argv[3];
    len = strlen( pPath );
    if ( *( pPath + 0 ) == '\'' || *( pPath + 0 ) == '"' &&
         *( pPath + len - 1 ) == '\'' || *( pPath + len - 1 ) == '"' )
    {
      /* the path is enclosed between quotes - we need to dequote it */
      strcpy( path, pPath + 1 );
      len = strlen( path );
      *( path + len - 1 ) = '\0';
    }
    else
      strcpy( path, pPath );
    pEncoding = argv[4];
    len = strlen( pEncoding );
    if ( *( pEncoding + 0 ) == '\'' || *( pEncoding + 0 ) == '"' &&
         *( pEncoding + len - 1 ) == '\'' || *( pEncoding + len - 1 ) == '"' )
    {
      /* the charset-name is enclosed between quotes - we need to dequote it */
      strcpy( encoding, pEncoding + 1 );
      len = strlen( encoding );
      *( encoding + len - 1 ) = '\0';
    }
    else
      strcpy( encoding, pEncoding );
    if ( argc >= 6 )
    {
      if ( *( argv[5] ) == '0' || *( argv[5] ) == 'n' || *( argv[5] ) == 'N' )
        first_line_titles = 0;
    }
    if ( argc >= 7 )
    {
      if ( strcasecmp( argv[6], "COMMA" ) == 0 )
        decimal_separator = ',';
    }
    if ( argc >= 8 )
    {
      if ( strcasecmp( argv[7], "SINGLEQUOTE" ) == 0 )
        text_separator = '\'';
    }
    if ( argc == 9 )
    {
      if ( strlen( argv[8] ) == 3 )
      {
        if ( strcasecmp( argv[8], "TAB" ) == 0 )
          field_separator = '\t';
        if ( *( argv[8] + 0 ) == '\'' && *( argv[8] + 2 ) == '\'' )
          field_separator = *( argv[8] + 1 );
      }
    }
  }
  else
  {
    *pzErr =
      sqlite3_mprintf
      ( "[VirtualText module] CREATE VIRTUAL: illegal arg list\n"
        "\t\t{ text_path, encoding [, titles [, [decimal_separator [, text_separator, [field_separator] ] ] ] }\n" );
    return SQLITE_ERROR;
  }
  p_vt = ( VirtualTextPtr ) sqlite3_malloc( sizeof( VirtualText ) );
  if ( !p_vt )
    return SQLITE_NOMEM;
  p_vt->pModule = &my_module;
  p_vt->nRef = 0;
  p_vt->zErrMsg = NULL;
  p_vt->db = db;
  text =
    text_parse( path, encoding, first_line_titles, field_separator,
                text_separator, decimal_separator );
  if ( !text )
  {
    /* something is going the wrong way; creating a stupid default table */
    sprintf( sql, "CREATE TABLE %s (ROWNO INTEGER)", vtable );
    if ( sqlite3_declare_vtab( db, sql ) != SQLITE_OK )
    {
      *pzErr =
        sqlite3_mprintf
        ( "[VirtualText module] cannot build a table from TEXT file\n" );
      return SQLITE_ERROR;
    }
    p_vt->buffer = NULL;
    *ppVTab = ( sqlite3_vtab * ) p_vt;
    return SQLITE_OK;
  }
  p_vt->buffer = text;
  /* preparing the COLUMNs for this VIRTUAL TABLE */
  sprintf( sql, "CREATE TABLE %s (ROWNO INTEGER", vtable );
  col_name = malloc( sizeof( char * ) * text->max_n_cells );
  seed = 0;
  for ( i = 0; i < text->max_n_cells; i++ )
  {
    strcat( sql, ", " );
    if ( gaiaIllegalSqlName( *( text->titles + i ) )
         || gaiaIsReservedSqlName( *( text->titles + i ) )
         || gaiaIsReservedSqliteName( *( text->titles + i ) ) )
      sprintf( dummyName, "COL_%d", seed++ );
    else
      strcpy( dummyName, *( text->titles + i ) );
    dup = 0;
    for ( idup = 0; idup < i; idup++ )
    {
      if ( strcasecmp( dummyName, *( col_name + idup ) ) == 0 )
        dup = 1;
    }
    if ( strcasecmp( dummyName, "PKUID" ) == 0 )
      dup = 1;
    if ( strcasecmp( dummyName, "Geometry" ) == 0 )
      dup = 1;
    if ( dup )
      sprintf( dummyName, "COL_%d", seed++ );
    len = strlen( dummyName );
    *( col_name + i ) = malloc( len + 1 );
    strcpy( *( col_name + i ), dummyName );
    strcat( sql, dummyName );
    if ( *( text->types + i ) == VRTTXT_INTEGER )
      strcat( sql, " INTEGER" );
    else if ( *( text->types + i ) == VRTTXT_DOUBLE )
      strcat( sql, " DOUBLE" );
    else
      strcat( sql, " TEXT" );
  }
  strcat( sql, ")" );
  if ( col_name )
  {
    /* releasing memory allocation for column names */
    for ( i = 0; i < text->max_n_cells; i++ )
      free( *( col_name + i ) );
    free( col_name );
  }
  if ( sqlite3_declare_vtab( db, sql ) != SQLITE_OK )
  {
    *pzErr =
      sqlite3_mprintf
      ( "[VirtualText module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
        sql );
    return SQLITE_ERROR;
  }
  *ppVTab = ( sqlite3_vtab * ) p_vt;
  return SQLITE_OK;
}

static int
vtxt_connect( sqlite3 * db, void *pAux, int argc, const char *const *argv,
              sqlite3_vtab ** ppVTab, char **pzErr )
{
  /* connects the virtual table to some shapefile - simply aliases vshp_create() */
  return vtxt_create( db, pAux, argc, argv, ppVTab, pzErr );
}

static int
vtxt_best_index( sqlite3_vtab * pVTab, sqlite3_index_info * pIndex )
{
  /* best index selection */
  return SQLITE_OK;
}

static int
vtxt_disconnect( sqlite3_vtab * pVTab )
{
  /* disconnects the virtual table */
  VirtualTextPtr p_vt = ( VirtualTextPtr ) pVTab;
  if ( p_vt->buffer )
    text_buffer_free( p_vt->buffer );
  sqlite3_free( p_vt );
  return SQLITE_OK;
}

static int
vtxt_destroy( sqlite3_vtab * pVTab )
{
  /* destroys the virtual table - simply aliases vtxt_disconnect() */
  return vtxt_disconnect( pVTab );
}

static int
vtxt_open( sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor )
{
  /* opening a new cursor */
  VirtualTextCursorPtr cursor =
    ( VirtualTextCursorPtr ) sqlite3_malloc( sizeof( VirtualTextCursor ) );
  if ( cursor == NULL )
    return SQLITE_NOMEM;
  cursor->pVtab = ( VirtualTextPtr ) pVTab;
  cursor->current_row = 0;
  cursor->eof = 0;
  *ppCursor = ( sqlite3_vtab_cursor * ) cursor;
  if ( !( cursor->pVtab->buffer ) )
    cursor->eof = 1;
  return SQLITE_OK;
}

static int
vtxt_close( sqlite3_vtab_cursor * pCursor )
{
  /* closing the cursor */
  VirtualTextCursorPtr cursor = ( VirtualTextCursorPtr ) pCursor;
  sqlite3_free( pCursor );
  return SQLITE_OK;
}

static int
vtxt_filter( sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
             int argc, sqlite3_value ** argv )
{
  /* setting up a cursor filter */
  return SQLITE_OK;
}

static int
vtxt_next( sqlite3_vtab_cursor * pCursor )
{
  /* fetching next row from cursor */
  VirtualTextCursorPtr cursor = ( VirtualTextCursorPtr ) pCursor;
  if ( !( cursor->pVtab->buffer ) )
  {
    cursor->eof = 1;
    return SQLITE_OK;
  }
  cursor->current_row++;
  if ( cursor->current_row >= cursor->pVtab->buffer->n_rows )
    cursor->eof = 1;
  return SQLITE_OK;
}

static int
vtxt_eof( sqlite3_vtab_cursor * pCursor )
{
  /* cursor EOF */
  VirtualTextCursorPtr cursor = ( VirtualTextCursorPtr ) pCursor;
  return cursor->eof;
}

static int
vtxt_column( sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
             int column )
{
  /* fetching value for the Nth column */
  struct row_buffer *row;
  int nCol = 1;
  int i;
  VirtualTextCursorPtr cursor = ( VirtualTextCursorPtr ) pCursor;
  struct text_buffer *text = cursor->pVtab->buffer;
  if ( column == 0 )
  {
    /* the ROWNO column */
    sqlite3_result_int( pContext, cursor->current_row + 1 );
    return SQLITE_OK;
  }
  row = *( text->rows + cursor->current_row );
  for ( i = 0; i < text->max_n_cells; i++ )
  {
    if ( nCol == column )
    {
      if ( i >= row->n_cells )
        sqlite3_result_null( pContext );
      else
      {
        if ( *( row->cells + i ) )
        {
          if ( *( text->types + i ) == VRTTXT_INTEGER )
            sqlite3_result_int( pContext,
                                atoi( *( row->cells + i ) ) );
          else if ( *( text->types + i ) == VRTTXT_DOUBLE )
            sqlite3_result_double( pContext,
                                   atof( *
                                         ( row->cells + i ) ) );
          else
            sqlite3_result_text( pContext,
                                 *( row->cells + i ),
                                 strlen( *( row->cells + i ) ),
                                 SQLITE_STATIC );
        }
        else
          sqlite3_result_null( pContext );
      }
    }
    nCol++;
  }
  return SQLITE_OK;
}

static int
vtxt_rowid( sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid )
{
  /* fetching the ROWID */
  VirtualTextCursorPtr cursor = ( VirtualTextCursorPtr ) pCursor;
  *pRowid = cursor->current_row;
  return SQLITE_OK;
}

static int
vtxt_update( sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
             sqlite_int64 * pRowid )
{
  /* generic update [INSERT / UPDATE / DELETE */
  return SQLITE_READONLY;
}

static int
vtxt_begin( sqlite3_vtab * pVTab )
{
  /* BEGIN TRANSACTION */
  return SQLITE_OK;
}

static int
vtxt_sync( sqlite3_vtab * pVTab )
{
  /* BEGIN TRANSACTION */
  return SQLITE_OK;
}

static int
vtxt_commit( sqlite3_vtab * pVTab )
{
  /* BEGIN TRANSACTION */
  return SQLITE_OK;
}

static int
vtxt_rollback( sqlite3_vtab * pVTab )
{
  /* BEGIN TRANSACTION */
  return SQLITE_OK;
}

int
sqlite3VirtualTextInit( sqlite3 * db )
{
  int rc = SQLITE_OK;
  my_module.iVersion = 1;
  my_module.xCreate = &vtxt_create;
  my_module.xConnect = &vtxt_connect;
  my_module.xBestIndex = &vtxt_best_index;
  my_module.xDisconnect = &vtxt_disconnect;
  my_module.xDestroy = &vtxt_destroy;
  my_module.xOpen = &vtxt_open;
  my_module.xClose = &vtxt_close;
  my_module.xFilter = &vtxt_filter;
  my_module.xNext = &vtxt_next;
  my_module.xEof = &vtxt_eof;
  my_module.xColumn = &vtxt_column;
  my_module.xRowid = &vtxt_rowid;
  my_module.xUpdate = &vtxt_update;
  my_module.xBegin = &vtxt_begin;
  my_module.xSync = &vtxt_sync;
  my_module.xCommit = &vtxt_commit;
  my_module.xRollback = &vtxt_rollback;
  my_module.xFindFunction = NULL;
  sqlite3_create_module_v2( db, "VirtualText", &my_module, NULL, 0 );
  return rc;
}

int
virtualtext_extension_init( sqlite3 * db, const sqlite3_api_routines * pApi )
{
  SQLITE_EXTENSION_INIT2( pApi ) return sqlite3VirtualTextInit( db );
}
