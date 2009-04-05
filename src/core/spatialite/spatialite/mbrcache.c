/*

 mbrcache.c -- SQLite3 extension [MBR CACHE VIRTUAL TABLE]

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
#include <float.h>

#include <spatialite/sqlite3ext.h>
#include <spatialite/spatialite.h>
#include <spatialite/gaiageo.h>

#ifdef _MSC_VER
#define strcasecmp(a,b) _stricmp(a,b)
#endif

#define LONG64_MAX 9223372036854775807LL
#define LONG64_MIN (-LONG64_MAX-1)

static SQLITE_EXTENSION_INIT1 static struct sqlite3_module my_module;

/*

memory structs used to store the MBR's cache

the basic idea is to implement a hierarchy in order to avoid
excessive memory fragmentation and achieve better performance

- the cache is a linked-list of cache page elements
  - each cache page contains an array of 32 cache blocks
    - each cache block contains an array of 32 cache cells
so a single cache page con store up to 1024 cache cells

*/

struct mbr_cache_cell
{
  /*
  a  cached entity
  */

  /* the entity's ROWID */
  sqlite3_int64 rowid;
  /* the MBR */
  double minx;
  double miny;
  double maxx;
  double maxy;
};

struct mbr_cache_block
{
  /*
  a block of 32 cached entities
  */

  /*
  allocation bitmap: the meaning of each bit is:
  1 - corresponding cache cell is in use
  0 - corresponding cache cell is unused
  */
  unsigned int bitmap;
  /*
  the MBR corresponding to this cache block
  i.e. the combined MBR for any contained cell
  */
  double minx;
  double miny;
  double maxx;
  double maxy;
  /* the cache cells array */
  struct mbr_cache_cell cells[32];
};

struct mbr_cache_page
{
  /*
  a page containing 32 cached blocks
  */

  /*
  allocation bitmap: the meaning of each bit is:
  1 - corresponding cache block is in full
  0 - corresponding cache block is not full
  */
  unsigned int bitmap;
  /*
  the MBR corresponding to this cache page
  i.e. the combined MBR for any contained block
  */
  double minx;
  double miny;
  double maxx;
  double maxy;
  /* the cache blocks array */
  struct mbr_cache_block blocks[32];
  /* the min-max rowid for this page */
  sqlite3_int64 min_rowid;
  sqlite3_int64 max_rowid;
  /* pointer to next element into the cached pages linked list */
  struct mbr_cache_page *next;
};

struct mbr_cache
{
  /*
  the MBR's cache
  implemented as a cache pages linked list
  */

  /* pointers used to handle the cache pages linked list */
  struct mbr_cache_page *first;
  struct mbr_cache_page *last;
  /*
   pointer used to identify the current cache page when inserting a new cache cell
   */
  struct mbr_cache_page *current;
};

typedef struct MbrCacheStruct
{
  /* extends the sqlite3_vtab struct */
  const sqlite3_module *pModule; /* ptr to sqlite module: USED INTERNALLY BY SQLITE */
  int nRef;   /* # references: USED INTERNALLY BY SQLITE */
  char *zErrMsg;  /* error message: USE INTERNALLY BY SQLITE */
  sqlite3 *db;  /* the sqlite db holding the virtual table */
  struct mbr_cache *cache; /* the  MBR's cache */
  char *table_name;  /* the main table to be cached */
  char *column_name;  /* the column to be cached */
  int error;   /* some previous error disables any operation */
} MbrCache;
typedef MbrCache *MbrCachePtr;

typedef struct MbrCacheCursortStruct
{
  /* extends the sqlite3_vtab_cursor struct */
  MbrCachePtr pVtab;  /* Virtual table of this cursor */
  int eof;   /* the EOF marker */
  /*
  positioning parameters while performing a cache search
  */
  struct mbr_cache_page *current_page;
  int current_block_index;
  int current_cell_index;
  struct mbr_cache_cell *current_cell;
  /*
  the stategy to use:
      0 = sequential scan
      1 = find rowid
      2 = spatial search
  */
  int strategy;
  /* the MBR to search for */
  double minx;
  double miny;
  double maxx;
  double maxy;
  /*
  the MBR search mode:
      0 = WITHIN
      1 = CONTAIN
  */
  int mbr_mode;
} MbrCacheCursor;
typedef MbrCacheCursor *MbrCacheCursorPtr;

static unsigned int
cache_bitmask( int x )
{
  /* return the bitmask corresponding to index X */
  switch ( x )
  {
    case 0:
      return 0x80000000;
    case 1:
      return 0x40000000;
    case 2:
      return 0x20000000;
    case 3:
      return 0x10000000;
    case 4:
      return 0x08000000;
    case 5:
      return 0x04000000;
    case 6:
      return 0x02000000;
    case 7:
      return 0x01000000;
    case 8:
      return 0x00800000;
    case 9:
      return 0x00400000;
    case 10:
      return 0x00200000;
    case 11:
      return 0x00100000;
    case 12:
      return 0x00080000;
    case 13:
      return 0x00040000;
    case 14:
      return 0x00020000;
    case 15:
      return 0x00010000;
    case 16:
      return 0x00008000;
    case 17:
      return 0x00004000;
    case 18:
      return 0x00002000;
    case 19:
      return 0x00001000;
    case 20:
      return 0x00000800;
    case 21:
      return 0x00000400;
    case 22:
      return 0x00000200;
    case 23:
      return 0x00000100;
    case 24:
      return 0x00000080;
    case 25:
      return 0x00000040;
    case 26:
      return 0x00000020;
    case 27:
      return 0x00000010;
    case 28:
      return 0x00000008;
    case 29:
      return 0x00000004;
    case 30:
      return 0x00000002;
    case 31:
      return 0x00000001;
  };
  return 0x00000000;
}

