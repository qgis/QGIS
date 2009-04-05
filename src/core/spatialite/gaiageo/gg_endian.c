/*

 gg_endian.c -- Gaia functions for litte/big endian values handling

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

#include <spatialite/sqlite3ext.h>
#include <spatialite/gaiageo.h>

GAIAGEO_DECLARE int
gaiaEndianArch()
{
  /* checking if target CPU is a little-endian one */
  union cvt
  {
    unsigned char byte[4];
    int int_value;
  } convert;
  convert.int_value = 1;
  if ( convert.byte[0] == 0 )
    return 0;
  return 1;
}

GAIAGEO_DECLARE short
gaiaImport16( const unsigned char *p, int little_endian, int little_endian_arch )
{
  /* fetches a 16bit int from BLOB respecting declared endiannes */
  union cvt
  {
    unsigned char byte[2];
    short short_value;
  } convert;
  if ( little_endian_arch )
  {
    /* Litte-Endian architecture [e.g. x86] */
    if ( !little_endian )
    {
      /* Big Endian data */
      convert.byte[0] = *( p + 1 );
      convert.byte[1] = *( p + 0 );
    }
    else
    {
      /* Little Endian data */
      convert.byte[0] = *( p + 0 );
      convert.byte[1] = *( p + 1 );
    }
  }
  else
  {
    /* Big Endian architecture [e.g. PPC] */
    if ( !little_endian )
    {
      /* Big Endian data */
      convert.byte[0] = *( p + 0 );
      convert.byte[1] = *( p + 1 );
    }
    else
    {
      /* Little Endian data */
      convert.byte[0] = *( p + 1 );
      convert.byte[1] = *( p + 0 );
    }
  }
  return convert.short_value;
}

GAIAGEO_DECLARE int
gaiaImport32( const unsigned char *p, int little_endian, int little_endian_arch )
{
  /* fetches a 32bit int from BLOB respecting declared endiannes */
  union cvt
  {
    unsigned char byte[4];
    int int_value;
  } convert;
  if ( little_endian_arch )
  {
    /* Litte-Endian architecture [e.g. x86] */
    if ( !little_endian )
    {
      /* Big Endian data */
      convert.byte[0] = *( p + 3 );
      convert.byte[1] = *( p + 2 );
      convert.byte[2] = *( p + 1 );
      convert.byte[3] = *( p + 0 );
    }
    else
    {
      /* Little Endian data */
      convert.byte[0] = *( p + 0 );
      convert.byte[1] = *( p + 1 );
      convert.byte[2] = *( p + 2 );
      convert.byte[3] = *( p + 3 );
    }
  }
  else
  {
    /* Big Endian architecture [e.g. PPC] */
    if ( !little_endian )
    {
      /* Big Endian data */
      convert.byte[0] = *( p + 0 );
      convert.byte[1] = *( p + 1 );
      convert.byte[2] = *( p + 2 );
      convert.byte[3] = *( p + 3 );
    }
    else
    {
      /* Little Endian data */
      convert.byte[0] = *( p + 3 );
      convert.byte[1] = *( p + 2 );
      convert.byte[2] = *( p + 1 );
      convert.byte[3] = *( p + 0 );
    }
  }
  return convert.int_value;
}

GAIAGEO_DECLARE double
gaiaImport64( const unsigned char *p, int little_endian, int little_endian_arch )
{
  /* fetches a 64bit double from BLOB respecting declared endiannes */
  union cvt
  {
    unsigned char byte[8];
    double double_value;
  } convert;
  if ( little_endian_arch )
  {
    /* Litte-Endian architecture [e.g. x86] */
    if ( !little_endian )
    {
      /* Big Endian data */
      convert.byte[0] = *( p + 7 );
      convert.byte[1] = *( p + 6 );
      convert.byte[2] = *( p + 5 );
      convert.byte[3] = *( p + 4 );
      convert.byte[4] = *( p + 3 );
      convert.byte[5] = *( p + 2 );
      convert.byte[6] = *( p + 1 );
      convert.byte[7] = *( p + 0 );
    }
    else
    {
      /* Little Endian data */
      convert.byte[0] = *( p + 0 );
      convert.byte[1] = *( p + 1 );
      convert.byte[2] = *( p + 2 );
      convert.byte[3] = *( p + 3 );
      convert.byte[4] = *( p + 4 );
      convert.byte[5] = *( p + 5 );
      convert.byte[6] = *( p + 6 );
      convert.byte[7] = *( p + 7 );
    }
  }
  else
  {
    /* Big Endian architecture [e.g. PPC] */
    if ( !little_endian )
    {
      /* Big Endian data */
      convert.byte[0] = *( p + 0 );
      convert.byte[1] = *( p + 1 );
      convert.byte[2] = *( p + 2 );
      convert.byte[3] = *( p + 3 );
      convert.byte[4] = *( p + 4 );
      convert.byte[5] = *( p + 5 );
      convert.byte[6] = *( p + 6 );
      convert.byte[7] = *( p + 7 );
    }
    else
    {
      /* Little Endian data */
      convert.byte[0] = *( p + 7 );
      convert.byte[1] = *( p + 6 );
      convert.byte[2] = *( p + 5 );
      convert.byte[3] = *( p + 4 );
      convert.byte[4] = *( p + 3 );
      convert.byte[5] = *( p + 2 );
      convert.byte[6] = *( p + 1 );
      convert.byte[7] = *( p + 0 );
    }
  }
  return convert.double_value;
}

