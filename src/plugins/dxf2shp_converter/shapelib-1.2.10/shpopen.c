/******************************************************************************
 * $Id: shpopen.c,v 1.39 2002/08/26 06:46:56 warmerda Exp $
 *
 * Project:  Shapelib
 * Purpose:  Implementation of core Shapefile read/write functions.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, 2001, Frank Warmerdam
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
 * $Log: shpopen.c,v $
 * Revision 1.39  2002/08/26 06:46:56  warmerda
 * avoid c++ comments
 *
 * Revision 1.38  2002/05/07 16:43:39  warmerda
 * Removed debugging printf.
 *
 * Revision 1.37  2002/04/10 17:35:22  warmerda
 * fixed bug in ring reversal code
 *
 * Revision 1.36  2002/04/10 16:59:54  warmerda
 * added SHPRewindObject
 *
 * Revision 1.35  2001/12/07 15:10:44  warmerda
 * fix if .shx fails to open
 *
 * Revision 1.34  2001/11/01 16:29:55  warmerda
 * move pabyRec into SHPInfo for thread safety
 *
 * Revision 1.33  2001/07/03 12:18:15  warmerda
 * Improved cleanup if SHX not found, provied by Riccardo Cohen.
 *
 * Revision 1.32  2001/06/22 01:58:07  warmerda
 * be more careful about establishing initial bounds in face of NULL shapes
 *
 * Revision 1.31  2001/05/31 19:35:29  warmerda
 * added support for writing null shapes
 *
 * Revision 1.30  2001/05/28 12:46:29  warmerda
 * Add some checking on reasonableness of record count when opening.
 *
 * Revision 1.29  2001/05/23 13:36:52  warmerda
 * added use of SHPAPI_CALL
 *
 * Revision 1.28  2001/02/06 22:25:06  warmerda
 * fixed memory leaks when SHPOpen() fails
 *
 * Revision 1.27  2000/07/18 15:21:33  warmerda
 * added better enforcement of -1 for append in SHPWriteObject
 *
 * Revision 1.26  2000/02/16 16:03:51  warmerda
 * added null shape support
 *
 * Revision 1.25  1999/12/15 13:47:07  warmerda
 * Fixed record size settings in .shp file (was 4 words too long)
 * Added stdlib.h.
 *
 * Revision 1.24  1999/11/05 14:12:04  warmerda
 * updated license terms
 *
 * Revision 1.23  1999/07/27 00:53:46  warmerda
 * added support for rewriting shapes
 *
 * Revision 1.22  1999/06/11 19:19:11  warmerda
 * Cleanup pabyRec static buffer on SHPClose().
 *
 * Revision 1.21  1999/06/02 14:57:56  kshih
 * Remove unused variables
 *
 * Revision 1.20  1999/04/19 21:04:17  warmerda
 * Fixed syntax error.
 *
 * Revision 1.19  1999/04/19 21:01:57  warmerda
 * Force access string to binary in SHPOpen().
 *
 * Revision 1.18  1999/04/01 18:48:07  warmerda
 * Try upper case extensions if lower case doesn't work.
 *
 * Revision 1.17  1998/12/31 15:29:39  warmerda
 * Disable writing measure values to multipatch objects if
 * DISABLE_MULTIPATCH_MEASURE is defined.
 *
 * Revision 1.16  1998/12/16 05:14:33  warmerda
 * Added support to write MULTIPATCH.  Fixed reading Z coordinate of
 * MULTIPATCH. Fixed record size written for all feature types.
 *
 * Revision 1.15  1998/12/03 16:35:29  warmerda
 * r+b is proper binary access string, not rb+.
 *
 * Revision 1.14  1998/12/03 15:47:56  warmerda
 * Fixed setting of nVertices in SHPCreateObject().
 *
 * Revision 1.13  1998/12/03 15:33:54  warmerda
 * Made SHPCalculateExtents() separately callable.
 *
 * Revision 1.12  1998/11/11 20:01:50  warmerda
 * Fixed bug writing ArcM/Z, and PolygonM/Z for big endian machines.
 *
 * Revision 1.11  1998/11/09 20:56:44  warmerda
 * Fixed up handling of file wide bounds.
 *
 * Revision 1.10  1998/11/09 20:18:51  warmerda
 * Converted to support 3D shapefiles, and use of SHPObject.
 *
 * Revision 1.9  1998/02/24 15:09:05  warmerda
 * Fixed memory leak.
 *
 * Revision 1.8  1997/12/04 15:40:29  warmerda
 * Fixed byte swapping of record number, and record length fields in the
 * .shp file.
 *
 * Revision 1.7  1995/10/21 03:15:58  warmerda
 * Added support for binary file access, the magic cookie 9997
 * and tried to improve the int32 selection logic for 16bit systems.
 *
 * Revision 1.6  1995/09/04  04:19:41  warmerda
 * Added fix for file bounds.
 *
 * Revision 1.5  1995/08/25  15:16:44  warmerda
 * Fixed a couple of problems with big endian systems ... one with bounds
 * and the other with multipart polygons.
 *
 * Revision 1.4  1995/08/24  18:10:17  warmerda
 * Switch to use SfRealloc() to avoid problems with pre-ANSI realloc()
 * functions (such as on the Sun).
 *
 * Revision 1.3  1995/08/23  02:23:15  warmerda
 * Added support for reading bounds, and fixed up problems in setting the
 * file wide bounds.
 *
 * Revision 1.2  1995/08/04  03:16:57  warmerda
 * Added header.
 *
 */

static char rcsid[] =
  "$Id: shpopen.c,v 1.39 2002/08/26 06:46:56 warmerda Exp $";

#include "shapefil.h"

#include <math.h>
#include <limits.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char uchar;

#if UINT_MAX == 65535
typedef long       int32;
#else
typedef int       int32;
#endif

#ifndef FALSE
#  define FALSE  0
#  define TRUE  1
#endif

#define ByteCopy( a, b, c ) memcpy( b, a, c )
#ifndef MAX
#  define MIN(a,b)      ((a<b) ? a : b)
#  define MAX(a,b)      ((a>b) ? a : b)
#endif

static int  bBigEndian;


/************************************************************************/
/*                              SwapWord()                              */
/*                                                                      */
/*      Swap a 2, 4 or 8 byte word.                                     */
/************************************************************************/

static void SwapWord( int length, void * wordP )

{
  int  i;
  uchar temp;

  for ( i = 0; i < length / 2; i++ )
  {
    temp = (( uchar * ) wordP )[i];
    (( uchar * )wordP )[i] = (( uchar * ) wordP )[length-i-1];
    (( uchar * ) wordP )[length-i-1] = temp;
  }
}

/************************************************************************/
/*                             SfRealloc()                              */
/*                                                                      */
/*      A realloc cover function that will access a NULL pointer as     */
/*      a valid input.                                                  */
/************************************************************************/

static void * SfRealloc( void * pMem, int nNewSize )

{
  if ( pMem == NULL )
    return(( void * ) malloc( nNewSize ) );
  else
    return(( void * ) realloc( pMem, nNewSize ) );
}

/************************************************************************/
/*                          SHPWriteHeader()                            */
/*                                                                      */
/*      Write out a header for the .shp and .shx files as well as the */
/* contents of the index (.shx) file.    */
/************************************************************************/

static void SHPWriteHeader( SHPHandle psSHP )