static struct mbr_cache *
      cache_alloc()
{
  /* allocates and initializes an empty cache struct */
  struct mbr_cache *p = malloc( sizeof( struct mbr_cache ) );
  p->first = NULL;
  p->last = NULL;
  p->current = NULL;
  return p;
}

static struct mbr_cache_page *
      cache_page_alloc()
{
  /* allocates and initializes a cache page */
  int i;
  struct mbr_cache_block *pb;
  struct mbr_cache_page *p = malloc( sizeof( struct mbr_cache_page ) );
  p->bitmap = 0x00000000;
  p->next = NULL;
  p->minx = DBL_MAX;
  p->miny = DBL_MAX;
  p->maxx = DBL_MIN;
  p->maxy = DBL_MIN;
  for ( i = 0; i < 32; i++ )
  {
    pb = p->blocks + i;
    pb->bitmap = 0x00000000;
    pb->minx = DBL_MAX;
    pb->miny = DBL_MAX;
    pb->maxx = DBL_MIN;
    pb->maxy = DBL_MAX;
  }
  p->max_rowid = LONG64_MIN;
  p->min_rowid = LONG64_MAX;
  return p;
}

static void
cache_destroy( struct mbr_cache *p )
{
  /* memory cleanup; destroying a cache and any page into the cache */
  struct mbr_cache_page *pp;
  struct mbr_cache_page *ppn;
  if ( !p )
    return;
  pp = p->first;
  while ( pp )
  {
    ppn = pp->next;
    free( pp );
    pp = ppn;
  }
  free( p );
}

static int
cache_get_free_block( struct mbr_cache_page *pp )
{
  /* scans a cache page, returning the index of the first available block containing a free cell */
  int ib;
  for ( ib = 0; ib < 32; ib++ )
  {
    if (( pp->bitmap & cache_bitmask( ib ) ) == 0x00000000 )
      return ib;
  }
  return -1;
}

static void
cache_fix_page_bitmap( struct mbr_cache_page *pp )
{
  /* updating the cache page bitmap */
  int ib;
  for ( ib = 0; ib < 32; ib++ )
  {
    if ( pp->blocks[ib].bitmap == 0xffffffff )
    {
      /* all the cells into this block are used; marking the page bitmap */
      pp->bitmap |= cache_bitmask( ib );
    }
  }
}

static int
cache_get_free_cell( struct mbr_cache_block *pb )
{
  /* scans a cache block, returning the index of the first free cell */
  int ic;
  for ( ic = 0; ic < 32; ic++ )
  {
    if (( pb->bitmap & cache_bitmask( ic ) ) == 0x00000000 )
      return ic;
  }
  return -1;
}

static struct mbr_cache_page *
      cache_get_free_page( struct mbr_cache *p )
{
  /* return a pointer to the first cache page containing a free cell */
  struct mbr_cache_page *pp;
  if ( !( p->first ) )
  {
    /* the cache is empty; so we surely need to allocate the first page */
    pp = cache_page_alloc();
    p->first = pp;
    p->last = pp;
    p->current = pp;
    return pp;
  }
  if ( p->current )
  {
    /* checking if there is at least a free block into the current page */
    if ( p->current->bitmap != 0xffffffff )
      return p->current;
  }
  pp = p->first;
  while ( pp )
  {
    /* scanning the page list in order to discover if there is an exixsting page not yet completly filled */
    if ( pp->bitmap != 0xffffffff )
    {
      p->current = pp;
      return pp;
    }
    pp = pp->next;
  }
  /* we have to allocate a new page */
  pp = cache_page_alloc();
  p->last->next = pp;
  p->last = pp;
  p->current = pp;
  return pp;
}

static void
cache_insert_cell( struct mbr_cache *p, sqlite3_int64 rowid, double minx,
                   double miny, double maxx, double maxy )
{
  /* inserting a new cell */
  struct mbr_cache_page *pp = cache_get_free_page( p );
  int ib = cache_get_free_block( pp );
  struct mbr_cache_block *pb = pp->blocks + ib;
  int ic = cache_get_free_cell( pb );
  struct mbr_cache_cell *pc = pb->cells + ic;
  pc->rowid = rowid;
  pc->minx = minx;
  pc->miny = miny;
  pc->maxx = maxx;
  pc->maxy = maxy;
  /* marking the cache cell as used into the block bitmap */
  pb->bitmap |= cache_bitmask( ic );
  /* updating the cache block MBR */
  if ( pb->minx > minx )
    pb->minx = minx;
  if ( pb->maxx < maxx )
    pb->maxx = maxx;
  if ( pb->miny > miny )
    pb->miny = miny;
  if ( pb->maxy < maxy )
    pb->maxy = maxy;
  /* updading the cache page MBR */
  if ( pp->minx > minx )
    pp->minx = minx;
  if ( pp->maxx < maxx )
    pp->maxx = maxx;
  if ( pp->miny > miny )
    pp->miny = miny;
  if ( pp->maxy < maxy )
    pp->maxy = maxy;
  /* fixing the cache page bitmap */
  cache_fix_page_bitmap( pp );
  /* updating min-max rowid into the cache page */
  if ( pp->min_rowid > rowid )
    pp->min_rowid = rowid;
  if ( pp->max_rowid < rowid )
    pp->max_rowid = rowid;
}

