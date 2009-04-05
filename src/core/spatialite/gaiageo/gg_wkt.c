/*

 gg_wkt.c -- Gaia common support for WKT encoded geometries

 version 2.3, 2008 October 13

 Author: Sandro Furieri a.furieri@lqt.it

 ------------------------------------------------------------------------------

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
Klaus Foerster klaus.foerster@svg.cc

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
#include <spatialite/gaiageo.h>

#ifdef _MSC_VER
#define strcasecmp(a,b) stricmp(a,b)
#define strncasecmp(a,b,n) _strnicmp(a,b,n)
#endif

typedef struct gaiaTokenStruct
{
  /* linked list of pre-parsed tokens - used in WKT parsing */
  int type;   /* token code */
  double coord;  /* a coordinate [if any] */
  struct gaiaTokenStruct *next; /* reference to next element in linked list */
} gaiaToken;
typedef gaiaToken *gaiaTokenPtr;

typedef struct gaiaListTokenStruct
{
  /*
   / a block of consecutive WKT tokens that identify a LINESTRING or RING,
   / in the form: (1 2, 3 4, 5 6, 7 8)
   */
  gaiaTokenPtr first;  /* reference to first element in linked list - has always to be a '(' */
  gaiaTokenPtr last;  /* reference to last element in linked list - has always to be a ')' */
  int points;   /* number of POINTS contained in the linkek list */
  gaiaLinestringPtr line; /* the LINESTRING builded after successful parsing */
  struct gaiaListTokenStruct *next; /* points to next element in linked list [if any] */
} gaiaListToken;
typedef gaiaListToken *gaiaListTokenPtr;

typedef struct gaiaMultiListTokenStruct
{
  /*
   / a group of the above token lists, that identify a POLYGON or a MULTILINESTRING
   / in the form: ((...),(...),(...))
   */
  gaiaListTokenPtr first; /* reference to first element in linked list - has always to be a '(' */
  gaiaListTokenPtr last; /* reference to last element in linked list - has always to be a ')' */
  struct gaiaMultiListTokenStruct *next; /* points to next element in linked list [if any] */
} gaiaMultiListToken;
typedef gaiaMultiListToken *gaiaMultiListTokenPtr;

typedef struct gaiaMultiMultiListTokenStruct
{
  /*
   / a group of the above token multi-lists, that identify a MULTIPOLYGON
   / in the form: (((...),(...),(...)),((...),(...)),((...)))
   */
  gaiaMultiListTokenPtr first; /* reference to first element in linked list - has always to be a '(' */
  gaiaMultiListTokenPtr last; /* reference to last element in linked list - has always to be a ')' */
} gaiaMultiMultiListToken;
typedef gaiaMultiMultiListToken *gaiaMultiMultiListTokenPtr;

typedef struct gaiaVarListTokenStruct
{
  /*
   / a multitype variable reference that may be associated to:
   / - a POINT element
   / - a LINESTRING element
   / - a POLYGON element
  */
  int type;   /* may be GAIA_POINT, GAIA_LINESTRING or GAIA_POLYGON */
  void *pointer;  /*
       / to be casted as *sple_multi_list_token_ptr* if type is GAIA_LINESTRING/ or as *gaiaMultiMultiListTokenPtr* if type is GAIA_POLYGON
     */
  double x;   /* X,Y are valids only if type is GAIA_POINT */
  double y;
  struct gaiaVarListTokenStruct *next; /* points to next element in linked list */
} gaiaVarListToken;
typedef gaiaVarListToken *gaiaVarListTokenPtr;

typedef struct gaiaGeomCollListTokenStruct
{
  /*
  / a group of lists and multi-lists that identifies a GEOMETRYCOLLECTION
  / in the form: (ELEM(...),ELEM(...),ELEM(....))
  */
  gaiaVarListTokenPtr first; /* reference to first element in linked list - has always to be a '(' */
  gaiaVarListTokenPtr last; /* reference to last element in linked list - has always to be a ')' */
} gaiaGeomCollListToken;
typedef gaiaGeomCollListToken *gaiaGeomCollListTokenPtr;

static char *
gaiaCleanWkt( const unsigned char *old )
{
  /* cleans and normalizes the WKT encoded string before effective parsing */
  int len;
  char *buf;
  char *pn;
  int error = 0;
  int space = 1;
  int opened = 0;
  int closed = 0;
  int ok;
  const unsigned char *po;
  len = strlen(( char * ) old );
  if ( len == 0 )
    return NULL;
  buf = malloc( len + 1 );
  pn = buf;
  po = old;
  while ( *po != '\0' )
  {
    if ( *po >= '0' && *po <= '9' )
    {
      *pn = *po;
      pn++;
      space = 0;
    }
    else if ( *po == '+' || *po == '-' )
    {
      *pn = *po;
      pn++;
      space = 0;
    }
    else if (( *po >= 'A' && *po <= 'Z' ) ||
             ( *po >= 'a' && *po <= 'z' ) ||
             *po == ',' || *po == '.' || *po == '(' || *po == ')' )
    {
      if ( pn > buf )
      {
        if ( *( pn - 1 ) == ' ' )
          *( pn - 1 ) = *po;
        else
        {
          *pn = *po;
          pn++;
        }
      }
      else
      {
        *pn = *po;
        pn++;
      }
      if ( *po == '(' )
        opened++;
      if ( *po == ')' )
        closed++;
      space = 1;
    }
    else if ( *po == ' ' || *po == '\t' || *po == '\n' || *po == '\r' )
    {
      if ( !space )
      {
        *pn = ' ';
        pn++;
      }
      space = 1;
    }
    else
    {
      error = 1;
      break;
    }
    po++;
  }
  if ( opened != closed )
    error = 1;
  *pn = '\0';
  len = strlen( buf );
  if ( buf[len - 1] != ')' )
    error = 1;
  ok = 0;
  if ( !error )
  {
    if ( len > 6 && strncasecmp( buf, "POINT(", 6 ) == 0 )
      ok = 1;
    if ( len > 11 && strncasecmp( buf, "LINESTRING(", 11 ) == 0 )
      ok = 1;
    if ( len > 8 && strncasecmp( buf, "POLYGON(", 8 ) == 0 )
      ok = 1;
    if ( len > 11 && strncasecmp( buf, "MULTIPOINT(", 11 ) == 0 )
      ok = 1;
    if ( len > 16 && strncasecmp( buf, "MULTILINESTRING(", 16 ) == 0 )
      ok = 1;
    if ( len > 13 && strncasecmp( buf, "MULTIPOLYGON(", 13 ) == 0 )
      ok = 1;
    if ( len > 19 && strncasecmp( buf, "GEOMETRYCOLLECTION(", 19 ) == 0 )
      ok = 1;
    if ( !ok )
      error = 1;
  }
  if ( error )
  {
    free( buf );
    return NULL;
  }
  return buf;
}

static void
gaiaFreeListToken( gaiaListTokenPtr p )
{
  /* cleans all memory allocations for list token */
  if ( !p )
    return;
  if ( p->line )
    gaiaFreeLinestring( p->line );
  free( p );
}

static void
gaiaFreeMultiListToken( gaiaMultiListTokenPtr p )
{
  /* cleans all memory allocations for multi list token */
  gaiaListTokenPtr pt;
  gaiaListTokenPtr ptn;
  if ( !p )
    return;
  pt = p->first;
  while ( pt )
  {
    ptn = pt->next;
    gaiaFreeListToken( pt );
    pt = ptn;
  }
  free( p );
}