{
  uchar      abyHeader[100];
  int  i;
  int32 i32;
  double dValue;
  int32 *panSHX;

  /* -------------------------------------------------------------------- */
  /*      Prepare header block for .shp file.                             */
  /* -------------------------------------------------------------------- */
  for ( i = 0; i < 100; i++ )
    abyHeader[i] = 0;

  abyHeader[2] = 0x27;    /* magic cookie */
  abyHeader[3] = 0x0a;

  i32 = psSHP->nFileSize / 2;  /* file size */
  ByteCopy( &i32, abyHeader + 24, 4 );
  if ( !bBigEndian ) SwapWord( 4, abyHeader + 24 );

  i32 = 1000;      /* version */
  ByteCopy( &i32, abyHeader + 28, 4 );
  if ( bBigEndian ) SwapWord( 4, abyHeader + 28 );

  i32 = psSHP->nShapeType;    /* shape type */
  ByteCopy( &i32, abyHeader + 32, 4 );
  if ( bBigEndian ) SwapWord( 4, abyHeader + 32 );

  dValue = psSHP->adBoundsMin[0];   /* set bounds */
  ByteCopy( &dValue, abyHeader + 36, 8 );
  if ( bBigEndian ) SwapWord( 8, abyHeader + 36 );

  dValue = psSHP->adBoundsMin[1];
  ByteCopy( &dValue, abyHeader + 44, 8 );
  if ( bBigEndian ) SwapWord( 8, abyHeader + 44 );

  dValue = psSHP->adBoundsMax[0];
  ByteCopy( &dValue, abyHeader + 52, 8 );
  if ( bBigEndian ) SwapWord( 8, abyHeader + 52 );

  dValue = psSHP->adBoundsMax[1];
  ByteCopy( &dValue, abyHeader + 60, 8 );
  if ( bBigEndian ) SwapWord( 8, abyHeader + 60 );

  dValue = psSHP->adBoundsMin[2];   /* z */
  ByteCopy( &dValue, abyHeader + 68, 8 );
  if ( bBigEndian ) SwapWord( 8, abyHeader + 68 );

  dValue = psSHP->adBoundsMax[2];
  ByteCopy( &dValue, abyHeader + 76, 8 );
  if ( bBigEndian ) SwapWord( 8, abyHeader + 76 );

  dValue = psSHP->adBoundsMin[3];   /* m */
  ByteCopy( &dValue, abyHeader + 84, 8 );
  if ( bBigEndian ) SwapWord( 8, abyHeader + 84 );

  dValue = psSHP->adBoundsMax[3];
  ByteCopy( &dValue, abyHeader + 92, 8 );
  if ( bBigEndian ) SwapWord( 8, abyHeader + 92 );

  /* -------------------------------------------------------------------- */
  /*      Write .shp file header.                                         */
  /* -------------------------------------------------------------------- */
  fseek( psSHP->fpSHP, 0, 0 );
  fwrite( abyHeader, 100, 1, psSHP->fpSHP );

  /* -------------------------------------------------------------------- */
  /*      Prepare, and write .shx file header.                            */
  /* -------------------------------------------------------------------- */
  i32 = ( psSHP->nRecords * 2 * sizeof( int32 ) + 100 ) / 2;   /* file size */
  ByteCopy( &i32, abyHeader + 24, 4 );
  if ( !bBigEndian ) SwapWord( 4, abyHeader + 24 );

  fseek( psSHP->fpSHX, 0, 0 );
  fwrite( abyHeader, 100, 1, psSHP->fpSHX );

  /* -------------------------------------------------------------------- */
  /*      Write out the .shx contents.                                    */
  /* -------------------------------------------------------------------- */
  panSHX = ( int32 * ) malloc( sizeof( int32 ) * 2 * psSHP->nRecords );

  for ( i = 0; i < psSHP->nRecords; i++ )
  {
    panSHX[i*2  ] = psSHP->panRecOffset[i] / 2;
    panSHX[i*2+1] = psSHP->panRecSize[i] / 2;
    if ( !bBigEndian ) SwapWord( 4, panSHX + i*2 );
    if ( !bBigEndian ) SwapWord( 4, panSHX + i*2 + 1 );
  }

  fwrite( panSHX, sizeof( int32 ) * 2, psSHP->nRecords, psSHP->fpSHX );

  free( panSHX );
}

/************************************************************************/
/*                              SHPOpen()                               */
/*                                                                      */
/*      Open the .shp and .shx files based on the basename of the       */
/*      files or either file name.                                      */
/************************************************************************/

SHPHandle SHPAPI_CALL
SHPOpen( const char * pszLayer, const char * pszAccess )

{
  char  *pszFullname, *pszBasename;
  SHPHandle  psSHP;

  uchar  *pabyBuf;
  int   i;
  double  dValue;

  /* -------------------------------------------------------------------- */
  /*      Ensure the access string is one of the legal ones.  We          */
  /*      ensure the result string indicates binary to avoid common       */
  /*      problems on Windows.                                            */
  /* -------------------------------------------------------------------- */
  if ( strcmp( pszAccess, "rb+" ) == 0 || strcmp( pszAccess, "r+b" ) == 0
       || strcmp( pszAccess, "r+" ) == 0 )
    pszAccess = "r+b";
  else
    pszAccess = "rb";

  /* -------------------------------------------------------------------- */
  /* Establish the byte order on this machine.   */
  /* -------------------------------------------------------------------- */
  i = 1;
  if ( *(( uchar * ) &i ) == 1 )
    bBigEndian = FALSE;
  else
    bBigEndian = TRUE;

  /* -------------------------------------------------------------------- */
  /* Initialize the info structure.     */
  /* -------------------------------------------------------------------- */
  psSHP = ( SHPHandle ) calloc( sizeof( SHPInfo ), 1 );

  psSHP->bUpdated = FALSE;

  /* -------------------------------------------------------------------- */
  /* Compute the base (layer) name.  If there is any extension */
  /* on the passed in filename we will strip it off.   */
  /* -------------------------------------------------------------------- */
  pszBasename = ( char * ) malloc( strlen( pszLayer ) + 5 );
  strcpy( pszBasename, pszLayer );
  for ( i = strlen( pszBasename ) - 1;
        i > 0 && pszBasename[i] != '.' && pszBasename[i] != '/'
        && pszBasename[i] != '\\';
        i-- ) {}

  if ( pszBasename[i] == '.' )
    pszBasename[i] = '\0';

  /* -------------------------------------------------------------------- */
  /* Open the .shp and .shx files.  Note that files pulled from */
  /* a PC to Unix with upper case filenames won't work!  */
  /* -------------------------------------------------------------------- */
  pszFullname = ( char * ) malloc( strlen( pszBasename ) + 5 );
  sprintf( pszFullname, "%s.shp", pszBasename );
  psSHP->fpSHP = fopen( pszFullname, pszAccess );
  if ( psSHP->fpSHP == NULL )
  {
    sprintf( pszFullname, "%s.SHP", pszBasename );
    psSHP->fpSHP = fopen( pszFullname, pszAccess );
  }

  if ( psSHP->fpSHP == NULL )
  {
    free( psSHP );
    free( pszBasename );
    free( pszFullname );
    return( NULL );
  }

  sprintf( pszFullname, "%s.shx", pszBasename );
  psSHP->fpSHX = fopen( pszFullname, pszAccess );
  if ( psSHP->fpSHX == NULL )
  {
    sprintf( pszFullname, "%s.SHX", pszBasename );
    psSHP->fpSHX = fopen( pszFullname, pszAccess );
  }

  if ( psSHP->fpSHX == NULL )
  {
    fclose( psSHP->fpSHP );
    free( psSHP );
    free( pszBasename );
    free( pszFullname );
    return( NULL );
  }

  free( pszFullname );
  free( pszBasename );

  /* -------------------------------------------------------------------- */
  /*  Read the file size from the SHP file.    */
  /* -------------------------------------------------------------------- */
  pabyBuf = ( uchar * ) malloc( 100 );
  fread( pabyBuf, 100, 1, psSHP->fpSHP );

  psSHP->nFileSize = ( pabyBuf[24] * 256 * 256 * 256
                       + pabyBuf[25] * 256 * 256
                       + pabyBuf[26] * 256
                       + pabyBuf[27] ) * 2;

  /* -------------------------------------------------------------------- */
  /*  Read SHX file Header info                                           */
  /* -------------------------------------------------------------------- */
  fread( pabyBuf, 100, 1, psSHP->fpSHX );

  if ( pabyBuf[0] != 0
       || pabyBuf[1] != 0
       || pabyBuf[2] != 0x27
       || ( pabyBuf[3] != 0x0a && pabyBuf[3] != 0x0d ) )
  {
    fclose( psSHP->fpSHP );
    fclose( psSHP->fpSHX );
    free( psSHP );

    return( NULL );
  }

  psSHP->nRecords = pabyBuf[27] + pabyBuf[26] * 256
                    + pabyBuf[25] * 256 * 256 + pabyBuf[24] * 256 * 256 * 256;
  psSHP->nRecords = ( psSHP->nRecords * 2 - 100 ) / 8;

  psSHP->nShapeType = pabyBuf[32];

  if ( psSHP->nRecords < 0 || psSHP->nRecords > 256000000 )
  {
    /* this header appears to be corrupt.  Give up. */
    fclose( psSHP->fpSHP );
    fclose( psSHP->fpSHX );
    free( psSHP );

    return( NULL );
  }

  /* -------------------------------------------------------------------- */
  /*      Read the bounds.                                                */
  /* -------------------------------------------------------------------- */
  if ( bBigEndian ) SwapWord( 8, pabyBuf + 36 );
  memcpy( &dValue, pabyBuf + 36, 8 );
  psSHP->adBoundsMin[0] = dValue;

  if ( bBigEndian ) SwapWord( 8, pabyBuf + 44 );
  memcpy( &dValue, pabyBuf + 44, 8 );
  psSHP->adBoundsMin[1] = dValue;

  if ( bBigEndian ) SwapWord( 8, pabyBuf + 52 );
  memcpy( &dValue, pabyBuf + 52, 8 );
  psSHP->adBoundsMax[0] = dValue;

  if ( bBigEndian ) SwapWord( 8, pabyBuf + 60 );
  memcpy( &dValue, pabyBuf + 60, 8 );
  psSHP->adBoundsMax[1] = dValue;

  if ( bBigEndian ) SwapWord( 8, pabyBuf + 68 );  /* z */
  memcpy( &dValue, pabyBuf + 68, 8 );
  psSHP->adBoundsMin[2] = dValue;

  if ( bBigEndian ) SwapWord( 8, pabyBuf + 76 );
  memcpy( &dValue, pabyBuf + 76, 8 );
  psSHP->adBoundsMax[2] = dValue;

  if ( bBigEndian ) SwapWord( 8, pabyBuf + 84 );  /* z */
  memcpy( &dValue, pabyBuf + 84, 8 );
  psSHP->adBoundsMin[3] = dValue;

  if ( bBigEndian ) SwapWord( 8, pabyBuf + 92 );
  memcpy( &dValue, pabyBuf + 92, 8 );
  psSHP->adBoundsMax[3] = dValue;

  free( pabyBuf );

  /* -------------------------------------------------------------------- */
  /* Read the .shx file to get the offsets to each record in  */
  /* the .shp file.       */
  /* -------------------------------------------------------------------- */
  psSHP->nMaxRecords = psSHP->nRecords;

  psSHP->panRecOffset =
    ( int * ) malloc( sizeof( int ) * MAX( 1, psSHP->nMaxRecords ) );
  psSHP->panRecSize =
    ( int * ) malloc( sizeof( int ) * MAX( 1, psSHP->nMaxRecords ) );

  pabyBuf = ( uchar * ) malloc( 8 * MAX( 1, psSHP->nRecords ) );
  fread( pabyBuf, 8, psSHP->nRecords, psSHP->fpSHX );

  for ( i = 0; i < psSHP->nRecords; i++ )
  {
    int32  nOffset, nLength;

    memcpy( &nOffset, pabyBuf + i * 8, 4 );
    if ( !bBigEndian ) SwapWord( 4, &nOffset );

    memcpy( &nLength, pabyBuf + i * 8 + 4, 4 );
    if ( !bBigEndian ) SwapWord( 4, &nLength );

    psSHP->panRecOffset[i] = nOffset * 2;
    psSHP->panRecSize[i] = nLength * 2;
  }
  free( pabyBuf );

  return( psSHP );
}