static struct mbr_cache *
      cache_load( sqlite3 * handle, const char *table, const char *column )
{
  /*
  initial loading the MBR cache
  retrieving any existing entity from the main table
  */
  sqlite3_stmt *stmt;
  int ret;
  char sql[256];
  sqlite3_int64 rowid;
  double minx;
  double maxx;
  double miny;
  double maxy;
  int v1;
  int v2;
  int v3;
  int v4;
  int v5;
  struct mbr_cache *p_cache;
  sprintf( sql,
           "SELECT ROWID, MbrMinX(%s), MbrMinY(%s), MbrMaxX(%s), MbrMaxY(%s) FROM %s",
           column, column, column, column, table );
  ret = sqlite3_prepare_v2( handle, sql, strlen( sql ), &stmt, NULL );
  if ( ret != SQLITE_OK )
  {
    /* some error occurred */
    fprintf( stderr, "cache SQL error: %s\n", sqlite3_errmsg( handle ) );
    return NULL;
  }
  p_cache = cache_alloc();
  while ( 1 )
  {
    ret = sqlite3_step( stmt );
    if ( ret == SQLITE_DONE )
      break;
    if ( ret == SQLITE_ROW )
    {
      v1 = 0;
      v2 = 0;
      v3 = 0;
      v4 = 0;
      v5 = 0;
      if ( sqlite3_column_type( stmt, 0 ) == SQLITE_INTEGER )
        v1 = 1;
      if ( sqlite3_column_type( stmt, 1 ) == SQLITE_FLOAT )
        v2 = 1;
      if ( sqlite3_column_type( stmt, 1 ) == SQLITE_FLOAT )
        v3 = 1;
      if ( sqlite3_column_type( stmt, 1 ) == SQLITE_FLOAT )
        v4 = 1;
      if ( sqlite3_column_type( stmt, 1 ) == SQLITE_FLOAT )
        v5 = 1;
      if ( v1 && v2 && v3 && v4 && v5 )
      {
        /* ok, this entity is a valid one; inserting them into the MBR's cache */
        rowid = sqlite3_column_int( stmt, 0 );
        minx = sqlite3_column_double( stmt, 1 );
        miny = sqlite3_column_double( stmt, 2 );
        maxx = sqlite3_column_double( stmt, 3 );
        maxy = sqlite3_column_double( stmt, 4 );
        cache_insert_cell( p_cache, rowid, minx, miny, maxx,
                           maxy );
      }
    }
    else
    {
      /* some unexpected error occurred */
      printf( "sqlite3_step() error: %s\n", sqlite3_errmsg( handle ) );
      sqlite3_finalize( stmt );
      cache_destroy( p_cache );
      return NULL;
    }
  }
  /* we have now to finalize the query [memory cleanup] */
  sqlite3_finalize( stmt );
  return p_cache;
}

static int
cache_find_next_cell( struct mbr_cache_page **page, int *i_block, int *i_cell,
                      struct mbr_cache_cell **cell )
{
  /* finding next cached cell */
  struct mbr_cache_page *pp = *page;
  struct mbr_cache_block *pb;
  struct mbr_cache_cell *pc;
  int ib;
  int ic;
  int sib = *i_block;
  int sic = *i_cell;
  while ( pp )
  {
    for ( ib = sib; ib < 32; ib++ )
    {
      pb = pp->blocks + ib;
      for ( ic = sic; ic < 32; ic++ )
      {
        if (( pb->bitmap & cache_bitmask( ic ) ) == 0x00000000 )
          continue;
        pc = pb->cells + ic;
        if ( pc == *cell )
        {
          /* this one is the current cell */
          continue;
        }
        /* next cell found */
        *page = pp;
        *i_block = ib;
        *i_cell = ic;
        *cell = pc;
        return 1;
      }
      sic = 0;
    }
    sib = 0;
    pp = pp->next;
  }
  return 0;
}

