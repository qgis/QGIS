#ifndef _SHAPEFILE_H_INCLUDED
#define _SHAPEFILE_H_INCLUDED

/******************************************************************************
 * $Id$
 *
 * Project:  Shapelib
 * Purpose:  Primary include file for Shapelib.
 * Author:   Frank Warmerdam, warmerda@home.com
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
 * $Log$
 * Revision 1.1  2004/04/24 00:59:39  timlinux
 * Added shapefile builder libs and start of generic shapefilemaker class that Ill eventually move into qgis core. Modified makefile to build this stuff.
 *
 * Revision 1.1  2004/04/05 15:39:45  timlinux
 * Initial commit of new plugin to build graticules - not working yet! And thus not added to higher level makefiles yet.
 *
 * Revision 1.1  2004/03/31 11:54:03  jobi
 * ** fixed small bug
 * ** renamed plugins/grid_maker/shapefil.h to shapefile.h
 *
 * Revision 1.1  2004/03/22 23:38:26  timlinux
 * This is a c++ first draft of a port of a perl script by Schuyler to import Garmin gps dump files as a shapefile. The resulting imported file will be displayed in the map view. At the moment it only generates a point layer of the waypoints but a future version will generate polylines and perhaps polygons too using similar logic to that used by Shuylers perl stuff. Note this plugin is still under construction and I am commiting it mainly so that other developers can assist me when I get stuck. Also note that the plugins Makefile builds a standalone app based on the plugin gui that can be run separately from qgis.
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
extern "C" {
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

/************************************************************************/
/*                             SHP Support.                             */
/************************************************************************/
typedef	struct
{
    FILE        *fpSHP;
    FILE	*fpSHX;

    int		nShapeType;				/* SHPT_* */
    
    int		nFileSize;				/* SHP file */

    int         nRecords;
    int		nMaxRecords;
    int		*panRecOffset;
    int		*panRecSize;

    double	adBoundsMin[4];
    double	adBoundsMax[4];

    int		bUpdated;
} SHPInfo;

typedef SHPInfo * SHPHandle;

/* -------------------------------------------------------------------- */
/*      Shape types (nSHPType)                                          */
/* -------------------------------------------------------------------- */
#define SHPT_NULL	0
#define SHPT_POINT	1
#define SHPT_ARC	3
#define SHPT_POLYGON	5
#define SHPT_MULTIPOINT	8
#define SHPT_POINTZ	11
#define SHPT_ARCZ	13
#define SHPT_POLYGONZ	15
#define SHPT_MULTIPOINTZ 18
#define SHPT_POINTM	21
#define SHPT_ARCM	23
#define SHPT_POLYGONM	25
#define SHPT_MULTIPOINTM 28
#define SHPT_MULTIPATCH 31


/* -------------------------------------------------------------------- */
/*      Part types - everything but SHPT_MULTIPATCH just uses           */
/*      SHPP_RING.                                                      */
/* -------------------------------------------------------------------- */

#define SHPP_TRISTRIP	0
#define SHPP_TRIFAN	1
#define SHPP_OUTERRING	2
#define SHPP_INNERRING	3
#define SHPP_FIRSTRING	4
#define SHPP_RING	5

/* -------------------------------------------------------------------- */
/*      SHPObject - represents on shape (without attributes) read       */
/*      from the .shp file.                                             */
/* -------------------------------------------------------------------- */
typedef struct
{
    int		nSHPType;

    int		nShapeId; /* -1 is unknown/unassigned */

    int		nParts;
    int		*panPartStart;
    int		*panPartType;
    
    int		nVertices;
    double	*padfX;
    double	*padfY;
    double	*padfZ;
    double	*padfM;

    double	dfXMin;
    double	dfYMin;
    double	dfZMin;
    double	dfMMin;

    double	dfXMax;
    double	dfYMax;
    double	dfZMax;
    double	dfMMax;
} SHPObject;

/* -------------------------------------------------------------------- */
/*      SHP API Prototypes                                              */
/* -------------------------------------------------------------------- */
SHPHandle SHPOpen( const char * pszShapeFile, const char * pszAccess );
SHPHandle SHPCreate( const char * pszShapeFile, int nShapeType );
void	SHPGetInfo( SHPHandle hSHP, int * pnEntities, int * pnShapeType,
                    double * padfMinBound, double * padfMaxBound );

SHPObject *SHPReadObject( SHPHandle hSHP, int iShape );
int	SHPWriteObject( SHPHandle hSHP, int iShape, SHPObject * psObject );

void	SHPDestroyObject( SHPObject * psObject );
void	SHPComputeExtents( SHPObject * psObject );
SHPObject *SHPCreateObject( int nSHPType, int nShapeId,
                            int nParts, int * panPartStart, int * panPartType,
                            int nVertices, double * padfX, double * padfY,
                            double * padfZ, double * padfM );
