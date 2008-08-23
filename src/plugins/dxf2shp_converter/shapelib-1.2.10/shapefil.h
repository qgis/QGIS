#ifndef _SHAPEFILE_H_INCLUDED
#define _SHAPEFILE_H_INCLUDED

/******************************************************************************
 * $Id: shapefil.h,v 1.26 2002/09/29 00:00:08 warmerda Exp $
 *
 * Project:  Shapelib
 * Purpose:  Primary include file for Shapelib.
 * Author:   Frank Warmerdam, warmerdam@pobox.com
 *
 ******************************************************************************
 * Copyright (c) 1999, Frank Warmerdam
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
 * $Log: shapefil.h,v $
 * Revision 1.26  2002/09/29 00:00:08  warmerda
 * added FTLogical and logical attribute read/write calls
 *
 * Revision 1.25  2002/05/07 13:46:30  warmerda
 * added DBFWriteAttributeDirectly().
 *
 * Revision 1.24  2002/04/10 16:59:54  warmerda
 * added SHPRewindObject
 *
 * Revision 1.23  2002/01/15 14:36:07  warmerda
 * updated email address
 *
 * Revision 1.22  2002/01/15 14:32:00  warmerda
 * try to improve SHPAPI_CALL docs
 *
 * Revision 1.21  2001/11/01 16:29:55  warmerda
 * move pabyRec into SHPInfo for thread safety
 *
 * Revision 1.20  2001/07/20 13:06:02  warmerda
 * fixed SHPAPI attribute for SHPTreeFindLikelyShapes
 *
 * Revision 1.19  2001/05/31 19:20:13  warmerda
 * added DBFGetFieldIndex()
 *
 * Revision 1.18  2001/05/31 18:15:40  warmerda
 * Added support for NULL fields in DBF files
 *
 * Revision 1.17  2001/05/23 13:36:52  warmerda
 * added use of SHPAPI_CALL
 *
 * Revision 1.16  2000/09/25 14:15:59  warmerda
 * added DBFGetNativeFieldType()
 *
 * Revision 1.15  2000/02/16 16:03:51  warmerda
 * added null shape support
 *
 * Revision 1.14  1999/11/05 14:12:05  warmerda
 * updated license terms
 *
 * Revision 1.13  1999/06/02 18:24:21  warmerda
 * added trimming code
 *
 * Revision 1.12  1999/06/02 17:56:12  warmerda
 * added quad'' subnode support for trees
 *
 * Revision 1.11  1999/05/18 19:11:11  warmerda
 * Added example searching capability
 *
 * Revision 1.10  1999/05/18 17:49:38  warmerda
 * added initial quadtree support
 *
 * Revision 1.9  1999/05/11 03:19:28  warmerda
 * added new Tuple api, and improved extension handling - add from candrsn
 *
 * Revision 1.8  1999/03/23 17:22:27  warmerda
 * Added extern "C" protection for C++ users of shapefil.h.
 *
 * Revision 1.7  1998/12/31 15:31:07  warmerda
 * Added the TRIM_DBF_WHITESPACE and DISABLE_MULTIPATCH_MEASURE options.
 *
 * Revision 1.6  1998/12/03 15:48:15  warmerda
 * Added SHPCalculateExtents().
 *
 * Revision 1.5  1998/11/09 20:57:16  warmerda
 * Altered SHPGetInfo() call.
 *
 * Revision 1.4  1998/11/09 20:19:33  warmerda
 * Added 3D support, and use of SHPObject.
 *
 * Revision 1.3  1995/08/23 02:24:05  warmerda
 * Added support for reading bounds.
 *
 * Revision 1.2  1995/08/04  03:17:39  warmerda
 * Added header.
 *
 */

#include <stdio.h>

#ifdef USE_DBMALLOC
#include <dbmalloc.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif

  /************************************************************************/
  /*                        Configuration options.                        */
  /************************************************************************/

  /* -------------------------------------------------------------------- */
  /*      Should the DBFReadStringAttribute() strip leading and           */
  /*      trailing white space?                                           */
  /* -------------------------------------------------------------------- */