static int
cache_find_next_mbr( struct mbr_cache_page **page, int *i_block, int *i_cell,
                     struct mbr_cache_cell **cell, double minx, double miny,
                     double maxx, double maxy, int mode )
{
  /* finding next cached cell */
  struct mbr_cache_page *pp = *page;
  struct mbr_cache_block *pb;
  struct mbr_cache_cell *pc;
  int ib;
  int ic;
  int sib = *i_block;
  int sic = *i_cell;
  int ok_mbr;
  while ( pp )
  {
    ok_mbr = 0;
    if ( pp->maxx >= minx && pp->minx <= maxx &&
         pp->maxy >= miny && pp->miny <= maxy )
      ok_mbr = 1;
    if ( ok_mbr )
    {
      for ( ib = sib; ib < 32; ib++ )
      {
        pb = pp->blocks + ib;
        ok_mbr = 0;
        if ( pb->maxx >= minx && pb->minx <= maxx &&
             pb->maxy >= miny && pb->miny <= maxy )
          ok_mbr = 1;
        if ( ok_mbr )
        {
          for ( ic = sic; ic < 32; ic++ )
          {
            if (( pb->bitmap & cache_bitmask( ic ) ) ==
                0x00000000 )
              continue;
            pc = pb->cells + ic;
            ok_mbr = 0;
            if ( mode == GAIA_FILTER_MBR_INTERSECTS )
            {
              /* MBR INTERSECTS */
              if ( pc->maxx >= minx && pc->minx <= maxx
                   && pc->maxy >= miny
                   && pc->miny <= maxy )
                ok_mbr = 1;
            }
            else if ( mode == GAIA_FILTER_MBR_CONTAINS )
            {
              /* MBR CONTAINS */
              if ( minx >= pc->minx && maxx <= pc->maxx
                   && miny >= pc->miny
                   && maxy <= pc->maxy )
                ok_mbr = 1;
            }
            else
            {
              /* MBR WITHIN */
              if ( pc->minx >= minx && pc->maxx <= maxx
                   && pc->miny >= miny
                   && pc->maxy <= maxy )
                ok_mbr = 1;
            }
            if ( ok_mbr )
            {
              if ( pc == *cell )
              {
                /* this one is the current cell */
                continue;
              }
              /* next cell found */
              *page = pp;
              *i_block = ib;
              *i_cell = ic;
              *cell = pc;
              return 1;
            }
          }
        }
        sic = 0;
      }
    }
    sib = 0;
    pp = pp->next;
  }
  return 0;
}

static struct mbr_cache_cell *
      cache_find_by_rowid( struct mbr_cache_page *pp, sqlite3_int64 rowid )
{
  /* trying to find a row by rowid from the Mbr cache */
  struct mbr_cache_block *pb;
  struct mbr_cache_cell *pc;
  int ib;
  int ic;
  while ( pp )
  {
    if ( rowid >= pp->min_rowid && rowid <= pp->max_rowid )
    {
      for ( ib = 0; ib < 32; ib++ )
      {
        pb = pp->blocks + ib;
        for ( ic = 0; ic < 32; ic++ )
        {
          if (( pb->bitmap & cache_bitmask( ic ) ) == 0x00000000 )
            continue;
          pc = pb->cells + ic;
          if ( pc->rowid == rowid )
            return pc;
        }
      }
    }
    pp = pp->next;
  }
  return 0;
}

static void
cache_update_page( struct mbr_cache_page *pp, int i_block )
{
  /* updating the cache block and cache page MBR after a DELETE or UPDATE occurred */
  struct mbr_cache_block *pb;
  struct mbr_cache_cell *pc;
  int ib;
  int ic;
  /* updating the cache block MBR */
  pb = pp->blocks + i_block;
  pb->minx = DBL_MAX;
  pb->miny = DBL_MAX;
  pb->maxx = DBL_MIN;
  pb->maxy = DBL_MIN;
  for ( ic = 0; ic < 32; ic++ )
  {
    if (( pb->bitmap & cache_bitmask( ic ) ) == 0x00000000 )
      continue;
    pc = pb->cells + ic;
    if ( pb->minx > pc->minx )
      pb->minx = pc->minx;
    if ( pb->miny > pc->miny )
      pb->miny = pc->miny;
    if ( pb->maxx < pc->maxx )
      pb->maxx = pc->maxx;
    if ( pb->maxy < pc->maxy )
      pb->maxy = pc->maxy;
  }
  /* updating the cache page MBR */
  pp->minx = DBL_MAX;
  pp->miny = DBL_MAX;
  pp->maxx = DBL_MIN;
  pp->maxy = DBL_MIN;
  pp->min_rowid = LONG64_MAX;
  pp->max_rowid = LONG64_MIN;
  for ( ib = 0; ib < 32; ib++ )
  {
    pb = pp->blocks + ib;
    for ( ic = 0; ic < 32; ic++ )
    {
      if (( pb->bitmap & cache_bitmask( ic ) ) == 0x00000000 )
        continue;
      pc = pb->cells + ic;
      if ( pp->minx > pc->minx )
        pp->minx = pc->minx;
      if ( pp->miny > pc->miny )
        pp->miny = pc->miny;
      if ( pp->maxx < pc->maxx )
        pp->maxx = pc->maxx;
      if ( pp->maxy < pc->maxy )
        pp->maxy = pc->maxy;
      if ( pp->min_rowid > pc->rowid )
        pp->min_rowid = pc->rowid;
      if ( pp->max_rowid < pc->rowid )
        pp->max_rowid = pc->rowid;
    }
  }
}

static int
cache_delete_cell( struct mbr_cache_page *pp, sqlite3_int64 rowid )
{
  /* trying to delete a row identified by rowid from the Mbr cache */
  struct mbr_cache_block *pb;
  struct mbr_cache_cell *pc;
  int ib;
  int ic;
  while ( pp )
  {
    if ( rowid >= pp->min_rowid && rowid <= pp->max_rowid )
    {
      for ( ib = 0; ib < 32; ib++ )
      {
        pb = pp->blocks + ib;
        for ( ic = 0; ic < 32; ic++ )
        {
          if (( pb->bitmap & cache_bitmask( ic ) ) == 0x00000000 )
            continue;
          pc = pb->cells + ic;
          if ( pc->rowid == rowid )
          {
            /* marking the cell as free */
            pb->bitmap &= ~( cache_bitmask( ic ) );
            /* marking the block as not full */
            pp->bitmap &= ~( cache_bitmask( ib ) );
            /* updating the cache block and cache page MBR */
            cache_update_page( pp, ib );
            return 1;
          }
        }
      }
    }
    pp = pp->next;
  }
  return 0;
}