/************************************************************************/
/*                              SHPClose()                              */
/*                */
/* Close the .shp and .shx files.     */
/************************************************************************/

void SHPAPI_CALL
SHPClose( SHPHandle psSHP )

{
  /* -------------------------------------------------------------------- */
  /* Update the header if we have modified anything.   */
  /* -------------------------------------------------------------------- */
  if ( psSHP->bUpdated )
  {
    SHPWriteHeader( psSHP );
  }

  /* -------------------------------------------------------------------- */
  /*      Free all resources, and close files.                            */
  /* -------------------------------------------------------------------- */
  free( psSHP->panRecOffset );
  free( psSHP->panRecSize );

  fclose( psSHP->fpSHX );
  fclose( psSHP->fpSHP );

  if ( psSHP->pabyRec != NULL )
  {
    free( psSHP->pabyRec );
  }

  free( psSHP );
}

/************************************************************************/
/*                             SHPGetInfo()                             */
/*                                                                      */
/*      Fetch general information about the shape file.                 */
/************************************************************************/

void SHPAPI_CALL
SHPGetInfo( SHPHandle psSHP, int * pnEntities, int * pnShapeType,
            double * padfMinBound, double * padfMaxBound )

{
  int  i;

  if ( pnEntities != NULL )
    *pnEntities = psSHP->nRecords;

  if ( pnShapeType != NULL )
    *pnShapeType = psSHP->nShapeType;

  for ( i = 0; i < 4; i++ )
  {
    if ( padfMinBound != NULL )
      padfMinBound[i] = psSHP->adBoundsMin[i];
    if ( padfMaxBound != NULL )
      padfMaxBound[i] = psSHP->adBoundsMax[i];
  }
}

/************************************************************************/
/*                             SHPCreate()                              */
/*                                                                      */
/*      Create a new shape file and return a handle to the open         */
/*      shape file with read/write access.                              */
/************************************************************************/

SHPHandle SHPAPI_CALL
SHPCreate( const char * pszLayer, int nShapeType )

{
  char *pszBasename, *pszFullname;
  int  i;
  FILE *fpSHP, *fpSHX;
  uchar      abyHeader[100];
  int32 i32;
  double dValue;

  /* -------------------------------------------------------------------- */
  /*      Establish the byte order on this system.                        */
  /* -------------------------------------------------------------------- */
  i = 1;
  if ( *(( uchar * ) &i ) == 1 )
    bBigEndian = FALSE;
  else
    bBigEndian = TRUE;

  /* -------------------------------------------------------------------- */
  /* Compute the base (layer) name.  If there is any extension */
  /* on the passed in filename we will strip it off.   */
  /* -------------------------------------------------------------------- */
  pszBasename = ( char * ) malloc( strlen( pszLayer ) + 5 );
  strcpy( pszBasename, pszLayer );
  for ( i = strlen( pszBasename ) - 1;
        i > 0 && pszBasename[i] != '.' && pszBasename[i] != '/'
        && pszBasename[i] != '\\';
        i-- ) {}

  if ( pszBasename[i] == '.' )
    pszBasename[i] = '\0';

  /* -------------------------------------------------------------------- */
  /*      Open the two files so we can write their headers.               */
  /* -------------------------------------------------------------------- */
  pszFullname = ( char * ) malloc( strlen( pszBasename ) + 5 );
  sprintf( pszFullname, "%s.shp", pszBasename );
  fpSHP = fopen( pszFullname, "wb" );
  if ( fpSHP == NULL )
    return( NULL );

  sprintf( pszFullname, "%s.shx", pszBasename );
  fpSHX = fopen( pszFullname, "wb" );
  if ( fpSHX == NULL )
    return( NULL );

  free( pszFullname );
  free( pszBasename );

  /* -------------------------------------------------------------------- */
  /*      Prepare header block for .shp file.                             */
  /* -------------------------------------------------------------------- */
  for ( i = 0; i < 100; i++ )
    abyHeader[i] = 0;

  abyHeader[2] = 0x27;    /* magic cookie */
  abyHeader[3] = 0x0a;

  i32 = 50;      /* file size */
  ByteCopy( &i32, abyHeader + 24, 4 );
  if ( !bBigEndian ) SwapWord( 4, abyHeader + 24 );

  i32 = 1000;      /* version */
  ByteCopy( &i32, abyHeader + 28, 4 );
  if ( bBigEndian ) SwapWord( 4, abyHeader + 28 );

  i32 = nShapeType;     /* shape type */
  ByteCopy( &i32, abyHeader + 32, 4 );
  if ( bBigEndian ) SwapWord( 4, abyHeader + 32 );

  dValue = 0.0;     /* set bounds */
  ByteCopy( &dValue, abyHeader + 36, 8 );
  ByteCopy( &dValue, abyHeader + 44, 8 );
  ByteCopy( &dValue, abyHeader + 52, 8 );
  ByteCopy( &dValue, abyHeader + 60, 8 );

  /* -------------------------------------------------------------------- */
  /*      Write .shp file header.                                         */
  /* -------------------------------------------------------------------- */
  fwrite( abyHeader, 100, 1, fpSHP );

  /* -------------------------------------------------------------------- */
  /*      Prepare, and write .shx file header.                            */
  /* -------------------------------------------------------------------- */
  i32 = 50;      /* file size */
  ByteCopy( &i32, abyHeader + 24, 4 );
  if ( !bBigEndian ) SwapWord( 4, abyHeader + 24 );

  fwrite( abyHeader, 100, 1, fpSHX );

  /* -------------------------------------------------------------------- */
  /*      Close the files, and then open them as regular existing files.  */
  /* -------------------------------------------------------------------- */
  fclose( fpSHP );
  fclose( fpSHX );

  return( SHPOpen( pszLayer, "r+b" ) );
}

/************************************************************************/
/*                           _SHPSetBounds()                            */
/*                                                                      */
/*      Compute a bounds rectangle for a shape, and set it into the     */
/*      indicated location in the record.                               */
/************************************************************************/

static void _SHPSetBounds( uchar * pabyRec, SHPObject * psShape )

{
  ByteCopy( &( psShape->dfXMin ), pabyRec +  0, 8 );
  ByteCopy( &( psShape->dfYMin ), pabyRec +  8, 8 );
  ByteCopy( &( psShape->dfXMax ), pabyRec + 16, 8 );
  ByteCopy( &( psShape->dfYMax ), pabyRec + 24, 8 );

  if ( bBigEndian )
  {
    SwapWord( 8, pabyRec + 0 );
    SwapWord( 8, pabyRec + 8 );
    SwapWord( 8, pabyRec + 16 );
    SwapWord( 8, pabyRec + 24 );
  }
}