GAIAGEO_DECLARE void
gaiaExport16( unsigned char *p, short value, int little_endian,
              int little_endian_arch )
{
  /* stores a 16bit int into a BLOB respecting declared endiannes */
  union cvt
  {
    unsigned char byte[2];
    short short_value;
  } convert;
  convert.short_value = value;
  if ( little_endian_arch )
  {
    /* Litte-Endian architecture [e.g. x86] */
    if ( !little_endian )
    {
      /* Big Endian data */
      *( p + 1 ) = convert.byte[1];
      *( p + 0 ) = convert.byte[0];
    }
    else
    {
      /* Little Endian data */
      *( p + 0 ) = convert.byte[0];
      *( p + 1 ) = convert.byte[1];
    }
  }
  else
  {
    /* Big Endian architecture [e.g. PPC] */
    if ( !little_endian )
    {
      /* Big Endian data */
      *( p + 0 ) = convert.byte[0];
      *( p + 1 ) = convert.byte[1];
    }
    else
    {
      /* Little Endian data */
      *( p + 1 ) = convert.byte[0];
      *( p + 0 ) = convert.byte[1];
    }
  }
}

GAIAGEO_DECLARE void
gaiaExport32( unsigned char *p, int value, int little_endian,
              int little_endian_arch )
{
  /* stores a 32bit int into a BLOB respecting declared endiannes */
  union cvt
  {
    unsigned char byte[4];
    int int_value;
  } convert;
  convert.int_value = value;
  if ( little_endian_arch )
  {
    /* Litte-Endian architecture [e.g. x86] */
    if ( !little_endian )
    {
      /* Big Endian data */
      *( p + 3 ) = convert.byte[0];
      *( p + 2 ) = convert.byte[1];
      *( p + 1 ) = convert.byte[2];
      *( p + 0 ) = convert.byte[3];
    }
    else
    {
      /* Little Endian data */
      *( p + 0 ) = convert.byte[0];
      *( p + 1 ) = convert.byte[1];
      *( p + 2 ) = convert.byte[2];
      *( p + 3 ) = convert.byte[3];
    }
  }
  else
  {
    /* Big Endian architecture [e.g. PPC] */
    if ( !little_endian )
    {
      /* Big Endian data */
      *( p + 0 ) = convert.byte[0];
      *( p + 1 ) = convert.byte[1];
      *( p + 2 ) = convert.byte[2];
      *( p + 3 ) = convert.byte[3];
    }
    else
    {
      /* Little Endian data */
      *( p + 3 ) = convert.byte[0];
      *( p + 2 ) = convert.byte[1];
      *( p + 1 ) = convert.byte[2];
      *( p + 0 ) = convert.byte[3];
    }
  }
}

GAIAGEO_DECLARE void
gaiaExport64( unsigned char *p, double value, int little_endian,
              int little_endian_arch )
{
  /* stores a 64bit double into a BLOB respecting declared endiannes */
  union cvt
  {
    unsigned char byte[8];
    double double_value;
  } convert;
  convert.double_value = value;
  if ( little_endian_arch )
  {
    /* Litte-Endian architecture [e.g. x86] */
    if ( !little_endian )
    {
      /* Big Endian data */
      *( p + 7 ) = convert.byte[0];
      *( p + 6 ) = convert.byte[1];
      *( p + 5 ) = convert.byte[2];
      *( p + 4 ) = convert.byte[3];
      *( p + 3 ) = convert.byte[4];
      *( p + 2 ) = convert.byte[5];
      *( p + 1 ) = convert.byte[6];
      *( p + 0 ) = convert.byte[7];
    }
    else
    {
      /* Little Endian data */
      *( p + 0 ) = convert.byte[0];
      *( p + 1 ) = convert.byte[1];
      *( p + 2 ) = convert.byte[2];
      *( p + 3 ) = convert.byte[3];
      *( p + 4 ) = convert.byte[4];
      *( p + 5 ) = convert.byte[5];
      *( p + 6 ) = convert.byte[6];
      *( p + 7 ) = convert.byte[7];
    }
  }
  else
  {
    /* Big Endian architecture [e.g. PPC] */
    if ( !little_endian )
    {
      /* Big Endian data */
      *( p + 0 ) = convert.byte[0];
      *( p + 1 ) = convert.byte[1];
      *( p + 2 ) = convert.byte[2];
      *( p + 3 ) = convert.byte[3];
      *( p + 4 ) = convert.byte[4];
      *( p + 5 ) = convert.byte[5];
      *( p + 6 ) = convert.byte[6];
      *( p + 7 ) = convert.byte[7];
    }
    else
    {
      /* Little Endian data */
      *( p + 7 ) = convert.byte[0];
      *( p + 6 ) = convert.byte[1];
      *( p + 5 ) = convert.byte[2];
      *( p + 4 ) = convert.byte[3];
      *( p + 3 ) = convert.byte[4];
      *( p + 2 ) = convert.byte[5];
      *( p + 1 ) = convert.byte[6];
      *( p + 0 ) = convert.byte[7];
    }
  }
}