static int
cache_update_cell( struct mbr_cache_page *pp, sqlite3_int64 rowid, double minx,
                   double miny, double maxx, double maxy )
{
  /* trying to update a row identified by rowid from the Mbr cache */
  struct mbr_cache_block *pb;
  struct mbr_cache_cell *pc;
  int ib;
  int ic;
  while ( pp )
  {
    if ( rowid >= pp->min_rowid && rowid <= pp->max_rowid )
    {
      for ( ib = 0; ib < 32; ib++ )
      {
        pb = pp->blocks + ib;
        for ( ic = 0; ic < 32; ic++ )
        {
          if (( pb->bitmap & cache_bitmask( ic ) ) == 0x00000000 )
            continue;
          pc = pb->cells + ic;
          if ( pc->rowid == rowid )
          {
            /* updating the cell MBR */
            pc->minx = minx;
            pc->miny = miny;
            pc->maxx = maxx;
            pc->maxy = maxy;
            /* updating the cache block and cache page MBR */
            cache_update_page( pp, ib );
            return 1;
          }
        }
      }
    }
    pp = pp->next;
  }
  return 0;
}

static int
mbrc_create( sqlite3 * db, void *pAux, int argc, const char *const *argv,
             sqlite3_vtab ** ppVTab, char **pzErr )
{
  /* creates the virtual table and caches related Geometry column */
  int err;
  int ret;
  int i;
  int len;
  int n_rows;
  int n_columns;
  const char *vtable;
  const char *table;
  const char *column;
  const char *col_name;
  char **results;
  char *err_msg = NULL;
  char sql[4096];
  int ok_tbl;
  int ok_col;
  MbrCachePtr p_vt;
  p_vt = ( MbrCachePtr ) sqlite3_malloc( sizeof( MbrCache ) );
  if ( !p_vt )
    return SQLITE_NOMEM;
  *ppVTab = ( sqlite3_vtab * ) p_vt;
  p_vt->pModule = &my_module;
  p_vt->nRef = 0;
  p_vt->zErrMsg = NULL;
  p_vt->db = db;
  p_vt->table_name = NULL;
  p_vt->column_name = NULL;
  p_vt->cache = NULL;
  /* checking for table_name and geo_column_name */
  if ( argc == 5 )
  {
    vtable = argv[2];
    table = argv[3];
    column = argv[4];
    len = strlen( table );
    p_vt->table_name = sqlite3_malloc( len + 1 );
    strcpy( p_vt->table_name, table );
    len = strlen( column );
    p_vt->column_name = sqlite3_malloc( len + 1 );
    strcpy( p_vt->column_name, column );
  }
  else
  {
    *pzErr =
      sqlite3_mprintf
      ( "[MbrCache module] CREATE VIRTUAL: illegal arg list {table_name, geo_column_name}" );
    return SQLITE_ERROR;
  }
  /* retrieving the base table columns */
  err = 0;
  ok_tbl = 0;
  ok_col = 0;
  sprintf( sql, "PRAGMA table_info(%s)", table );
  ret = sqlite3_get_table( db, sql, &results, &n_rows, &n_columns, &err_msg );
  if ( ret != SQLITE_OK )
  {
    err = 1;
    goto illegal;
  }
  if ( n_rows > 1 )
  {
    ok_tbl = 1;
    for ( i = 1; i <= n_rows; i++ )
    {
      col_name = results[( i * n_columns ) + 1];
      if ( strcasecmp( col_name, column ) == 0 )
        ok_col = 1;
    }
    sqlite3_free_table( results );
    if ( !ok_col )
      err = 1;
  }
  else
    err = 1;
illegal:
  if ( err )
  {
    /* something is going the wrong way; creating a stupid default table */
    sprintf( sql, "CREATE TABLE %s (rowid INTEGER, mbr BLOB)", vtable );
    if ( sqlite3_declare_vtab( db, sql ) != SQLITE_OK )
    {
      *pzErr =
        sqlite3_mprintf
        ( "[VirtualText module] cannot build a table from TEXT file\n" );
      return SQLITE_ERROR;
    }
    p_vt->error = 1;
    *ppVTab = ( sqlite3_vtab * ) p_vt;
    return SQLITE_OK;
  }
  p_vt->error = 0;
  sprintf( sql, "CREATE TABLE %s (", vtable );
  strcat( sql, "rowid INTEGER, mbr BLOB)" );
  if ( sqlite3_declare_vtab( db, sql ) != SQLITE_OK )
  {
    *pzErr =
      sqlite3_mprintf
      ( "[MbrCache module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
        sql );
    return SQLITE_ERROR;
  }
  *ppVTab = ( sqlite3_vtab * ) p_vt;
  return SQLITE_OK;
}

static int
mbrc_connect( sqlite3 * db, void *pAux, int argc, const char *const *argv,
              sqlite3_vtab ** ppVTab, char **pzErr )
{
  /* connects the virtual table - simply aliases mbrc_create() */
  return mbrc_create( db, pAux, argc, argv, ppVTab, pzErr );
}