/************************************************************************/
/*                         SHPComputeExtents()                          */
/*                                                                      */
/*      Recompute the extents of a shape.  Automatically done by        */
/*      SHPCreateObject().                                              */
/************************************************************************/

void SHPAPI_CALL
SHPComputeExtents( SHPObject * psObject )

{
  int  i;

  /* -------------------------------------------------------------------- */
  /*      Build extents for this object.                                  */
  /* -------------------------------------------------------------------- */
  if ( psObject->nVertices > 0 )
  {
    psObject->dfXMin = psObject->dfXMax = psObject->padfX[0];
    psObject->dfYMin = psObject->dfYMax = psObject->padfY[0];
    psObject->dfZMin = psObject->dfZMax = psObject->padfZ[0];
    psObject->dfMMin = psObject->dfMMax = psObject->padfM[0];
  }

  for ( i = 0; i < psObject->nVertices; i++ )
  {
    psObject->dfXMin = MIN( psObject->dfXMin, psObject->padfX[i] );
    psObject->dfYMin = MIN( psObject->dfYMin, psObject->padfY[i] );
    psObject->dfZMin = MIN( psObject->dfZMin, psObject->padfZ[i] );
    psObject->dfMMin = MIN( psObject->dfMMin, psObject->padfM[i] );

    psObject->dfXMax = MAX( psObject->dfXMax, psObject->padfX[i] );
    psObject->dfYMax = MAX( psObject->dfYMax, psObject->padfY[i] );
    psObject->dfZMax = MAX( psObject->dfZMax, psObject->padfZ[i] );
    psObject->dfMMax = MAX( psObject->dfMMax, psObject->padfM[i] );
  }
}

/************************************************************************/
/*                          SHPCreateObject()                           */
/*                                                                      */
/*      Create a shape object.  It should be freed with                 */
/*      SHPDestroyObject().                                             */
/************************************************************************/

SHPObject SHPAPI_CALL1( * )
SHPCreateObject( int nSHPType, int nShapeId, int nParts,
                 int * panPartStart, int * panPartType,
                 int nVertices, double * padfX, double * padfY,
                 double * padfZ, double * padfM )

{
  SHPObject *psObject;
  int  i, bHasM, bHasZ;

  psObject = ( SHPObject * ) calloc( 1, sizeof( SHPObject ) );
  psObject->nSHPType = nSHPType;
  psObject->nShapeId = nShapeId;

  /* -------------------------------------------------------------------- */
  /* Establish whether this shape type has M, and Z values.  */
  /* -------------------------------------------------------------------- */
  if ( nSHPType == SHPT_ARCM
       || nSHPType == SHPT_POINTM
       || nSHPType == SHPT_POLYGONM
       || nSHPType == SHPT_MULTIPOINTM )
  {
    bHasM = TRUE;
    bHasZ = FALSE;
  }
  else if ( nSHPType == SHPT_ARCZ
            || nSHPType == SHPT_POINTZ
            || nSHPType == SHPT_POLYGONZ
            || nSHPType == SHPT_MULTIPOINTZ
            || nSHPType == SHPT_MULTIPATCH )
  {
    bHasM = TRUE;
    bHasZ = TRUE;
  }
  else
  {
    bHasM = FALSE;
    bHasZ = FALSE;
  }

  /* -------------------------------------------------------------------- */
  /*      Capture parts.  Note that part type is optional, and            */
  /*      defaults to ring.                                               */
  /* -------------------------------------------------------------------- */
  if ( nSHPType == SHPT_ARC || nSHPType == SHPT_POLYGON
       || nSHPType == SHPT_ARCM || nSHPType == SHPT_POLYGONM
       || nSHPType == SHPT_ARCZ || nSHPType == SHPT_POLYGONZ
       || nSHPType == SHPT_MULTIPATCH )
  {
    psObject->nParts = MAX( 1, nParts );

    psObject->panPartStart = ( int * )
                             malloc( sizeof( int ) * psObject->nParts );
    psObject->panPartType = ( int * )
                            malloc( sizeof( int ) * psObject->nParts );

    psObject->panPartStart[0] = 0;
    psObject->panPartType[0] = SHPP_RING;

    for ( i = 0; i < nParts; i++ )
    {
      psObject->panPartStart[i] = panPartStart[i];
      if ( panPartType != NULL )
        psObject->panPartType[i] = panPartType[i];
      else
        psObject->panPartType[i] = SHPP_RING;
    }
  }

  /* -------------------------------------------------------------------- */
  /*      Capture vertices.  Note that Z and M are optional, but X and    */
  /*      Y are not.                                                      */
  /* -------------------------------------------------------------------- */
  if ( nVertices > 0 )
  {
    psObject->padfX = ( double * ) calloc( sizeof( double ), nVertices );
    psObject->padfY = ( double * ) calloc( sizeof( double ), nVertices );
    psObject->padfZ = ( double * ) calloc( sizeof( double ), nVertices );
    psObject->padfM = ( double * ) calloc( sizeof( double ), nVertices );

    assert( padfX != NULL );
    assert( padfY != NULL );

    for ( i = 0; i < nVertices; i++ )
    {
      psObject->padfX[i] = padfX[i];
      psObject->padfY[i] = padfY[i];
      if ( padfZ != NULL && bHasZ )
        psObject->padfZ[i] = padfZ[i];
      if ( padfM != NULL && bHasM )
        psObject->padfM[i] = padfM[i];
    }
  }

  /* -------------------------------------------------------------------- */
  /*      Compute the extents.                                            */
  /* -------------------------------------------------------------------- */
  psObject->nVertices = nVertices;
  SHPComputeExtents( psObject );

  return( psObject );
}

/************************************************************************/
/*                       SHPCreateSimpleObject()                        */
/*                                                                      */
/*      Create a simple (common) shape object.  Destroy with            */
/*      SHPDestroyObject().                                             */
/************************************************************************/

SHPObject SHPAPI_CALL1( * )
SHPCreateSimpleObject( int nSHPType, int nVertices,
                       double * padfX, double * padfY,
                       double * padfZ )

{
  return( SHPCreateObject( nSHPType, -1, 0, NULL, NULL,
                           nVertices, padfX, padfY, padfZ, NULL ) );
}

/************************************************************************/
/*                           SHPWriteObject()                           */
/*                                                                      */
/*      Write out the vertices of a new structure.  Note that it is     */
/*      only possible to write vertices at the end of the file.         */
/************************************************************************/

int SHPAPI_CALL
SHPWriteObject( SHPHandle psSHP, int nShapeId, SHPObject * psObject )