static void
gaiaFreeMultiMultiListToken( gaiaMultiMultiListTokenPtr p )
{
  /* cleans all memory allocations for multi-multi list token */
  gaiaMultiListTokenPtr pt;
  gaiaMultiListTokenPtr ptn;
  if ( !p )
    return;
  pt = p->first;
  while ( pt )
  {
    ptn = pt->next;
    gaiaFreeMultiListToken( pt );
    pt = ptn;
  }
  free( p );
}

static void
gaiaFreeGeomCollListToken( gaiaGeomCollListTokenPtr p )
{
  /* cleans all memory allocations for geocoll list token */
  gaiaVarListTokenPtr pt;
  gaiaVarListTokenPtr ptn;
  if ( !p )
    return;
  pt = p->first;
  while ( pt )
  {
    ptn = pt->next;
    if ( pt->type == GAIA_LINESTRING )
      gaiaFreeListToken(( gaiaListTokenPtr )( pt->pointer ) );
    if ( pt->type == GAIA_POLYGON )
      gaiaFreeMultiListToken(( gaiaMultiListTokenPtr )( pt->pointer ) );
    pt = ptn;
  }
  free( p );
}

static int
gaiaParseDouble( char *token, double *coord )
{
  /* checks if this token is a valid double */
  int i;
  int digits = 0;
  int errs = 0;
  int commas = 0;
  int signs = 0;
  *coord = 0.0;
  for ( i = 0; i < strlen( token ); i++ )
  {
    if ( token[i] == '+' || token[i] == '-' )
      signs++;
    else if ( token[i] == '.' )
      commas++;
    else if ( token[i] >= '0' && token[i] <= '9' )
      digits++;
    else
      errs++;
  }
  if ( errs > 0 )
    return 0;
  if ( digits == 0 )
    return 0;
  if ( commas > 1 )
    return 0;
  if ( signs > 1 )
    return 0;
  if ( signs )
  {
    switch ( token[0] )
    {
      case '-':
      case '+':
        break;
      default:
        return 0;
    };
  }
  *coord = atof( token );
  return 1;
}

static void
gaiaAddToken( char *token, gaiaTokenPtr * first, gaiaTokenPtr * last )
{
  /* inserts a token at the end of the linked list */
  double coord;
  gaiaTokenPtr p;
  if ( strlen( token ) == 0 )
    return;
  p = malloc( sizeof( gaiaToken ) );
  p->type = GAIA_UNKNOWN;
  p->coord = 0.0;
  if ( strcasecmp( token, "POINT" ) == 0 )
    p->type = GAIA_POINT;
  if ( strcasecmp( token, "LINESTRING" ) == 0 )
    p->type = GAIA_LINESTRING;
  if ( strcasecmp( token, "POLYGON" ) == 0 )
    p->type = GAIA_POLYGON;
  if ( strcasecmp( token, "MULTIPOINT" ) == 0 )
    p->type = GAIA_MULTIPOINT;
  if ( strcasecmp( token, "MULTILINESTRING" ) == 0 )
    p->type = GAIA_MULTILINESTRING;
  if ( strcasecmp( token, "MULTIPOLYGON" ) == 0 )
    p->type = GAIA_MULTIPOLYGON;
  if ( strcasecmp( token, "GEOMETRYCOLLECTION" ) == 0 )
    p->type = GAIA_GEOMETRYCOLLECTION;
  if ( strcmp( token, "(" ) == 0 )
    p->type = GAIA_OPENED;
  if ( strcmp( token, ")" ) == 0 )
    p->type = GAIA_CLOSED;
  if ( strcmp( token, "," ) == 0 )
    p->type = GAIA_COMMA;
  if ( strcmp( token, " " ) == 0 )
    p->type = GAIA_SPACE;
  if ( p->type == GAIA_UNKNOWN )
  {
    if ( gaiaParseDouble( token, &coord ) )
    {
      p->type = GAIA_COORDINATE;
      p->coord = coord;
    }
  }
  p->next = NULL;
  if ( *first == NULL )
    *first = p;
  if ( *last != NULL )
    ( *last )->next = p;
  *last = p;
}

static gaiaListTokenPtr
gaiaBuildListToken( gaiaTokenPtr first, gaiaTokenPtr last )
{
  /* builds a list of tokens representing a list in the form (1 2, 3 4, 5 6), as required by LINESTRING, MULTIPOINT or RING */
  gaiaListTokenPtr list = NULL;
  gaiaTokenPtr pt;
  int i = 0;
  int ip = 0;
  int err = 0;
  int nx = 0;
  int ny = 0;
  int iv;
  double x = 0.0;
  double y;
  pt = first;
  while ( pt != NULL )
  {
    /* check if this one is a valid list of POINTS */
    if ( i == 0 )
    {
      if ( pt->type != GAIA_OPENED )
        err = 1;
    }
    else if ( pt == last )
    {
      if ( pt->type != GAIA_CLOSED )
        err = 1;
    }
    else
    {
      if ( ip == 0 )
      {
        if ( pt->type != GAIA_COORDINATE )
          err = 1;
        else
          nx++;
      }
      else if ( ip == 1 )
      {
        if ( pt->type != GAIA_SPACE )
          err = 1;
      }
      else if ( ip == 2 )
      {
        if ( pt->type != GAIA_COORDINATE )
          err = 1;
        else
          ny++;
      }
      else if ( ip == 3 )
      {
        if ( pt->type != GAIA_COMMA )
          err = 1;
      }
      ip++;
      if ( ip > 3 )
        ip = 0;
    }
    i++;
    pt = pt->next;
    if ( pt == last )
      break;
  }
  if ( nx != ny )
    err = 1;
  if ( nx < 1 )
    err = 1;
  if ( err )
    return NULL;
  /* ok, there is no erorr. finally we can build the POINTS list */
  list = malloc( sizeof( gaiaListToken ) );
  list->points = nx;
  list->line = gaiaAllocLinestring( nx );
  list->next = NULL;
  iv = 0;
  ip = 0;
  i = 0;
  pt = first;
  while ( pt != NULL )
  {
    /* sets coords for all POINTS */
    if ( i == 0 )
      ;
    else if ( pt == last )
      ;
    else
    {
      if ( ip == 0 )
        x = pt->coord;
      else if ( ip == 2 )
      {
        y = pt->coord;
        gaiaSetPoint( list->line->Coords, iv, x, y );
        iv++;
      }
      ip++;
      if ( ip > 3 )
        ip = 0;
    }
    i++;
    pt = pt->next;
    if ( pt == last )
      break;
  }
  return list;
}

static gaiaMultiListTokenPtr
gaiaBuildMultiListToken( gaiaTokenPtr first, gaiaTokenPtr last )
{
  /* builds a multi list of tokens representing an array of elementar lists in the form ((...),(....),(...)), as required by MULTILINESTRING and POLYGON */
  gaiaMultiListTokenPtr multi_list = NULL;
  gaiaTokenPtr pt;
  gaiaTokenPtr p_first = NULL;
  gaiaListTokenPtr list;
  int opened = 0;
  pt = first;
  while ( pt != NULL )
  {
    /* identifies the sub-lists contained in this multi list */
    if ( pt->type == GAIA_OPENED )
    {
      opened++;
      if ( opened == 2 )
        p_first = pt;
    }
    if ( pt->type == GAIA_CLOSED )
    {
      if ( p_first )
      {
        list = gaiaBuildListToken( p_first, pt );
        if ( !multi_list )
        {
          multi_list = malloc( sizeof( gaiaMultiListToken ) );
          multi_list->first = NULL;
          multi_list->last = NULL;
          multi_list->next = NULL;
        }
        if ( multi_list->first == NULL )
          multi_list->first = list;
        if ( multi_list->last != NULL )
          multi_list->last->next = list;
        multi_list->last = list;
        p_first = NULL;
      }
      opened--;
    }
    pt = pt->next;
    if ( pt == last )
      break;
  }
  return multi_list;
}