static int
mbrc_best_index( sqlite3_vtab * pVTab, sqlite3_index_info * pIdxInfo )
{
  /* best index selection */
  int i;
  int err = 1;
  int errors = 0;
  int mbr = 0;
  int rowid = 0;
  for ( i = 0; i < pIdxInfo->nConstraint; i++ )
  {
    /* verifying the constraints */
    struct sqlite3_index_constraint *p = &( pIdxInfo->aConstraint[i] );
    if ( p->usable )
    {
      if ( p->iColumn == 0 && p->op == SQLITE_INDEX_CONSTRAINT_EQ )
        rowid++;
      else if ( p->iColumn == 1 && p->op == SQLITE_INDEX_CONSTRAINT_EQ )
        mbr++;
      else
        errors++;
    }
  }
  if ( mbr == 1 && rowid == 0 && errors == 0 )
  {
    /* this one is a valid spatially-filtered query */
    pIdxInfo->idxNum = 2;
    for ( i = 0; i < pIdxInfo->nConstraint; i++ )
    {
      pIdxInfo->aConstraintUsage[i].argvIndex = 1;
      pIdxInfo->aConstraintUsage[i].omit = 1;
    }
    err = 0;
  }
  if ( mbr == 0 && rowid == 1 && errors == 0 )
  {
    /* this one is a valid rowid-filtered query */
    pIdxInfo->idxNum = 1;
    pIdxInfo->estimatedCost = 1.0;
    for ( i = 0; i < pIdxInfo->nConstraint; i++ )
    {
      if ( pIdxInfo->aConstraint[i].usable )
      {
        pIdxInfo->aConstraintUsage[i].argvIndex = 1;
        pIdxInfo->aConstraintUsage[i].omit = 1;
      }
    }
    err = 0;
  }
  if ( mbr == 0 && rowid == 0 && errors == 0 )
  {
    /* this one is a valid unfiltered query */
    pIdxInfo->idxNum = 0;
    err = 0;
  }
  if ( err )
  {
    /* illegal query */
    pIdxInfo->idxNum = -1;
  }
  return SQLITE_OK;
}

static int
mbrc_disconnect( sqlite3_vtab * pVTab )
{
  /* disconnects the virtual table */
  MbrCachePtr p_vt = ( MbrCachePtr ) pVTab;
  if ( p_vt->cache )
    cache_destroy( p_vt->cache );
  if ( p_vt->table_name )
    sqlite3_free( p_vt->table_name );
  if ( p_vt->column_name )
    sqlite3_free( p_vt->column_name );
  sqlite3_free( p_vt );
  return SQLITE_OK;
}

static int
mbrc_destroy( sqlite3_vtab * pVTab )
{
  /* destroys the virtual table - simply aliases mbrc_disconnect() */
  return mbrc_disconnect( pVTab );
}

static void
mbrc_read_row_unfiltered( MbrCacheCursorPtr cursor )
{
  /* trying to read the next row from the Mbr cache - unfiltered mode */
  struct mbr_cache_page *page = cursor->current_page;
  struct mbr_cache_cell *cell = cursor->current_cell;
  int i_block = cursor->current_block_index;
  int i_cell = cursor->current_cell_index;
  if ( cache_find_next_cell( &page, &i_block, &i_cell, &cell ) )
  {
    cursor->current_page = page;
    cursor->current_block_index = i_block;
    cursor->current_cell_index = i_cell;
    cursor->current_cell = cell;
  }
  else
    cursor->eof = 1;
}

static void
mbrc_read_row_filtered( MbrCacheCursorPtr cursor )
{
  /* trying to read the next row from the Mbr cache - spatially filter mode */
  struct mbr_cache_page *page = cursor->current_page;
  struct mbr_cache_cell *cell = cursor->current_cell;
  int i_block = cursor->current_block_index;
  int i_cell = cursor->current_cell_index;
  if ( cache_find_next_mbr
       ( &page, &i_block, &i_cell, &cell, cursor->minx, cursor->miny,
         cursor->maxx, cursor->maxy, cursor->mbr_mode ) )
  {
    cursor->current_page = page;
    cursor->current_block_index = i_block;
    cursor->current_cell_index = i_cell;
    cursor->current_cell = cell;
  }
  else
    cursor->eof = 1;
}

static void
mbrc_read_row_by_rowid( MbrCacheCursorPtr cursor, sqlite3_int64 rowid )
{
  /* trying to find a row by rowid from the Mbr cache */
  struct mbr_cache_cell *cell =
          cache_find_by_rowid( cursor->pVtab->cache->first, rowid );
  if ( cell )
    cursor->current_cell = cell;
  else
{
    cursor->current_cell = NULL;
    cursor->eof = 1;
  }
}