{
  int         nRecordOffset, i, nRecordSize;
  uchar *pabyRec;
  int32 i32;

  psSHP->bUpdated = TRUE;

  /* -------------------------------------------------------------------- */
  /*      Ensure that shape object matches the type of the file it is     */
  /*      being written to.                                               */
  /* -------------------------------------------------------------------- */
  assert( psObject->nSHPType == psSHP->nShapeType
          || psObject->nSHPType == SHPT_NULL );

  /* -------------------------------------------------------------------- */
  /*      Ensure that -1 is used for appends.  Either blow an             */
  /*      assertion, or if they are disabled, set the shapeid to -1       */
  /*      for appends.                                                    */
  /* -------------------------------------------------------------------- */
  assert( nShapeId == -1
          || ( nShapeId >= 0 && nShapeId < psSHP->nRecords ) );

  if ( nShapeId != -1 && nShapeId >= psSHP->nRecords )
    nShapeId = -1;

  /* -------------------------------------------------------------------- */
  /*      Add the new entity to the in memory index.                      */
  /* -------------------------------------------------------------------- */
  if ( nShapeId == -1 && psSHP->nRecords + 1 > psSHP->nMaxRecords )
  {
    psSHP->nMaxRecords = ( int )( psSHP->nMaxRecords * 1.3 + 100 );

    psSHP->panRecOffset = ( int * )
                          SfRealloc( psSHP->panRecOffset, sizeof( int ) * psSHP->nMaxRecords );
    psSHP->panRecSize = ( int * )
                        SfRealloc( psSHP->panRecSize, sizeof( int ) * psSHP->nMaxRecords );
  }

  /* -------------------------------------------------------------------- */
  /*      Initialize record.                                              */
  /* -------------------------------------------------------------------- */
  pabyRec = ( uchar * ) malloc( psObject->nVertices * 4 * sizeof( double )
                                + psObject->nParts * 8 + 128 );

  /* -------------------------------------------------------------------- */
  /*  Extract vertices for a Polygon or Arc.    */
  /* -------------------------------------------------------------------- */
  if ( psObject->nSHPType == SHPT_POLYGON
       || psObject->nSHPType == SHPT_POLYGONZ
       || psObject->nSHPType == SHPT_POLYGONM
       || psObject->nSHPType == SHPT_ARC
       || psObject->nSHPType == SHPT_ARCZ
       || psObject->nSHPType == SHPT_ARCM
       || psObject->nSHPType == SHPT_MULTIPATCH )
  {
    int32  nPoints, nParts;
    int      i;

    nPoints = psObject->nVertices;
    nParts = psObject->nParts;

    _SHPSetBounds( pabyRec + 12, psObject );

    if ( bBigEndian ) SwapWord( 4, &nPoints );
    if ( bBigEndian ) SwapWord( 4, &nParts );

    ByteCopy( &nPoints, pabyRec + 40 + 8, 4 );
    ByteCopy( &nParts, pabyRec + 36 + 8, 4 );

    nRecordSize = 52;

    /*
     * Write part start positions.
     */
    ByteCopy( psObject->panPartStart, pabyRec + 44 + 8,
              4 * psObject->nParts );
    for ( i = 0; i < psObject->nParts; i++ )
    {
      if ( bBigEndian ) SwapWord( 4, pabyRec + 44 + 8 + 4*i );
      nRecordSize += 4;
    }

    /*
     * Write multipatch part types if needed.
     */
    if ( psObject->nSHPType == SHPT_MULTIPATCH )
    {
      memcpy( pabyRec + nRecordSize, psObject->panPartType,
              4*psObject->nParts );
      for ( i = 0; i < psObject->nParts; i++ )
      {
        if ( bBigEndian ) SwapWord( 4, pabyRec + nRecordSize );
        nRecordSize += 4;
      }
    }

    /*
     * Write the (x,y) vertex values.
     */
    for ( i = 0; i < psObject->nVertices; i++ )
    {
      ByteCopy( psObject->padfX + i, pabyRec + nRecordSize, 8 );
      ByteCopy( psObject->padfY + i, pabyRec + nRecordSize + 8, 8 );

      if ( bBigEndian )
        SwapWord( 8, pabyRec + nRecordSize );

      if ( bBigEndian )
        SwapWord( 8, pabyRec + nRecordSize + 8 );

      nRecordSize += 2 * 8;
    }

    /*
     * Write the Z coordinates (if any).
     */
    if ( psObject->nSHPType == SHPT_POLYGONZ
         || psObject->nSHPType == SHPT_ARCZ
         || psObject->nSHPType == SHPT_MULTIPATCH )
    {
      ByteCopy( &( psObject->dfZMin ), pabyRec + nRecordSize, 8 );
      if ( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
      nRecordSize += 8;

      ByteCopy( &( psObject->dfZMax ), pabyRec + nRecordSize, 8 );
      if ( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
      nRecordSize += 8;

      for ( i = 0; i < psObject->nVertices; i++ )
      {
        ByteCopy( psObject->padfZ + i, pabyRec + nRecordSize, 8 );
        if ( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
        nRecordSize += 8;
      }
    }

    /*
     * Write the M values, if any.
     */
    if ( psObject->nSHPType == SHPT_POLYGONM
         || psObject->nSHPType == SHPT_ARCM
#ifndef DISABLE_MULTIPATCH_MEASURE
         || psObject->nSHPType == SHPT_MULTIPATCH
#endif
         || psObject->nSHPType == SHPT_POLYGONZ
         || psObject->nSHPType == SHPT_ARCZ )
    {
      ByteCopy( &( psObject->dfMMin ), pabyRec + nRecordSize, 8 );
      if ( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
      nRecordSize += 8;

      ByteCopy( &( psObject->dfMMax ), pabyRec + nRecordSize, 8 );
      if ( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
      nRecordSize += 8;

      for ( i = 0; i < psObject->nVertices; i++ )
      {
        ByteCopy( psObject->padfM + i, pabyRec + nRecordSize, 8 );
        if ( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
        nRecordSize += 8;
      }
    }
  }

  /* -------------------------------------------------------------------- */
  /*  Extract vertices for a MultiPoint.     */
  /* -------------------------------------------------------------------- */
  else if ( psObject->nSHPType == SHPT_MULTIPOINT
            || psObject->nSHPType == SHPT_MULTIPOINTZ
            || psObject->nSHPType == SHPT_MULTIPOINTM )
  {
    int32  nPoints;
    int      i;

    nPoints = psObject->nVertices;

    _SHPSetBounds( pabyRec + 12, psObject );

    if ( bBigEndian ) SwapWord( 4, &nPoints );
    ByteCopy( &nPoints, pabyRec + 44, 4 );

    for ( i = 0; i < psObject->nVertices; i++ )
    {
      ByteCopy( psObject->padfX + i, pabyRec + 48 + i*16, 8 );
      ByteCopy( psObject->padfY + i, pabyRec + 48 + i*16 + 8, 8 );

      if ( bBigEndian ) SwapWord( 8, pabyRec + 48 + i*16 );
      if ( bBigEndian ) SwapWord( 8, pabyRec + 48 + i*16 + 8 );
    }

    nRecordSize = 48 + 16 * psObject->nVertices;

    if ( psObject->nSHPType == SHPT_MULTIPOINTZ )
    {
      ByteCopy( &( psObject->dfZMin ), pabyRec + nRecordSize, 8 );
      if ( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
      nRecordSize += 8;

      ByteCopy( &( psObject->dfZMax ), pabyRec + nRecordSize, 8 );
      if ( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
      nRecordSize += 8;

      for ( i = 0; i < psObject->nVertices; i++ )
      {
        ByteCopy( psObject->padfZ + i, pabyRec + nRecordSize, 8 );
        if ( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
        nRecordSize += 8;
      }
    }

    if ( psObject->nSHPType == SHPT_MULTIPOINTZ
         || psObject->nSHPType == SHPT_MULTIPOINTM )
    {
      ByteCopy( &( psObject->dfMMin ), pabyRec + nRecordSize, 8 );
      if ( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
      nRecordSize += 8;

      ByteCopy( &( psObject->dfMMax ), pabyRec + nRecordSize, 8 );
      if ( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
      nRecordSize += 8;

      for ( i = 0; i < psObject->nVertices; i++ )
      {
        ByteCopy( psObject->padfM + i, pabyRec + nRecordSize, 8 );
        if ( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
        nRecordSize += 8;
      }
    }
  }

  /* -------------------------------------------------------------------- */
  /*      Write point.       */
  /* -------------------------------------------------------------------- */
  else if ( psObject->nSHPType == SHPT_POINT
            || psObject->nSHPType == SHPT_POINTZ
            || psObject->nSHPType == SHPT_POINTM )
  {
    ByteCopy( psObject->padfX, pabyRec + 12, 8 );
    ByteCopy( psObject->padfY, pabyRec + 20, 8 );

    if ( bBigEndian ) SwapWord( 8, pabyRec + 12 );
    if ( bBigEndian ) SwapWord( 8, pabyRec + 20 );

    nRecordSize = 28;

    if ( psObject->nSHPType == SHPT_POINTZ )
    {
      ByteCopy( psObject->padfZ, pabyRec + nRecordSize, 8 );
      if ( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
      nRecordSize += 8;
    }

    if ( psObject->nSHPType == SHPT_POINTZ
         || psObject->nSHPType == SHPT_POINTM )
    {
      ByteCopy( psObject->padfM, pabyRec + nRecordSize, 8 );
      if ( bBigEndian ) SwapWord( 8, pabyRec + nRecordSize );
      nRecordSize += 8;
    }
  }

  /* -------------------------------------------------------------------- */
  /*      Not much to do for null geometries.                             */
  /* -------------------------------------------------------------------- */
  else if ( psObject->nSHPType == SHPT_NULL )
  {
    nRecordSize = 12;
  }

  else
  {
    /* unknown type */
    assert( FALSE );
  }

  /* -------------------------------------------------------------------- */
  /*      Establish where we are going to put this record. If we are      */
  /*      rewriting and existing record, and it will fit, then put it     */
  /*      back where the original came from.  Otherwise write at the end. */
  /* -------------------------------------------------------------------- */
  if ( nShapeId == -1 || psSHP->panRecSize[nShapeId] < nRecordSize - 8 )
  {
    if ( nShapeId == -1 )
      nShapeId = psSHP->nRecords++;

    psSHP->panRecOffset[nShapeId] = nRecordOffset = psSHP->nFileSize;
    psSHP->panRecSize[nShapeId] = nRecordSize - 8;
    psSHP->nFileSize += nRecordSize;
  }
  else
  {
    nRecordOffset = psSHP->panRecOffset[nShapeId];
  }

  /* -------------------------------------------------------------------- */
  /*      Set the shape type, record number, and record size.             */
  /* -------------------------------------------------------------------- */
  i32 = nShapeId + 1;   /* record # */
  if ( !bBigEndian ) SwapWord( 4, &i32 );
  ByteCopy( &i32, pabyRec, 4 );

  i32 = ( nRecordSize - 8 ) / 2;    /* record size */
  if ( !bBigEndian ) SwapWord( 4, &i32 );
  ByteCopy( &i32, pabyRec + 4, 4 );

  i32 = psObject->nSHPType;    /* shape type */
  if ( bBigEndian ) SwapWord( 4, &i32 );
  ByteCopy( &i32, pabyRec + 8, 4 );

  /* -------------------------------------------------------------------- */
  /*      Write out record.                                               */
  /* -------------------------------------------------------------------- */
  if ( fseek( psSHP->fpSHP, nRecordOffset, 0 ) != 0
       || fwrite( pabyRec, nRecordSize, 1, psSHP->fpSHP ) < 1 )
  {
    printf( "Error in fseek() or fwrite().\n" );
    free( pabyRec );
    return -1;
  }

  free( pabyRec );

  /* -------------------------------------------------------------------- */
  /* Expand file wide bounds based on this shape.   */
  /* -------------------------------------------------------------------- */
  if ( psSHP->adBoundsMin[0] == 0.0
       && psSHP->adBoundsMax[0] == 0.0
       && psSHP->adBoundsMin[1] == 0.0
       && psSHP->adBoundsMax[1] == 0.0
       && psObject->nSHPType != SHPT_NULL )
  {
    psSHP->adBoundsMin[0] = psSHP->adBoundsMax[0] = psObject->padfX[0];
    psSHP->adBoundsMin[1] = psSHP->adBoundsMax[1] = psObject->padfY[0];
    psSHP->adBoundsMin[2] = psSHP->adBoundsMax[2] = psObject->padfZ[0];
    psSHP->adBoundsMin[3] = psSHP->adBoundsMax[3] = psObject->padfM[0];
  }

  for ( i = 0; i < psObject->nVertices; i++ )
  {
    psSHP->adBoundsMin[0] = MIN( psSHP->adBoundsMin[0], psObject->padfX[i] );
    psSHP->adBoundsMin[1] = MIN( psSHP->adBoundsMin[1], psObject->padfY[i] );
    psSHP->adBoundsMin[2] = MIN( psSHP->adBoundsMin[2], psObject->padfZ[i] );
    psSHP->adBoundsMin[3] = MIN( psSHP->adBoundsMin[3], psObject->padfM[i] );
    psSHP->adBoundsMax[0] = MAX( psSHP->adBoundsMax[0], psObject->padfX[i] );
    psSHP->adBoundsMax[1] = MAX( psSHP->adBoundsMax[1], psObject->padfY[i] );
    psSHP->adBoundsMax[2] = MAX( psSHP->adBoundsMax[2], psObject->padfZ[i] );
    psSHP->adBoundsMax[3] = MAX( psSHP->adBoundsMax[3], psObject->padfM[i] );
  }

  return( nShapeId );
}

/************************************************************************/
/*                          SHPReadObject()                             */
/*                                                                      */
/*      Read the vertices, parts, and other non-attribute information */
/* for one shape.       */
/************************************************************************/

SHPObject SHPAPI_CALL1( * )
SHPReadObject( SHPHandle psSHP, int hEntity )

{
  SHPObject  *psShape;

  /* -------------------------------------------------------------------- */
  /*      Validate the record/entity number.                              */
  /* -------------------------------------------------------------------- */
  if ( hEntity < 0 || hEntity >= psSHP->nRecords )
    return( NULL );

  /* -------------------------------------------------------------------- */
  /*      Ensure our record buffer is large enough.                       */
  /* -------------------------------------------------------------------- */
  if ( psSHP->panRecSize[hEntity] + 8 > psSHP->nBufSize )
  {
    psSHP->nBufSize = psSHP->panRecSize[hEntity] + 8;
    psSHP->pabyRec = ( uchar * ) SfRealloc( psSHP->pabyRec, psSHP->nBufSize );
  }

  /* -------------------------------------------------------------------- */
  /*      Read the record.                                                */
  /* -------------------------------------------------------------------- */
  fseek( psSHP->fpSHP, psSHP->panRecOffset[hEntity], 0 );
  fread( psSHP->pabyRec, psSHP->panRecSize[hEntity] + 8, 1, psSHP->fpSHP );

  /* -------------------------------------------------------------------- */
  /* Allocate and minimally initialize the object.   */
  /* -------------------------------------------------------------------- */
  psShape = ( SHPObject * ) calloc( 1, sizeof( SHPObject ) );
  psShape->nShapeId = hEntity;

  memcpy( &psShape->nSHPType, psSHP->pabyRec + 8, 4 );
  if ( bBigEndian ) SwapWord( 4, &( psShape->nSHPType ) );

  /* ==================================================================== */
  /*  Extract vertices for a Polygon or Arc.    */
  /* ==================================================================== */
  if ( psShape->nSHPType == SHPT_POLYGON || psShape->nSHPType == SHPT_ARC
       || psShape->nSHPType == SHPT_POLYGONZ
       || psShape->nSHPType == SHPT_POLYGONM
       || psShape->nSHPType == SHPT_ARCZ
       || psShape->nSHPType == SHPT_ARCM
       || psShape->nSHPType == SHPT_MULTIPATCH )
  {
    int32  nPoints, nParts;
    int      i, nOffset;

    /* -------------------------------------------------------------------- */
    /* Get the X/Y bounds.      */
    /* -------------------------------------------------------------------- */
    memcpy( &( psShape->dfXMin ), psSHP->pabyRec + 8 +  4, 8 );
    memcpy( &( psShape->dfYMin ), psSHP->pabyRec + 8 + 12, 8 );
    memcpy( &( psShape->dfXMax ), psSHP->pabyRec + 8 + 20, 8 );
    memcpy( &( psShape->dfYMax ), psSHP->pabyRec + 8 + 28, 8 );

    if ( bBigEndian ) SwapWord( 8, &( psShape->dfXMin ) );
    if ( bBigEndian ) SwapWord( 8, &( psShape->dfYMin ) );
    if ( bBigEndian ) SwapWord( 8, &( psShape->dfXMax ) );
    if ( bBigEndian ) SwapWord( 8, &( psShape->dfYMax ) );

    /* -------------------------------------------------------------------- */
    /*      Extract part/point count, and build vertex and part arrays      */
    /*      to proper size.                                                 */
    /* -------------------------------------------------------------------- */
    memcpy( &nPoints, psSHP->pabyRec + 40 + 8, 4 );
    memcpy( &nParts, psSHP->pabyRec + 36 + 8, 4 );

    if ( bBigEndian ) SwapWord( 4, &nPoints );
    if ( bBigEndian ) SwapWord( 4, &nParts );

    psShape->nVertices = nPoints;
    psShape->padfX = ( double * ) calloc( nPoints, sizeof( double ) );
    psShape->padfY = ( double * ) calloc( nPoints, sizeof( double ) );
    psShape->padfZ = ( double * ) calloc( nPoints, sizeof( double ) );
    psShape->padfM = ( double * ) calloc( nPoints, sizeof( double ) );

    psShape->nParts = nParts;
    psShape->panPartStart = ( int * ) calloc( nParts, sizeof( int ) );
    psShape->panPartType = ( int * ) calloc( nParts, sizeof( int ) );

    for ( i = 0; i < nParts; i++ )
      psShape->panPartType[i] = SHPP_RING;

    /* -------------------------------------------------------------------- */
    /*      Copy out the part array from the record.                        */
    /* -------------------------------------------------------------------- */
    memcpy( psShape->panPartStart, psSHP->pabyRec + 44 + 8, 4 * nParts );
    for ( i = 0; i < nParts; i++ )
    {
      if ( bBigEndian ) SwapWord( 4, psShape->panPartStart + i );
    }

    nOffset = 44 + 8 + 4 * nParts;

    /* -------------------------------------------------------------------- */
    /*      If this is a multipatch, we will also have parts types.         */
    /* -------------------------------------------------------------------- */
    if ( psShape->nSHPType == SHPT_MULTIPATCH )
    {
      memcpy( psShape->panPartType, psSHP->pabyRec + nOffset, 4*nParts );
      for ( i = 0; i < nParts; i++ )
      {
        if ( bBigEndian ) SwapWord( 4, psShape->panPartType + i );
      }

      nOffset += 4 * nParts;
    }

    /* -------------------------------------------------------------------- */
    /*      Copy out the vertices from the record.                          */
    /* -------------------------------------------------------------------- */
    for ( i = 0; i < nPoints; i++ )
    {
      memcpy( psShape->padfX + i,
              psSHP->pabyRec + nOffset + i * 16,
              8 );

      memcpy( psShape->padfY + i,
              psSHP->pabyRec + nOffset + i * 16 + 8,
              8 );

      if ( bBigEndian ) SwapWord( 8, psShape->padfX + i );
      if ( bBigEndian ) SwapWord( 8, psShape->padfY + i );
    }

    nOffset += 16 * nPoints;

    /* -------------------------------------------------------------------- */
    /*      If we have a Z coordinate, collect that now.                    */
    /* -------------------------------------------------------------------- */
    if ( psShape->nSHPType == SHPT_POLYGONZ
         || psShape->nSHPType == SHPT_ARCZ
         || psShape->nSHPType == SHPT_MULTIPATCH )
    {
      memcpy( &( psShape->dfZMin ), psSHP->pabyRec + nOffset, 8 );
      memcpy( &( psShape->dfZMax ), psSHP->pabyRec + nOffset + 8, 8 );

      if ( bBigEndian ) SwapWord( 8, &( psShape->dfZMin ) );
      if ( bBigEndian ) SwapWord( 8, &( psShape->dfZMax ) );

      for ( i = 0; i < nPoints; i++ )
      {
        memcpy( psShape->padfZ + i,
                psSHP->pabyRec + nOffset + 16 + i*8, 8 );
        if ( bBigEndian ) SwapWord( 8, psShape->padfZ + i );
      }

      nOffset += 16 + 8 * nPoints;
    }

    /* -------------------------------------------------------------------- */
    /*      If we have a M measure value, then read it now.  We assume      */
    /*      that the measure can be present for any shape if the size is    */
    /*      big enough, but really it will only occur for the Z shapes      */
    /*      (options), and the M shapes.                                    */
    /* -------------------------------------------------------------------- */
    if ( psSHP->panRecSize[hEntity] + 8 >= nOffset + 16 + 8*nPoints )
    {
      memcpy( &( psShape->dfMMin ), psSHP->pabyRec + nOffset, 8 );
      memcpy( &( psShape->dfMMax ), psSHP->pabyRec + nOffset + 8, 8 );

      if ( bBigEndian ) SwapWord( 8, &( psShape->dfMMin ) );
      if ( bBigEndian ) SwapWord( 8, &( psShape->dfMMax ) );

      for ( i = 0; i < nPoints; i++ )
      {
        memcpy( psShape->padfM + i,
                psSHP->pabyRec + nOffset + 16 + i*8, 8 );
        if ( bBigEndian ) SwapWord( 8, psShape->padfM + i );
      }
    }

  }

  /* ==================================================================== */
  /*  Extract vertices for a MultiPoint.     */
  /* ==================================================================== */
  else if ( psShape->nSHPType == SHPT_MULTIPOINT
            || psShape->nSHPType == SHPT_MULTIPOINTM
            || psShape->nSHPType == SHPT_MULTIPOINTZ )
  {
    int32  nPoints;
    int      i, nOffset;

    memcpy( &nPoints, psSHP->pabyRec + 44, 4 );
    if ( bBigEndian ) SwapWord( 4, &nPoints );

    psShape->nVertices = nPoints;
    psShape->padfX = ( double * ) calloc( nPoints, sizeof( double ) );
    psShape->padfY = ( double * ) calloc( nPoints, sizeof( double ) );
    psShape->padfZ = ( double * ) calloc( nPoints, sizeof( double ) );
    psShape->padfM = ( double * ) calloc( nPoints, sizeof( double ) );

    for ( i = 0; i < nPoints; i++ )
    {
      memcpy( psShape->padfX + i, psSHP->pabyRec + 48 + 16 * i, 8 );
      memcpy( psShape->padfY + i, psSHP->pabyRec + 48 + 16 * i + 8, 8 );

      if ( bBigEndian ) SwapWord( 8, psShape->padfX + i );
      if ( bBigEndian ) SwapWord( 8, psShape->padfY + i );
    }

    nOffset = 48 + 16 * nPoints;

    /* -------------------------------------------------------------------- */
    /* Get the X/Y bounds.      */
    /* -------------------------------------------------------------------- */
    memcpy( &( psShape->dfXMin ), psSHP->pabyRec + 8 +  4, 8 );
    memcpy( &( psShape->dfYMin ), psSHP->pabyRec + 8 + 12, 8 );
    memcpy( &( psShape->dfXMax ), psSHP->pabyRec + 8 + 20, 8 );
    memcpy( &( psShape->dfYMax ), psSHP->pabyRec + 8 + 28, 8 );

    if ( bBigEndian ) SwapWord( 8, &( psShape->dfXMin ) );
    if ( bBigEndian ) SwapWord( 8, &( psShape->dfYMin ) );
    if ( bBigEndian ) SwapWord( 8, &( psShape->dfXMax ) );
    if ( bBigEndian ) SwapWord( 8, &( psShape->dfYMax ) );

    /* -------------------------------------------------------------------- */
    /*      If we have a Z coordinate, collect that now.                    */
    /* -------------------------------------------------------------------- */
    if ( psShape->nSHPType == SHPT_MULTIPOINTZ )
    {
      memcpy( &( psShape->dfZMin ), psSHP->pabyRec + nOffset, 8 );
      memcpy( &( psShape->dfZMax ), psSHP->pabyRec + nOffset + 8, 8 );

      if ( bBigEndian ) SwapWord( 8, &( psShape->dfZMin ) );
      if ( bBigEndian ) SwapWord( 8, &( psShape->dfZMax ) );

      for ( i = 0; i < nPoints; i++ )
      {
        memcpy( psShape->padfZ + i,
                psSHP->pabyRec + nOffset + 16 + i*8, 8 );
        if ( bBigEndian ) SwapWord( 8, psShape->padfZ + i );
      }

      nOffset += 16 + 8 * nPoints;
    }

    /* -------------------------------------------------------------------- */
    /*      If we have a M measure value, then read it now.  We assume      */
    /*      that the measure can be present for any shape if the size is    */
    /*      big enough, but really it will only occur for the Z shapes      */
    /*      (options), and the M shapes.                                    */
    /* -------------------------------------------------------------------- */
    if ( psSHP->panRecSize[hEntity] + 8 >= nOffset + 16 + 8*nPoints )
    {
      memcpy( &( psShape->dfMMin ), psSHP->pabyRec + nOffset, 8 );
      memcpy( &( psShape->dfMMax ), psSHP->pabyRec + nOffset + 8, 8 );

      if ( bBigEndian ) SwapWord( 8, &( psShape->dfMMin ) );
      if ( bBigEndian ) SwapWord( 8, &( psShape->dfMMax ) );

      for ( i = 0; i < nPoints; i++ )
      {
        memcpy( psShape->padfM + i,
                psSHP->pabyRec + nOffset + 16 + i*8, 8 );
        if ( bBigEndian ) SwapWord( 8, psShape->padfM + i );
      }
    }
  }

  /* ==================================================================== */
  /*      Extract vertices for a point.                                   */
  /* ==================================================================== */
  else if ( psShape->nSHPType == SHPT_POINT
            || psShape->nSHPType == SHPT_POINTM
            || psShape->nSHPType == SHPT_POINTZ )
  {
    int nOffset;

    psShape->nVertices = 1;
    psShape->padfX = ( double * ) calloc( 1, sizeof( double ) );
    psShape->padfY = ( double * ) calloc( 1, sizeof( double ) );
    psShape->padfZ = ( double * ) calloc( 1, sizeof( double ) );
    psShape->padfM = ( double * ) calloc( 1, sizeof( double ) );

    memcpy( psShape->padfX, psSHP->pabyRec + 12, 8 );
    memcpy( psShape->padfY, psSHP->pabyRec + 20, 8 );

    if ( bBigEndian ) SwapWord( 8, psShape->padfX );
    if ( bBigEndian ) SwapWord( 8, psShape->padfY );

    nOffset = 20 + 8;

    /* -------------------------------------------------------------------- */
    /*      If we have a Z coordinate, collect that now.                    */
    /* -------------------------------------------------------------------- */
    if ( psShape->nSHPType == SHPT_POINTZ )
    {
      memcpy( psShape->padfZ, psSHP->pabyRec + nOffset, 8 );

      if ( bBigEndian ) SwapWord( 8, psShape->padfZ );

      nOffset += 8;
    }

    /* -------------------------------------------------------------------- */
    /*      If we have a M measure value, then read it now.  We assume      */
    /*      that the measure can be present for any shape if the size is    */
    /*      big enough, but really it will only occur for the Z shapes      */
    /*      (options), and the M shapes.                                    */
    /* -------------------------------------------------------------------- */
    if ( psSHP->panRecSize[hEntity] + 8 >= nOffset + 8 )
    {
      memcpy( psShape->padfM, psSHP->pabyRec + nOffset, 8 );

      if ( bBigEndian ) SwapWord( 8, psShape->padfM );
    }

    /* -------------------------------------------------------------------- */
    /*      Since no extents are supplied in the record, we will apply      */
    /*      them from the single vertex.                                    */
    /* -------------------------------------------------------------------- */
    psShape->dfXMin = psShape->dfXMax = psShape->padfX[0];
    psShape->dfYMin = psShape->dfYMax = psShape->padfY[0];
    psShape->dfZMin = psShape->dfZMax = psShape->padfZ[0];
    psShape->dfMMin = psShape->dfMMax = psShape->padfM[0];
  }

  return( psShape );
}

/************************************************************************/
/*                            SHPTypeName()                             */
/************************************************************************/

const char SHPAPI_CALL1( * )
SHPTypeName( int nSHPType )

{
  switch ( nSHPType )
  {
    case SHPT_NULL:
      return "NullShape";

    case SHPT_POINT:
      return "Point";

    case SHPT_ARC:
      return "Arc";

    case SHPT_POLYGON:
      return "Polygon";

    case SHPT_MULTIPOINT:
      return "MultiPoint";

    case SHPT_POINTZ:
      return "PointZ";

    case SHPT_ARCZ:
      return "ArcZ";

    case SHPT_POLYGONZ:
      return "PolygonZ";

    case SHPT_MULTIPOINTZ:
      return "MultiPointZ";

    case SHPT_POINTM:
      return "PointM";

    case SHPT_ARCM:
      return "ArcM";

    case SHPT_POLYGONM:
      return "PolygonM";

    case SHPT_MULTIPOINTM:
      return "MultiPointM";

    case SHPT_MULTIPATCH:
      return "MultiPatch";

    default:
      return "UnknownShapeType";
  }
}

/************************************************************************/
/*                          SHPPartTypeName()                           */
/************************************************************************/

const char SHPAPI_CALL1( * )
SHPPartTypeName( int nPartType )

{
  switch ( nPartType )
  {
    case SHPP_TRISTRIP:
      return "TriangleStrip";

    case SHPP_TRIFAN:
      return "TriangleFan";

    case SHPP_OUTERRING:
      return "OuterRing";

    case SHPP_INNERRING:
      return "InnerRing";

    case SHPP_FIRSTRING:
      return "FirstRing";

    case SHPP_RING:
      return "Ring";

    default:
      return "UnknownPartType";
  }
}

/************************************************************************/
/*                          SHPDestroyObject()                          */
/************************************************************************/

void SHPAPI_CALL
SHPDestroyObject( SHPObject * psShape )

{
  if ( psShape == NULL )
    return;

  if ( psShape->padfX != NULL )
    free( psShape->padfX );
  if ( psShape->padfY != NULL )
    free( psShape->padfY );
  if ( psShape->padfZ != NULL )
    free( psShape->padfZ );
  if ( psShape->padfM != NULL )
    free( psShape->padfM );

  if ( psShape->panPartStart != NULL )
    free( psShape->panPartStart );
  if ( psShape->panPartType != NULL )
    free( psShape->panPartType );

  free( psShape );
}

/************************************************************************/
/*                          SHPRewindObject()                           */
/*                                                                      */
/*      Reset the winding of polygon objects to adhere to the           */
/*      specification.                                                  */
/************************************************************************/

int SHPAPI_CALL
SHPRewindObject( SHPHandle hSHP, SHPObject * psObject )

{
  int  iOpRing, bAltered = 0;

  /* -------------------------------------------------------------------- */
  /*      Do nothing if this is not a polygon object.                     */
  /* -------------------------------------------------------------------- */
  if ( psObject->nSHPType != SHPT_POLYGON
       && psObject->nSHPType != SHPT_POLYGONZ
       && psObject->nSHPType != SHPT_POLYGONM )
    return 0;

  /* -------------------------------------------------------------------- */
  /*      Process each of the rings.                                      */
  /* -------------------------------------------------------------------- */
  for ( iOpRing = 0; iOpRing < psObject->nParts; iOpRing++ )
  {
    int      bInner, iVert, nVertCount, nVertStart, iCheckRing;
    double   dfSum, dfTestX, dfTestY;

    /* -------------------------------------------------------------------- */
    /*      Determine if this ring is an inner ring or an outer ring        */
    /*      relative to all the other rings.  For now we assume the         */
    /*      first ring is outer and all others are inner, but eventually    */
    /*      we need to fix this to handle multiple island polygons and      */
    /*      unordered sets of rings.                                        */
    /* -------------------------------------------------------------------- */
    dfTestX = psObject->padfX[psObject->panPartStart[iOpRing]];
    dfTestY = psObject->padfY[psObject->panPartStart[iOpRing]];

    bInner = FALSE;
    for ( iCheckRing = 0; iCheckRing < psObject->nParts; iCheckRing++ )
    {
      int iEdge;

      if ( iCheckRing == iOpRing )
        continue;

      nVertStart = psObject->panPartStart[iCheckRing];

      if ( iCheckRing == psObject->nParts - 1 )
        nVertCount = psObject->nVertices
                     - psObject->panPartStart[iCheckRing];
      else
        nVertCount = psObject->panPartStart[iCheckRing+1]
                     - psObject->panPartStart[iCheckRing];

      for ( iEdge = 0; iEdge < nVertCount; iEdge++ )
      {
        int iNext;

        if ( iEdge < nVertCount - 1 )
          iNext = iEdge + 1;
        else
          iNext = 0;

        if (( psObject->padfY[iEdge+nVertStart] < dfTestY
              && psObject->padfY[iNext+nVertStart] >= dfTestY )
            || ( psObject->padfY[iNext+nVertStart] < dfTestY
                 && psObject->padfY[iEdge+nVertStart] >= dfTestY ) )
        {
          if ( psObject->padfX[iEdge+nVertStart]
               + ( dfTestY - psObject->padfY[iEdge+nVertStart] )
               / ( psObject->padfY[iNext+nVertStart]
                   - psObject->padfY[iEdge+nVertStart] )
               *( psObject->padfX[iNext+nVertStart]
                  - psObject->padfX[iEdge+nVertStart] ) < dfTestX )
            bInner = !bInner;
        }
      }
    }

    /* -------------------------------------------------------------------- */
    /*      Determine the current order of this ring so we will know if     */
    /*      it has to be reversed.                                          */
    /* -------------------------------------------------------------------- */
    nVertStart = psObject->panPartStart[iOpRing];

    if ( iOpRing == psObject->nParts - 1 )
      nVertCount = psObject->nVertices - psObject->panPartStart[iOpRing];
    else
      nVertCount = psObject->panPartStart[iOpRing+1]
                   - psObject->panPartStart[iOpRing];

    dfSum = 0.0;
    for ( iVert = nVertStart; iVert < nVertStart + nVertCount - 1; iVert++ )
    {
      dfSum += psObject->padfX[iVert] * psObject->padfY[iVert+1]
               - psObject->padfY[iVert] * psObject->padfX[iVert+1];
    }

    dfSum += psObject->padfX[iVert] * psObject->padfY[nVertStart]
             - psObject->padfY[iVert] * psObject->padfX[nVertStart];

    /* -------------------------------------------------------------------- */
    /*      Reverse if necessary.                                           */
    /* -------------------------------------------------------------------- */
    if (( dfSum < 0.0 && bInner ) || ( dfSum > 0.0 && !bInner ) )
    {
      int   i;

      bAltered++;
      for ( i = 0; i < nVertCount / 2; i++ )
      {
        double dfSaved;

        /* Swap X */
        dfSaved = psObject->padfX[nVertStart+i];
        psObject->padfX[nVertStart+i] =
          psObject->padfX[nVertStart+nVertCount-i-1];
        psObject->padfX[nVertStart+nVertCount-i-1] = dfSaved;

        /* Swap Y */
        dfSaved = psObject->padfY[nVertStart+i];
        psObject->padfY[nVertStart+i] =
          psObject->padfY[nVertStart+nVertCount-i-1];
        psObject->padfY[nVertStart+nVertCount-i-1] = dfSaved;

        /* Swap Z */
        if ( psObject->padfZ )
        {
          dfSaved = psObject->padfZ[nVertStart+i];
          psObject->padfZ[nVertStart+i] =
            psObject->padfZ[nVertStart+nVertCount-i-1];
          psObject->padfZ[nVertStart+nVertCount-i-1] = dfSaved;
        }

        /* Swap M */
        if ( psObject->padfM )
        {
          dfSaved = psObject->padfM[nVertStart+i];
          psObject->padfM[nVertStart+i] =
            psObject->padfM[nVertStart+nVertCount-i-1];
          psObject->padfM[nVertStart+nVertCount-i-1] = dfSaved;
        }
      }
    }
  }

  return bAltered;
}