#define TRIM_DBF_WHITESPACE

  /* -------------------------------------------------------------------- */
  /*      Should we write measure values to the Multipatch object?        */
  /*      Reportedly ArcView crashes if we do write it, so for now it     */
  /*      is disabled.                                                    */
  /* -------------------------------------------------------------------- */
#define DISABLE_MULTIPATCH_MEASURE

  /* -------------------------------------------------------------------- */
  /*      SHPAPI_CALL                                                     */
  /*                                                                      */
  /*      The following two macros are present to allow forcing           */
  /*      various calling conventions on the Shapelib API.                */
  /*                                                                      */
  /*      To force __stdcall conventions (needed to call Shapelib         */
  /*      from Visual Basic and/or Dephi I believe) the makefile could    */
  /*      be modified to define:                                          */
  /*                                                                      */
  /*        /DSHPAPI_CALL=__stdcall                                       */
  /*                                                                      */
  /*      If it is desired to force export of the Shapelib API without    */
  /*      using the shapelib.def file, use the following definition.      */
  /*                                                                      */
  /*        /DSHAPELIB_DLLEXPORT                                          */
  /*                                                                      */
  /*      To get both at once it will be necessary to hack this           */
  /*      include file to define:                                         */
  /*                                                                      */
  /*        #define SHPAPI_CALL __declspec(dllexport) __stdcall           */
  /*        #define SHPAPI_CALL1 __declspec(dllexport) * __stdcall        */
  /*                                                                      */
  /*      The complexity of the situtation is partly caused by the        */
  /*      peculiar requirement of Visual C++ that __stdcall appear        */
  /*      after any "*"'s in the return value of a function while the     */
  /*      __declspec(dllexport) must appear before them.                  */
  /* -------------------------------------------------------------------- */

#ifdef SHAPELIB_DLLEXPORT
#  define SHPAPI_CALL __declspec(dllexport)
#  define SHPAPI_CALL1(x)  __declspec(dllexport) x
#endif

#ifndef SHPAPI_CALL
#  define SHPAPI_CALL
#endif

#ifndef SHPAPI_CALL1
#  define SHPAPI_CALL1(x)      x SHPAPI_CALL
#endif

  /************************************************************************/
  /*                             SHP Support.                             */
  /************************************************************************/
  typedef struct
  {
    FILE        *fpSHP;
    FILE *fpSHX;

    int  nShapeType;    /* SHPT_* */

    int  nFileSize;    /* SHP file */

    int         nRecords;
    int  nMaxRecords;
    int  *panRecOffset;
    int  *panRecSize;

    double adBoundsMin[4];
    double adBoundsMax[4];

    int  bUpdated;

    unsigned char *pabyRec;
    int         nBufSize;
  } SHPInfo;

  typedef SHPInfo * SHPHandle;

  /* -------------------------------------------------------------------- */
  /*      Shape types (nSHPType)                                          */
  /* -------------------------------------------------------------------- */
#define SHPT_NULL 0
#define SHPT_POINT 1
#define SHPT_ARC 3
#define SHPT_POLYGON 5
#define SHPT_MULTIPOINT 8
#define SHPT_POINTZ 11
#define SHPT_ARCZ 13
#define SHPT_POLYGONZ 15
#define SHPT_MULTIPOINTZ 18
#define SHPT_POINTM 21
#define SHPT_ARCM 23
#define SHPT_POLYGONM 25
#define SHPT_MULTIPOINTM 28
#define SHPT_MULTIPATCH 31


  /* -------------------------------------------------------------------- */
  /*      Part types - everything but SHPT_MULTIPATCH just uses           */
  /*      SHPP_RING.                                                      */
  /* -------------------------------------------------------------------- */