static gaiaMultiMultiListTokenPtr
gaiaBuildMultiMultiListToken( gaiaTokenPtr first, gaiaTokenPtr last )
{
  /* builds a multi list of tokens representing an array of complex lists in the form (((...),(....),(...)),((...),(...)),((...))), as required by MULTIPOLYGON */
  gaiaMultiMultiListTokenPtr multi_multi_list = NULL;
  gaiaTokenPtr pt;
  gaiaTokenPtr p_first = NULL;
  gaiaMultiListTokenPtr multi_list;
  int opened = 0;
  pt = first;
  while ( pt != NULL )
  {
    /* identifies the sub-lists contained in this multi list */
    if ( pt->type == GAIA_OPENED )
    {
      opened++;
      if ( opened == 2 )
        p_first = pt;
    }
    if ( pt->type == GAIA_CLOSED )
    {
      if ( p_first && opened == 2 )
      {
        multi_list = gaiaBuildMultiListToken( p_first, pt );
        if ( !multi_multi_list )
        {
          multi_multi_list =
            malloc( sizeof( gaiaMultiMultiListToken ) );
          multi_multi_list->first = NULL;
          multi_multi_list->last = NULL;
        }
        if ( multi_multi_list->first == NULL )
          multi_multi_list->first = multi_list;
        if ( multi_multi_list->last != NULL )
          multi_multi_list->last->next = multi_list;
        multi_multi_list->last = multi_list;
        p_first = NULL;
      }
      opened--;
    }
    pt = pt->next;
    if ( pt == last )
      break;
  }
  return multi_multi_list;
}

static gaiaGeomCollListTokenPtr
gaiaBuildGeomCollListToken( gaiaTokenPtr first, gaiaTokenPtr last )
{
  /* builds a variable list of tokens representing an array of entities in the form (ELEM(),ELEM(),ELEM())  as required by GEOMETRYCOLLECTION */
  gaiaGeomCollListTokenPtr geocoll_list = NULL;
  gaiaTokenPtr pt;
  gaiaTokenPtr pt2;
  gaiaTokenPtr p_first = NULL;
  gaiaListTokenPtr list;
  gaiaMultiListTokenPtr multi_list;
  gaiaVarListTokenPtr var_list;
  int opened = 0;
  int i;
  int err;
  double x = 0;
  double y = 0;
  pt = first;
  while ( pt != NULL )
  {
    /* identifies the sub-lists contained in this complex list */
    if ( pt->type == GAIA_POINT )
    {
      /* parsing a POINT list */
      err = 0;
      i = 0;
      pt2 = pt->next;
      while ( pt2 != NULL )
      {
        /* check if this one is a valid POINT */
        switch ( i )
        {
          case 0:
            if ( pt2->type != GAIA_OPENED )
              err = 1;
            break;
          case 1:
            if ( pt2->type != GAIA_COORDINATE )
              err = 1;
            else
              x = pt2->coord;
            break;
          case 2:
            if ( pt2->type != GAIA_SPACE )
              err = 1;
            break;
          case 3:
            if ( pt2->type != GAIA_COORDINATE )
              err = 1;
            else
              y = pt2->coord;
            break;
          case 4:
            if ( pt2->type != GAIA_CLOSED )
              err = 1;
            break;
        };
        i++;
        if ( i > 4 )
          break;
        pt2 = pt2->next;
      }
      if ( err )
        goto error;
      var_list = malloc( sizeof( gaiaVarListToken ) );
      var_list->type = GAIA_POINT;
      var_list->x = x;
      var_list->y = y;
      var_list->next = NULL;
      if ( !geocoll_list )
      {
        geocoll_list = malloc( sizeof( gaiaGeomCollListToken ) );
        geocoll_list->first = NULL;
        geocoll_list->last = NULL;
      }
      if ( geocoll_list->first == NULL )
        geocoll_list->first = var_list;
      if ( geocoll_list->last != NULL )
        geocoll_list->last->next = var_list;
      geocoll_list->last = var_list;
    }
    else if ( pt->type == GAIA_LINESTRING )
    {
      /* parsing a LINESTRING list */
      p_first = NULL;
      pt2 = pt->next;
      while ( pt2 != NULL )
      {
        if ( pt2->type == GAIA_OPENED )
          p_first = pt2;
        if ( pt2->type == GAIA_CLOSED )
        {
          list = gaiaBuildListToken( p_first, pt2 );
          if ( list )
          {
            var_list = malloc( sizeof( gaiaVarListToken ) );
            var_list->type = GAIA_LINESTRING;
            var_list->pointer = list;
            var_list->next = NULL;
            if ( !geocoll_list )
            {
              geocoll_list =
                malloc( sizeof
                        ( gaiaGeomCollListToken ) );
              geocoll_list->first = NULL;
              geocoll_list->last = NULL;
            }
            if ( geocoll_list->first == NULL )
              geocoll_list->first = var_list;
            if ( geocoll_list->last != NULL )
              geocoll_list->last->next = var_list;
            geocoll_list->last = var_list;
            break;
          }
          else
            goto error;
        }
        pt2 = pt2->next;
        if ( pt2 == last )
          break;
      }
    }
    else if ( pt->type == GAIA_POLYGON )
    {
      /* parsing a POLYGON list */
      opened = 0;
      p_first = NULL;
      pt2 = pt->next;
      while ( pt2 != NULL )
      {
        if ( pt2->type == GAIA_OPENED )
        {
          opened++;
          if ( opened == 1 )
            p_first = pt2;
        }
        if ( pt2->type == GAIA_CLOSED )
        {
          if ( p_first && opened == 1 )
          {
            multi_list =
              gaiaBuildMultiListToken( p_first, pt2 );
            if ( multi_list )
            {
              var_list =
                malloc( sizeof( gaiaVarListToken ) );
              var_list->type = GAIA_POLYGON;
              var_list->pointer = multi_list;
              var_list->next = NULL;
              if ( !geocoll_list )
              {
                geocoll_list =
                  malloc( sizeof
                          ( gaiaGeomCollListToken ) );
                geocoll_list->first = NULL;
                geocoll_list->last = NULL;
              }
              if ( geocoll_list->first == NULL )
                geocoll_list->first = var_list;
              if ( geocoll_list->last != NULL )
                geocoll_list->last->next = var_list;
              geocoll_list->last = var_list;
              break;
            }
            else
              goto error;
          }
          opened--;
        }
        pt2 = pt2->next;
        if ( pt2 == last )
          break;
      }
    }
    pt = pt->next;
  }
  return geocoll_list;
error:
  gaiaFreeGeomCollListToken( geocoll_list );
  return NULL;
}