static int
mbrc_open( sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor )
{
  /* opening a new cursor */
  MbrCachePtr p_vt = ( MbrCachePtr ) pVTab;
  MbrCacheCursorPtr cursor =
    ( MbrCacheCursorPtr ) sqlite3_malloc( sizeof( MbrCacheCursor ) );
  if ( cursor == NULL )
    return SQLITE_ERROR;
  cursor->pVtab = p_vt;
  if ( p_vt->error )
  {
    cursor->eof = 1;
    *ppCursor = ( sqlite3_vtab_cursor * ) cursor;
    return SQLITE_OK;
  }
  if ( !( p_vt->cache ) )
    p_vt->cache =
      cache_load( p_vt->db, p_vt->table_name, p_vt->column_name );
  cursor->current_page = cursor->pVtab->cache->first;
  cursor->current_block_index = 0;
  cursor->current_cell_index = 0;
  cursor->current_cell = NULL;
  cursor->eof = 0;
  *ppCursor = ( sqlite3_vtab_cursor * ) cursor;
  return SQLITE_OK;
}

static int
mbrc_close( sqlite3_vtab_cursor * pCursor )
{
  /* closing the cursor */
  sqlite3_free( pCursor );
  return SQLITE_OK;
}

static int
mbrc_filter( sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
             int argc, sqlite3_value ** argv )
{
  /* setting up a cursor filter */
  MbrCacheCursorPtr cursor = ( MbrCacheCursorPtr ) pCursor;
  if ( cursor->pVtab->error )
  {
    cursor->eof = 1;
    return SQLITE_OK;
  }
  cursor->current_page = cursor->pVtab->cache->first;
  cursor->current_block_index = 0;
  cursor->current_cell_index = 0;
  cursor->current_cell = NULL;
  cursor->eof = 0;
  cursor->strategy = idxNum;
  if ( idxNum == 0 )
  {
    /* unfiltered mode */
    mbrc_read_row_unfiltered( cursor );
    return SQLITE_OK;
  }
  if ( idxNum == 1 )
  {
    /* filtering by ROWID */
    sqlite3_int64 rowid = sqlite3_value_int64( argv[0] );
    mbrc_read_row_by_rowid( cursor, rowid );
    return SQLITE_OK;
  }
  if ( idxNum == 2 )
  {
    /* filtering by MBR spatial relation */
    unsigned char *p_blob;
    int n_bytes;
    double minx;
    double miny;
    double maxx;
    double maxy;
    int mode;
    if ( sqlite3_value_type( argv[0] ) != SQLITE_BLOB )
      cursor->eof = 1;
    else
    {
      p_blob = ( unsigned char * ) sqlite3_value_blob( argv[0] );
      n_bytes = sqlite3_value_bytes( argv[0] );
      if ( gaiaParseFilterMbr
           ( p_blob, n_bytes, &minx, &miny, &maxx, &maxy, &mode ) )
      {
        if ( mode == GAIA_FILTER_MBR_WITHIN
             || mode == GAIA_FILTER_MBR_CONTAINS
             || mode == GAIA_FILTER_MBR_INTERSECTS )
        {
          cursor->minx = minx;
          cursor->miny = miny;
          cursor->maxx = maxx;
          cursor->maxy = maxy;
          cursor->mbr_mode = mode;
          mbrc_read_row_filtered( cursor );
        }
        else
          cursor->eof = 1;
      }
    }
    return SQLITE_OK;
  }
  /* illegal query mode */
  cursor->eof = 1;
  return SQLITE_OK;
}

static int
mbrc_next( sqlite3_vtab_cursor * pCursor )
{
  /* fetching a next row from cursor */
  MbrCacheCursorPtr cursor = ( MbrCacheCursorPtr ) pCursor;
  if ( cursor->pVtab->error )
  {
    cursor->eof = 1;
    return SQLITE_OK;
  }
  if ( cursor->strategy == 0 )
    mbrc_read_row_unfiltered( cursor );
  else if ( cursor->strategy == 2 )
    mbrc_read_row_filtered( cursor );
  else
    cursor->eof = 1;
  return SQLITE_OK;
}

static int
mbrc_eof( sqlite3_vtab_cursor * pCursor )
{
  /* cursor EOF */
  MbrCacheCursorPtr cursor = ( MbrCacheCursorPtr ) pCursor;
  return cursor->eof;
}

static int
mbrc_column( sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
             int column )
{
  /* fetching value for the Nth column */
  MbrCacheCursorPtr cursor = ( MbrCacheCursorPtr ) pCursor;
  if ( !( cursor->current_cell ) )
    sqlite3_result_null( pContext );
  else
  {
    if ( column == 0 )
    {
      /* the PRIMARY KEY column */
      sqlite3_result_int( pContext, cursor->current_cell->rowid );
    }
    if ( column == 1 )
    {
      /* the MBR column */
      char envelope[1024];
      sprintf( envelope,
               "POLYGON((%1.2lf %1.2lf, %1.2lf %1.2lf, %1.2lf %1.2lf, %1.2lf %1.2lf, %1.2lf %1.2lf))",
               cursor->current_cell->minx, cursor->current_cell->miny,
               cursor->current_cell->maxx, cursor->current_cell->miny,
               cursor->current_cell->maxx, cursor->current_cell->maxy,
               cursor->current_cell->minx, cursor->current_cell->maxy,
               cursor->current_cell->minx,
               cursor->current_cell->miny );
      sqlite3_result_text( pContext, envelope, strlen( envelope ),
                           SQLITE_TRANSIENT );
    }
  }
  return SQLITE_OK;
}

static int
mbrc_rowid( sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid )
{
  /* fetching the ROWID */
  MbrCacheCursorPtr cursor = ( MbrCacheCursorPtr ) pCursor;
  *pRowid = cursor->current_cell->rowid;
  return SQLITE_OK;
}