#define SHPP_TRISTRIP 0
#define SHPP_TRIFAN 1
#define SHPP_OUTERRING 2
#define SHPP_INNERRING 3
#define SHPP_FIRSTRING 4
#define SHPP_RING 5

  /* -------------------------------------------------------------------- */
  /*      SHPObject - represents on shape (without attributes) read       */
  /*      from the .shp file.                                             */
  /* -------------------------------------------------------------------- */
  typedef struct
  {
    int  nSHPType;

    int  nShapeId; /* -1 is unknown/unassigned */

    int  nParts;
    int  *panPartStart;
    int  *panPartType;

    int  nVertices;
    double *padfX;
    double *padfY;
    double *padfZ;
    double *padfM;

    double dfXMin;
    double dfYMin;
    double dfZMin;
    double dfMMin;

    double dfXMax;
    double dfYMax;
    double dfZMax;
    double dfMMax;
  } SHPObject;

  /* -------------------------------------------------------------------- */
  /*      SHP API Prototypes                                              */
  /* -------------------------------------------------------------------- */
  SHPHandle SHPAPI_CALL
  SHPOpen( const char * pszShapeFile, const char * pszAccess );
  SHPHandle SHPAPI_CALL
  SHPCreate( const char * pszShapeFile, int nShapeType );
  void SHPAPI_CALL
  SHPGetInfo( SHPHandle hSHP, int * pnEntities, int * pnShapeType,
              double * padfMinBound, double * padfMaxBound );

  SHPObject SHPAPI_CALL1( * )
  SHPReadObject( SHPHandle hSHP, int iShape );
  int SHPAPI_CALL
  SHPWriteObject( SHPHandle hSHP, int iShape, SHPObject * psObject );

  void SHPAPI_CALL
  SHPDestroyObject( SHPObject * psObject );
  void SHPAPI_CALL
  SHPComputeExtents( SHPObject * psObject );
  SHPObject SHPAPI_CALL1( * )
  SHPCreateObject( int nSHPType, int nShapeId,
                   int nParts, int * panPartStart, int * panPartType,
                   int nVertices, double * padfX, double * padfY,
                   double * padfZ, double * padfM );
  SHPObject SHPAPI_CALL1( * )
  SHPCreateSimpleObject( int nSHPType, int nVertices,
                         double * padfX, double * padfY, double * padfZ );

  int SHPAPI_CALL
  SHPRewindObject( SHPHandle hSHP, SHPObject * psObject );

  void SHPAPI_CALL
  SHPClose( SHPHandle hSHP );

  const char SHPAPI_CALL1( * )
  SHPTypeName( int nSHPType );
  const char SHPAPI_CALL1( * )
  SHPPartTypeName( int nPartType );

  /* -------------------------------------------------------------------- */
  /*      Shape quadtree indexing API.                                    */
  /* -------------------------------------------------------------------- */

  /* this can be two or four for binary or quad tree */
#define MAX_SUBNODE 4

  typedef struct shape_tree_node
  {
    /* region covered by this node */
    double adfBoundsMin[4];
    double adfBoundsMax[4];

    /* list of shapes stored at this node.  The papsShapeObj pointers
       or the whole list can be NULL */
    int  nShapeCount;
    int  *panShapeIds;
    SHPObject   **papsShapeObj;

    int  nSubNodes;
    struct shape_tree_node *apsSubNode[MAX_SUBNODE];

  } SHPTreeNode;

  typedef struct
  {
    SHPHandle   hSHP;

    int  nMaxDepth;
    int  nDimension;

    SHPTreeNode *psRoot;
  } SHPTree;

  SHPTree SHPAPI_CALL1( * )
  SHPCreateTree( SHPHandle hSHP, int nDimension, int nMaxDepth,
                 double *padfBoundsMin, double *padfBoundsMax );
  void    SHPAPI_CALL
  SHPDestroyTree( SHPTree * hTree );

  int SHPAPI_CALL
  SHPWriteTree( SHPTree *hTree, const char * pszFileName );
  SHPTree SHPAPI_CALL
  SHPReadTree( const char * pszFileName );

  int SHPAPI_CALL
  SHPTreeAddObject( SHPTree * hTree, SHPObject * psObject );
  int SHPAPI_CALL
  SHPTreeAddShapeId( SHPTree * hTree, SHPObject * psObject );
  int SHPAPI_CALL
  SHPTreeRemoveShapeId( SHPTree * hTree, int nShapeId );

  void  SHPAPI_CALL
  SHPTreeTrimExtraNodes( SHPTree * hTree );

  int    SHPAPI_CALL1( * )
  SHPTreeFindLikelyShapes( SHPTree * hTree,
                           double * padfBoundsMin,
                           double * padfBoundsMax,
                           int * );
  int     SHPAPI_CALL
  SHPCheckBoundsOverlap( double *, double *, double *, double *, int );

  /************************************************************************/
  /*                             DBF Support.                             */
  /************************************************************************/
  typedef struct
  {
    FILE *fp;

    int         nRecords;

    int  nRecordLength;
    int  nHeaderLength;
    int  nFields;
    int  *panFieldOffset;
    int  *panFieldSize;
    int  *panFieldDecimals;
    char *pachFieldType;

    char *pszHeader;

    int  nCurrentRecord;
    int  bCurrentRecordModified;
    char *pszCurrentRecord;

    int  bNoHeader;
    int  bUpdated;
  } DBFInfo;

  typedef DBFInfo * DBFHandle;

  typedef enum
  {
    FTString,
    FTInteger,
    FTDouble,
    FTLogical,
    FTInvalid
  } DBFFieldType;

