/******************************************************************************
 * $Id: shprewind.c,v 1.2 2002/04/10 17:23:11 warmerda Exp $
 *
 * Project:  Shapelib
 * Purpose:  Utility to validate and reset the winding order of rings in
 *           polygon geometries to match the ordering required by spec.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 2002, Frank Warmerdam
 *
 * This software is available under the following "MIT Style" license,
 * or at the option of the licensee under the LGPL (see LICENSE.LGPL).  This
 * option is discussed in more detail in shapelib.html.
 *
 * --
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************
 *
 * $Log: shprewind.c,v $
 * Revision 1.2  2002/04/10 17:23:11  warmerda
 * copy from source to destination now
 *
 * Revision 1.1  2002/04/10 16:56:36  warmerda
 * New
 *
 */

#include "shapefil.h"

int main( int argc, char ** argv )

{
  SHPHandle hSHP, hSHPOut;
  int  nShapeType, nEntities, i, nInvalidCount = 0;
  double  adfMinBound[4], adfMaxBound[4];

  /* -------------------------------------------------------------------- */
  /*      Display a usage message.                                        */
  /* -------------------------------------------------------------------- */
  if ( argc != 3 )
  {
    printf( "shprewind in_shp_file out_shp_file\n" );
    exit( 1 );
  }

  /* -------------------------------------------------------------------- */
  /*      Open the passed shapefile.                                      */
  /* -------------------------------------------------------------------- */
  hSHP = SHPOpen( argv[1], "rb" );

  if ( hSHP == NULL )
  {
    printf( "Unable to open:%s\n", argv[1] );
    exit( 1 );
  }

  SHPGetInfo( hSHP, &nEntities, &nShapeType, adfMinBound, adfMaxBound );

  /* -------------------------------------------------------------------- */
  /*      Create output shapefile.                                        */
  /* -------------------------------------------------------------------- */
  hSHPOut = SHPCreate( argv[2], nShapeType );

  if ( hSHPOut == NULL )
  {
    printf( "Unable to create:%s\n", argv[2] );
    exit( 1 );
  }

  /* -------------------------------------------------------------------- */
  /* Skim over the list of shapes, printing all the vertices. */
  /* -------------------------------------------------------------------- */
  for ( i = 0; i < nEntities; i++ )
  {
    int  j;
    SHPObject *psShape;

    psShape = SHPReadObject( hSHP, i );
    if ( SHPRewindObject( hSHP, psShape ) )
      nInvalidCount++;
    SHPWriteObject( hSHPOut, -1, psShape );
    SHPDestroyObject( psShape );
  }

  SHPClose( hSHP );
  SHPClose( hSHPOut );

  printf( "%d objects rewound.\n", nInvalidCount );

  exit( 0 );
}