SHPObject *SHPCreateSimpleObject( int nSHPType, int nVertices,
                              double * padfX, double * padfY, double * padfZ );

void	SHPClose( SHPHandle hSHP );

const char *SHPTypeName( int nSHPType );
const char *SHPPartTypeName( int nPartType );

/* -------------------------------------------------------------------- */
/*      Shape quadtree indexing API.                                    */
/* -------------------------------------------------------------------- */

/* this can be two or four for binary or quad tree */
#define MAX_SUBNODE	4

typedef struct shape_tree_node
{
    /* region covered by this node */
    double	adfBoundsMin[4];
    double	adfBoundsMax[4];

    /* list of shapes stored at this node.  The papsShapeObj pointers
       or the whole list can be NULL */
    int		nShapeCount;
    int		*panShapeIds;
    SHPObject   **papsShapeObj;

    int		nSubNodes;
    struct shape_tree_node *apsSubNode[MAX_SUBNODE];
    
} SHPTreeNode;

typedef struct
{
    SHPHandle   hSHP;
    
    int		nMaxDepth;
    int		nDimension;
    
    SHPTreeNode	*psRoot;
} SHPTree;

SHPTree *SHPCreateTree( SHPHandle hSHP, int nDimension, int nMaxDepth,
                        double *padfBoundsMin, double *padfBoundsMax );
void     SHPDestroyTree( SHPTree * hTree );

int	SHPWriteTree( SHPTree *hTree, const char * pszFilename );
SHPTree SHPReadTree( const char * pszFilename );

int	SHPTreeAddObject( SHPTree * hTree, SHPObject * psObject );
int	SHPTreeAddShapeId( SHPTree * hTree, SHPObject * psObject );
int	SHPTreeRemoveShapeId( SHPTree * hTree, int nShapeId );

void 	SHPTreeTrimExtraNodes( SHPTree * hTree );

int    *SHPTreeFindLikelyShapes( SHPTree * hTree,
                                 double * padfBoundsMin,
                                 double * padfBoundsMax,
                                 int * );
int     SHPCheckBoundsOverlap( double *, double *, double *, double *, int );

/************************************************************************/
/*                             DBF Support.                             */
/************************************************************************/
typedef	struct
{
    FILE	*fp;

    int         nRecords;

    int		nRecordLength;
    int		nHeaderLength;
    int		nFields;
    int		*panFieldOffset;
    int		*panFieldSize;
    int		*panFieldDecimals;
    char	*pachFieldType;

    char	*pszHeader;

    int		nCurrentRecord;
    int		bCurrentRecordModified;
    char	*pszCurrentRecord;
    
    int		bNoHeader;
    int		bUpdated;
} DBFInfo;

typedef DBFInfo * DBFHandle;

typedef enum {
  FTString,
  FTInteger,
  FTDouble,
  FTInvalid
} DBFFieldType;

#define XBASE_FLDHDR_SZ       32

DBFHandle DBFOpen( const char * pszDBFFile, const char * pszAccess );
DBFHandle DBFCreate( const char * pszDBFFile );

int	DBFGetFieldCount( DBFHandle psDBF );
int	DBFGetRecordCount( DBFHandle psDBF );
int	DBFAddField( DBFHandle hDBF, const char * pszFieldName,
		     DBFFieldType eType, int nWidth, int nDecimals );

DBFFieldType DBFGetFieldInfo( DBFHandle psDBF, int iField, 
			      char * pszFieldName, 
			      int * pnWidth, int * pnDecimals );

int 	DBFReadIntegerAttribute( DBFHandle hDBF, int iShape, int iField );
double 	DBFReadDoubleAttribute( DBFHandle hDBF, int iShape, int iField );
const char *DBFReadStringAttribute( DBFHandle hDBF, int iShape, int iField );

int DBFWriteIntegerAttribute( DBFHandle hDBF, int iShape, int iField, 
			      int nFieldValue );
int DBFWriteDoubleAttribute( DBFHandle hDBF, int iShape, int iField,
			     double dFieldValue );
int DBFWriteStringAttribute( DBFHandle hDBF, int iShape, int iField,
			     const char * pszFieldValue );

const char *DBFReadTuple(DBFHandle psDBF, int hEntity );
int DBFWriteTuple(DBFHandle psDBF, int hEntity, void * pRawTuple );

DBFHandle DBFCloneEmpty(DBFHandle psDBF, const char * pszFilename );
 
void	DBFClose( DBFHandle hDBF );

#ifdef __cplusplus
}
#endif

#endif /* ndef _SHAPEFILE_H_INCLUDED */