#define XBASE_FLDHDR_SZ       32

  DBFHandle SHPAPI_CALL
  DBFOpen( const char * pszDBFFile, const char * pszAccess );
  DBFHandle SHPAPI_CALL
  DBFCreate( const char * pszDBFFile );

  int SHPAPI_CALL
  DBFGetFieldCount( DBFHandle psDBF );
  int SHPAPI_CALL
  DBFGetRecordCount( DBFHandle psDBF );
  int SHPAPI_CALL
  DBFAddField( DBFHandle hDBF, const char * pszFieldName,
               DBFFieldType eType, int nWidth, int nDecimals );

  DBFFieldType SHPAPI_CALL
  DBFGetFieldInfo( DBFHandle psDBF, int iField,
                   char * pszFieldName, int * pnWidth, int * pnDecimals );

  int SHPAPI_CALL
  DBFGetFieldIndex( DBFHandle psDBF, const char *pszFieldName );

  int  SHPAPI_CALL
  DBFReadIntegerAttribute( DBFHandle hDBF, int iShape, int iField );
  double  SHPAPI_CALL
  DBFReadDoubleAttribute( DBFHandle hDBF, int iShape, int iField );
  const char SHPAPI_CALL1( * )
  DBFReadStringAttribute( DBFHandle hDBF, int iShape, int iField );
  const char SHPAPI_CALL1( * )
  DBFReadLogicalAttribute( DBFHandle hDBF, int iShape, int iField );
  int     SHPAPI_CALL
  DBFIsAttributeNULL( DBFHandle hDBF, int iShape, int iField );

  int SHPAPI_CALL
  DBFWriteIntegerAttribute( DBFHandle hDBF, int iShape, int iField,
                            int nFieldValue );
  int SHPAPI_CALL
  DBFWriteDoubleAttribute( DBFHandle hDBF, int iShape, int iField,
                           double dFieldValue );
  int SHPAPI_CALL
  DBFWriteStringAttribute( DBFHandle hDBF, int iShape, int iField,
                           const char * pszFieldValue );
  int SHPAPI_CALL
  DBFWriteNULLAttribute( DBFHandle hDBF, int iShape, int iField );

  int SHPAPI_CALL
  DBFWriteLogicalAttribute( DBFHandle hDBF, int iShape, int iField,
                            const char lFieldValue );
  int SHPAPI_CALL
  DBFWriteAttributeDirectly( DBFHandle psDBF, int hEntity, int iField,
                             void * pValue );
  const char SHPAPI_CALL1( * )
  DBFReadTuple( DBFHandle psDBF, int hEntity );
  int SHPAPI_CALL
  DBFWriteTuple( DBFHandle psDBF, int hEntity, void * pRawTuple );

  DBFHandle SHPAPI_CALL
  DBFCloneEmpty( DBFHandle psDBF, const char * pszFileName );

  void SHPAPI_CALL
  DBFClose( DBFHandle hDBF );
  char    SHPAPI_CALL
  DBFGetNativeFieldType( DBFHandle hDBF, int iField );

#ifdef __cplusplus
}
#endif

#endif /* ndef _SHAPEFILE_H_INCLUDED */