static gaiaPointPtr
gaiaBuildPoint( gaiaTokenPtr first )
{
  /* builds a POINT, if this token's list contains a valid POINT */
  gaiaPointPtr point = NULL;
  gaiaTokenPtr pt = first;
  int i = 0;
  int err = 0;
  double x = 0.0;
  double y = 0.0;
  while ( pt != NULL )
  {
    /* check if this one is a valid POINT */
    switch ( i )
    {
      case 0:
        if ( pt->type != GAIA_OPENED )
          err = 1;
        break;
      case 1:
        if ( pt->type != GAIA_COORDINATE )
          err = 1;
        else
          x = pt->coord;
        break;
      case 2:
        if ( pt->type != GAIA_SPACE )
          err = 1;
        break;
      case 3:
        if ( pt->type != GAIA_COORDINATE )
          err = 1;
        else
          y = pt->coord;
        break;
      case 4:
        if ( pt->type != GAIA_CLOSED )
          err = 1;
        break;
      default:
        err = 1;
        break;
    };
    i++;
    pt = pt->next;
  }
  if ( err )
    return NULL;
  point = gaiaAllocPoint( x, y );
  return point;
}

static gaiaGeomCollPtr
gaiaGeometryFromPoint( gaiaPointPtr point )
{
  /* builds a GEOMETRY containing a POINT */
  gaiaGeomCollPtr geom = NULL;
  geom = gaiaAllocGeomColl();
  geom->DeclaredType = GAIA_POINT;
  gaiaAddPointToGeomColl( geom, point->X, point->Y );
  gaiaFreePoint( point );
  return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromLinestring( gaiaLinestringPtr line )
{
  /* builds a GEOMETRY containing a LINESTRING */
  gaiaGeomCollPtr geom = NULL;
  gaiaLinestringPtr line2;
  int iv;
  double x;
  double y;
  geom = gaiaAllocGeomColl();
  geom->DeclaredType = GAIA_LINESTRING;
  line2 = gaiaAddLinestringToGeomColl( geom, line->Points );
  for ( iv = 0; iv < line2->Points; iv++ )
  {
    /* sets the POINTS for the exterior ring */
    gaiaGetPoint( line->Coords, iv, &x, &y );
    gaiaSetPoint( line2->Coords, iv, x, y );
  }
  gaiaFreeLinestring( line );
  return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromPolygon( gaiaMultiListTokenPtr polygon )
{
  /* builds a GEOMETRY containing a POLYGON */
  int iv;
  int ib;
  int borders = 0;
  double x;
  double y;
  gaiaPolygonPtr pg;
  gaiaRingPtr ring;
  gaiaLinestringPtr line;
  gaiaGeomCollPtr geom = NULL;
  gaiaListTokenPtr pt;
  pt = polygon->first;
  while ( pt != NULL )
  {
    /* counts how many rings are in the list */
    borders++;
    pt = pt->next;
  }
  if ( !borders )
    return NULL;
  geom = gaiaAllocGeomColl();
  geom->DeclaredType = GAIA_POLYGON;
  /* builds the polygon */
  line = polygon->first->line;
  pg = gaiaAddPolygonToGeomColl( geom, line->Points, borders - 1 );
  ring = pg->Exterior;
  for ( iv = 0; iv < ring->Points; iv++ )
  {
    /* sets the POINTS for the exterior ring */
    gaiaGetPoint( line->Coords, iv, &x, &y );
    gaiaSetPoint( ring->Coords, iv, x, y );
  }
  ib = 0;
  pt = polygon->first->next;
  while ( pt != NULL )
  {
    /* builds the interior rings [if any] */
    line = pt->line;
    ring = gaiaAddInteriorRing( pg, ib, line->Points );
    for ( iv = 0; iv < ring->Points; iv++ )
    {
      /* sets the POINTS for some interior ring */
      gaiaGetPoint( line->Coords, iv, &x, &y );
      gaiaSetPoint( ring->Coords, iv, x, y );
    }
    ib++;
    pt = pt->next;
  }
  return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromMPoint( gaiaLinestringPtr mpoint )
{
  /* builds a GEOMETRY containing a MULTIPOINT */
  int ie;
  double x;
  double y;
  gaiaGeomCollPtr geom = NULL;
  geom = gaiaAllocGeomColl();
  geom->DeclaredType = GAIA_MULTIPOINT;
  for ( ie = 0; ie < mpoint->Points; ie++ )
  {
    gaiaGetPoint( mpoint->Coords, ie, &x, &y );
    gaiaAddPointToGeomColl( geom, x, y );
  }
  return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromMLine( gaiaMultiListTokenPtr mline )
{
  /* builds a GEOMETRY containing a MULTILINESTRING */
  int iv;
  int lines = 0;
  double x;
  double y;
  gaiaListTokenPtr pt;
  gaiaLinestringPtr line;
  gaiaLinestringPtr line2;
  gaiaGeomCollPtr geom = NULL;
  pt = mline->first;
  while ( pt != NULL )
  {
    /* counts how many linestrings are in the list */
    lines++;
    pt = pt->next;
  }
  if ( !lines )
    return NULL;
  geom = gaiaAllocGeomColl();
  geom->DeclaredType = GAIA_MULTILINESTRING;
  pt = mline->first;
  while ( pt != NULL )
  {
    /* creates and initializes one linestring for each iteration */
    line = pt->line;
    line2 = gaiaAddLinestringToGeomColl( geom, line->Points );
    for ( iv = 0; iv < line->Points; iv++ )
    {
      gaiaGetPoint( line->Coords, iv, &x, &y );
      gaiaSetPoint( line2->Coords, iv, x, y );
    }
    pt = pt->next;
  }
  return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromMPoly( gaiaMultiMultiListTokenPtr mpoly )
{
  /* builds a GEOMETRY containing a MULTIPOLYGON */
  int iv;
  int ib;
  int borders;
  int entities = 0;
  double x;
  double y;
  gaiaPolygonPtr pg;
  gaiaRingPtr ring;
  gaiaLinestringPtr line;
  gaiaGeomCollPtr geom = NULL;
  gaiaMultiListTokenPtr multi;
  gaiaListTokenPtr pt;
  multi = mpoly->first;
  while ( multi != NULL )
  {
    /* counts how many polygons are in the list */
    entities++;
    multi = multi->next;
  }
  if ( !entities )
    return NULL;
  /* allocates and initializes the geometry to be returned */
  geom = gaiaAllocGeomColl();
  geom->DeclaredType = GAIA_MULTIPOLYGON;
  multi = mpoly->first;
  while ( multi != NULL )
  {
    borders = 0;
    pt = multi->first;
    while ( pt != NULL )
    {
      /* counts how many rings are in the list */
      borders++;
      pt = pt->next;
    }
    /* builds one polygon */
    line = multi->first->line;
    pg = gaiaAddPolygonToGeomColl( geom, line->Points, borders - 1 );
    ring = pg->Exterior;
    for ( iv = 0; iv < ring->Points; iv++ )
    {
      /* sets the POINTS for the exterior ring */
      gaiaGetPoint( line->Coords, iv, &x, &y );
      gaiaSetPoint( ring->Coords, iv, x, y );
    }
    ib = 0;
    pt = multi->first->next;
    while ( pt != NULL )
    {
      /* builds the interior rings [if any] */
      line = pt->line;
      ring = gaiaAddInteriorRing( pg, ib, line->Points );
      for ( iv = 0; iv < ring->Points; iv++ )
      {
        /* sets the POINTS for the exterior ring */
        gaiaGetPoint( line->Coords, iv, &x, &y );
        gaiaSetPoint( ring->Coords, iv, x, y );
      }
      ib++;
      pt = pt->next;
    }
    multi = multi->next;
  }
  return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromGeomColl( gaiaGeomCollListTokenPtr geocoll )
{
  /* builds a GEOMETRY containing a GEOMETRYCOLLECTION */
  int iv;
  int ib;
  int borders;
  int entities = 0;
  double x;
  double y;
  gaiaPolygonPtr pg;
  gaiaRingPtr ring;
  gaiaLinestringPtr line2;
  gaiaLinestringPtr line;
  gaiaGeomCollPtr geom = NULL;
  gaiaListTokenPtr linestring;
  gaiaMultiListTokenPtr polyg;
  gaiaVarListTokenPtr multi;
  gaiaListTokenPtr pt;
  multi = geocoll->first;
  while ( multi != NULL )
  {
    /* counts how many polygons are in the list */
    entities++;
    multi = multi->next;
  }
  if ( !entities )
    return NULL;
  /* allocates and initializes the geometry to be returned */
  geom = gaiaAllocGeomColl();
  geom->DeclaredType = GAIA_GEOMETRYCOLLECTION;
  multi = geocoll->first;
  while ( multi != NULL )
  {
    switch ( multi->type )
    {
      case GAIA_POINT:
        gaiaAddPointToGeomColl( geom, multi->x, multi->y );
        break;
      case GAIA_LINESTRING:
        linestring = ( gaiaListTokenPtr )( multi->pointer );
        line = linestring->line;
        line2 = gaiaAddLinestringToGeomColl( geom, line->Points );
        for ( iv = 0; iv < line2->Points; iv++ )
        {
          /* sets the POINTS for the LINESTRING */
          gaiaGetPoint( line->Coords, iv, &x, &y );
          gaiaSetPoint( line2->Coords, iv, x, y );
        }
        break;
      case GAIA_POLYGON:
        polyg = multi->pointer;
        borders = 0;
        pt = polyg->first;
        while ( pt != NULL )
        {
          /* counts how many rings are in the list */
          borders++;
          pt = pt->next;
        }
        /* builds one polygon */
        line = polyg->first->line;
        pg = gaiaAddPolygonToGeomColl( geom, line->Points, borders - 1 );
        ring = pg->Exterior;
        for ( iv = 0; iv < ring->Points; iv++ )
        {
          /* sets the POINTS for the exterior ring */
          gaiaGetPoint( line->Coords, iv, &x, &y );
          gaiaSetPoint( ring->Coords, iv, x, y );
        }
        ib = 0;
        pt = polyg->first->next;
        while ( pt != NULL )
        {
          /* builds the interior rings [if any] */
          line = pt->line;
          ring = gaiaAddInteriorRing( pg, ib, line->Points );
          for ( iv = 0; iv < ring->Points; iv++ )
          {
            /* sets the POINTS for the exterior ring */
            gaiaGetPoint( line->Coords, iv, &x, &y );
            gaiaSetPoint( ring->Coords, iv, x, y );
          }
          ib++;
          pt = pt->next;
        }
        break;
    };
    multi = multi->next;
  }
  return geom;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaParseWkt( const unsigned char *dirty_buffer, short type )
{
  /* tryes to build a GEOMETRY by parsing a WKT encoded string */
  gaiaTokenPtr first = NULL;
  gaiaTokenPtr last = NULL;
  gaiaTokenPtr pt;
  gaiaTokenPtr ptn;
  char *buffer = NULL;
  char dummy[256];
  char *po = dummy;
  char *p;
  int opened;
  int max_opened;
  int closed;
  gaiaListTokenPtr list_token = NULL;
  gaiaMultiListTokenPtr multi_list_token = NULL;
  gaiaMultiMultiListTokenPtr multi_multi_list_token = NULL;
  gaiaGeomCollListTokenPtr geocoll_list_token = NULL;
  gaiaPointPtr point;
  gaiaLinestringPtr line;
  gaiaGeomCollPtr geo = NULL;
  /* normalizing the WKT string */
  buffer = gaiaCleanWkt( dirty_buffer );
  if ( !buffer )
    return NULL;
  p = buffer;
  while ( *p != '\0' )
  {
    /* breaking the WKT string into tokens */
    if ( *p == '(' )
    {
      *po = '\0';
      gaiaAddToken( dummy, &first, &last );
      gaiaAddToken( "(", &first, &last );
      po = dummy;
      p++;
      continue;
    }
    if ( *p == ')' )
    {
      *po = '\0';
      gaiaAddToken( dummy, &first, &last );
      gaiaAddToken( ")", &first, &last );
      po = dummy;
      p++;
      continue;
    }
    if ( *p == ' ' )
    {
      *po = '\0';
      gaiaAddToken( dummy, &first, &last );
      gaiaAddToken( " ", &first, &last );
      po = dummy;
      p++;
      continue;
    }
    if ( *p == ',' )
    {
      *po = '\0';
      gaiaAddToken( dummy, &first, &last );
      gaiaAddToken( ",", &first, &last );
      po = dummy;
      p++;
      continue;
    }
    *po++ = *p++;
  }
  if ( first == NULL )
    goto err;
  max_opened = 0;
  opened = 0;
  closed = 0;
  pt = first;
  while ( pt != NULL )
  {
    /* checks for absence of serious errors */
    if ( pt->type == GAIA_UNKNOWN )
      goto err;
    if ( pt->type == GAIA_OPENED )
    {
      opened++;
      if ( opened > 3 )
        goto err;
      if ( opened > max_opened )
        max_opened = opened;
    }
    if ( pt->type == GAIA_CLOSED )
    {
      closed++;
      if ( closed > opened )
        goto err;
      opened--;
      closed--;
    }
    pt = pt->next;
  }
  if ( opened == 0 && closed == 0 )
    ;
  else
    goto err;
  if ( type < 0 )
    ;   /* no restrinction about GEOMETRY CLASS TYPE */
  else
  {
    if ( first->type != type )
      goto err;  /* invalid CLASS TYPE for request */
  }
  /* preparing the organized token-list structures */
  switch ( first->type )
  {
    case GAIA_POINT:
      if ( max_opened != 1 )
        goto err;
      break;
    case GAIA_LINESTRING:
      if ( max_opened != 1 )
        goto err;
      list_token = gaiaBuildListToken( first->next, last );
      break;
    case GAIA_POLYGON:
      if ( max_opened != 2 )
        goto err;
      multi_list_token = gaiaBuildMultiListToken( first->next, last );
      break;
    case GAIA_MULTIPOINT:
      if ( max_opened != 1 )
        goto err;
      list_token = gaiaBuildListToken( first->next, last );
      break;
    case GAIA_MULTILINESTRING:
      if ( max_opened != 2 )
        goto err;
      multi_list_token = gaiaBuildMultiListToken( first->next, last );
      break;
    case GAIA_MULTIPOLYGON:
      if ( max_opened != 3 )
        goto err;
      multi_multi_list_token =
        gaiaBuildMultiMultiListToken( first->next, last );
      break;
    case GAIA_GEOMETRYCOLLECTION:
      if ( max_opened == 2 || max_opened == 3 )
        ;
      else
        goto err;
      geocoll_list_token = gaiaBuildGeomCollListToken( first->next, last );
      break;
  }
  switch ( first->type )
  {
    case GAIA_POINT:
      point = gaiaBuildPoint( first->next );
      if ( point )
        geo = gaiaGeometryFromPoint( point );
      break;
    case GAIA_LINESTRING:
      line = list_token->line;
      if ( line )
        geo = gaiaGeometryFromLinestring( line );
      list_token->line = NULL;
      break;
    case GAIA_POLYGON:
      if ( multi_list_token )
        geo = gaiaGeometryFromPolygon( multi_list_token );
      break;
    case GAIA_MULTIPOINT:
      line = list_token->line;
      if ( line )
        geo = gaiaGeometryFromMPoint( line );
      list_token->line = NULL;
      break;
    case GAIA_MULTILINESTRING:
      if ( multi_list_token )
        geo = gaiaGeometryFromMLine( multi_list_token );
      break;
    case GAIA_MULTIPOLYGON:
      if ( multi_multi_list_token )
        geo = gaiaGeometryFromMPoly( multi_multi_list_token );
      break;
    case GAIA_GEOMETRYCOLLECTION:
      if ( geocoll_list_token )
        geo = gaiaGeometryFromGeomColl( geocoll_list_token );
      break;
  }
  if ( buffer )
    free( buffer );
  gaiaFreeListToken( list_token );
  gaiaFreeMultiListToken( multi_list_token );
  gaiaFreeMultiMultiListToken( multi_multi_list_token );
  gaiaFreeGeomCollListToken( geocoll_list_token );
  pt = first;
  while ( pt != NULL )
  {
    /* cleans the token's list */
    ptn = pt->next;
    free( pt );
    pt = ptn;
  }
  gaiaMbrGeometry( geo );
  return geo;
err:
  if ( buffer )
    free( buffer );
  gaiaFreeListToken( list_token );
  gaiaFreeMultiListToken( multi_list_token );
  gaiaFreeMultiMultiListToken( multi_multi_list_token );
  gaiaFreeGeomCollListToken( geocoll_list_token );
  pt = first;
  while ( pt != NULL )
  {
    /* cleans the token's list */
    ptn = pt->next;
    free( pt );
    pt = ptn;
  }
  return NULL;
}

static void
gaiaOutClean( char *buffer )
{
  /* cleans unneeded trailing zeros */
  int i;
  for ( i = strlen( buffer ) - 1; i > 0; i-- )
  {
    if ( buffer[i] == '0' )
      buffer[i] = '\0';
    else
      break;
  }
  if ( buffer[i] == '.' )
    buffer[i] = '\0';
}

static void
gaiaOutCheckBuffer( char **buffer, int *size )
{
  /* checks if the receiving buffer has enough free room, and in case reallocates it */
  char *old = *buffer;
  int len = strlen( *buffer );
  if (( *size - len ) < 256 )
  {
    *size += 4096;
    *buffer = realloc( old, *size );
  }
}

static void
gaiaOutText( char *text, char **buffer, int *size )
{
  /* formats a WKT generic text */
  gaiaOutCheckBuffer( buffer, size );
  strcat( *buffer, text );
}

static void
gaiaOutPoint( gaiaPointPtr point, char **buffer, int *size )
{
  /* formats a WKT POINT */
  char buf_x[128];
  char buf_y[128];
  char buf[256];
  gaiaOutCheckBuffer( buffer, size );
  sprintf( buf_x, "%1.6lf", point->X );
  gaiaOutClean( buf_x );
  sprintf( buf_y, "%1.6lf", point->Y );
  gaiaOutClean( buf_y );
  sprintf( buf, "%s %s", buf_x, buf_y );
  strcat( *buffer, buf );
}

static void
gaiaOutLinestring( gaiaLinestringPtr line, char **buffer, int *size )
{
  /* formats a WKT LINESTRING */
  char buf_x[128];
  char buf_y[128];
  char buf[256];
  double x;
  double y;
  int iv;
  for ( iv = 0; iv < line->Points; iv++ )
  {
    gaiaGetPoint( line->Coords, iv, &x, &y );
    gaiaOutCheckBuffer( buffer, size );
    sprintf( buf_x, "%1.6lf", x );
    gaiaOutClean( buf_x );
    sprintf( buf_y, "%1.6lf", y );
    gaiaOutClean( buf_y );
    if ( iv > 0 )
      sprintf( buf, ", %s %s", buf_x, buf_y );
    else
      sprintf( buf, "%s %s", buf_x, buf_y );
    strcat( *buffer, buf );
  }
}

static void
gaiaOutPolygon( gaiaPolygonPtr polyg, char **buffer, int *size )
{
  /* formats a WKT POLYGON */
  char buf_x[128];
  char buf_y[128];
  char buf[256];
  int ib;
  int iv;
  double x;
  double y;
  gaiaRingPtr ring = polyg->Exterior;
  for ( iv = 0; iv < ring->Points; iv++ )
  {
    gaiaGetPoint( ring->Coords, iv, &x, &y );
    gaiaOutCheckBuffer( buffer, size );
    sprintf( buf_x, "%1.6lf", x );
    gaiaOutClean( buf_x );
    sprintf( buf_y, "%1.6lf", y );
    gaiaOutClean( buf_y );
    if ( iv == 0 )
      sprintf( buf, "(%s %s", buf_x, buf_y );
    else if ( iv == ( ring->Points - 1 ) )
      sprintf( buf, ", %s %s)", buf_x, buf_y );
    else
      sprintf( buf, ", %s %s", buf_x, buf_y );
    strcat( *buffer, buf );
  }
  for ( ib = 0; ib < polyg->NumInteriors; ib++ )
  {
    ring = polyg->Interiors + ib;
    for ( iv = 0; iv < ring->Points; iv++ )
    {
      gaiaGetPoint( ring->Coords, iv, &x, &y );
      gaiaOutCheckBuffer( buffer, size );
      sprintf( buf_x, "%1.6lf", x );
      gaiaOutClean( buf_x );
      sprintf( buf_y, "%1.6lf", y );
      gaiaOutClean( buf_y );
      if ( iv == 0 )
        sprintf( buf, ", (%s %s", buf_x, buf_y );
      else if ( iv == ( ring->Points - 1 ) )
        sprintf( buf, ", %s %s)", buf_x, buf_y );
      else
        sprintf( buf, ", %s %s", buf_x, buf_y );
      strcat( *buffer, buf );
    }
  }
}

GAIAGEO_DECLARE void
gaiaOutWkt( gaiaGeomCollPtr geom, char **result )
{
  /*
  / prints the WKT representation of current geometry
  / *result* returns the decoded WKT or NULL if any error is encountered
  */
  int txt_size = 1024;
  int pts = 0;
  int lns = 0;
  int pgs = 0;
  gaiaPointPtr point;
  gaiaLinestringPtr line;
  gaiaPolygonPtr polyg;
  if ( !geom )
  {
    *result = NULL;
    return;
  }
  *result = malloc( txt_size );
  memset( *result, '\0', txt_size );
  point = geom->FirstPoint;
  while ( point )
  {
    /* counting how many POINTs are there */
    pts++;
    point = point->Next;
  }
  line = geom->FirstLinestring;
  while ( line )
  {
    /* counting how many LINESTRINGs are there */
    lns++;
    line = line->Next;
  }
  polyg = geom->FirstPolygon;
  while ( polyg )
  {
    /* counting how many POLYGONs are there */
    pgs++;
    polyg = polyg->Next;
  }
  if (( pts + lns + pgs ) == 1
      && ( geom->DeclaredType == GAIA_POINT
           || geom->DeclaredType == GAIA_LINESTRING
           || geom->DeclaredType == GAIA_POLYGON ) )
  {
    /* we have only one elementary geometry */
    point = geom->FirstPoint;
    while ( point )
    {
      /* processing POINT */
      strcpy( *result, "POINT(" );
      gaiaOutPoint( point, result, &txt_size );
      gaiaOutText( ")", result, &txt_size );
      point = point->Next;
    }
    line = geom->FirstLinestring;
    while ( line )
    {
      /* processing LINESTRING */
      strcpy( *result, "LINESTRING(" );
      gaiaOutLinestring( line, result, &txt_size );
      gaiaOutText( ")", result, &txt_size );
      line = line->Next;
    }
    polyg = geom->FirstPolygon;
    while ( polyg )
    {
      /* counting how many POLYGON */
      strcpy( *result, "POLYGON(" );
      gaiaOutPolygon( polyg, result, &txt_size );
      gaiaOutText( ")", result, &txt_size );
      polyg = polyg->Next;
    }
  }
  else
  {
    /* we have some kind of complex geometry */
    if ( pts > 0 && lns == 0 && pgs == 0 )
    {
      /* this one is a MULTIPOINT */
      strcpy( *result, "MULTIPOINT(" );
      point = geom->FirstPoint;
      while ( point )
      {
        /* processing POINTs */
        if ( point != geom->FirstPoint )
          gaiaOutText( ", ", result, &txt_size );
        gaiaOutPoint( point, result, &txt_size );
        point = point->Next;
      }
      gaiaOutText( ")", result, &txt_size );
    }
    else if ( pts == 0 && lns > 0 && pgs == 0 )
    {
      /* this one is a MULTILINESTRING */
      strcpy( *result, "MULTILINESTRING(" );
      line = geom->FirstLinestring;
      while ( line )
      {
        /* processing LINESTRINGs */
        if ( line != geom->FirstLinestring )
          gaiaOutText( ", (", result, &txt_size );
        else
          gaiaOutText( "(", result, &txt_size );
        gaiaOutLinestring( line, result, &txt_size );
        gaiaOutText( ")", result, &txt_size );
        line = line->Next;
      }
      gaiaOutText( ")", result, &txt_size );
    }
    else if ( pts == 0 && lns == 0 && pgs > 0 )
    {
      /* this one is a MULTIPOLYGON */
      strcpy( *result, "MULTIPOLYGON(" );
      polyg = geom->FirstPolygon;
      while ( polyg )
      {
        /* processing POLYGONs */
        if ( polyg != geom->FirstPolygon )
          gaiaOutText( ", (", result, &txt_size );
        else
          gaiaOutText( "(", result, &txt_size );
        gaiaOutPolygon( polyg, result, &txt_size );
        gaiaOutText( ")", result, &txt_size );
        polyg = polyg->Next;
      }
      gaiaOutText( ")", result, &txt_size );
    }
    else
    {
      /* this one is a GEOMETRYCOLLECTION */
      int ie = 0;
      strcpy( *result, "GEOMETRYCOLLECTION(" );
      point = geom->FirstPoint;
      while ( point )
      {
        /* processing POINTs */
        if ( ie > 0 )
          gaiaOutText( ", ", result, &txt_size );
        ie++;
        strcat( *result, "POINT(" );
        gaiaOutPoint( point, result, &txt_size );
        gaiaOutText( ")", result, &txt_size );
        point = point->Next;
      }
      line = geom->FirstLinestring;
      while ( line )
      {
        /* processing LINESTRINGs */
        if ( ie > 0 )
          gaiaOutText( ", ", result, &txt_size );
        ie++;
        strcat( *result, "LINESTRING(" );
        gaiaOutLinestring( line, result, &txt_size );
        gaiaOutText( ")", result, &txt_size );
        line = line->Next;
      }
      polyg = geom->FirstPolygon;
      while ( polyg )
      {
        /* processing POLYGONs */
        if ( ie > 0 )
          gaiaOutText( ", ", result, &txt_size );
        ie++;
        strcat( *result, "POLYGON(" );
        gaiaOutPolygon( polyg, result, &txt_size );
        gaiaOutText( ")", result, &txt_size );
        polyg = polyg->Next;
      }
      gaiaOutText( ")", result, &txt_size );
    }
  }
}

/*
/
/  Gaia common support for SVG encoded geometries
/
////////////////////////////////////////////////////////////
/
/ Author: Klaus Foerster klaus.foerster@svg.cc
/ version 0.9. 2008 September 21
 /
 */

static void
SvgCoords( gaiaPointPtr point, char **buffer, int *size, int relative,
           int precision )
{
  /* formats POINT as SVG-attributes x,y */
  char buf_x[128];
  char buf_y[128];
  char buf[256];
  gaiaOutCheckBuffer( buffer, size );
  sprintf( buf_x, "%.*f", precision, point->X );
  gaiaOutClean( buf_x );
  sprintf( buf_y, "%.*f", precision, point->Y * -1 );
  gaiaOutClean( buf_y );
  sprintf( buf, "x=\"%s\" y=\"%s\"", buf_x, buf_y );
  strcat( *buffer, buf );
}

static void
SvgCircle( gaiaPointPtr point, char **buffer, int *size, int relative,
           int precision )
{
  /* formats POINT as SVG-attributes cx,cy */
  char buf_x[128];
  char buf_y[128];
  char buf[256];
  gaiaOutCheckBuffer( buffer, size );
  sprintf( buf_x, "%.*f", precision, point->X );
  gaiaOutClean( buf_x );
  sprintf( buf_y, "%.*f", precision, point->Y * -1 );
  gaiaOutClean( buf_y );
  sprintf( buf, "cx=\"%s\" cy=\"%s\"", buf_x, buf_y );
  strcat( *buffer, buf );
}

static void
SvgPathRelative( int points, double *coords, char **buffer, int *size,
                 int relative, int precision, int closePath )
{
  /* formats LINESTRING as SVG-path d-attribute with relative coordinate moves */
  char buf_x[128];
  char buf_y[128];
  char buf[256];
  double x;
  double y;
  double lastX;
  double lastY;
  int iv;
  for ( iv = 0; iv < points; iv++ )
  {
    gaiaGetPoint( coords, iv, &x, &y );
    gaiaOutCheckBuffer( buffer, size );
    if ( iv == 0 )
    {
      sprintf( buf_x, "%.*f", precision, x );
      gaiaOutClean( buf_x );
      sprintf( buf_y, "%.*f", precision, y * -1 );
      gaiaOutClean( buf_y );
      sprintf( buf, "M %s %s l ", buf_x, buf_y );
    }
    else
    {
      sprintf( buf_x, "%.*f", precision, ( x - lastX ) );
      gaiaOutClean( buf_x );
      sprintf( buf_y, "%.*f", precision, ( y - lastY ) * -1 );
      gaiaOutClean( buf_y );
      sprintf( buf, "%s %s ", buf_x, buf_y );
    }
    lastX = x;
    lastY = y;
    if ( iv == points - 1 && closePath == 1 )
      sprintf( buf, "z " );
    strcat( *buffer, buf );
  }
}

static void
SvgPathAbsolute( int points, double *coords, char **buffer, int *size,
                 int relative, int precision, int closePath )
{
  /* formats LINESTRING as SVG-path d-attribute with relative coordinate moves */
  char buf_x[128];
  char buf_y[128];
  char buf[256];
  double x;
  double y;
  int iv;
  for ( iv = 0; iv < points; iv++ )
  {
    gaiaGetPoint( coords, iv, &x, &y );
    gaiaOutCheckBuffer( buffer, size );
    sprintf( buf_x, "%.*f", precision, x );
    gaiaOutClean( buf_x );
    sprintf( buf_y, "%.*f", precision, y * -1 );
    gaiaOutClean( buf_y );
    if ( iv == 0 )
      sprintf( buf, "M %s %s ", buf_x, buf_y );
    else
      sprintf( buf, "%s %s ", buf_x, buf_y );
    if ( iv == points - 1 && closePath == 1 )
      sprintf( buf, "z " );
    strcat( *buffer, buf );
  }
}

GAIAGEO_DECLARE void
gaiaOutSvg( gaiaGeomCollPtr geom, char **result, int relative, int precision )
{
  /*
  / prints the SVG representation of current geometry
  / *result* returns the decoded SVG or NULL if any error is encountered
  */
  int txt_size = 1024;
  int pts = 0;
  int lns = 0;
  int pgs = 0;
  int ib;
  gaiaPointPtr point;
  gaiaLinestringPtr line;
  gaiaPolygonPtr polyg;
  if ( !geom )
  {
    *result = NULL;
    return;
  }
  *result = malloc( txt_size );
  memset( *result, '\0', txt_size );
  point = geom->FirstPoint;
  while ( point )
  {
    /* counting how many POINTs are there */
    pts++;
    point = point->Next;
  }
  line = geom->FirstLinestring;
  while ( line )
  {
    /* counting how many LINESTRINGs are there */
    lns++;
    line = line->Next;
  }
  polyg = geom->FirstPolygon;
  while ( polyg )
  {
    /* counting how many POLYGONs are there */
    pgs++;
    polyg = polyg->Next;
  }

  if (( pts + lns + pgs ) == 1 )
  {
    /* we have only one elementary geometry */
    point = geom->FirstPoint;
    while ( point )
    {
      /* processing POINT */
      if ( relative == 1 )
        SvgCoords( point, result, &txt_size, relative, precision );
      else
        SvgCircle( point, result, &txt_size, relative, precision );
      point = point->Next;
    }
    line = geom->FirstLinestring;
    while ( line )
    {
      /* processing LINESTRING */
      if ( relative == 1 )
        SvgPathRelative( line->Points, line->Coords, result,
                         &txt_size, relative, precision, 0 );
      else
        SvgPathAbsolute( line->Points, line->Coords, result,
                         &txt_size, relative, precision, 0 );
      line = line->Next;
    }
    polyg = geom->FirstPolygon;
    while ( polyg )
    {
      /* process exterior and interior rings */
      gaiaRingPtr ring = polyg->Exterior;
      if ( relative == 1 )
      {
        SvgPathRelative( ring->Points, ring->Coords, result,
                         &txt_size, relative, precision, 1 );
        for ( ib = 0; ib < polyg->NumInteriors; ib++ )
        {
          ring = polyg->Interiors + ib;
          SvgPathRelative( ring->Points, ring->Coords, result,
                           &txt_size, relative, precision, 1 );
        }
      }
      else
      {
        SvgPathAbsolute( ring->Points, ring->Coords, result,
                         &txt_size, relative, precision, 1 );
        for ( ib = 0; ib < polyg->NumInteriors; ib++ )
        {
          ring = polyg->Interiors + ib;
          SvgPathAbsolute( ring->Points, ring->Coords, result,
                           &txt_size, relative, precision, 1 );
        }
      }
      polyg = polyg->Next;
    }
  }
  else
  {
    /* we have some kind of complex geometry */
    if ( pts > 0 && lns == 0 && pgs == 0 )
    {
      /* this one is a MULTIPOINT */
      point = geom->FirstPoint;
      while ( point )
      {
        /* processing POINTs */
        if ( point != geom->FirstPoint )
          gaiaOutText( ",", result, &txt_size );
        if ( relative == 1 )
          SvgCoords( point, result, &txt_size, relative,
                     precision );
        else
          SvgCircle( point, result, &txt_size, relative,
                     precision );
        point = point->Next;
      }
    }
    else if ( pts == 0 && lns > 0 && pgs == 0 )
    {
      /* this one is a MULTILINESTRING */
      line = geom->FirstLinestring;
      while ( line )
      {
        /* processing LINESTRINGs */
        if ( relative == 1 )
          SvgPathRelative( line->Points, line->Coords, result,
                           &txt_size, relative, precision, 0 );
        else
          SvgPathAbsolute( line->Points, line->Coords, result,
                           &txt_size, relative, precision, 0 );
        line = line->Next;
      }
    }
    else if ( pts == 0 && lns == 0 && pgs > 0 )
    {
      /* this one is a MULTIPOLYGON */
      polyg = geom->FirstPolygon;
      while ( polyg )
      {
        /* processing POLYGONs */
        gaiaRingPtr ring = polyg->Exterior;
        if ( relative == 1 )
        {
          SvgPathRelative( ring->Points, ring->Coords, result,
                           &txt_size, relative, precision, 1 );
          for ( ib = 0; ib < polyg->NumInteriors; ib++ )
          {
            ring = polyg->Interiors + ib;
            SvgPathRelative( ring->Points, ring->Coords,
                             result, &txt_size, relative,
                             precision, 1 );
          }
        }
        else
        {
          SvgPathAbsolute( ring->Points, ring->Coords, result,
                           &txt_size, relative, precision, 1 );
          for ( ib = 0; ib < polyg->NumInteriors; ib++ )
          {
            ring = polyg->Interiors + ib;
            SvgPathAbsolute( ring->Points, ring->Coords,
                             result, &txt_size, relative,
                             precision, 1 );
          }
        }
        polyg = polyg->Next;
      }
    }
    else
    {
      /* this one is a GEOMETRYCOLLECTION */
      int ie = 0;
      point = geom->FirstPoint;
      while ( point )
      {
        /* processing POINTs */
        if ( ie > 0 )
        {
          gaiaOutText( ";", result, &txt_size );
        }
        ie++;
        if ( relative == 1 )
          SvgCoords( point, result, &txt_size, relative,
                     precision );
        else
          SvgCircle( point, result, &txt_size, relative,
                     precision );
        point = point->Next;
      }
      line = geom->FirstLinestring;
      while ( line )
      {
        /* processing LINESTRINGs */
        if ( ie > 0 )
          gaiaOutText( ";", result, &txt_size );
        ie++;
        if ( relative == 1 )
          SvgPathRelative( line->Points, line->Coords, result,
                           &txt_size, relative, precision, 0 );
        else
          SvgPathAbsolute( line->Points, line->Coords, result,
                           &txt_size, relative, precision, 0 );
        line = line->Next;
      }
      polyg = geom->FirstPolygon;
      while ( polyg )
      {
        gaiaRingPtr ring;
        /* processing POLYGONs */
        ie++;
        /* process exterior and interior rings */
        ring = polyg->Exterior;
        if ( relative == 1 )
        {
          SvgPathRelative( ring->Points, ring->Coords, result,
                           &txt_size, relative, precision, 1 );
          for ( ib = 0; ib < polyg->NumInteriors; ib++ )
          {
            ring = polyg->Interiors + ib;
            SvgPathRelative( ring->Points, ring->Coords,
                             result, &txt_size, relative,
                             precision, 1 );
          }
        }
        else
        {
          SvgPathAbsolute( ring->Points, ring->Coords, result,
                           &txt_size, relative, precision, 1 );
          for ( ib = 0; ib < polyg->NumInteriors; ib++ )
          {
            ring = polyg->Interiors + ib;
            SvgPathAbsolute( ring->Points, ring->Coords,
                             result, &txt_size, relative,
                             precision, 1 );
          }
        }
        polyg = polyg->Next;
      }
    }
  }
}

/* END of Klaus Foerster SVG implementation */