static int
mbrc_update( sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
             sqlite_int64 * pRowid )
{
  /* generic update [INSERT / UPDATE / DELETE */
  sqlite3_int64 rowid;
  unsigned char *p_blob;
  int n_bytes;
  double minx;
  double miny;
  double maxx;
  double maxy;
  int mode;
  int illegal = 0;
  MbrCachePtr p_vtab = ( MbrCachePtr ) pVTab;
  if ( p_vtab->error )
    return SQLITE_OK;
  if ( !( p_vtab->cache ) )
    p_vtab->cache =
      cache_load( p_vtab->db, p_vtab->table_name, p_vtab->column_name );
  if ( argc == 1 )
  {
    /* performing a DELETE */
    if ( sqlite3_value_type( argv[0] ) == SQLITE_INTEGER )
    {
      rowid = sqlite3_value_int64( argv[0] );
      cache_delete_cell( p_vtab->cache->first, rowid );
    }
    else
      illegal = 1;
  }
  else
  {
    if ( sqlite3_value_type( argv[0] ) == SQLITE_NULL )
    {
      /* performing an INSERT */
      if ( argc == 4 )
      {
        if ( sqlite3_value_type( argv[2] ) == SQLITE_INTEGER &&
             sqlite3_value_type( argv[3] ) == SQLITE_BLOB )
        {
          rowid = sqlite3_value_int64( argv[2] );
          p_blob =
            ( unsigned char * ) sqlite3_value_blob( argv[3] );
          n_bytes = sqlite3_value_bytes( argv[3] );
          if ( gaiaParseFilterMbr
               ( p_blob, n_bytes, &minx, &miny, &maxx, &maxy,
                 &mode ) )
          {
            if ( mode == GAIA_FILTER_MBR_DECLARE )
            {
              if ( !cache_find_by_rowid
                   ( p_vtab->cache->first, rowid ) )
                cache_insert_cell( p_vtab->cache,
                                   rowid, minx,
                                   miny, maxx,
                                   maxy );
            }
            else
              illegal = 1;
          }
          else
            illegal = 1;
        }
        else
          illegal = 1;
      }
      else
        illegal = 1;
    }
    else
    {
      /* performing an UPDATE */
      if ( argc == 4 )
      {
        if ( sqlite3_value_type( argv[0] ) == SQLITE_INTEGER &&
             sqlite3_value_type( argv[3] ) == SQLITE_BLOB )
        {
          rowid = sqlite3_value_int64( argv[0] );
          p_blob =
            ( unsigned char * ) sqlite3_value_blob( argv[3] );
          n_bytes = sqlite3_value_bytes( argv[3] );
          if ( gaiaParseFilterMbr
               ( p_blob, n_bytes, &minx, &miny, &maxx, &maxy,
                 &mode ) )
          {
            if ( mode == GAIA_FILTER_MBR_DECLARE )
              cache_update_cell( p_vtab->cache->first,
                                 rowid, minx, miny,
                                 maxx, maxy );
            else
              illegal = 1;
          }
          else
            illegal = 1;
        }
        else
          illegal = 1;
      }
      else
        illegal = 1;
    }
  }
  if ( illegal )
    return SQLITE_MISMATCH;
  return SQLITE_OK;
}

static int
mbrc_begin( sqlite3_vtab * pVTab )
{
  /* BEGIN TRANSACTION */
  return SQLITE_OK;
}

static int
mbrc_sync( sqlite3_vtab * pVTab )
{
  /* BEGIN TRANSACTION */
  return SQLITE_OK;
}

static int
mbrc_commit( sqlite3_vtab * pVTab )
{
  /* BEGIN TRANSACTION */
  return SQLITE_OK;
}

static int
mbrc_rollback( sqlite3_vtab * pVTab )
{
  /* BEGIN TRANSACTION */
  return SQLITE_OK;
}

int
sqlite3MbrCacheInit( sqlite3 * db )
{
  int rc = SQLITE_OK;
  my_module.iVersion = 1;
  my_module.xCreate = &mbrc_create;
  my_module.xConnect = &mbrc_connect;
  my_module.xBestIndex = &mbrc_best_index;
  my_module.xDisconnect = &mbrc_disconnect;
  my_module.xDestroy = &mbrc_destroy;
  my_module.xOpen = &mbrc_open;
  my_module.xClose = &mbrc_close;
  my_module.xFilter = &mbrc_filter;
  my_module.xNext = &mbrc_next;
  my_module.xEof = &mbrc_eof;
  my_module.xColumn = &mbrc_column;
  my_module.xRowid = &mbrc_rowid;
  my_module.xUpdate = &mbrc_update;
  my_module.xBegin = &mbrc_begin;
  my_module.xSync = &mbrc_sync;
  my_module.xCommit = &mbrc_commit;
  my_module.xRollback = &mbrc_rollback;
  my_module.xFindFunction = NULL;
  sqlite3_create_module_v2( db, "MbrCache", &my_module, NULL, 0 );
  return rc;
}

int
mbrcache_extension_init( sqlite3 * db, const sqlite3_api_routines * pApi )
{
  SQLITE_EXTENSION_INIT2( pApi ) return sqlite3MbrCacheInit( db );
}
