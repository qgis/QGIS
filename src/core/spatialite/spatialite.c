/******************************************************************************
** This file is an amalgamation of many separate C source files from SpatiaLite
** version 2.3.0.  By combining all the individual C code files into this
** single large file, the entire code can be compiled as a one translation
** unit.  This allows many compilers to do optimizations that would not be
** possible if the files were compiled separately.  Performance improvements
** of 5% are more are commonly seen when SQLite is compiled as a single
** translation unit.
**
** This amalgamation was generated on 2009-04-11 15:32:35 +0200.

Author: Alessandro (Sandro) Furieri <a.furieri@lqt.it>

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
Luigi Costalli luigi.costalli@gmail.com

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <memory.h>
#include <limits.h>
#include <math.h>
#include <float.h>
#include <locale.h>
#include <errno.h>

#include "sqlite3ext.h"

#if defined(__MINGW32__) || defined(_WIN32)
#define LIBICONV_STATIC
#include <iconv.h>
#define LIBCHARSET_STATIC
#include <localcharset.h>
#else /* not WINDOWS */
#ifdef __APPLE__
#include <iconv.h>
#include <localcharset.h>
#else /* not Mac OsX */
#include <iconv.h>
#include <langinfo.h>
#endif
#endif

#if OMIT_GEOS == 0		/* including GEOS */
#include <geos_c.h>
#endif

#if OMIT_PROJ == 0		/* including PROJ.4 */
#include <proj_api.h>
#endif

#ifdef _WIN32
#define strcasecmp	stricmp
#define strncasecmp	strnicmp
#define atoll	_atoi64
#endif /* not WIN32 */


/**************** Begin file: spatialite.h **********/

#ifdef DLL_EXPORT
#define SPATIALITE_DECLARE __declspec(dllexport)
#else
#define SPATIALITE_DECLARE extern
#endif

#ifndef _SPATIALITE_H
#define _SPATIALITE_H

#ifdef __cplusplus
extern "C"
{
#endif

    SPATIALITE_DECLARE const char *spatialite_version (void);
    SPATIALITE_DECLARE const char *virtualtext_version (void);
    SPATIALITE_DECLARE void spatialite_init (int verbose);
    SPATIALITE_DECLARE int dump_shapefile (sqlite3 * sqlite, char *table,
					   char *column, char *charset,
					   char *shp_path, char *geom_type,
					   int verbose, int *rows);
    SPATIALITE_DECLARE int load_shapefile (sqlite3 * sqlite, char *shp_path,
					   char *table, char *charset, int srid,
					   char *column, int verbose,
					   int *rows);
    SPATIALITE_DECLARE double math_round (double value);
    SPATIALITE_DECLARE sqlite3_int64 math_llabs (sqlite3_int64 value);

#ifdef __cplusplus
}
#endif

#endif				/* _SPATIALITE_H */
/**************** End file: spatialite.h **********/


/**************** Begin file: gaiaaux.h **********/

#ifdef DLL_EXPORT
#define GAIAAUX_DECLARE __declspec(dllexport)
#else
#define GAIAAUX_DECLARE extern
#endif

#ifndef _GAIAAUX_H
#define _GAIAAUX_H

#ifdef __cplusplus
extern "C"
{
#endif

/* function prototipes */

    GAIAAUX_DECLARE const char *gaiaGetLocaleCharset ();
    GAIAAUX_DECLARE int gaiaConvertCharset (char **buf, const char *fromCs,
					    const char *toCs);
    GAIAAUX_DECLARE int gaiaToUTF8 (char **buf, const char *fromCs,
				    const char *toCs);
    GAIAAUX_DECLARE void *gaiaCreateUTF8Converter (const char *fromCS);
    GAIAAUX_DECLARE void gaiaFreeUTF8Converter (void *cvtCS);
    GAIAAUX_DECLARE char *gaiaConvertToUTF8 (void *cvtCS, const char *buf,
					     int len, int *err);
    GAIAAUX_DECLARE int gaiaIsReservedSqliteName (const char *name);
    GAIAAUX_DECLARE int gaiaIsReservedSqlName (const char *name);
    GAIAAUX_DECLARE int gaiaIllegalSqlName (const char *name);
    GAIAAUX_DECLARE void gaiaCleanSqlString (char *value);

#ifdef __cplusplus
}
#endif

#endif				/* _GAIAAUX_H */
/**************** End file: gaiaaux.h **********/


/**************** Begin file: gaiaexif.h **********/

#ifdef DLL_EXPORT
#define GAIAEXIF_DECLARE __declspec(dllexport)
#else
#define GAIAEXIF_DECLARE extern
#endif

#ifndef _GAIAEXIF_H
#define _GAIAEXIF_H

#ifdef __cplusplus
extern "C"
{
#endif

/* constants used for BLOB value types */
#define GAIA_HEX_BLOB		0
#define GAIA_GIF_BLOB		1
#define GAIA_PNG_BLOB		2
#define GAIA_JPEG_BLOB		3
#define GAIA_EXIF_BLOB		4
#define GAIA_EXIF_GPS_BLOB	5
#define GAIA_ZIP_BLOB		6
#define GAIA_PDF_BLOB		7
#define GAIA_GEOMETRY_BLOB	8

/* constants used for EXIF value types */
#define GAIA_EXIF_NONE		0
#define GAIA_EXIF_BYTE		1
#define GAIA_EXIF_SHORT		2
#define GAIA_EXIF_STRING	3
#define GAIA_EXIF_LONG		4
#define GAIA_EXIF_RATIONAL	5
#define GAIA_EXIF_SLONG		9
#define GAIA_EXIF_SRATIONAL	10

    typedef struct gaiaExifTagStruct
    {
/* an EXIF TAG */
	char Gps;
	unsigned short TagId;
	unsigned short Type;
	unsigned short Count;
	unsigned char TagOffset[4];
	unsigned char *ByteValue;
	char *StringValue;
	unsigned short *ShortValues;
	unsigned int *LongValues;
	unsigned int *LongRationals1;
	unsigned int *LongRationals2;
	short *SignedShortValues;
	int *SignedLongValues;
	int *SignedLongRationals1;
	int *SignedLongRationals2;
	float *FloatValues;
	double *DoubleValues;
	struct gaiaExifTagStruct *Next;
    } gaiaExifTag;
    typedef gaiaExifTag *gaiaExifTagPtr;

    typedef struct gaiaExifTagListStruct
    {
/* an EXIF TAG LIST */
	gaiaExifTagPtr First;
	gaiaExifTagPtr Last;
	int NumTags;
	gaiaExifTagPtr *TagsArray;
    } gaiaExifTagList;
    typedef gaiaExifTagList *gaiaExifTagListPtr;

/* function prototipes */

    GAIAEXIF_DECLARE gaiaExifTagListPtr gaiaGetExifTags (const unsigned char
							 *blob, int size);
    GAIAEXIF_DECLARE void gaiaExifTagsFree (gaiaExifTagListPtr tag_list);
    GAIAEXIF_DECLARE int gaiaGetExifTagsCount (gaiaExifTagListPtr tag_list);
    GAIAEXIF_DECLARE gaiaExifTagPtr gaiaGetExifTagByPos (gaiaExifTagListPtr
							 tag_list,
							 const int pos);
    GAIAEXIF_DECLARE gaiaExifTagPtr gaiaGetExifTagById (const gaiaExifTagListPtr
							tag_list,
							const unsigned short
							tag_id);
    GAIAEXIF_DECLARE gaiaExifTagPtr gaiaGetExifGpsTagById (const
							   gaiaExifTagListPtr
							   tag_list,
							   const unsigned short
							   tag_id);
    GAIAEXIF_DECLARE gaiaExifTagPtr gaiaGetExifTagByName (const
							  gaiaExifTagListPtr
							  tag_list,
							  const char *tag_name);
    GAIAEXIF_DECLARE unsigned short gaiaExifTagGetId (const gaiaExifTagPtr tag);
    GAIAEXIF_DECLARE void gaiaExifTagGetName (const gaiaExifTagPtr tag,
					      char *tag_name, int len);
    GAIAEXIF_DECLARE int gaiaIsExifGpsTag (const gaiaExifTagPtr tag);
    GAIAEXIF_DECLARE unsigned short gaiaExifTagGetValueType (const
							     gaiaExifTagPtr
							     tag);
    GAIAEXIF_DECLARE unsigned short gaiaExifTagGetNumValues (const
							     gaiaExifTagPtr
							     tag);
    GAIAEXIF_DECLARE unsigned char gaiaExifTagGetByteValue (const gaiaExifTagPtr
							    tag, const int ind,
							    int *ok);
    GAIAEXIF_DECLARE void gaiaExifTagGetStringValue (const gaiaExifTagPtr tag,
						     char *str, int len,
						     int *ok);
    GAIAEXIF_DECLARE unsigned short gaiaExifTagGetShortValue (const
							      gaiaExifTagPtr
							      tag,
							      const int ind,
							      int *ok);
    GAIAEXIF_DECLARE unsigned int gaiaExifTagGetLongValue (const gaiaExifTagPtr
							   tag, const int ind,
							   int *ok);
    GAIAEXIF_DECLARE unsigned int gaiaExifTagGetRational1Value (const
								gaiaExifTagPtr
								tag,
								const int ind,
								int *ok);
    GAIAEXIF_DECLARE unsigned int gaiaExifTagGetRational2Value (const
								gaiaExifTagPtr
								tag,
								const int ind,
								int *ok);
    GAIAEXIF_DECLARE double gaiaExifTagGetRationalValue (const gaiaExifTagPtr
							 tag, const int ind,
							 int *ok);
    GAIAEXIF_DECLARE short gaiaExifTagGetSignedShortValue (const gaiaExifTagPtr
							   tag, const int ind,
							   int *ok);
    GAIAEXIF_DECLARE int gaiaExifTagGetSignedLongValue (const gaiaExifTagPtr
							tag, const int ind,
							int *ok);
    GAIAEXIF_DECLARE int gaiaExifTagGetSignedRational1Value (const
							     gaiaExifTagPtr tag,
							     const int ind,
							     int *ok);
    GAIAEXIF_DECLARE int gaiaExifTagGetSignedRational2Value (const
							     gaiaExifTagPtr tag,
							     const int ind,
							     int *ok);
    GAIAEXIF_DECLARE double gaiaExifTagGetSignedRationalValue (const
							       gaiaExifTagPtr
							       tag,
							       const int ind,
							       int *ok);
    GAIAEXIF_DECLARE float gaiaExifTagGetFloatValue (const gaiaExifTagPtr tag,
						     const int ind, int *ok);
    GAIAEXIF_DECLARE double gaiaExifTagGetDoubleValue (const gaiaExifTagPtr tag,
						       const int ind, int *ok);
    GAIAEXIF_DECLARE void gaiaExifTagGetHumanReadable (const gaiaExifTagPtr tag,
						       char *str, int len,
						       int *ok);
    GAIAEXIF_DECLARE int gaiaGuessBlobType (const unsigned char *blob,
					    int size);
    GAIAEXIF_DECLARE int gaiaGetGpsCoords (const unsigned char *blob, int size,
					   double *longitude, double *latitude);
    GAIAEXIF_DECLARE int gaiaGetGpsLatLong (const unsigned char *blob, int size,
					    char *latlong, int ll_size);

#ifdef __cplusplus
}
#endif

#endif				/* _GAIAEXIF_H */
/**************** End file: gaiaexif.h **********/


/**************** Begin file: gaiageo.h **********/

/* stdio.h included for FILE objects. */
/* #include <stdio.h> */

#ifdef DLL_EXPORT
#define GAIAGEO_DECLARE __declspec(dllexport)
#else
#define GAIAGEO_DECLARE extern
#endif

#ifndef _GAIAGEO_H
#define _GAIAGEO_H

#ifdef __cplusplus
extern "C"
{
#endif

/* constant values for generic geometry classes */
#define GAIA_TYPE_NONE		0
#define GAIA_TYPE_POINT		1
#define GAIA_TYPE_LINESTRING	2
#define GAIA_TYPE_POLYGON	3

/* constants that defines byte storage order  */
#define GAIA_BIG_ENDIAN		0
#define GAIA_LITTLE_ENDIAN	1

/* constants that defines special markers used for encoding of SpatiaLite internal BLOB geometries  */
#define GAIA_MARK_START		0x00
#define GAIA_MARK_END		0xFE
#define GAIA_MARK_MBR		0x7C
#define GAIA_MARK_ENTITY	0x69

/* constants that defines GEOMETRY CLASSes */
#define GAIA_UNKNOWN		0
#define GAIA_POINT		1
#define GAIA_LINESTRING		2
#define GAIA_POLYGON		3
#define GAIA_MULTIPOINT		4
#define GAIA_MULTILINESTRING	5
#define GAIA_MULTIPOLYGON	6
#define GAIA_GEOMETRYCOLLECTION	7

/* constants that defines token codes for WKT parsing */
#define GAIA_COORDINATE		8
#define GAIA_OPENED		9
#define GAIA_CLOSED		10
#define GAIA_COMMA		11
#define GAIA_SPACE		12

/* constants that defines multitype values */
#define GAIA_NULL_VALUE		0
#define GAIA_TEXT_VALUE		1
#define GAIA_INT_VALUE		2
#define GAIA_DOUBLE_VALUE	3

/* constants that defines POINT index for LINESTRING */
#define GAIA_START_POINT	1
#define GAIA_END_POINT		2
#define GAIA_POINTN		3

/* constants that defines MBRs spatial relationships */
#define GAIA_MBR_CONTAINS	1
#define GAIA_MBR_DISJOINT	2
#define GAIA_MBR_EQUAL		3
#define GAIA_MBR_INTERSECTS	4
#define GAIA_MBR_OVERLAPS	5
#define GAIA_MBR_TOUCHES	6
#define GAIA_MBR_WITHIN		7

/* constants used for FilterMBR */
#define GAIA_FILTER_MBR_WITHIN			74
#define GAIA_FILTER_MBR_CONTAINS		77
#define GAIA_FILTER_MBR_INTERSECTS		79
#define GAIA_FILTER_MBR_DECLARE			89

/* constants defining SVG default values */
#define GAIA_SVG_DEFAULT_RELATIVE 0
#define GAIA_SVG_DEFAULT_PRECISION 6
#define GAIA_SVG_DEFAULT_MAX_PRECISION 15

/* constants used for VirtualNetwork */
#define GAIA_NET_START	0x67
#define GAIA_NET_END	0x87
#define GAIA_NET_HEADER	0xc0
#define GAIA_NET_CODE	0xa6
#define GAIA_NET_ID		0xb5
#define GAIA_NET_NODE	0xde
#define GAIA_NET_ARC	0x54
#define GAIA_NET_TABLE	0xa0
#define GAIA_NET_FROM	0xa1
#define GAIA_NET_TO		0xa2
#define GAIA_NET_GEOM	0xa3
#define GAIA_NET_BLOCK	0xed

/* constants used for Coordinate Dimensions */
#define GAIA_XY		0x00
#define GAIA_XY_Z	0x01
#define GAIA_XY_M	0x02
#define GAIA_XY_Z_M	0x03

/* macros */
#define gaiaGetPoint(xy,v,x,y)	{*x = xy[(v) * 2]; \
				 *y = xy[(v) * 2 + 1];}

#define gaiaSetPoint(xy,v,x,y)	{xy[(v) * 2] = x; \
				 xy[(v) * 2 + 1] = y;}

    typedef struct gaiaPointStruct
    {
/* an OpenGis POINT */
	double X;		/* X,Y coordinates */
	double Y;
	struct gaiaPointStruct *Next;	/* for double-linked list */
	struct gaiaPointStruct *Prev;	/* for double-linked list */
    } gaiaPoint;
    typedef gaiaPoint *gaiaPointPtr;

    typedef struct gaiaDynamicLineStruct
    {
/* a generic DYNAMIC LINE object */
	gaiaPointPtr First;	/* Points linked list - first */
	gaiaPointPtr Last;	/* Points linked list - last */
    } gaiaDynamicLine;
    typedef gaiaDynamicLine *gaiaDynamicLinePtr;

    typedef struct gaiaLinestringStruct
    {
/* an OpenGis LINESTRING */
	int Points;		/* number of vertices */
	double *Coords;		/* X,Y [vertices] array */
	double MinX;		/* MBR - BBOX */
	double MinY;		/* MBR - BBOX */
	double MaxX;		/* MBR - BBOX */
	double MaxY;		/* MBR - BBOX */
	struct gaiaLinestringStruct *Next;	/* for linked list */
    } gaiaLinestring;
    typedef gaiaLinestring *gaiaLinestringPtr;

    typedef struct gaiaRingStruct
    {
/* a GIS ring - OpenGis LINESTRING, closed */
	int Points;		/* number of vertices */
	double *Coords;		/* X,Y [vertices] array */
	int Clockwise;		/* clockwise / counterclockwise */
	double MinX;		/* MBR - BBOX */
	double MinY;		/* MBR - BBOX */
	double MaxX;		/* MBR - BBOX */
	double MaxY;		/* MBR - BBOX */
	struct gaiaRingStruct *Next;	/* for linked list */
	struct gaiaPolygonStruct *Link;	/* polygon reference */
    } gaiaRing;
    typedef gaiaRing *gaiaRingPtr;

    typedef struct gaiaPolygonStruct
    {
/* an OpenGis POLYGON */
	gaiaRingPtr Exterior;	/* exterior ring */
	int NumInteriors;	/* number of interior rings */
	gaiaRingPtr Interiors;	/* interior rings array */
	int NextInterior;	/* first free interior ring */
	double MinX;		/* MBR - BBOX */
	double MinY;		/* MBR - BBOX */
	double MaxX;		/* MBR - BBOX */
	double MaxY;		/* MBR - BBOX */
	struct gaiaPolygonStruct *Next;	/* for linked list */
    } gaiaPolygon;
    typedef gaiaPolygon *gaiaPolygonPtr;

    typedef struct gaiaGeomCollStruct
    {
/* OpenGis GEOMETRYCOLLECTION */
	int Srid;		/* the SRID value for this GEOMETRY */
	char endian_arch;	/* littleEndian - bigEndian arch for targer CPU */
	char endian;		/* littleEndian - bigEndian */
	const unsigned char *blob;	/* WKB encoded buffer */
	unsigned long size;	/* buffer size */
	unsigned long offset;	/* current offset [for parsing] */
	gaiaPointPtr FirstPoint;	/* Points linked list - first */
	gaiaPointPtr LastPoint;	/* Points linked list - last */
	gaiaLinestringPtr FirstLinestring;	/* Linestrings linked list - first */
	gaiaLinestringPtr LastLinestring;	/* Linestrings linked list - last */
	gaiaPolygonPtr FirstPolygon;	/* Polygons linked list - first */
	gaiaPolygonPtr LastPolygon;	/* Polygons linked list - last */
	double MinX;		/* MBR - BBOX */
	double MinY;		/* MBR - BBOX */
	double MaxX;		/* MBR - BBOX */
	double MaxY;		/* MBR - BBOX */
	int DeclaredType;	/* the declared TYPE for this Geometry */
    } gaiaGeomColl;
    typedef gaiaGeomColl *gaiaGeomCollPtr;

    typedef struct gaiaPreRingStruct
    {
/* a LINESTRING used to build rings */
	gaiaLinestringPtr Line;	/* a LINESTRING pointer */
	int AlreadyUsed;	/* a switch to mark an already used line element */
	struct gaiaPreRingStruct *Next;	/* for linked list */
    } gaiaPreRing;
    typedef gaiaPreRing *gaiaPreRingPtr;

    typedef struct gaiaValueStruct
    {
/* a DBF field multitype value */
	short Type;		/* the type */
	char *TxtValue;		/* the text value */
	sqlite3_int64 IntValue;	/* the integer value */
	double DblValue;	/* the double value */
    } gaiaValue;
    typedef gaiaValue *gaiaValuePtr;

    typedef struct gaiaDbfFieldStruct
    {
/* a DBF field definition - shapefile attribute */
	char *Name;		/* field name */
	unsigned char Type;	/* field type */
	int Offset;		/* buffer offset [this field begins at *buffer+offset* and extends for *length* bytes */
	unsigned char Length;	/* field total lenght [in bytes] */
	unsigned char Decimals;	/* decimal positions */
	gaiaValuePtr Value;	/* the current multitype value for this attribute */
	struct gaiaDbfFieldStruct *Next;	/* pointer to next element in linked list */
    } gaiaDbfField;
    typedef gaiaDbfField *gaiaDbfFieldPtr;

    typedef struct gaiaDbfListStruct
    {
/* a linked list to containt the DBF fields definitions - shapefile attributes */
	int RowId;		/* the current RowId */
	gaiaGeomCollPtr Geometry;	/* geometry for current entity */
	gaiaDbfFieldPtr First;	/* pointer to first element in linked list */
	gaiaDbfFieldPtr Last;	/* pointer to last element in linker list */
    } gaiaDbfList;
    typedef gaiaDbfList *gaiaDbfListPtr;

    typedef struct gaiaShapefileStruct
    {
/* SHAPEFILE TYPE */
	int endian_arch;
	int Valid;		/* 1 = ready to process */
	int ReadOnly;		/* read or wite mode */
	char *Path;		/* the shapefile abstract path [no suffixes] */
	FILE *flShx;		/* the SHX file handle */
	FILE *flShp;		/* the SHP file handle */
	FILE *flDbf;		/* the DBF file handle */
	int Shape;		/* the SHAPE code for the whole shapefile */
	gaiaDbfListPtr Dbf;	/* the DBF attributes list */
	unsigned char *BufDbf;	/* the DBF I/O buffer */
	int DbfHdsz;		/* the DBF header length */
	int DbfReclen;		/* the DBF record length */
	int DbfSize;		/* current DBF size */
	int DbfRecno;		/* current DBF record number */
	unsigned char *BufShp;	/* the SHP I/O buffer */
	int ShpBfsz;		/* the SHP buffer current size */
	int ShpSize;		/* current SHP size */
	int ShxSize;		/* current SHX size */
	double MinX;		/* the MBR/BBOX for the whole shapefile */
	double MinY;
	double MaxX;
	double MaxY;
	void *IconvObj;		/* opaque reference to ICONV converter */
	char *LastError;	/* last error message */
	unsigned char EffectiveType;	/* the effective Geometry-type, as determined by gaiaShpAnalyze() */
    } gaiaShapefile;
    typedef gaiaShapefile *gaiaShapefilePtr;

/* function prototipes */

    GAIAGEO_DECLARE int gaiaEndianArch (void);
    GAIAGEO_DECLARE short gaiaImport16 (const unsigned char *p,
					int little_endian,
					int little_endian_arch);
    GAIAGEO_DECLARE int gaiaImport32 (const unsigned char *p, int little_endian,
				      int little_endian_arch);
    GAIAGEO_DECLARE double gaiaImport64 (const unsigned char *p,
					 int little_endian,
					 int little_endian_arch);
    GAIAGEO_DECLARE void gaiaExport16 (unsigned char *p, short value,
				       int little_endian,
				       int little_endian_arch);
    GAIAGEO_DECLARE void gaiaExport32 (unsigned char *p, int value,
				       int little_endian,
				       int little_endian_arch);
    GAIAGEO_DECLARE void gaiaExport64 (unsigned char *p, double value,
				       int little_endian,
				       int little_endian_arch);
    GAIAGEO_DECLARE gaiaPointPtr gaiaAllocPoint (double x, double y);
    GAIAGEO_DECLARE void gaiaFreePoint (gaiaPointPtr ptr);
    GAIAGEO_DECLARE gaiaLinestringPtr gaiaAllocLinestring (int vert);
    GAIAGEO_DECLARE void gaiaFreeLinestring (gaiaLinestringPtr ptr);
    GAIAGEO_DECLARE gaiaRingPtr gaiaAllocRing (int vert);
    GAIAGEO_DECLARE void gaiaFreeRing (gaiaRingPtr ptr);
    GAIAGEO_DECLARE gaiaPolygonPtr gaiaAllocPolygon (int vert, int excl);
    GAIAGEO_DECLARE gaiaPolygonPtr gaiaCreatePolygon (gaiaRingPtr ring);
    GAIAGEO_DECLARE void gaiaFreePolygon (gaiaPolygonPtr p);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaAllocGeomColl (void);
    GAIAGEO_DECLARE void gaiaFreeGeomColl (gaiaGeomCollPtr p);
    GAIAGEO_DECLARE void gaiaAddPointToGeomColl (gaiaGeomCollPtr p, double x,
						 double y);
    GAIAGEO_DECLARE void gaiaMbrLinestring (gaiaLinestringPtr line);
    GAIAGEO_DECLARE void gaiaMbrRing (gaiaRingPtr rng);
    GAIAGEO_DECLARE void gaiaMbrPolygon (gaiaPolygonPtr polyg);
    GAIAGEO_DECLARE void gaiaMbrGeometry (gaiaGeomCollPtr geom);
    GAIAGEO_DECLARE gaiaLinestringPtr
	gaiaAddLinestringToGeomColl (gaiaGeomCollPtr p, int vert);
    GAIAGEO_DECLARE void gaiaInsertLinestringInGeomColl (gaiaGeomCollPtr p,
							 gaiaLinestringPtr
							 line);
    GAIAGEO_DECLARE gaiaPolygonPtr gaiaAddPolygonToGeomColl (gaiaGeomCollPtr p,
							     int vert,
							     int interiors);
    GAIAGEO_DECLARE gaiaPolygonPtr gaiaInsertPolygonInGeomColl (gaiaGeomCollPtr
								p,
								gaiaRingPtr
								ring);
    GAIAGEO_DECLARE gaiaRingPtr gaiaAddInteriorRing (gaiaPolygonPtr p, int pos,
						     int vert);
    GAIAGEO_DECLARE void gaiaInsertInteriorRing (gaiaPolygonPtr p,
						 gaiaRingPtr ring);
    GAIAGEO_DECLARE void gaiaAddRingToPolyg (gaiaPolygonPtr polyg,
					     gaiaRingPtr ring);
    GAIAGEO_DECLARE gaiaDynamicLinePtr gaiaAllocDynamicLine (void);
    GAIAGEO_DECLARE void gaiaFreeDynamicLine (gaiaDynamicLinePtr p);
    GAIAGEO_DECLARE gaiaPointPtr
	gaiaAppendPointToDynamicLine (gaiaDynamicLinePtr p, double x, double y);
    GAIAGEO_DECLARE gaiaPointPtr
	gaiaPrependPointToDynamicLine (gaiaDynamicLinePtr p, double x,
				       double y);
    GAIAGEO_DECLARE gaiaPointPtr gaiaDynamicLineInsertAfter (gaiaDynamicLinePtr
							     p, gaiaPointPtr pt,
							     double x,
							     double y);
    GAIAGEO_DECLARE gaiaPointPtr gaiaDynamicLineInsertBefore (gaiaDynamicLinePtr
							      p,
							      gaiaPointPtr pt,
							      double x,
							      double y);
    GAIAGEO_DECLARE void gaiaDynamicLineDeletePoint (gaiaDynamicLinePtr p,
						     gaiaPointPtr pt);
    GAIAGEO_DECLARE gaiaDynamicLinePtr gaiaCloneDynamicLine (gaiaDynamicLinePtr
							     org);
    GAIAGEO_DECLARE gaiaDynamicLinePtr
	gaiaReverseDynamicLine (gaiaDynamicLinePtr org);
    GAIAGEO_DECLARE gaiaDynamicLinePtr
	gaiaDynamicLineSplitBefore (gaiaDynamicLinePtr org, gaiaPointPtr point);
    GAIAGEO_DECLARE gaiaDynamicLinePtr
	gaiaDynamicLineSplitAfter (gaiaDynamicLinePtr org, gaiaPointPtr point);
    GAIAGEO_DECLARE gaiaDynamicLinePtr
	gaiaDynamicLineJoinAfter (gaiaDynamicLinePtr org, gaiaPointPtr point,
				  gaiaDynamicLinePtr toJoin);
    GAIAGEO_DECLARE gaiaDynamicLinePtr
	gaiaDynamicLineJoinBefore (gaiaDynamicLinePtr org, gaiaPointPtr point,
				   gaiaDynamicLinePtr toJoin);
    GAIAGEO_DECLARE gaiaPointPtr gaiaDynamicLineFindByCoords (gaiaDynamicLinePtr
							      p, double x,
							      double y);
    GAIAGEO_DECLARE gaiaPointPtr gaiaDynamicLineFindByPos (gaiaDynamicLinePtr p,
							   int pos);
    GAIAGEO_DECLARE gaiaDynamicLinePtr gaiaCreateDynamicLine (double *coords,
							      int points);
    GAIAGEO_DECLARE double gaiaMeasureLength (double *coords, int vert);
    GAIAGEO_DECLARE double gaiaMeasureArea (gaiaRingPtr ring);
    GAIAGEO_DECLARE void gaiaRingCentroid (gaiaRingPtr ring, double *rx,
					   double *ry);
    GAIAGEO_DECLARE void gaiaClockwise (gaiaRingPtr p);
    GAIAGEO_DECLARE int gaiaIsPointOnRingSurface (gaiaRingPtr ring, double pt_x,
						  double pt_y);
    GAIAGEO_DECLARE double gaiaMinDistance (double x0, double y0,
					    double *coords, int n_vert);
    GAIAGEO_DECLARE int gaiaIsPointOnPolygonSurface (gaiaPolygonPtr polyg,
						     double x, double y);
    GAIAGEO_DECLARE int gaiaIntersect (double *x0, double *y0, double x1,
				       double y1, double x2, double y2,
				       double x3, double y3, double x4,
				       double y4);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaFromSpatiaLiteBlobWkb (const unsigned
							       char *blob,
							       unsigned int
							       size);
    GAIAGEO_DECLARE void gaiaToSpatiaLiteBlobWkb (gaiaGeomCollPtr geom,
						  unsigned char **result,
						  int *size);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaFromSpatiaLiteBlobMbr (const unsigned
							       char *blob,
							       unsigned int
							       size);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaFromWkb (const unsigned char *blob,
						 unsigned int size);
    GAIAGEO_DECLARE void gaiaToWkb (gaiaGeomCollPtr geom,
				    unsigned char **result, int *size);
    GAIAGEO_DECLARE int gaiaFromWkbNoCheck (const unsigned char *in,
					    unsigned int szin,
					    unsigned char **out, int *szout,
					    int srid);
    GAIAGEO_DECLARE int gaiaToWkbNoCheck (const unsigned char *in,
					  unsigned int szin,
					  unsigned char **out, int *szout);
    GAIAGEO_DECLARE char *gaiaToHexWkb (gaiaGeomCollPtr geom);
    GAIAGEO_DECLARE void gaiaFreeValue (gaiaValuePtr p);
    GAIAGEO_DECLARE void gaiaSetNullValue (gaiaDbfFieldPtr field);
    GAIAGEO_DECLARE void gaiaSetIntValue (gaiaDbfFieldPtr field,
					  sqlite3_int64 value);
    GAIAGEO_DECLARE void gaiaSetDoubleValue (gaiaDbfFieldPtr field,
					     double value);
    GAIAGEO_DECLARE void gaiaSetStrValue (gaiaDbfFieldPtr field, char *str);
    GAIAGEO_DECLARE gaiaDbfFieldPtr gaiaAllocDbfField (char *name,
						       unsigned char type,
						       int offset,
						       unsigned char length,
						       unsigned char decimals);
    GAIAGEO_DECLARE void gaiaFreeDbfField (gaiaDbfFieldPtr p);
    GAIAGEO_DECLARE gaiaDbfFieldPtr gaiaCloneDbfField (gaiaDbfFieldPtr org);
    GAIAGEO_DECLARE gaiaDbfListPtr gaiaAllocDbfList (void);
    GAIAGEO_DECLARE void gaiaFreeDbfList (gaiaDbfListPtr list);
    GAIAGEO_DECLARE int gaiaIsValidDbfList (gaiaDbfListPtr list);
    GAIAGEO_DECLARE gaiaDbfFieldPtr gaiaAddDbfField (gaiaDbfListPtr list,
						     char *name,
						     unsigned char type,
						     int offset,
						     unsigned char length,
						     unsigned char decimals);
    GAIAGEO_DECLARE void gaiaResetDbfEntity (gaiaDbfListPtr list);
    GAIAGEO_DECLARE gaiaValuePtr gaiaCloneValue (gaiaValuePtr org);
    GAIAGEO_DECLARE gaiaDbfListPtr gaiaCloneDbfEntity (gaiaDbfListPtr org);
    GAIAGEO_DECLARE gaiaShapefilePtr gaiaAllocShapefile (void);
    GAIAGEO_DECLARE void gaiaFreeShapefile (gaiaShapefilePtr shp);
    GAIAGEO_DECLARE void gaiaOpenShpRead (gaiaShapefilePtr shp,
					  const char *path,
					  const char *charFrom,
					  const char *charTo);
    GAIAGEO_DECLARE void gaiaOpenShpWrite (gaiaShapefilePtr shp,
					   const char *path, int shape,
					   gaiaDbfListPtr list,
					   const char *charFrom,
					   const char *charTo);
    GAIAGEO_DECLARE int gaiaReadShpEntity (gaiaShapefilePtr shp,
					   int current_row, int srid);
    GAIAGEO_DECLARE void gaiaShpAnalyze (gaiaShapefilePtr shp);
    GAIAGEO_DECLARE int gaiaWriteShpEntity (gaiaShapefilePtr shp,
					    gaiaDbfListPtr entity);
    GAIAGEO_DECLARE void gaiaFlushShpHeaders (gaiaShapefilePtr shp);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaParseWkt (const unsigned char
						  *dirty_buffer, short type);
    GAIAGEO_DECLARE void gaiaOutWkt (gaiaGeomCollPtr geom, char **result);
    GAIAGEO_DECLARE void gaiaOutSvg (gaiaGeomCollPtr geom, char **result,
				     int relative, int precision);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaFromFgf (const unsigned char *blob,
						 unsigned int size);
    GAIAGEO_DECLARE void gaiaToFgf (gaiaGeomCollPtr geom,
				    unsigned char **result, int *size,
				    int coord_dims);
    GAIAGEO_DECLARE int gaiaDimension (gaiaGeomCollPtr geom);
    GAIAGEO_DECLARE int gaiaGeometryType (gaiaGeomCollPtr geom);
    GAIAGEO_DECLARE int gaiaGeometryAliasType (gaiaGeomCollPtr geom);
    GAIAGEO_DECLARE int gaiaIsEmpty (gaiaGeomCollPtr geom);
    GAIAGEO_DECLARE int gaiaMbrsContains (gaiaGeomCollPtr mbr1,
					  gaiaGeomCollPtr mbr2);
    GAIAGEO_DECLARE int gaiaMbrsDisjoint (gaiaGeomCollPtr mbr1,
					  gaiaGeomCollPtr mbr2);
    GAIAGEO_DECLARE int gaiaMbrsEqual (gaiaGeomCollPtr mbr1,
				       gaiaGeomCollPtr mbr2);
    GAIAGEO_DECLARE int gaiaMbrsIntersects (gaiaGeomCollPtr mbr1,
					    gaiaGeomCollPtr mbr2);
    GAIAGEO_DECLARE int gaiaMbrsOverlaps (gaiaGeomCollPtr mbr1,
					  gaiaGeomCollPtr mbr2);
    GAIAGEO_DECLARE int gaiaMbrsTouches (gaiaGeomCollPtr mbr1,
					 gaiaGeomCollPtr mbr2);
    GAIAGEO_DECLARE int gaiaMbrsWithin (gaiaGeomCollPtr mbr1,
					gaiaGeomCollPtr mbr2);
    GAIAGEO_DECLARE void gaiaShiftCoords (gaiaGeomCollPtr geom, double shift_x,
					  double shift_y);
    GAIAGEO_DECLARE void gaiaScaleCoords (gaiaGeomCollPtr geom, double scale_x,
					  double scale_y);
    GAIAGEO_DECLARE void gaiaRotateCoords (gaiaGeomCollPtr geom, double angle);
    GAIAGEO_DECLARE void gaiaReflectCoords (gaiaGeomCollPtr geom, int x_axis,
					    int y_axis);
    GAIAGEO_DECLARE void gaiaSwapCoords (gaiaGeomCollPtr geom);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaCloneGeomColl (gaiaGeomCollPtr geom);
    GAIAGEO_DECLARE gaiaLinestringPtr gaiaCloneLinestring (gaiaLinestringPtr
							   line);
    GAIAGEO_DECLARE gaiaRingPtr gaiaCloneRing (gaiaRingPtr ring);
    GAIAGEO_DECLARE gaiaPolygonPtr gaiaClonePolygon (gaiaPolygonPtr polyg);
    GAIAGEO_DECLARE int gaiaLinestringEquals (gaiaLinestringPtr line1,
					      gaiaLinestringPtr line2);
    GAIAGEO_DECLARE int gaiaPolygonEquals (gaiaPolygonPtr geom1,
					   gaiaPolygonPtr geom2);
    GAIAGEO_DECLARE void gaiaMakePoint (double x, double y, int srid,
					unsigned char **result, int *size);
    GAIAGEO_DECLARE void gaiaBuildMbr (double x1, double y1, double x2,
				       double y2, int srid,
				       unsigned char **result, int *size);
    GAIAGEO_DECLARE void gaiaBuildFilterMbr (double x1, double y1, double x2,
					     double y2, int mode,
					     unsigned char **result, int *size);
    GAIAGEO_DECLARE int gaiaParseFilterMbr (unsigned char *result, int size,
					    double *minx, double *miny,
					    double *maxx, double *maxy,
					    int *mode);
    GAIAGEO_DECLARE void gaiaBuildCircleMbr (double x, double y, double radius,
					     int srid, unsigned char **result,
					     int *size);
    GAIAGEO_DECLARE int gaiaGetMbrMinX (const unsigned char *blob,
					unsigned int size, double *minx);
    GAIAGEO_DECLARE int gaiaGetMbrMaxX (const unsigned char *blob,
					unsigned int size, double *maxx);
    GAIAGEO_DECLARE int gaiaGetMbrMinY (const unsigned char *blob,
					unsigned int size, double *miny);
    GAIAGEO_DECLARE int gaiaGetMbrMaxY (const unsigned char *blob,
					unsigned int size, double *maxy);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaBuildRings (gaiaGeomCollPtr geom);

#if OMIT_PROJ == 0		/* including PROJ.4 */

    GAIAGEO_DECLARE double gaiaRadsToDegs (double rads);
    GAIAGEO_DECLARE double gaiaDegsToRads (double degs);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaTransform (gaiaGeomCollPtr org,
						   char *proj_from,
						   char *proj_to);

#endif				/* end including PROJ.4 */

#if OMIT_GEOS == 0		/* including GEOS */

    GAIAGEO_DECLARE int gaiaGeomCollEquals (gaiaGeomCollPtr geom1,
					    gaiaGeomCollPtr geom2);
    GAIAGEO_DECLARE int gaiaGeomCollDisjoint (gaiaGeomCollPtr geom1,
					      gaiaGeomCollPtr geom2);
    GAIAGEO_DECLARE int gaiaGeomCollIntersects (gaiaGeomCollPtr geom1,
						gaiaGeomCollPtr geom2);
    GAIAGEO_DECLARE int gaiaGeomCollOverlaps (gaiaGeomCollPtr geom1,
					      gaiaGeomCollPtr geom2);
    GAIAGEO_DECLARE int gaiaGeomCollCrosses (gaiaGeomCollPtr geom1,
					     gaiaGeomCollPtr geom2);
    GAIAGEO_DECLARE int gaiaGeomCollContains (gaiaGeomCollPtr geom1,
					      gaiaGeomCollPtr geom2);
    GAIAGEO_DECLARE int gaiaGeomCollWithin (gaiaGeomCollPtr geom1,
					    gaiaGeomCollPtr geom2);
    GAIAGEO_DECLARE int gaiaGeomCollTouches (gaiaGeomCollPtr geom1,
					     gaiaGeomCollPtr geom2);
    GAIAGEO_DECLARE int gaiaGeomCollRelate (gaiaGeomCollPtr geom1,
					    gaiaGeomCollPtr geom2,
					    const char *pattern);
    GAIAGEO_DECLARE int gaiaGeomCollDistance (gaiaGeomCollPtr geom1,
					      gaiaGeomCollPtr geom2,
					      double *dist);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaGeometryIntersection (gaiaGeomCollPtr
							      geom1,
							      gaiaGeomCollPtr
							      geom2);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaGeometryUnion (gaiaGeomCollPtr geom1,
						       gaiaGeomCollPtr geom2);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaGeometryDifference (gaiaGeomCollPtr
							    geom1,
							    gaiaGeomCollPtr
							    geom2);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaGeometrySymDifference (gaiaGeomCollPtr
							       geom1,
							       gaiaGeomCollPtr
							       geom2);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaBoundary (gaiaGeomCollPtr geom);
    GAIAGEO_DECLARE int gaiaGeomCollCentroid (gaiaGeomCollPtr geom, double *x,
					      double *y);
    GAIAGEO_DECLARE int gaiaGetPointOnSurface (gaiaGeomCollPtr geom, double *x,
					       double *y);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaGeomCollSimplify (gaiaGeomCollPtr geom,
							  double tolerance);
    GAIAGEO_DECLARE gaiaGeomCollPtr
	gaiaGeomCollSimplifyPreserveTopology (gaiaGeomCollPtr geom,
					      double tolerance);
    GAIAGEO_DECLARE int gaiaGeomCollLength (gaiaGeomCollPtr geom,
					    double *length);
    GAIAGEO_DECLARE int gaiaGeomCollArea (gaiaGeomCollPtr geom, double *area);
    GAIAGEO_DECLARE int gaiaIsSimple (gaiaGeomCollPtr geom);
    GAIAGEO_DECLARE int gaiaIsClosed (gaiaLinestringPtr line);
    GAIAGEO_DECLARE int gaiaIsRing (gaiaLinestringPtr line);
    GAIAGEO_DECLARE int gaiaIsValid (gaiaGeomCollPtr geom);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaConvexHull (gaiaGeomCollPtr geom);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaGeomCollBuffer (gaiaGeomCollPtr geom,
							double radius,
							int points);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaPolygonize (gaiaGeomCollPtr geom,
						    int force_multipolygon);

#endif				/* end including GEOS */

#ifdef __cplusplus
}
#endif

#endif				/* _GAIAGEO_H */
/**************** End file: gaiageo.h **********/


/**************** Begin file: spatialite.h **********/

int virtualshape_extension_init (sqlite3 * db);
int virtualtext_extension_init (sqlite3 * db);
int virtualnetwork_extension_init (sqlite3 * db);
int virtualfdo_extension_init (sqlite3 * db);
int mbrcache_extension_init (sqlite3 * db);
/**************** End file: spatialite.h **********/


/**************** Begin file: gg_sqlaux.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <string.h> */

/* #include <spatialite/sqlite3ext.h> */
/* #include <spatialite/gaiageo.h> */

GAIAGEO_DECLARE int
gaiaIllegalSqlName (const char *name)
{
/* checks if column-name is an SQL illegal name */
    int i;
    int len;
    if (!name)
	return 1;
    len = strlen (name);
    if (len == 0)
	return 1;
    for (i = 0; i < len; i++)
      {
	  if (name[i] >= 'a' && name[i] <= 'z')
	      continue;
	  if (name[i] >= 'A' && name[i] <= 'Z')
	      continue;
	  if (name[i] >= '0' && name[i] <= '9')
	      continue;
	  if (name[i] == '_')
	      continue;
	  /* the name contains an illegal char */
	  return 1;
      }
    if (name[0] >= 'a' && name[0] <= 'z')
	return 0;
    if (name[0] >= 'A' && name[0] <= 'Z')
	return 0;
/* the first char in the name isn't a letter */
    return 1;
}

GAIAGEO_DECLARE int
gaiaIsReservedSqliteName (const char *name)
{
/* checks if column-name is an SQLite reserved keyword */
    char *reserved[] = {
	"ALL",
	"ALTER",
	"AND",
	"AS",
	"AUTOINCREMENT",
	"BETWEEN",
	"BY",
	"CASE",
	"CHECK",
	"COLLATE",
	"COMMIT",
	"CONSTRAINT",
	"CREATE",
	"CROSS",
	"DEFAULT",
	"DEFERRABLE",
	"DELETE",
	"DISTINCT",
	"DROP",
	"ELSE",
	"ESCAPE",
	"EXCEPT",
	"FOREIGN",
	"FROM",
	"FULL",
	"GLOB",
	"GROUP",
	"HAVING",
	"IN",
	"INDEX",
	"INNER",
	"INSERT",
	"INTERSECT",
	"INTO",
	"IS",
	"ISNULL",
	"JOIN",
	"LEFT",
	"LIKE",
	"LIMIT",
	"NATURAL",
	"NOT",
	"NOTNULL",
	"NULL",
	"ON",
	"OR",
	"ORDER",
	"OUTER",
	"PRIMARY",
	"REFERENCES",
	"RIGHT",
	"ROLLBACK",
	"SELECT",
	"SET",
	"TABLE",
	"THEN",
	"TO",
	"TRANSACTION",
	"UNION",
	"UNIQUE",
	"UPDATE",
	"USING",
	"VALUES",
	"WHEN",
	"WHERE",
	NULL
    };
    char **pw = reserved;
    while (*pw != NULL)
      {
	  if (strcasecmp (name, *pw) == 0)
	      return 1;
	  pw++;
      }
    return 0;
}

GAIAGEO_DECLARE int
gaiaIsReservedSqlName (const char *name)
{
/* checks if column-name is an SQL reserved keyword */
    char *reserved[] = {
	"ABSOLUTE",
	"ACTION",
	"ADD",
	"AFTER",
	"ALL",
	"ALLOCATE",
	"ALTER",
	"AND",
	"ANY",
	"ARE",
	"ARRAY",
	"AS",
	"ASC",
	"ASENSITIVE",
	"ASSERTION",
	"ASYMMETRIC",
	"AT",
	"ATOMIC",
	"AUTHORIZATION",
	"AVG",
	"BEFORE",
	"BEGIN",
	"BETWEEN",
	"BIGINT",
	"BINARY",
	"BIT",
	"BIT_LENGTH",
	"BLOB",
	"BOOLEAN",
	"BOTH",
	"BREADTH",
	"BY",
	"CALL",
	"CALLED",
	"CASCADE",
	"CASCADED",
	"CASE",
	"CAST",
	"CATALOG",
	"CHAR",
	"CHARACTER",
	"CHARACTER_LENGTH",
	"CHAR_LENGTH",
	"CHECK",
	"CLOB",
	"CLOSE",
	"COALESCE",
	"COLLATE",
	"COLLATION",
	"COLUMN",
	"COMMIT",
	"CONDITION",
	"CONNECT",
	"CONNECTION",
	"CONSTRAINT",
	"CONSTRAINTS",
	"CONSTRUCTOR",
	"CONTAINS",
	"CONTINUE",
	"CONVERT",
	"CORRESPONDING",
	"COUNT",
	"CREATE",
	"CROSS",
	"CUBE",
	"CURRENT",
	"CURRENT_DATE",
	"CURRENT_DEFAULT_TRANSFORM_GROUP",
	"CURRENT_PATH",
	"CURRENT_ROLE",
	"CURRENT_TIME",
	"CURRENT_TIMESTAMP",
	"CURRENT_TRANSFORM_GROUP_FOR_TYPE",
	"CURRENT_USER",
	"CURSOR",
	"CYCLE",
	"DATA",
	"DATE",
	"DAY",
	"DEALLOCATE",
	"DEC",
	"DECIMAL",
	"DECLARE",
	"DEFAULT",
	"DEFERRABLE",
	"DEFERRED",
	"DELETE",
	"DEPTH",
	"DEREF",
	"DESC",
	"DESCRIBE",
	"DESCRIPTOR",
	"DETERMINISTIC",
	"DIAGNOSTICS",
	"DISCONNECT",
	"DISTINCT",
	"DO",
	"DOMAIN",
	"DOUBLE",
	"DROP",
	"DYNAMIC",
	"EACH",
	"ELEMENT",
	"ELSE",
	"ELSEIF",
	"END",
	"EQUALS",
	"ESCAPE",
	"EXCEPT",
	"EXCEPTION",
	"EXEC",
	"EXECUTE",
	"EXISTS",
	"EXIT",
	"external",
	"EXTRACT",
	"FALSE",
	"FETCH",
	"FILTER",
	"FIRST",
	"FLOAT",
	"FOR",
	"FOREIGN",
	"FOUND",
	"FREE",
	"FROM",
	"FULL",
	"FUNCTION",
	"GENERAL",
	"GET",
	"GLOBAL",
	"GO",
	"GOTO",
	"GRANT",
	"GROUP",
	"GROUPING",
	"HANDLER",
	"HAVING",
	"HOLD",
	"HOUR",
	"IDENTITY",
	"IF",
	"IMMEDIATE",
	"IN",
	"INDICATOR",
	"INITIALLY",
	"INNER",
	"INOUT",
	"INPUT",
	"INSENSITIVE",
	"INSERT",
	"INT",
	"INTEGER",
	"INTERSECT",
	"INTERVAL",
	"INTO",
	"IS",
	"ISOLATION",
	"ITERATE",
	"JOIN",
	"KEY",
	"LANGUAGE",
	"LARGE",
	"LAST",
	"LATERAL",
	"LEADING",
	"LEAVE",
	"LEFT",
	"LEVEL",
	"LIKE",
	"LOCAL",
	"LOCALTIME",
	"LOCALTIMESTAMP",
	"LOCATOR",
	"LOOP",
	"LOWER",
	"MAP",
	"MATCH",
	"MAX",
	"MEMBER",
	"MERGE",
	"METHOD",
	"MIN",
	"MINUTE",
	"MODIFIES",
	"MODULE",
	"MONTH",
	"MULTISET",
	"NAMES",
	"NATIONAL",
	"NATURAL",
	"NCHAR",
	"NCLOB",
	"NEW",
	"NEXT",
	"NO",
	"NONE",
	"NOT",
	"NULL",
	"NULLIF",
	"NUMERIC",
	"OBJECT",
	"OCTET_LENGTH",
	"OF",
	"OLD",
	"ON",
	"ONLY",
	"OPEN",
	"OPTION",
	"OR",
	"ORDER",
	"ORDINALITY",
	"OUT",
	"OUTER",
	"OUTPUT",
	"OVER",
	"OVERLAPS",
	"PAD",
	"PARAMETER",
	"PARTIAL",
	"PARTITION",
	"PATH",
	"POSITION",
	"PRECISION",
	"PREPARE",
	"PRESERVE",
	"PRIMARY",
	"PRIOR",
	"PRIVILEGES",
	"PROCEDURE",
	"PUBLIC",
	"RANGE",
	"READ",
	"READS",
	"REAL",
	"RECURSIVE",
	"REF",
	"REFERENCES",
	"REFERENCING",
	"RELATIVE",
	"RELEASE",
	"REPEAT",
	"RESIGNAL",
	"RESTRICT",
	"RESULT",
	"RETURN",
	"RETURNS",
	"REVOKE",
	"RIGHT",
	"ROLE",
	"ROLLBACK",
	"ROLLUP",
	"ROUTINE",
	"ROW",
	"ROWS",
	"SAVEPOINT",
	"SCHEMA",
	"SCOPE",
	"SCROLL",
	"SEARCH",
	"SECOND",
	"SECTION",
	"SELECT",
	"SENSITIVE",
	"SESSION",
	"SESSION_USER",
	"SET",
	"SETS",
	"SIGNAL",
	"SIMILAR",
	"SIZE",
	"SMALLINT",
	"SOME",
	"SPACE",
	"SPECIFIC",
	"SPECIFICTYPE",
	"SQL",
	"SQLCODE",
	"SQLERROR",
	"SQLEXCEPTION",
	"SQLSTATE",
	"SQLWARNING",
	"START",
	"STATE",
	"STATIC",
	"SUBMULTISET",
	"SUBSTRING",
	"SUM",
	"SYMMETRIC",
	"SYSTEM",
	"SYSTEM_USER",
	"TABLE",
	"TABLESAMPLE",
	"TEMPORARY",
	"THEN",
	"TIME",
	"TIMESTAMP",
	"TIMEZONE_HOUR",
	"TIMEZONE_MINUTE",
	"TO",
	"TRAILING",
	"TRANSACTION",
	"TRANSLATE",
	"TRANSLATION",
	"TREAT",
	"TRIGGER",
	"TRIM",
	"TRUE",
	"UNDER",
	"UNDO",
	"UNION",
	"UNIQUE",
	"UNKNOWN",
	"UNNEST",
	"UNTIL",
	"UPDATE",
	"UPPER",
	"USAGE",
	"USER",
	"USING",
	"VALUE",
	"VALUES",
	"VARCHAR",
	"VARYING",
	"VIEW",
	"WHEN",
	"WHENEVER",
	"WHERE",
	"WHILE",
	"WINDOW",
	"WITH",
	"WITHIN",
	"WITHOUT",
	"WORK",
	"WRITE",
	"YEAR",
	"ZONE",
	NULL
    };
    char **pw = reserved;
    while (*pw != NULL)
      {
	  if (strcasecmp (name, *pw) == 0)
	      return 1;
	  pw++;
      }
    return 0;
}

GAIAGEO_DECLARE void
gaiaCleanSqlString (char *value)
{
/*
/ returns a well formatted TEXT value for SQL
/ 1] strips trailing spaces
/ 2] masks any ' inside the string, appending another '
*/
    char new_value[1024];
    char *p;
    int len;
    int i;
    len = strlen (value);
    for (i = (len - 1); i >= 0; i--)
      {
	  /* stripping trailing spaces */
	  if (value[i] == ' ')
	      value[i] = '\0';
	  else
	      break;
      }
    p = new_value;
    for (i = 0; i < len; i++)
      {
	  if (value[i] == '\'')
	      *(p++) = '\'';
	  *(p++) = value[i];
      }
    *p = '\0';
    strcpy (value, new_value);
}
/**************** End file: gg_sqlaux.c **********/


/**************** Begin file: gg_utf8.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <string.h> */
/* #include <errno.h> */

#if defined(__MINGW32__) || defined(_WIN32)
#define LIBICONV_STATIC
/* #include <iconv.h> */
#define LIBCHARSET_STATIC
/* #include <localcharset.h> */
#else /* not MINGW32 - WIN32 */
#ifdef __APPLE__
/* #include <iconv.h> */
/* #include <localcharset.h> */
#else /* not Mac OsX */
/* #include <iconv.h> */
/* #include <langinfo.h> */
#endif
#endif

/* #include <spatialite/gaiaaux.h> */

GAIAAUX_DECLARE const char *
gaiaGetLocaleCharset ()
{
/* indentifies the locale charset */
#if defined(__MINGW32__) || defined(_WIN32)
    return locale_charset ();
#else /* not MINGW32 - WIN32 */
#ifdef __APPLE__
    return locale_charset ();
#else /* not Mac OsX */
    return nl_langinfo (CODESET);
#endif
#endif
}

GAIAAUX_DECLARE int
gaiaConvertCharset (char **buf, const char *fromCs, const char *toCs)
{
/* converting a string from a charset to another "by-the-fly" */
    char utf8buf[65536];
#if defined(__MINGW32__) || defined(_WIN32)
    const char *pBuf;
#else /* not MINGW32 - WIN32 */
    char *pBuf;
#endif
    size_t len;
    size_t utf8len;
    char *pUtf8buf;
    iconv_t cvt = iconv_open (toCs, fromCs);
    if (cvt == (iconv_t) - 1)
	goto unsupported;
    len = strlen (*buf);
    utf8len = 65536;
    pBuf = *buf;
    pUtf8buf = utf8buf;
    if (iconv (cvt, &pBuf, &len, &pUtf8buf, &utf8len) < 0)
	goto error;
    utf8buf[65536 - utf8len] = '\0';
    memcpy (*buf, utf8buf, (65536 - utf8len) + 1);
    iconv_close (cvt);
    return 1;
  error:
    iconv_close (cvt);
  unsupported:
    return 0;
}

GAIAAUX_DECLARE void *
gaiaCreateUTF8Converter (const char *fromCS)
{
/* creating an UTF8 converter and returning on opaque reference to it */
    iconv_t cvt = iconv_open ("UTF-8", fromCS);
    if (cvt == (iconv_t) - 1)
	return NULL;
    return cvt;
}

GAIAAUX_DECLARE void
gaiaFreeUTF8Converter (void *cvtCS)
{
/* destroyng an UTF8 converter */
    if (cvtCS)
	iconv_close (cvtCS);
}

GAIAAUX_DECLARE char *
gaiaConvertToUTF8 (void *cvtCS, const char *buf, int buflen, int *err)
{
/* converting a string to UTF8 */
    char *utf8buf = 0;
#if defined(__MINGW32__) || defined(_WIN32)
    const char *pBuf;
#else
    char *pBuf;
#endif
    size_t len;
    size_t utf8len;
    int maxlen = buflen * 4;
    char *pUtf8buf;
    *err = 0;
    if (!cvtCS)
      {
	  *err = 1;
	  return NULL;
      }
    utf8buf = malloc (maxlen);
    len = buflen;
    utf8len = maxlen;
    pBuf = (char *) buf;
    pUtf8buf = utf8buf;
    if (iconv (cvtCS, &pBuf, &len, &pUtf8buf, &utf8len) < 0)
      {
	  free (utf8buf);
	  *err = 1;
	  return NULL;
      }
    utf8buf[maxlen - utf8len] = '\0';
    return utf8buf;
}
/**************** End file: gg_utf8.c **********/


/**************** Begin file: gaia_exif.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <memory.h> */
/* #include <math.h> */
/* #include <float.h> */

/* #include <spatialite/sqlite3ext.h> */
/* #include <spatialite/gaiageo.h> */
/* #include <spatialite/gaiaexif.h> */
/* #include <spatialite.h> */

static void
exifTagName (char gps, unsigned short tag_id, char *str, int len)
{
/* returns the canonical name corresponding to an EXIF TAG ID */
    int l;
    char *name = "UNKNOWN";
    if (gps)
      {
	  switch (tag_id)
	    {
	    case 0x00:
		name = "GPSVersionID";
		break;
	    case 0x01:
		name = "GPSLatitudeRef";
		break;
	    case 0x02:
		name = "GPSLatitude";
		break;
	    case 0x03:
		name = "GPSLongitudeRef";
		break;
	    case 0x04:
		name = "GPSLongitude";
		break;
	    case 0x05:
		name = "GPSAltitudeRef";
		break;
	    case 0x06:
		name = "GPSAltitude";
		break;
	    case 0x07:
		name = "GPSTimeStamp";
		break;
	    case 0x08:
		name = "GPSSatellites";
		break;
	    case 0x09:
		name = "GPSStatus";
		break;
	    case 0x0A:
		name = "GPSMeasureMode";
		break;
	    case 0x0B:
		name = "GPSDOP";
		break;
	    case 0x0C:
		name = "GPSSpeedRef";
		break;
	    case 0x0D:
		name = "GPSSpeed";
		break;
	    case 0x0E:
		name = "GPSTrackRef";
		break;
	    case 0x0F:
		name = "GPSTrack";
		break;
	    case 0x10:
		name = "GPSImgDirectionRef";
		break;
	    case 0x11:
		name = "GPSImgDirection";
		break;
	    case 0x12:
		name = "GPSMapDatum";
		break;
	    case 0x13:
		name = "GPSDestLatitudeRef";
		break;
	    case 0x14:
		name = "GPSDestLatitude";
		break;
	    case 0x15:
		name = "GPSDestLongitudeRef";
		break;
	    case 0x16:
		name = "GPSDestLongitude";
		break;
	    case 0x17:
		name = "GPSDestBearingRef";
		break;
	    case 0x18:
		name = "GPSDestBearing";
		break;
	    case 0x19:
		name = "GPSDestDistanceRef";
		break;
	    case 0x1A:
		name = "GPSDestDistance";
		break;
	    case 0x1B:
		name = "GPSProcessingMethod";
		break;
	    case 0x1C:
		name = "GPSAreaInformation";
		break;
	    case 0x1D:
		name = "GPSDateStamp";
		break;
	    case 0x1E:
		name = "GPSDifferential";
		break;
	    };
      }
    else
      {
	  switch (tag_id)
	    {
	    case 0x000B:
		name = "ACDComment";
		break;
	    case 0x00FE:
		name = "NewSubFile";
		break;
	    case 0x00FF:
		name = "SubFile";
		break;
	    case 0x0100:
		name = "ImageWidth";
		break;
	    case 0x0101:
		name = "ImageLength";
		break;
	    case 0x0102:
		name = "BitsPerSample";
		break;
	    case 0x0103:
		name = "Compression";
		break;
	    case 0x0106:
		name = "PhotometricInterpretation";
		break;
	    case 0x010A:
		name = "FillOrder";
		break;
	    case 0x010D:
		name = "DocumentName";
		break;
	    case 0x010E:
		name = "ImageDescription";
		break;
	    case 0x010F:
		name = "Make";
		break;
	    case 0x0110:
		name = "Model";
		break;
	    case 0x0111:
		name = "StripOffsets";
		break;
	    case 0x0112:
		name = "Orientation";
		break;
	    case 0x0115:
		name = "SamplesPerPixel";
		break;
	    case 0x0116:
		name = "RowsPerStrip";
		break;
	    case 0x0117:
		name = "StripByteCounts";
		break;
	    case 0x0118:
		name = "MinSampleValue";
		break;
	    case 0x0119:
		name = "MaxSampleValue";
		break;
	    case 0x011A:
		name = "XResolution";
		break;
	    case 0x011B:
		name = "YResolution";
		break;
	    case 0x011C:
		name = "PlanarConfiguration";
		break;
	    case 0x011D:
		name = "PageName";
		break;
	    case 0x011E:
		name = "XPosition";
		break;
	    case 0x011F:
		name = "YPosition";
		break;
	    case 0x0120:
		name = "FreeOffsets";
		break;
	    case 0x0121:
		name = "FreeByteCounts";
		break;
	    case 0x0122:
		name = "GrayResponseUnit";
		break;
	    case 0x0123:
		name = "GrayResponseCurve";
		break;
	    case 0x0124:
		name = "T4Options";
		break;
	    case 0x0125:
		name = "T6Options";
		break;
	    case 0x0128:
		name = "ResolutionUnit";
		break;
	    case 0x0129:
		name = "PageNumber";
		break;
	    case 0x012D:
		name = "TransferFunction";
		break;
	    case 0x0131:
		name = "Software";
		break;
	    case 0x0132:
		name = "DateTime";
		break;
	    case 0x013B:
		name = "Artist";
		break;
	    case 0x013C:
		name = "HostComputer";
		break;
	    case 0x013D:
		name = "Predictor";
		break;
	    case 0x013E:
		name = "WhitePoint";
		break;
	    case 0x013F:
		name = "PrimaryChromaticities";
		break;
	    case 0x0140:
		name = "ColorMap";
		break;
	    case 0x0141:
		name = "HalfToneHints";
		break;
	    case 0x0142:
		name = "TileWidth";
		break;
	    case 0x0143:
		name = "TileLength";
		break;
	    case 0x0144:
		name = "TileOffsets";
		break;
	    case 0x0145:
		name = "TileByteCounts";
		break;
	    case 0x014A:
		name = "SubIFD";
		break;
	    case 0x014C:
		name = "InkSet";
		break;
	    case 0x014D:
		name = "InkNames";
		break;
	    case 0x014E:
		name = "NumberOfInks";
		break;
	    case 0x0150:
		name = "DotRange";
		break;
	    case 0x0151:
		name = "TargetPrinter";
		break;
	    case 0x0152:
		name = "ExtraSample";
		break;
	    case 0x0153:
		name = "SampleFormat";
		break;
	    case 0x0154:
		name = "SMinSampleValue";
		break;
	    case 0x0155:
		name = "SMaxSampleValue";
		break;
	    case 0x0156:
		name = "TransferRange";
		break;
	    case 0x0157:
		name = "ClipPath";
		break;
	    case 0x0158:
		name = "XClipPathUnits";
		break;
	    case 0x0159:
		name = "YClipPathUnits";
		break;
	    case 0x015A:
		name = "Indexed";
		break;
	    case 0x015B:
		name = "JPEGTables";
		break;
	    case 0x015F:
		name = "OPIProxy";
		break;
	    case 0x0200:
		name = "JPEGProc";
		break;
	    case 0x0201:
		name = "JPEGInterchangeFormat";
		break;
	    case 0x0202:
		name = "JPEGInterchangeFormatLength";
		break;
	    case 0x0203:
		name = "JPEGRestartInterval";
		break;
	    case 0x0205:
		name = "JPEGLosslessPredictors";
		break;
	    case 0x0206:
		name = "JPEGPointTransforms";
		break;
	    case 0x0207:
		name = "JPEGQTables";
		break;
	    case 0x0208:
		name = "JPEGDCTables";
		break;
	    case 0x0209:
		name = "JPEGACTables";
		break;
	    case 0x0211:
		name = "YCbCrCoefficients";
		break;
	    case 0x0212:
		name = "YCbCrSubSampling";
		break;
	    case 0x0213:
		name = "YCbCrPositioning";
		break;
	    case 0x0214:
		name = "ReferenceBlackWhite";
		break;
	    case 0x02BC:
		name = "ExtensibleMetadataPlatform";
		break;
	    case 0x0301:
		name = "Gamma";
		break;
	    case 0x0302:
		name = "ICCProfileDescriptor";
		break;
	    case 0x0303:
		name = "SRGBRenderingIntent";
		break;
	    case 0x0320:
		name = "ImageTitle";
		break;
	    case 0x5001:
		name = "ResolutionXUnit";
		break;
	    case 0x5002:
		name = "ResolutionYUnit";
		break;
	    case 0x5003:
		name = "ResolutionXLengthUnit";
		break;
	    case 0x5004:
		name = "ResolutionYLengthUnit";
		break;
	    case 0x5005:
		name = "PrintFlags";
		break;
	    case 0x5006:
		name = "PrintFlagsVersion";
		break;
	    case 0x5007:
		name = "PrintFlagsCrop";
		break;
	    case 0x5008:
		name = "PrintFlagsBleedWidth";
		break;
	    case 0x5009:
		name = "PrintFlagsBleedWidthScale";
		break;
	    case 0x500A:
		name = "HalftoneLPI";
		break;
	    case 0x500B:
		name = "HalftoneLPIUnit";
		break;
	    case 0x500C:
		name = "HalftoneDegree";
		break;
	    case 0x500D:
		name = "HalftoneShape";
		break;
	    case 0x500E:
		name = "HalftoneMisc";
		break;
	    case 0x500F:
		name = "HalftoneScreen";
		break;
	    case 0x5010:
		name = "JPEGQuality";
		break;
	    case 0x5011:
		name = "GridSize";
		break;
	    case 0x5012:
		name = "ThumbnailFormat";
		break;
	    case 0x5013:
		name = "ThumbnailWidth";
		break;
	    case 0x5014:
		name = "ThumbnailHeight";
		break;
	    case 0x5015:
		name = "ThumbnailColorDepth";
		break;
	    case 0x5016:
		name = "ThumbnailPlanes";
		break;
	    case 0x5017:
		name = "ThumbnailRawBytes";
		break;
	    case 0x5018:
		name = "ThumbnailSize";
		break;
	    case 0x5019:
		name = "ThumbnailCompressedSize";
		break;
	    case 0x501A:
		name = "ColorTransferFunction";
		break;
	    case 0x501B:
		name = "ThumbnailData";
		break;
	    case 0x5020:
		name = "ThumbnailImageWidth";
		break;
	    case 0x5021:
		name = "ThumbnailImageHeight";
		break;
	    case 0x5022:
		name = "ThumbnailBitsPerSample";
		break;
	    case 0x5023:
		name = "ThumbnailCompression";
		break;
	    case 0x5024:
		name = "ThumbnailPhotometricInterp";
		break;
	    case 0x5025:
		name = "ThumbnailImageDescription";
		break;
	    case 0x5026:
		name = "ThumbnailEquipMake";
		break;
	    case 0x5027:
		name = "ThumbnailEquipModel";
		break;
	    case 0x5028:
		name = "ThumbnailStripOffsets";
		break;
	    case 0x5029:
		name = "ThumbnailOrientation";
		break;
	    case 0x502A:
		name = "ThumbnailSamplesPerPixel";
		break;
	    case 0x502B:
		name = "ThumbnailRowsPerStrip";
		break;
	    case 0x502C:
		name = "ThumbnailStripBytesCount";
		break;
	    case 0x502D:
		name = "ThumbnailResolutionX";
		break;
	    case 0x502E:
		name = "ThumbnailResolutionY";
		break;
	    case 0x502F:
		name = "ThumbnailPlanarConfig";
		break;
	    case 0x5030:
		name = "ThumbnailResolutionUnit";
		break;
	    case 0x5031:
		name = "ThumbnailTransferFunction";
		break;
	    case 0x5032:
		name = "ThumbnailSoftwareUsed";
		break;
	    case 0x5033:
		name = "ThumbnailDateTime";
		break;
	    case 0x5034:
		name = "ThumbnailArtist";
		break;
	    case 0x5035:
		name = "ThumbnailWhitePoint";
		break;
	    case 0x5036:
		name = "ThumbnailPrimaryChromaticities";
		break;
	    case 0x5037:
		name = "ThumbnailYCbCrCoefficients";
		break;
	    case 0x5038:
		name = "ThumbnailYCbCrSubsampling";
		break;
	    case 0x5039:
		name = "ThumbnailYCbCrPositioning";
		break;
	    case 0x503A:
		name = "ThumbnailRefBlackWhite";
		break;
	    case 0x503B:
		name = "ThumbnailCopyRight";
		break;
	    case 0x5090:
		name = "LuminanceTable";
		break;
	    case 0x5091:
		name = "ChrominanceTable";
		break;
	    case 0x5100:
		name = "FrameDelay";
		break;
	    case 0x5101:
		name = "LoopCount";
		break;
	    case 0x5110:
		name = "PixelUnit";
		break;
	    case 0x5111:
		name = "PixelPerUnitX";
		break;
	    case 0x5112:
		name = "PixelPerUnitY";
		break;
	    case 0x5113:
		name = "PaletteHistogram";
		break;
	    case 0x1000:
		name = "RelatedImageFileFormat";
		break;
	    case 0x800D:
		name = "ImageID";
		break;
	    case 0x80E3:
		name = "Matteing";
		break;
	    case 0x80E4:
		name = "DataType";
		break;
	    case 0x80E5:
		name = "ImageDepth";
		break;
	    case 0x80E6:
		name = "TileDepth";
		break;
	    case 0x828D:
		name = "CFARepeatPatternDim";
		break;
	    case 0x828E:
		name = "CFAPattern";
		break;
	    case 0x828F:
		name = "BatteryLevel";
		break;
	    case 0x8298:
		name = "Copyright";
		break;
	    case 0x829A:
		name = "ExposureTime";
		break;
	    case 0x829D:
		name = "FNumber";
		break;
	    case 0x83BB:
		name = "IPTC/NAA";
		break;
	    case 0x84E3:
		name = "IT8RasterPadding";
		break;
	    case 0x84E5:
		name = "IT8ColorTable";
		break;
	    case 0x8649:
		name = "ImageResourceInformation";
		break;
	    case 0x8769:
		name = "Exif IFD Pointer";
		break;
	    case 0x8773:
		name = "ICC_Profile";
		break;
	    case 0x8822:
		name = "ExposureProgram";
		break;
	    case 0x8824:
		name = "SpectralSensitivity";
		break;
	    case 0x8825:
		name = "GPSInfo IFD Pointer";
		break;
	    case 0x8827:
		name = "ISOSpeedRatings";
		break;
	    case 0x8828:
		name = "OECF";
		break;
	    case 0x9000:
		name = "ExifVersion";
		break;
	    case 0x9003:
		name = "DateTimeOriginal";
		break;
	    case 0x9004:
		name = "DateTimeDigitized";
		break;
	    case 0x9101:
		name = "ComponentsConfiguration";
		break;
	    case 0x9102:
		name = "CompressedBitsPerPixel";
		break;
	    case 0x9201:
		name = "ShutterSpeedValue";
		break;
	    case 0x9202:
		name = "ApertureValue";
		break;
	    case 0x9203:
		name = "BrightnessValue";
		break;
	    case 0x9204:
		name = "ExposureBiasValue";
		break;
	    case 0x9205:
		name = "MaxApertureValue";
		break;
	    case 0x9206:
		name = "SubjectDistance";
		break;
	    case 0x9207:
		name = "MeteringMode";
		break;
	    case 0x9208:
		name = "LightSource";
		break;
	    case 0x9209:
		name = "Flash";
		break;
	    case 0x920A:
		name = "FocalLength";
		break;
	    case 0x920B:
	    case 0xA20B:
		name = "FlashEnergy";
		break;
	    case 0x920C:
	    case 0xA20C:
		name = "SpatialFrequencyResponse";
		break;
	    case 0x920D:
		name = "Noise";
		break;
	    case 0x920E:
	    case 0xA20E:
		name = "FocalPlaneXResolution";
		break;
	    case 0x920F:
	    case 0XA20F:
		name = "FocalPlaneYResolution";
		break;
	    case 0x9210:
	    case 0xA210:
		name = "FocalPlaneResolutionUnit";
		break;
	    case 0x9211:
		name = "ImageNumber";
		break;
	    case 0x9212:
		name = "SecurityClassification";
		break;
	    case 0x9213:
		name = "ImageHistory";
		break;
	    case 0x9214:
	    case 0xA214:
		name = "SubjectLocation";
		break;
	    case 0x9215:
	    case 0xA215:
		name = "ExposureIndex";
		break;
	    case 0x9216:
		name = "TIFF/EPStandardID";
		break;
	    case 0x9217:
	    case 0xA217:
		name = "SensingMethod";
		break;
	    case 0x923F:
		name = "StoNits";
		break;
	    case 0x927C:
		name = "MakerNote";
		break;
	    case 0x9286:
		name = "UserComment";
		break;
	    case 0x9290:
		name = "SubSecTime";
		break;
	    case 0x9291:
		name = "SubSecTimeOriginal";
		break;
	    case 0x9292:
		name = "SubSecTimeDigitized";
		break;
	    case 0xA000:
		name = "FlashpixVersion";
		break;
	    case 0xA001:
		name = "ColorSpace";
		break;
	    case 0xA002:
		name = "ExifImageWidth";
		break;
	    case 0xA003:
		name = "ExifImageLength";
		break;
	    case 0xA004:
		name = "RelatedSoundFile";
		break;
	    case 0xA005:
		name = "Interoperability IFD Pointer";
		break;
	    case 0xA20D:
		name = "Noise";
		break;
	    case 0xA211:
		name = "ImageNumber";
		break;
	    case 0xA212:
		name = "SecurityClassification";
		break;
	    case 0xA213:
		name = "ImageHistory";
		break;
	    case 0xA216:
		name = "TIFF/EPStandardID";
		break;
	    case 0xA300:
		name = "FileSource";
		break;
	    case 0xA301:
		name = "SceneType";
		break;
	    case 0xA302:
		name = "CFAPattern";
		break;
	    case 0xA401:
		name = "CustomRendered";
		break;
	    case 0xA402:
		name = "ExposureMode";
		break;
	    case 0xA403:
		name = "WhiteBalance";
		break;
	    case 0xA404:
		name = "DigitalZoomRatio";
		break;
	    case 0xA405:
		name = "FocalLengthIn35mmFilm";
		break;
	    case 0xA406:
		name = "SceneCaptureType";
		break;
	    case 0xA407:
		name = "GainControl";
		break;
	    case 0xA408:
		name = "Contrast";
		break;
	    case 0xA409:
		name = "Saturation";
		break;
	    case 0xA40A:
		name = "Sharpness";
		break;
	    case 0xA40B:
		name = "DeviceSettingDescription";
		break;
	    case 0xA40C:
		name = "SubjectDistanceRange";
		break;
	    case 0xA420:
		name = "ImageUniqueID";
		break;
	    };
      }
    l = strlen (name);
    if (len > l)
	strcpy (str, name);
    else
      {
	  memset (str, '\0', len);
	  memcpy (str, name, len - 1);
      }
}

static unsigned short
exifImportU16 (const unsigned char *p, int little_endian,
	       int little_endian_arch)
{
/* fetches an unsigned 16bit int from BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[2];
	unsigned short short_value;
    } convert;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 1);
		convert.byte[1] = *(p + 0);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 1);
		convert.byte[1] = *(p + 0);
	    }
      }
    return convert.short_value;
}

static unsigned int
exifImportU32 (const unsigned char *p, int little_endian,
	       int little_endian_arch)
{
/* fetches an unsigned 32bit int from BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[4];
	unsigned int int_value;
    } convert;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 3);
		convert.byte[1] = *(p + 2);
		convert.byte[2] = *(p + 1);
		convert.byte[3] = *(p + 0);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 3);
		convert.byte[1] = *(p + 2);
		convert.byte[2] = *(p + 1);
		convert.byte[3] = *(p + 0);
	    }
      }
    return convert.int_value;
}

static float
exifImportFloat32 (const unsigned char *p, int little_endian,
		   int little_endian_arch)
{
/* fetches a 32bit FLOAT from BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[4];
	float float_value;
    } convert;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 3);
		convert.byte[1] = *(p + 2);
		convert.byte[2] = *(p + 1);
		convert.byte[3] = *(p + 0);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 3);
		convert.byte[1] = *(p + 2);
		convert.byte[2] = *(p + 1);
		convert.byte[3] = *(p + 0);
	    }
      }
    return convert.float_value;
}

static void
exifSetTagValue (gaiaExifTagPtr tag, const unsigned char *blob, int endian_mode,
		 int endian_arch)
{
/* setting the TAG value */
    int i;
    int sz = 0;
    unsigned int offset;
    const unsigned char *ptr;
    unsigned short short_value;
    unsigned int int_value;
    short sign_short_value;
    int sign_int_value;
    float float_value;
    double double_value;
    if (tag->Type == 1 || tag->Type == 2 || tag->Type == 6 || tag->Type == 7)
	sz = tag->Count;
    if (tag->Type == 3 || tag->Type == 8)
	sz = tag->Count * 2;
    if (tag->Type == 4 || tag->Type == 9 || tag->Type == 11)
	sz = tag->Count * 4;
    if (tag->Type == 5 || tag->Type == 10 || tag->Type == 12)
	sz = tag->Count * 8;
    if (sz <= 4)
      {
	  /* TAG values is stored within the offset */
	  ptr = tag->TagOffset;
      }
    else
      {
	  /* jumping to offset */
	  offset = exifImportU32 (tag->TagOffset, endian_mode, endian_arch);
	  offset += 12;
	  ptr = blob + offset;
      }
    if (tag->Type == 1 || tag->Type == 6 || tag->Type == 7)
      {
	  /* BYTE type */
	  tag->ByteValue = malloc (tag->Count);
	  memcpy (tag->ByteValue, ptr, tag->Count);
      }
    if (tag->Type == 2)
      {
	  /* STRING type */
	  tag->StringValue = malloc (tag->Count);
	  memcpy (tag->StringValue, ptr, tag->Count);
      }
    if (tag->Type == 3)
      {
	  /* SHORT type */
	  tag->ShortValues = malloc (tag->Count * sizeof (unsigned short));
	  for (i = 0; i < tag->Count; i++)
	    {
		short_value =
		    exifImportU16 (ptr + (i * 2), endian_mode, endian_arch);
		*(tag->ShortValues + i) = short_value;
	    }
      }
    if (tag->Type == 4)
      {
	  /* LONG type */
	  tag->LongValues = malloc (tag->Count * sizeof (unsigned int));
	  for (i = 0; i < tag->Count; i++)
	    {
		int_value =
		    exifImportU32 (ptr + (i * 4), endian_mode, endian_arch);
		*(tag->LongValues + i) = int_value;
	    }
      }
    if (tag->Type == 5)
      {
	  /* RATIONAL type */
	  tag->LongRationals1 = malloc (tag->Count * sizeof (unsigned int));
	  tag->LongRationals2 = malloc (tag->Count * sizeof (unsigned int));
	  for (i = 0; i < tag->Count; i++)
	    {
		int_value =
		    exifImportU32 (ptr + (i * 8), endian_mode, endian_arch);
		*(tag->LongRationals1 + i) = int_value;
		int_value =
		    exifImportU32 (ptr + (i * 8) + 4, endian_mode, endian_arch);
		*(tag->LongRationals2 + i) = int_value;
	    }
      }
    if (tag->Type == 8)
      {
	  /* SSHORT type */
	  tag->SignedShortValues = malloc (tag->Count * sizeof (short));
	  for (i = 0; i < tag->Count; i++)
	    {
		sign_short_value =
		    gaiaImport16 (ptr + (i * 2), endian_mode, endian_arch);
		*(tag->SignedShortValues + i) = sign_short_value;
	    }
      }
    if (tag->Type == 9)
      {
	  /* SIGNED LONG type */
	  tag->SignedLongValues = malloc (tag->Count * sizeof (int));
	  for (i = 0; i < tag->Count; i++)
	    {
		sign_int_value =
		    gaiaImport32 (ptr + (i * 4), endian_mode, endian_arch);
		*(tag->SignedLongValues + i) = sign_int_value;
	    }
      }
    if (tag->Type == 10)
      {
	  /* SIGNED RATIONAL type */
	  tag->SignedLongRationals1 = malloc (tag->Count * sizeof (int));
	  tag->SignedLongRationals2 = malloc (tag->Count * sizeof (int));
	  for (i = 0; i < tag->Count; i++)
	    {
		sign_int_value =
		    gaiaImport32 (ptr + (i * 8), endian_mode, endian_arch);
		*(tag->SignedLongRationals1 + i) = sign_int_value;
		sign_int_value =
		    gaiaImport32 (ptr + (i * 8) + 4, endian_mode, endian_arch);
		*(tag->SignedLongRationals2 + i) = sign_int_value;
	    }
      }
    if (tag->Type == 11)
      {
	  /* FLOAT type */
	  tag->FloatValues = malloc (tag->Count * sizeof (float));
	  for (i = 0; i < tag->Count; i++)
	    {
		float_value =
		    exifImportFloat32 (ptr + (i * 4), endian_mode, endian_arch);
		*(tag->FloatValues + i) = float_value;
	    }
      }
    if (tag->Type == 12)
      {
	  /* DOUBLE type */
	  tag->DoubleValues = malloc (tag->Count * sizeof (double));
	  for (i = 0; i < tag->Count; i++)
	    {
		double_value =
		    gaiaImport64 (ptr + (i * 8), endian_mode, endian_arch);
		*(tag->DoubleValues + i) = double_value;
	    }
      }
}

static void
exifParseTag (const unsigned char *blob, unsigned int offset, int endian_mode,
	      int endian_arch, gaiaExifTagListPtr list, int gps)
{
/* parsing some TAG and inserting into the list */
    unsigned short tag_id;
    unsigned short type;
    unsigned int count;
    gaiaExifTagPtr tag;
    tag_id = exifImportU16 (blob + offset, endian_mode, endian_arch);
    type = exifImportU16 (blob + offset + 2, endian_mode, endian_arch);
    count = exifImportU32 (blob + offset + 4, endian_mode, endian_arch);
    tag = malloc (sizeof (gaiaExifTag));
    tag->Gps = (char) gps;
    tag->TagId = tag_id;
    tag->Type = type;
    tag->Count = (unsigned short) count;
    memcpy (tag->TagOffset, blob + offset + 8, 4);
    tag->ByteValue = NULL;
    tag->StringValue = NULL;
    tag->ShortValues = NULL;
    tag->LongValues = NULL;
    tag->LongRationals1 = NULL;
    tag->LongRationals2 = NULL;
    tag->SignedShortValues = NULL;
    tag->SignedLongValues = NULL;
    tag->SignedLongRationals1 = NULL;
    tag->SignedLongRationals2 = NULL;
    tag->FloatValues = NULL;
    tag->DoubleValues = NULL;
    exifSetTagValue (tag, blob, endian_mode, endian_arch);
    tag->Next = NULL;
    if (!(list->First))
	list->First = tag;
    if (list->Last)
	(list->Last)->Next = tag;
    list->Last = tag;
    (list->NumTags)++;
}

static void
exifExpandIFD (gaiaExifTagListPtr list, const unsigned char *blob,
	       int endian_mode, int endian_arch)
{
/* trying to expand the EXIF-IFD */
    unsigned int offset;
    unsigned short items;
    unsigned short i;
    gaiaExifTagPtr tag = list->First;
    if (!list)
	return;
    while (tag)
      {
	  if (tag->TagId == 34665)
	    {
		/* ok, this one is an IFD pointer */
		offset =
		    exifImportU32 (tag->TagOffset, endian_mode, endian_arch);
		offset += 12;
		items = exifImportU16 (blob + offset, endian_mode, endian_arch);
		offset += 2;
		for (i = 0; i < items; i++)
		  {
		      /* fetching the TAGs */
		      exifParseTag (blob, offset, endian_mode, endian_arch,
				    list, 0);
		      offset += 12;
		  }
	    }
	  tag = tag->Next;
      }
}

static void
exifExpandGPS (gaiaExifTagListPtr list, const unsigned char *blob,
	       int endian_mode, int endian_arch)
{
/* trying to expand the EXIF-GPS */
    unsigned int offset;
    unsigned short items;
    unsigned short i;
    gaiaExifTagPtr tag = list->First;
    if (!list)
	return;
    while (tag)
      {
	  if (tag->TagId == 34853)
	    {
		/* ok, this one is a GPSinfo-IFD pointer */
		offset =
		    exifImportU32 (tag->TagOffset, endian_mode, endian_arch);
		offset += 12;
		items = exifImportU16 (blob + offset, endian_mode, endian_arch);
		offset += 2;
		for (i = 0; i < items; i++)
		  {
		      /* fetching the TAGs */
		      exifParseTag (blob, offset, endian_mode, endian_arch,
				    list, 1);
		      offset += 12;
		  }
	    }
	  tag = tag->Next;
      }
}

GAIAEXIF_DECLARE gaiaExifTagListPtr
gaiaGetExifTags (const unsigned char *blob, int size)
{
/* trying to parse a BLOB as an EXIF photo */
    gaiaExifTagListPtr list;
    int endian_arch = gaiaEndianArch ();
    int endian_mode;
    unsigned short app1_size;
    unsigned int offset;
    unsigned short items;
    unsigned short i;
    gaiaExifTagPtr pT;
    if (!blob)
	goto error;
    if (size < 14)
	goto error;
/* cecking for SOI [Start Of Image] */
    if (*(blob + 0) == 0xff && *(blob + 1) == 0xd8)
	;
    else
	goto error;
/* checking for APP1 Marker */
    if (*(blob + 2) == 0xff && *(blob + 3) == 0xe1)
	;
    else
	goto error;
/* checking for EXIF identifier */
    if (memcmp (blob + 6, "Exif", 4) == 0)
	;
    else
	goto error;
/* checking for Pad */
    if (*(blob + 10) == 0x00 && *(blob + 11) == 0x00)
	;
    else
	goto error;
    if (memcmp (blob + 12, "II", 2) == 0)
	endian_mode = GAIA_LITTLE_ENDIAN;
    else if (memcmp (blob + 12, "MM", 2) == 0)
	endian_mode = GAIA_BIG_ENDIAN;
    else
	goto error;
/* OK: this BLOB seems to contain a valid EXIF */
    app1_size = exifImportU16 (blob + 4, endian_mode, endian_arch);
    if ((app1_size + 6) > size)
	goto error;
/* checking for marker */
    if (endian_mode == GAIA_BIG_ENDIAN)
      {
	  if (*(blob + 14) == 0x00 && *(blob + 15) == 0x2a)
	      ;
	  else
	      goto error;
      }
    else
      {
	  if (*(blob + 14) == 0x2a && *(blob + 15) == 0x00)
	      ;
	  else
	      goto error;
      }
/* allocating an EXIF TAG LIST */
    list = malloc (sizeof (gaiaExifTagList));
    list->First = NULL;
    list->Last = NULL;
    list->NumTags = 0;
    list->TagsArray = NULL;
    offset = exifImportU32 (blob + 16, endian_mode, endian_arch);
    offset += 12;
/* jump to offset */
    items = exifImportU16 (blob + offset, endian_mode, endian_arch);
    offset += 2;
    for (i = 0; i < items; i++)
      {
/* fetching the EXIF TAGs */
	  exifParseTag (blob, offset, endian_mode, endian_arch, list, 0);
	  offset += 12;
      }
/* expanding the IFD and GPS tags */
    exifExpandIFD (list, blob, endian_mode, endian_arch);
    exifExpandGPS (list, blob, endian_mode, endian_arch);
    if (list->NumTags)
      {
	  /* organizing the EXIF TAGS as an Array */
	  list->TagsArray = malloc (sizeof (gaiaExifTagPtr) * list->NumTags);
	  pT = list->First;
	  i = 0;
	  while (pT)
	    {
		*(list->TagsArray + i++) = pT;
		pT = pT->Next;
	    }
      }
    return list;
  error:
    return NULL;
}

GAIAEXIF_DECLARE void
gaiaExifTagsFree (gaiaExifTagListPtr p)
{
/* memory cleanup; freeing the EXIF TAG list */
    gaiaExifTagPtr pT;
    gaiaExifTagPtr pTn;
    if (!p)
	return;
    pT = p->First;
    while (pT)
      {
	  pTn = pT->Next;
	  if (pT->ByteValue)
	      free (pT->ByteValue);
	  if (pT->StringValue)
	      free (pT->StringValue);
	  if (pT->ShortValues)
	      free (pT->ShortValues);
	  if (pT->LongValues)
	      free (pT->LongValues);
	  if (pT->LongRationals1)
	      free (pT->LongRationals1);
	  if (pT->LongRationals2)
	      free (pT->LongRationals2);
	  if (pT->SignedShortValues)
	      free (pT->SignedShortValues);
	  if (pT->SignedLongValues)
	      free (pT->SignedLongValues);
	  if (pT->SignedLongRationals1)
	      free (pT->SignedLongRationals1);
	  if (pT->SignedLongRationals2)
	      free (pT->SignedLongRationals2);
	  if (pT->FloatValues)
	      free (pT->FloatValues);
	  if (pT->DoubleValues)
	      free (pT->DoubleValues);
	  free (pT);
	  pT = pTn;
      }
    if (p->TagsArray)
	free (p->TagsArray);
    free (p);
}

GAIAEXIF_DECLARE int
gaiaGetExifTagsCount (gaiaExifTagListPtr tag_list)
{
/* returns the # TAGSs into this list */
    return tag_list->NumTags;
}

GAIAEXIF_DECLARE gaiaExifTagPtr
gaiaGetExifTagByPos (gaiaExifTagListPtr tag_list, const int pos)
{
/* returns the Nth TAG from this list */
    if (pos >= 0 && pos < tag_list->NumTags)
	return *(tag_list->TagsArray + pos);
    return NULL;
}

GAIAEXIF_DECLARE gaiaExifTagPtr
gaiaGetExifTagById (const gaiaExifTagListPtr tag_list,
		    const unsigned short tag_id)
{
/* returns a not-GPS TAG identified by its ID */
    gaiaExifTagPtr pT = tag_list->First;
    while (pT)
      {
	  if (!(pT->Gps) && pT->TagId == tag_id)
	      return pT;
	  pT = pT->Next;
      }
    return NULL;
}

GAIAEXIF_DECLARE gaiaExifTagPtr
gaiaGetExifGpsTagById (const gaiaExifTagListPtr tag_list,
		       const unsigned short tag_id)
{
/* returns a GPS TAG identified by its ID */
    gaiaExifTagPtr pT = tag_list->First;
    while (pT)
      {
	  if (pT->Gps && pT->TagId == tag_id)
	      return pT;
	  pT = pT->Next;
      }
    return NULL;
}

GAIAEXIF_DECLARE gaiaExifTagPtr
gaiaGetExifTagByName (const gaiaExifTagListPtr tag_list, const char *tag_name)
{
/* returns a TAG identified by its Name */
    char name[128];
    gaiaExifTagPtr pT = tag_list->First;
    while (pT)
      {
	  exifTagName (pT->Gps, pT->TagId, name, 128);
	  if (strcasecmp (name, tag_name) == 0)
	      return pT;
	  pT = pT->Next;
      }
    return NULL;
}

GAIAEXIF_DECLARE unsigned short
gaiaExifTagGetId (const gaiaExifTagPtr tag)
{
/* returns the TAG ID */
    return tag->TagId;
}

GAIAEXIF_DECLARE int
gaiaIsExifGpsTag (const gaiaExifTagPtr tag)
{
/* checks if this one is a GPS tag */
    return tag->Gps;
}

GAIAEXIF_DECLARE void
gaiaExifTagGetName (const gaiaExifTagPtr tag, char *str, int len)
{
/* returns the TAG symbolic Name */
    exifTagName (tag->Gps, tag->TagId, str, len);
}

GAIAEXIF_DECLARE unsigned short
gaiaExifTagGetValueType (const gaiaExifTagPtr tag)
{
/* returns the TAG value Type */
    return tag->Type;
}

GAIAEXIF_DECLARE unsigned short
gaiaExifTagGetNumValues (const gaiaExifTagPtr tag)
{
/* returns the # TAG Values */
    return tag->Count;
}

GAIAEXIF_DECLARE unsigned char
gaiaExifTagGetByteValue (const gaiaExifTagPtr tag, const int ind, int *ok)
{
/* returns the Nth Byte value */
    if (ind >= 0
	&& ind <
	tag->Count && (tag->Type == 1 || tag->Type == 6 || tag->Type == 7))
      {
	  *ok = 1;
	  return *(tag->ByteValue + ind);
      }
    *ok = 0;
    return 0;
}

GAIAEXIF_DECLARE void
gaiaExifTagGetStringValue (const gaiaExifTagPtr tag, char *str, int len,
			   int *ok)
{
/* returns the String value */
    int l;
    if (tag->Type == 2)
      {
	  *ok = 1;
	  l = strlen (tag->StringValue);
	  if (len > l)
	      strcpy (str, tag->StringValue);
	  else
	    {
		memset (str, '\0', len);
		memcpy (str, tag->StringValue, len - 1);
	    }
	  return;
      }
    *ok = 0;
}

GAIAEXIF_DECLARE unsigned short
gaiaExifTagGetShortValue (const gaiaExifTagPtr tag, const int ind, int *ok)
{
/* returns the Nth Short value */
    if (ind >= 0 && ind < tag->Count && tag->Type == 3)
      {
	  *ok = 1;
	  return *(tag->ShortValues + ind);
      }
    *ok = 0;
    return 0;
}

GAIAEXIF_DECLARE unsigned int
gaiaExifTagGetLongValue (const gaiaExifTagPtr tag, const int ind, int *ok)
{
/* returns the Nth Long value */
    if (ind >= 0 && ind < tag->Count && tag->Type == 4)
      {
	  *ok = 1;
	  return *(tag->LongValues + ind);
      }
    *ok = 0;
    return 0;
}

GAIAEXIF_DECLARE unsigned int
gaiaExifTagGetRational1Value (const gaiaExifTagPtr tag, const int ind, int *ok)
{
/* returns the Nth Rational (1) value */
    if (ind >= 0 && ind < tag->Count && tag->Type == 5)
      {
	  *ok = 1;
	  return *(tag->LongRationals1 + ind);
      }
    *ok = 0;
    return 0;
}

GAIAEXIF_DECLARE unsigned int
gaiaExifTagGetRational2Value (const gaiaExifTagPtr tag, const int ind, int *ok)
{
/* returns the Nth Rational (2) value */
    if (ind >= 0 && ind < tag->Count && tag->Type == 5)
      {
	  *ok = 1;
	  return *(tag->LongRationals2 + ind);
      }
    *ok = 0;
    return 0;
}

GAIAEXIF_DECLARE double
gaiaExifTagGetRationalValue (const gaiaExifTagPtr tag, const int ind, int *ok)
{
/* returns the Nth Rational  value as Double */
    double x;
    if (ind >= 0
	&& ind < tag->Count && tag->Type == 5 && *(tag->LongRationals2 + ind))
      {
	  *ok = 1;
	  x = (double) (*(tag->LongRationals1 + ind)) /
	      (double) (*(tag->LongRationals2 + ind));
	  return x;
      }
    *ok = 0;
    return 0;
}

GAIAEXIF_DECLARE short
gaiaExifTagGetSignedShortValue (const gaiaExifTagPtr tag, const int ind,
				int *ok)
{
/* returns the Nth Signed Short value */
    if (ind >= 0 && ind < tag->Count && tag->Type == 8)
      {
	  *ok = 1;
	  return *(tag->SignedShortValues + ind);
      }
    *ok = 0;
    return 0;
}

GAIAEXIF_DECLARE int
gaiaExifTagGetSignedLongValue (const gaiaExifTagPtr tag, const int ind, int *ok)
{
/* returns the Nth Signed Long value */
    if (ind >= 0 && ind < tag->Count && tag->Type == 9)
      {
	  *ok = 1;
	  return *(tag->SignedLongValues + ind);
      }
    *ok = 0;
    return 0;
}

GAIAEXIF_DECLARE int
gaiaExifTagGetSignedRational1Value (const gaiaExifTagPtr tag, const int ind,
				    int *ok)
{
/* returns the Nth Signed Rational (1) value */
    if (ind >= 0 && ind < tag->Count && tag->Type == 10)
      {
	  *ok = 1;
	  return *(tag->SignedLongRationals1 + ind);
      }
    *ok = 0;
    return 0;
}

GAIAEXIF_DECLARE int
gaiaExifTagGetSignedRational2Value (const gaiaExifTagPtr tag, const int ind,
				    int *ok)
{
/* returns the Nth Signed Rational (2) value */
    if (ind >= 0 && ind < tag->Count && tag->Type == 10)
      {
	  *ok = 1;
	  return *(tag->SignedLongRationals2 + ind);
      }
    *ok = 0;
    return 0;
}

GAIAEXIF_DECLARE double
gaiaExifTagGetSignedRationalValue (const gaiaExifTagPtr tag, const int ind,
				   int *ok)
{
/* returns the Nth Signed Rational  value as Double */
    double x;
    if (ind >= 0
	&& ind <
	tag->Count && tag->Type == 10 && *(tag->SignedLongRationals2 + ind))
      {
	  *ok = 1;
	  x = (double) (*(tag->SignedLongRationals1 + ind)) /
	      (double) (*(tag->SignedLongRationals2 + ind));
	  return x;
      }
    *ok = 0;
    return 0;
}

GAIAEXIF_DECLARE float
gaiaExifTagGetFloatValue (const gaiaExifTagPtr tag, const int ind, int *ok)
{
/* returns the Nth Float value */
    if (ind >= 0 && ind < tag->Count && tag->Type == 11)
      {
	  *ok = 1;
	  return *(tag->FloatValues + ind);
      }
    *ok = 0;
    return 0;
}

GAIAEXIF_DECLARE double
gaiaExifTagGetDoubleValue (const gaiaExifTagPtr tag, const int ind, int *ok)
{
/* returns the Nth Double value */
    if (ind >= 0 && ind < tag->Count && tag->Type == 12)
      {
	  *ok = 1;
	  return *(tag->DoubleValues + ind);
      }
    *ok = 0;
    return 0;
}

GAIAEXIF_DECLARE void
gaiaExifTagGetHumanReadable (const gaiaExifTagPtr tag, char *str, int len,
			     int *ok)
{
/* returns the Human Readable value */
    char *human = "";
    char dummy[1024];
    int l;
    int xok;
    double dblval;
    switch (tag->TagId)
      {
      case 0x0128:		/* ResolutionUnit */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 2:
		      human = "Inches";
		      break;
		  case 3:
		      human = "Centimeters";
		      break;
		  };
	    }
	  break;
      case 0x8822:		/* ExposureProgram */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 0:
		      human = "Not defined";
		      break;
		  case 1:
		      human = "Manual";
		      break;
		  case 2:
		      human = "Normal program";
		      break;
		  case 3:
		      human = "Aperture priority";
		      break;
		  case 4:
		      human = "Shutter priority";
		      break;
		  case 5:
		      human = "Creative program (biased toward depth of field)";
		      break;
		  case 6:
		      human =
			  "Action program (biased toward fast shutter speed)";
		      break;
		  case 7:
		      human =
			  "Portrait mode (for closeup photos with the background out of focus)";
		      break;
		  case 8:
		      human =
			  "Landscape mode (for landscape photos with the background in focus)";
		      break;
		  };
	    }
	  break;
      case 0xA402:		/* ExposureMode */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 0:
		      human = "Auto exposure";
		      break;
		  case 1:
		      human = "Manual exposure";
		      break;
		  case 2:
		      human = "Auto bracket";
		      break;
		  };
	    }
	  break;
      case 0x0112:		/* Orientation */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 1:
		      human = "Normal";
		      break;
		  case 2:
		      human = "Mirrored";
		      break;
		  case 3:
		      human = "Upsidedown";
		      break;
		  case 4:
		      human = "Upsidedown Mirrored";
		      break;
		  case 5:
		      human = "90 deg Clockwise Mirrored";
		      break;
		  case 6:
		      human = "90 deg Counterclocwise";
		      break;
		  case 7:
		      human = "90 deg Counterclocwise Mirrored";
		      break;
		  case 8:
		      human = "90 deg Mirrored";
		      break;
		  };
	    }
	  break;
      case 0x9207:		/* MeteringMode */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 1:
		      human = "Average";
		      break;
		  case 2:
		      human = "Center Weighted Average";
		      break;
		  case 3:
		      human = "Spot";
		      break;
		  case 4:
		      human = "MultiSpot";
		      break;
		  case 5:
		      human = "MultiSegment";
		      break;
		  case 6:
		      human = "Partial";
		      break;
		  case 255:
		      human = "Other";
		      break;
		  };
	    }
	  break;
      case 0xA403:		/* WhiteBalance */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 0:
		      human = "Auto";
		      break;
		  case 1:
		      human = "Sunny";
		      break;
		  case 2:
		      human = "Cloudy";
		      break;
		  case 3:
		      human = "Tungsten";
		      break;
		  case 4:
		      human = "Fluorescent";
		      break;
		  case 5:
		      human = "Flash";
		      break;
		  case 6:
		      human = "Custom";
		      break;
		  case 129:
		      human = "Manual";
		      break;
		  };
	    }
	  break;
      case 0x9209:		/* Flash */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 0:
		  case 16:
		  case 24:
		  case 32:
		      human = "No Flash";
		      break;
		  case 1:
		      human = "Flash";
		      break;
		  case 5:
		      human = "Flash, strobe return light not detected";
		      break;
		  case 7:
		      human = "Flash, strobe return light detected";
		      break;
		  case 9:
		      human = "Compulsory Flash";
		      break;
		  case 13:
		      human = "Compulsory Flash, Return light not detected";
		      break;
		  case 15:
		      human = "Compulsory Flash, Return light detected";
		      break;
		  case 25:
		      human = "Flash, Auto-Mode";
		      break;
		  case 29:
		      human = "Flash, Auto-Mode, Return light not detected";
		      break;
		  case 31:
		      human = "Flash, Auto-Mode, Return light detected";
		      break;
		  case 65:
		      human = "Red Eye";
		      break;
		  case 69:
		      human = "Red Eye, Return light not detected";
		      break;
		  case 71:
		      human = "Red Eye, Return light detected";
		      break;
		  case 73:
		      human = "Red Eye, Compulsory Flash";
		      break;
		  case 77:
		      human =
			  "Red Eye, Compulsory Flash, Return light not detected";
		      break;
		  case 79:
		      human =
			  "Red Eye, Compulsory Flash, Return light detected";
		      break;
		  case 89:
		      human = "Red Eye, Auto-Mode";
		      break;
		  case 93:
		      human = "Red Eye, Auto-Mode, Return light not detected";
		      break;
		  case 95:
		      human = "Red Eye, Auto-Mode, Return light detected";
		      break;
		  };
	    }
	  break;
      case 0xA217:		/* SensingMethod */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 1:
		      human = "Not defined";
		      break;
		  case 2:
		      human = "One Chip Color Area Sensor";
		      break;
		  case 3:
		      human = "Two Chip Color Area Sensor";
		      break;
		  case 4:
		      human = "Three Chip Color Area Sensor";
		      break;
		  case 5:
		      human = "Color Sequential Area Sensor";
		      break;
		  case 7:
		      human = "Trilinear Sensor";
		      break;
		  case 8:
		      human = "Color Sequential Linear Sensor";
		      break;
		  };
	    }
	  break;
      case 0xA406:		/* SceneCaptureType */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 0:
		      human = "Standard";
		      break;
		  case 1:
		      human = "Landscape";
		      break;
		  case 2:
		      human = "Portrait";
		      break;
		  case 3:
		      human = "Night scene";
		      break;
		  };
	    }
	  break;
      case 0xA407:		/* GainControl */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 0:
		      human = "None";
		      break;
		  case 1:
		      human = "Low gain up";
		      break;
		  case 2:
		      human = "High gain up";
		      break;
		  case 3:
		      human = "Low gain down";
		      break;
		  case 4:
		      human = "High gain down";
		      break;
		  };
	    }
	  break;
      case 0xA408:		/* Contrast */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 0:
		      human = "Normal";
		      break;
		  case 1:
		      human = "Soft";
		      break;
		  case 2:
		      human = "Hard";
		      break;
		  };
	    }
	  break;
      case 0xA409:		/* Saturation */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 0:
		      human = "Normal";
		      break;
		  case 1:
		      human = "Low saturation";
		      break;
		  case 2:
		      human = "High saturation";
		      break;
		  };
	    }
	  break;
      case 0xA40A:		/* Sharpness */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 0:
		      human = "Normal";
		      break;
		  case 1:
		      human = "Soft";
		      break;
		  case 2:
		      human = "Hard";
		      break;
		  };
	    }
	  break;
      case 0xA40C:		/* SubjectDistanceRange */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 0:
		      human = "Unknown";
		      break;
		  case 1:
		      human = "Macro";
		      break;
		  case 2:
		      human = "Close view";
		      break;
		  case 3:
		      human = "Distant view";
		      break;
		  };
	    }
	  break;
      case 0x9208:		/* LightSource */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 0:
		      human = "Unknown";
		      break;
		  case 1:
		      human = "Daylight";
		      break;
		  case 2:
		      human = "Fluorescent";
		      break;
		  case 3:
		      human = "Tungsten (incandescent light)";
		      break;
		  case 4:
		      human = "Flash";
		      break;
		  case 9:
		      human = "Fine weather";
		      break;
		  case 10:
		      human = "Cloudy weather";
		      break;
		  case 11:
		      human = "Shade";
		  case 12:
		      human = "Daylight fluorescent (D 5700 – 7100K)";
		      break;
		  case 13:
		      human = "Day white fluorescent (N 4600 – 5400K)";
		      break;
		  case 14:
		      human = "Cool white fluorescent (W 3900 – 4500K)";
		  case 15:
		      human = "White fluorescent (WW 3200 – 3700K)";
		      break;
		  case 17:
		      human = "Standard light A";
		      break;
		  case 18:
		      human = "Standard light B";
		      break;
		  case 19:
		      human = "Standard light C";
		      break;
		  case 20:
		      human = "D55";
		      break;
		  case 21:
		      human = "D65";
		      break;
		  case 22:
		      human = "D75";
		      break;
		  case 23:
		      human = "D50";
		      break;
		  case 24:
		      human = "ISO studio tungsten";
		      break;
		  case 255:
		      human = "other light source";
		      break;
		  };
	    }
	  break;
      case 0xA001:		/* ColorSpace */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		switch (*(tag->ShortValues + 0))
		  {
		  case 1:
		      human = "sRGB";
		      break;
		  case 0xffff:
		      human = "Uncalibrated";
		      break;
		  };
	    }
	  break;
      case 0x8827:		/* ISOSpeedRatings */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		sprintf (dummy, "%u ISO", *(tag->ShortValues + 0));
		human = dummy;
	    }
	  break;
      case 0xA002:		/* ExifImageWidth */
      case 0xA003:		/* ExifImageLength */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		sprintf (dummy, "%u pixels", *(tag->ShortValues + 0));
		human = dummy;
	    }
	  else if (tag->Type == 4 && tag->Count == 1)
	    {
		sprintf (dummy, "%u pixels", *(tag->LongValues + 0));
		human = dummy;
	    }
	  break;
      case 0x829A:		/* ExposureTime */
	  if (tag->Type == 5 && tag->Count == 1)
	    {
		dblval = gaiaExifTagGetRationalValue (tag, 0, &xok);
		if (xok)
		  {
		      if (dblval < 1.0)
			{
			    dblval = 1.0 / dblval;
			    sprintf (dummy, "1/%1.0lf sec", dblval);
			    human = dummy;
			}
		      else
			{
			    sprintf (dummy, "%1.0lf sec", dblval);
			    human = dummy;
			}
		  }
	    }
	  break;
      case 0x9201:		/* ShutterSpeedValue */
	  if (tag->Type == 10 && tag->Count == 1)
	    {
		dblval = gaiaExifTagGetSignedRationalValue (tag, 0, &xok);
		if (xok)
		  {
		      dblval = exp (dblval * log (2));
		      if (dblval > 1.0)
			  dblval = floor (dblval);
		      if (dblval < 1.0)
			{
			    dblval = math_round (1.0 / dblval);
			    sprintf (dummy, "%1.0lf sec", dblval);
			    human = dummy;
			}
		      else
			{
			    sprintf (dummy, "1/%1.0lf sec", dblval);
			    human = dummy;
			}
		  }
	    }
	  break;
      case 0x829D:		/* FNumber */
	  if (tag->Type == 5 && tag->Count == 1)
	    {
		dblval = gaiaExifTagGetRationalValue (tag, 0, &xok);
		if (xok)
		  {
		      sprintf (dummy, "F %1.1lf", dblval);
		      human = dummy;
		  }
	    }
	  break;
      case 0x9202:		/* ApertureValue */
      case 0x9205:		/* MaxApertureValue */
	  if (tag->Type == 5 && tag->Count == 1)
	    {
		dblval = gaiaExifTagGetRationalValue (tag, 0, &xok);
		if (xok)
		  {
		      dblval = exp ((dblval * log (2)) / 2.0);
		      sprintf (dummy, "F %1.1lf", dblval);
		      human = dummy;
		  }
	    }
	  break;
      case 0x920A:		/* FocalLength */
	  if (tag->Type == 5 && tag->Count == 1)
	    {
		dblval = gaiaExifTagGetRationalValue (tag, 0, &xok);
		if (xok)
		  {
		      sprintf (dummy, "%1.1lf mm", dblval);
		      human = dummy;
		  }
	    }
	  break;
      case 0xA405:		/* FocalLengthIn35mmFilm */
	  if (tag->Type == 3 && tag->Count == 1)
	    {
		sprintf (dummy, "%u mm", *(tag->ShortValues + 0));
		human = dummy;
	    }
	  break;
      case 0x9204:		/* ExposureBiasValue */
	  if (tag->Type == 10 && tag->Count == 1)
	    {
		dblval = gaiaExifTagGetSignedRationalValue (tag, 0, &xok);
		if (xok)
		  {
		      sprintf (dummy, "%1.2lf EV", dblval);
		      human = dummy;
		  }
	    }
	  break;
      };
    l = strlen (human);
    if (l > 0)
      {
	  if (len > l)
	      strcpy (str, human);
	  else
	    {
		memset (str, '\0', len);
		memcpy (str, human, len - 1);
	    }
	  *ok = 1;
	  return;
      }
    *ok = 0;
}

GAIAEXIF_DECLARE int
gaiaGuessBlobType (const unsigned char *blob, int size)
{
/* returns the BLOB content type */
    int jpeg = 0;
    int exif = 0;
    int exif_gps = 0;
    int geom = 1;
    gaiaExifTagListPtr exif_list;
    gaiaExifTagPtr pT;
    unsigned char jpeg1_signature[2];
    unsigned char jpeg2_signature[2];
    unsigned char jpeg3_signature[4];
    unsigned char jfif_signature[4];
    unsigned char exif_signature[4];
    unsigned char png_signature[8];
    unsigned char zip_signature[4];
    jpeg1_signature[0] = 0xff;
    jpeg1_signature[1] = 0xd8;
    jpeg2_signature[0] = 0xff;
    jpeg2_signature[1] = 0xd9;
    jpeg3_signature[0] = 0xff;
    jpeg3_signature[1] = 0xd8;
    jpeg3_signature[2] = 0xff;
    jpeg3_signature[3] = 0xe0;
    jfif_signature[0] = 0x4a;
    jfif_signature[1] = 0x46;
    jfif_signature[2] = 0x49;
    jfif_signature[3] = 0x46;
    exif_signature[0] = 0x45;
    exif_signature[1] = 0x78;
    exif_signature[2] = 0x69;
    exif_signature[3] = 0x66;
    png_signature[0] = 0x89;
    png_signature[1] = 0x50;
    png_signature[2] = 0x4e;
    png_signature[3] = 0x47;
    png_signature[4] = 0x0d;
    png_signature[5] = 0x0a;
    png_signature[6] = 0x1a;
    png_signature[7] = 0x0a;
    zip_signature[0] = 0x50;
    zip_signature[1] = 0x4b;
    zip_signature[2] = 0x03;
    zip_signature[3] = 0x04;
    if (size < 1 || !blob)
	return GAIA_HEX_BLOB;
    if (size > 5)
      {
	  if (strncmp ((char *) blob, "%PDF-", 5) == 0)
	      return GAIA_PDF_BLOB;
      }
    if (size > 4)
      {
	  if (memcmp (blob, zip_signature, 4) == 0)
	      return GAIA_ZIP_BLOB;
      }
    if (size > 6)
      {
	  if (strncmp ((char *) blob, "GIF87a", 6) == 0
	      || strncmp ((char *) blob, "GIF89a", 6) == 0)
	      return GAIA_GIF_BLOB;
      }
    if (size > 8)
      {
	  if (memcmp (blob, png_signature, 8) == 0)
	      return GAIA_PNG_BLOB;
      }
    if (size > 4)
      {
	  if (memcmp (blob, jpeg1_signature, 2) == 0
	      && memcmp (blob + size - 2, jpeg2_signature, 2) == 0)
	      jpeg = 1;		/* this one is the standard JPEG signature */
	  if (memcmp (blob, jpeg3_signature, 4) == 0)
	      jpeg = 1;		/* another common JPEG signature */
      }
    if (size > 10)
      {
	  if (memcmp (blob + 6, jfif_signature, 4) == 0)
	      jpeg = 1;		/* standard JFIF signature */
	  if (memcmp (blob + 6, exif_signature, 4) == 0)
	      jpeg = 1;		/* standard EXIF signature */
      }
    if (jpeg)
      {
	  exif_list = gaiaGetExifTags (blob, size);
	  if (exif_list)
	    {
		exif = 1;
		pT = exif_list->First;
		while (pT)
		  {
		      if (pT->Gps)
			{
			    exif_gps = 1;
			    break;
			}
		      pT = pT->Next;
		  }
		gaiaExifTagsFree (exif_list);
	    }
      }
    if (jpeg && exif && exif_gps)
	return GAIA_EXIF_GPS_BLOB;
    if (jpeg && exif)
	return GAIA_EXIF_BLOB;
    if (jpeg)
	return GAIA_JPEG_BLOB;
/* testing for GEOMETRY */
    if (size < 45)
	geom = 0;
    else
      {
	  if (*(blob + 0) != GAIA_MARK_START)
	      geom = 0;
	  if (*(blob + (size - 1)) != GAIA_MARK_END)
	      geom = 0;
	  if (*(blob + 38) != GAIA_MARK_MBR)
	      geom = 0;
	  if (*(blob + 1) == 0 || *(blob + 1) == 1)
	      ;
	  else
	      geom = 0;
      }
    if (geom)
	return GAIA_GEOMETRY_BLOB;
    return GAIA_HEX_BLOB;
}

GAIAEXIF_DECLARE int
gaiaGetGpsCoords (const unsigned char *blob, int size, double *longitude,
		  double *latitude)
{
/* returns the ExifGps coords, if they exists */
    gaiaExifTagListPtr exif_list;
    gaiaExifTagPtr pT;
    char lat_ref = '\0';
    char long_ref = '\0';
    double lat_degs = -DBL_MAX;
    double lat_mins = -DBL_MAX;
    double lat_secs = -DBL_MAX;
    double long_degs = -DBL_MAX;
    double long_mins = -DBL_MAX;
    double long_secs = -DBL_MAX;
    double dblval;
    double sign;
    int ok;
    if (size < 1 || !blob)
	return 0;
    exif_list = gaiaGetExifTags (blob, size);
    if (exif_list)
      {
	  pT = exif_list->First;
	  while (pT)
	    {
		if (pT->Gps && pT->TagId == 0x01)
		  {
		      // ok, this one is the GPSLatitudeRef tag
		      if (pT->Type == 2)
			  lat_ref = *(pT->StringValue);
		  }
		if (pT->Gps && pT->TagId == 0x03)
		  {
		      // ok, this one is the GPSLongitudeRef tag
		      if (pT->Type == 2)
			  long_ref = *(pT->StringValue);
		  }
		if (pT->Gps && pT->TagId == 0x02)
		  {
		      // ok, this one is the GPSLatitude tag 
		      if (pT->Type == 5 && pT->Count == 3)
			{
			    dblval = gaiaExifTagGetRationalValue (pT, 0, &ok);
			    if (ok)
				lat_degs = dblval;
			    dblval = gaiaExifTagGetRationalValue (pT, 1, &ok);
			    if (ok)
				lat_mins = dblval;
			    dblval = gaiaExifTagGetRationalValue (pT, 2, &ok);
			    if (ok)
				lat_secs = dblval;
			}
		  }
		if (pT->Gps && pT->TagId == 0x04)
		  {
		      // ok, this one is the GPSLongitude tag
		      if (pT->Type == 5 && pT->Count == 3)
			{
			    dblval = gaiaExifTagGetRationalValue (pT, 0, &ok);
			    if (ok)
				long_degs = dblval;
			    dblval = gaiaExifTagGetRationalValue (pT, 1, &ok);
			    if (ok)
				long_mins = dblval;
			    dblval = gaiaExifTagGetRationalValue (pT, 2, &ok);
			    if (ok)
				long_secs = dblval;
			}
		  }
		pT = pT->Next;
	    }
	  gaiaExifTagsFree (exif_list);
	  if ((lat_ref == 'N' || lat_ref == 'S' || long_ref == 'E'
	       || long_ref == 'W') && lat_degs != -DBL_MAX
	      && lat_mins != -DBL_MAX && lat_secs != -DBL_MAX
	      && long_degs != -DBL_MAX && long_mins != -DBL_MAX
	      && long_secs != -DBL_MAX)
	    {
		if (lat_ref == 'S')
		    sign = -1.0;
		else
		    sign = 1.0;
		lat_degs = math_round (lat_degs * 1000000.0);
		lat_mins = math_round (lat_mins * 1000000.0);
		lat_secs = math_round (lat_secs * 1000000.0);
		dblval =
		    math_round (lat_degs + (lat_mins / 60.0) +
				(lat_secs / 3600.0)) * (sign / 1000000.0);
		*latitude = dblval;
		if (long_ref == 'W')
		    sign = -1.0;
		else
		    sign = 1.0;
		long_degs = math_round (long_degs * 1000000.0);
		long_mins = math_round (long_mins * 1000000.0);
		long_secs = math_round (long_secs * 1000000.0);
		dblval =
		    math_round (long_degs + (long_mins / 60.0) +
				(long_secs / 3600.0)) * (sign / 1000000.0);
		*longitude = dblval;
		return 1;
	    }
      }
    return 0;
}

GAIAEXIF_DECLARE int
gaiaGetGpsLatLong (const unsigned char *blob, int size, char *latlong,
		   int ll_size)
{
/* returns the ExifGps Latitude and Longitude, if they exists */
    gaiaExifTagListPtr exif_list;
    gaiaExifTagPtr pT;
    char lat_ref = '\0';
    char long_ref = '\0';
    double lat_degs = -DBL_MAX;
    double lat_mins = -DBL_MAX;
    double lat_secs = -DBL_MAX;
    double long_degs = -DBL_MAX;
    double long_mins = -DBL_MAX;
    double long_secs = -DBL_MAX;
    double dblval;
    int ok;
    char ll[1024];
    int len;
    *latlong = '\0';
    if (size < 1 || !blob)
	return 0;
    exif_list = gaiaGetExifTags (blob, size);
    if (exif_list)
      {
	  pT = exif_list->First;
	  while (pT)
	    {
		if (pT->Gps && pT->TagId == 0x01)
		  {
		      // ok, this one is the GPSLatitudeRef tag
		      if (pT->Type == 2)
			  lat_ref = *(pT->StringValue);
		  }
		if (pT->Gps && pT->TagId == 0x03)
		  {
		      // ok, this one is the GPSLongitudeRef tag
		      if (pT->Type == 2)
			  long_ref = *(pT->StringValue);
		  }
		if (pT->Gps && pT->TagId == 0x02)
		  {
		      // ok, this one is the GPSLatitude tag 
		      if (pT->Type == 5 && pT->Count == 3)
			{
			    dblval = gaiaExifTagGetRationalValue (pT, 0, &ok);
			    if (ok)
				lat_degs = dblval;
			    dblval = gaiaExifTagGetRationalValue (pT, 1, &ok);
			    if (ok)
				lat_mins = dblval;
			    dblval = gaiaExifTagGetRationalValue (pT, 2, &ok);
			    if (ok)
				lat_secs = dblval;
			}
		  }
		if (pT->Gps && pT->TagId == 0x04)
		  {
		      // ok, this one is the GPSLongitude tag
		      if (pT->Type == 5 && pT->Count == 3)
			{
			    dblval = gaiaExifTagGetRationalValue (pT, 0, &ok);
			    if (ok)
				long_degs = dblval;
			    dblval = gaiaExifTagGetRationalValue (pT, 1, &ok);
			    if (ok)
				long_mins = dblval;
			    dblval = gaiaExifTagGetRationalValue (pT, 2, &ok);
			    if (ok)
				long_secs = dblval;
			}
		  }
		pT = pT->Next;
	    }
	  gaiaExifTagsFree (exif_list);
	  if ((lat_ref == 'N' || lat_ref == 'S' || long_ref == 'E'
	       || long_ref == 'W') && lat_degs != -DBL_MAX
	      && lat_mins != -DBL_MAX && lat_secs != -DBL_MAX
	      && long_degs != -DBL_MAX && long_mins != -DBL_MAX
	      && long_secs != -DBL_MAX)
	    {
		sprintf (ll,
			 "%c %1.2lf %1.2lf %1.2lf / %c %1.2lf %1.2lf %1.2lf",
			 lat_ref, lat_degs, lat_mins, lat_secs, long_ref,
			 long_degs, long_mins, long_secs);
		len = strlen (ll);
		if (len < ll_size)
		    strcpy (latlong, ll);
		else
		  {
		      memcpy (latlong, ll, ll_size - 1);
		      latlong[ll_size] = '\0';
		  }
		return 1;
	    }
      }
    return 0;
}
/**************** End file: gaia_exif.c **********/


/**************** Begin file: gg_advanced.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <math.h> */
/* #include <float.h> */

/* #include <spatialite/sqlite3ext.h> */
/* #include <spatialite/gaiageo.h> */

GAIAGEO_DECLARE double
gaiaMeasureLength (double *coords, int vert)
{
/* computes the total length */
    double lung = 0.0;
    double xx1;
    double xx2;
    double yy1;
    double yy2;
    double x;
    double y;
    double dist;
    int ind;

    gaiaGetPoint (coords, 0, &xx1, &yy1);

    for (ind = 1; ind < vert; ind++)
      {
		gaiaGetPoint (coords, ind, &xx2, &yy2);
		x = xx1 - xx2;
		y = yy1 - yy2;
		dist = sqrt ((x * x) + (y * y));
		lung += dist;
		xx1 = xx2;
		yy1 = yy2;
      }
    return lung;
}

GAIAGEO_DECLARE double
gaiaMeasureArea (gaiaRingPtr ring)
{
/* computes the area */
    int iv;
    double xx;
    double yy;
    double x;
    double y;
    double area = 0.0;
    if (!ring)
	return 0.0;
    gaiaGetPoint (ring->Coords, 0, &xx, &yy);
    for (iv = 1; iv < ring->Points; iv++)
      {
	  gaiaGetPoint (ring->Coords, iv, &x, &y);
	  area += ((xx * y) - (x * yy));
	  xx = x;
	  yy = y;
      }
    area /= 2.0;
    return fabs (area);
}

GAIAGEO_DECLARE void
gaiaRingCentroid (gaiaRingPtr ring, double *rx, double *ry)
{
/* computes the simple ring centroid */
    double cx = 0.0;
    double cy = 0.0;
    double xx;
    double yy;
    double x;
    double y;
    double coeff;
    double area;
    double term;
    int iv;
    if (!ring)
      {
	  *rx = -DBL_MAX;
	  *ry = -DBL_MAX;
	  return;
      }
    area = gaiaMeasureArea (ring);
    coeff = 1.0 / (area * 6.0);
    gaiaGetPoint (ring->Coords, 0, &xx, &yy);
    for (iv = 0; iv < ring->Points; iv++)
      {
	  gaiaGetPoint (ring->Coords, iv, &x, &y);
	  term = (xx * y) - (x * yy);
	  cx += (xx + x) * term;
	  cy += (yy + y) * term;
	  xx = x;
	  yy = y;
      }
    *rx = fabs (cx * coeff);
    *ry = fabs (cy * coeff);
}

GAIAGEO_DECLARE void
gaiaClockwise (gaiaRingPtr p)
{
/* determines clockwise or anticlockwise direction */
    int ind;
    int ix;
    double xx;
    double yy;
    double x;
    double y;
    double area = 0.0;
    gaiaGetPoint (p->Coords, 0, &x, &y);
    for (ind = 0; ind < p->Points; ind++)
      {
	  gaiaGetPoint (p->Coords, ind, &xx, &yy);
	  ix = (ind + 1) % p->Points;
	  gaiaGetPoint (p->Coords, ix, &x, &y);
	  area += ((xx * y) - (x * yy));
      }
    area /= 2.0;
    if (area >= 0.0)
	p->Clockwise = 0;
    else
	p->Clockwise = 1;
}

GAIAGEO_DECLARE int
gaiaIsPointOnRingSurface (gaiaRingPtr ring, double pt_x, double pt_y)
{
/* tests if a POINT falls inside a RING */
    int isInternal = 0;
    int cnt;
    int i;
    int j;
    double x;
    double y;
    double *vert_x;
    double *vert_y;
    double minx = DBL_MAX;
    double miny = DBL_MAX;
    double maxx = -DBL_MAX;
    double maxy = -DBL_MAX;
    cnt = ring->Points;
    cnt--;			/* ignoring last vertex because surely identical to the first one */
    if (cnt < 2)
	return 0;
/* allocating and loading an array of vertices */
    vert_x = malloc (sizeof (double) * (cnt));
    vert_y = malloc (sizeof (double) * (cnt));
    for (i = 0; i < cnt; i++)
      {
	  gaiaGetPoint (ring->Coords, i, &x, &y);
	  vert_x[i] = x;
	  vert_y[i] = y;
	  if (x < minx)
	      minx = x;
	  if (x > maxx)
	      maxx = x;
	  if (y < miny)
	      miny = y;
	  if (y > maxy)
	      maxy = y;
      }
    for (i = 0, j = cnt - 1; i < cnt; j = i++)
      {
/* The definitive reference is "Point in Polyon Strategies" by
/  Eric Haines [Gems IV]  pp. 24-46.
/  The code in the Sedgewick book Algorithms (2nd Edition, p.354) is 
/  incorrect.
*/
	  if ((((vert_y[i] <= pt_y) && (pt_y < vert_y[j]))
	       || ((vert_y[j] <= pt_y) && (pt_y < vert_y[i])))
	      && (pt_x <
		  (vert_x[j] - vert_x[i]) * (pt_y - vert_y[i]) / (vert_y[j] -
								  vert_y[i]) +
		  vert_x[i]))
	      isInternal = !isInternal;
      }
    free (vert_x);
    free (vert_y);
    return isInternal;
}

GAIAGEO_DECLARE double
gaiaMinDistance (double x0, double y0, double *coords, int n_vert)
{
/* computing minimal distance between a POINT and a linestring/ring */
    double x;
    double y;
    double ox;
    double oy;
    double lineMag;
    double u;
    double px;
    double py;
    double dist;
    double min_dist = DBL_MAX;
    int iv;
    if (n_vert < 2)
	return min_dist;	/* not a valid linestring */
/* computing distance from first vertex */
    ox = *(coords + 0);
    oy = *(coords + 1);
    min_dist = sqrt (((x0 - ox) * (x0 - ox)) + ((y0 - oy) * (y0 - oy)));
    for (iv = 1; iv < n_vert; iv++)
      {
	  /* segment start-end coordinates */
	  gaiaGetPoint (coords, iv - 1, &ox, &oy);
	  gaiaGetPoint (coords, iv, &x, &y);
	  /* computing distance from vertex */
	  dist = sqrt (((x0 - x) * (x0 - x)) + ((y0 - y) * (y0 - y)));
	  if (dist < min_dist)
	      min_dist = dist;
	  /* computing a projection */
	  lineMag = ((x - ox) * (x - ox)) + ((y - oy) * (y - oy));
	  u = (((x0 - ox) * (x - ox)) + ((y0 - oy) * (y - oy))) / lineMag;
	  if (u < 0.0 || u > 1.0)
	      ;			/* closest point does not fall within the line segment */
	  else
	    {
		px = ox + u * (x - ox);
		py = oy + u * (y - oy);
		dist = sqrt (((x0 - px) * (x0 - px)) + ((y0 - py) * (y0 - py)));
		if (dist < min_dist)
		    min_dist = dist;
	    }
      }
    return min_dist;
}

GAIAGEO_DECLARE int
gaiaIsPointOnPolygonSurface (gaiaPolygonPtr polyg, double x, double y)
{
/* tests if a POINT falls inside a POLYGON */
    int ib;
    gaiaRingPtr ring = polyg->Exterior;
    if (gaiaIsPointOnRingSurface (ring, x, y))
      {
	  /* ok, the POINT falls inside the polygon */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		ring = polyg->Interiors + ib;
		if (gaiaIsPointOnRingSurface (ring, x, y))
		  {
		      /* no, the POINT fall inside some hole */
		      return 0;
		  }
	    }
	  return 1;
      }
    return 0;
}

GAIAGEO_DECLARE int
gaiaIntersect (double *x0, double *y0, double x1, double y1, double x2,
	       double y2, double x3, double y3, double x4, double y4)
{
/* computes intersection [if any] between two line segments
/  the intersection POINT has coordinates (x0, y0) 
/  first line is identified by(x1, y1)  and (x2, y2)
/  second line is identified by (x3, y3) and (x4, y4)
*/
    double x;
    double y;
    double a1;
    double b1;
    double c1;
    double a2;
    double b2;
    double c2;
    double m1;
    double m2;
    double p;
    double det_inv;
    double minx1;
    double miny1;
    double maxx1;
    double maxy1;
    double minx2;
    double miny2;
    double maxx2;
    double maxy2;
    int ok1 = 0;
    int ok2 = 0;
/* building line segment's MBRs */
    if (x2 < x1)
      {
	  minx1 = x2;
	  maxx1 = x1;
      }
    else
      {
	  minx1 = x1;
	  maxx1 = x2;
      }
    if (y2 < y1)
      {
	  miny1 = y2;
	  maxy1 = y1;
      }
    else
      {
	  miny1 = y1;
	  maxy1 = y2;
      }
    if (x4 < x3)
      {
	  minx2 = x4;
	  maxx2 = x3;
      }
    else
      {
	  minx2 = x3;
	  maxx2 = x4;
      }
    if (y4 < y3)
      {
	  miny2 = y4;
	  maxy2 = y3;
      }
    else
      {
	  miny2 = y3;
	  maxy2 = y4;
      }
/* checkinkg MBRs first */
    if (minx1 >= maxx2)
	return 0;
    if (miny1 >= maxy2)
	return 0;
    if (maxx1 <= minx2)
	return 0;
    if (maxy1 <= miny2)
	return 0;
    if (minx2 >= maxx1)
	return 0;
    if (miny2 >= maxy1)
	return 0;
    if (maxx2 <= minx1)
	return 0;
    if (maxy2 <= miny1)
	return 0;
/* there is an MBRs intersection - proceeding */
    if ((x2 - x1) != 0.0)
	m1 = (y2 - y1) / (x2 - x1);
    else
	m1 = DBL_MAX;
    if ((x4 - x3) != 0)
	m2 = (y4 - y3) / (x4 - x3);
    else
	m2 = DBL_MAX;
    if (m1 == m2)		/* parallel lines */
	return 0;
    if (m1 == DBL_MAX)
	c1 = y1;
    else
	c1 = (y1 - m1 * x1);
    if (m2 == DBL_MAX)
	c2 = y3;
    else
	c2 = (y3 - m2 * x3);
    if (m1 == DBL_MAX)
      {
	  x = x1;
	  p = m2 * x1;
	  y = p + c2;		/*  first line is vertical */
	  goto check_bbox;
      }
    if (m2 == DBL_MAX)
      {
	  x = x3;
	  p = m1 * x3;
	  y = p + c1;		/* second line is vertical */
	  goto check_bbox;
      }
    a1 = m1;
    a2 = m2;
    b1 = -1;
    b2 = -1;
    det_inv = 1 / (a1 * b2 - a2 * b1);
    x = ((b1 * c2 - b2 * c1) * det_inv);
    y = ((a2 * c1 - a1 * c2) * det_inv);
/* now checking if intersection falls within both segment boundaries */
  check_bbox:
    if (x >= minx1 && x <= maxx1 && y >= miny1 && y <= maxy1)
	ok1 = 1;
    if (x >= minx2 && x <= maxx2 && y >= miny2 && y <= maxy2)
	ok2 = 1;
    if (ok1 && ok2)
      {
	  /* intersection point falls within the segments */
	  *x0 = x;
	  *y0 = y;
	  return 1;
      }
    return 0;
}

static void
appendRingLine (gaiaDynamicLinePtr dyn, gaiaLinestringPtr line, int reverse)
{
/* appending a line to an already existing ring */
    int i;
    double x;
    double y;
    if (!reverse)
      {
	  /* appending points (except the first one) in natural order) */
	  for (i = 1; i < line->Points; i++)
	    {
		gaiaGetPoint (line->Coords, i, &x, &y);
		gaiaAppendPointToDynamicLine (dyn, x, y);
	    }
      }
    else
      {
	  /* appending points (except the last one) in reverse order) */
	  for (i = line->Points - 2; i >= 0; i--)
	    {
		gaiaGetPoint (line->Coords, i, &x, &y);
		gaiaAppendPointToDynamicLine (dyn, x, y);
	    }
      }
}

static void
prependRingLine (gaiaDynamicLinePtr dyn, gaiaLinestringPtr line, int reverse)
{
/* prepending a line to an already existing ring */
    int i;
    double x;
    double y;
    if (!reverse)
      {
	  /* prepending points (except the first one) in natural order) */
	  for (i = 1; i < line->Points; i++)
	    {
		gaiaGetPoint (line->Coords, i, &x, &y);
		gaiaPrependPointToDynamicLine (dyn, x, y);
	    }
      }
    else
      {
	  /* prepending points (except the last one) in reverse order) */
	  for (i = line->Points - 2; i >= 0; i--)
	    {
		gaiaGetPoint (line->Coords, i, &x, &y);
		gaiaPrependPointToDynamicLine (dyn, x, y);
	    }
      }
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaBuildRings (gaiaGeomCollPtr line_geom)
{
/* trying to build a set of RINGs by reassembling a set of sparse LINESTRINGs */
    gaiaLinestringPtr ln;
    gaiaPreRingPtr first = NULL;
    gaiaPreRingPtr last = NULL;
    gaiaPreRingPtr pre;
    gaiaPreRingPtr pre2;
    gaiaDynamicLinePtr dyn = NULL;
    gaiaGeomCollPtr ring_geom = NULL;
    gaiaPointPtr pt;
    int ok;
    int i;
    int cnt;
    int srid;
    double x0;
    double y0;
    double xn;
    double yn;
    if (line_geom->FirstPoint || line_geom->FirstPolygon)
	return NULL;
    ln = line_geom->FirstLinestring;
    while (ln)
      {
	  /* feeding the pre-rings struct */
	  pre = malloc (sizeof (gaiaPreRing));
	  pre->Line = ln;
	  pre->AlreadyUsed = 0;
	  pre->Next = NULL;
	  if (!first)
	      first = pre;
	  if (last)
	      last->Next = pre;
	  last = pre;
	  ln = ln->Next;
      }
    if (!first)
	return NULL;
    srid = line_geom->Srid;
/* now we'll try to reassemble the line elements into closed rings */
    dyn = NULL;
    ok = 1;
    while (ok)
      {
	  /* looping on line elements */
	  ok = 0;
	  pre = first;
	  while (pre)
	    {
		if (pre->AlreadyUsed)
		  {
		      pre = pre->Next;
		      continue;
		  }
		if (dyn)
		  {
		      /* there is a current ring; adding a consecutive line */
		      gaiaGetPoint (pre->Line->Coords, 0, &x0, &y0);
		      gaiaGetPoint (pre->Line->Coords, pre->Line->Points - 1,
				    &xn, &yn);
		      if (dyn->Last->X == x0 && dyn->Last->Y == y0)
			{
			    /* appending in natural direction */
			    appendRingLine (dyn, pre->Line, 0);
			    pre->AlreadyUsed = 1;
			    ok = 1;
			    break;
			}
		      if (dyn->Last->X == xn && dyn->Last->Y == yn)
			{
			    /* appending in reverse direction */
			    appendRingLine (dyn, pre->Line, 1);
			    pre->AlreadyUsed = 1;
			    ok = 1;
			    break;
			}
		      if (dyn->First->X == x0 && dyn->First->Y == y0)
			{
			    /* prepending in natural direction */
			    prependRingLine (dyn, pre->Line, 0);
			    pre->AlreadyUsed = 1;
			    ok = 1;
			    break;
			}
		      if (dyn->First->X == xn && dyn->First->Y == yn)
			{
			    /* prepending in reverse direction */
			    prependRingLine (dyn, pre->Line, 1);
			    pre->AlreadyUsed = 1;
			    ok = 1;
			    break;
			}
		  }
		else
		  {
		      /* there is no current ring; starting a new one */
		      dyn =
			  gaiaCreateDynamicLine (pre->Line->Coords,
						 pre->Line->Points);
		      pre->AlreadyUsed = 1;
		      ok = 1;
		      break;
		  }
		pre = pre->Next;
	    }
	  if (!dyn)
	      break;
	  if (!ok)
	      break;
	  if (dyn->First->X == dyn->Last->X && dyn->First->Y == dyn->Last->Y)
	    {
		/* we have found a closed ring */
		cnt = 0;
		pt = dyn->First;
		while (pt)
		  {
		      /* counting how many points are into the dynamic line */
		      cnt++;
		      pt = pt->Next;
		  }
		if (!ring_geom)
		  {
		      ring_geom = gaiaAllocGeomColl ();
		      ring_geom->Srid = srid;
		  }
		ln = gaiaAddLinestringToGeomColl (ring_geom, cnt);
		i = 0;
		pt = dyn->First;
		while (pt)
		  {
		      /* copying points into the closed ring */
		      gaiaSetPoint (ln->Coords, i, pt->X, pt->Y);
		      i++;
		      pt = pt->Next;
		  }
		gaiaFreeDynamicLine (dyn);
		dyn = NULL;
	    }
      }
    ok = 1;
    pre = first;
    while (pre)
      {
	  /* memory cleanup; pre-rings structs */
	  pre2 = pre->Next;
	  if (!(pre->AlreadyUsed))
	      ok = 0;
	  free (pre);
	  pre = pre2;
      }
/* checking for validity */
    if (!ring_geom)
	return NULL;
    if (!ok)
      {
	  gaiaFreeGeomColl (ring_geom);
	  return NULL;
      }
    return ring_geom;
}
/**************** End file: gg_advanced.c **********/


/**************** Begin file: gg_endian.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */

/* #include <spatialite/sqlite3ext.h> */
/* #include <spatialite/gaiageo.h> */

GAIAGEO_DECLARE int
gaiaEndianArch ()
{
/* checking if target CPU is a little-endian one */
    union cvt
    {
	unsigned char byte[4];
	int int_value;
    } convert;
    convert.int_value = 1;
    if (convert.byte[0] == 0)
	return 0;
    return 1;
}

GAIAGEO_DECLARE short
gaiaImport16 (const unsigned char *p, int little_endian, int little_endian_arch)
{
/* fetches a 16bit int from BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[2];
	short short_value;
    } convert;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 1);
		convert.byte[1] = *(p + 0);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 1);
		convert.byte[1] = *(p + 0);
	    }
      }
    return convert.short_value;
}

GAIAGEO_DECLARE int
gaiaImport32 (const unsigned char *p, int little_endian, int little_endian_arch)
{
/* fetches a 32bit int from BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[4];
	int int_value;
    } convert;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 3);
		convert.byte[1] = *(p + 2);
		convert.byte[2] = *(p + 1);
		convert.byte[3] = *(p + 0);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 3);
		convert.byte[1] = *(p + 2);
		convert.byte[2] = *(p + 1);
		convert.byte[3] = *(p + 0);
	    }
      }
    return convert.int_value;
}

GAIAGEO_DECLARE double
gaiaImport64 (const unsigned char *p, int little_endian, int little_endian_arch)
{
/* fetches a 64bit double from BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[8];
	double double_value;
    } convert;
    if (little_endian_arch)
      {
/* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 7);
		convert.byte[1] = *(p + 6);
		convert.byte[2] = *(p + 5);
		convert.byte[3] = *(p + 4);
		convert.byte[4] = *(p + 3);
		convert.byte[5] = *(p + 2);
		convert.byte[6] = *(p + 1);
		convert.byte[7] = *(p + 0);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
		convert.byte[4] = *(p + 4);
		convert.byte[5] = *(p + 5);
		convert.byte[6] = *(p + 6);
		convert.byte[7] = *(p + 7);
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		convert.byte[0] = *(p + 0);
		convert.byte[1] = *(p + 1);
		convert.byte[2] = *(p + 2);
		convert.byte[3] = *(p + 3);
		convert.byte[4] = *(p + 4);
		convert.byte[5] = *(p + 5);
		convert.byte[6] = *(p + 6);
		convert.byte[7] = *(p + 7);
	    }
	  else
	    {
		/* Little Endian data */
		convert.byte[0] = *(p + 7);
		convert.byte[1] = *(p + 6);
		convert.byte[2] = *(p + 5);
		convert.byte[3] = *(p + 4);
		convert.byte[4] = *(p + 3);
		convert.byte[5] = *(p + 2);
		convert.byte[6] = *(p + 1);
		convert.byte[7] = *(p + 0);
	    }
      }
    return convert.double_value;
}

GAIAGEO_DECLARE void
gaiaExport16 (unsigned char *p, short value, int little_endian,
	      int little_endian_arch)
{
/* stores a 16bit int into a BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[2];
	short short_value;
    } convert;
    convert.short_value = value;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 1) = convert.byte[1];
		*(p + 0) = convert.byte[0];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 1) = convert.byte[0];
		*(p + 0) = convert.byte[1];
	    }
      }
}

GAIAGEO_DECLARE void
gaiaExport32 (unsigned char *p, int value, int little_endian,
	      int little_endian_arch)
{
/* stores a 32bit int into a BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[4];
	int int_value;
    } convert;
    convert.int_value = value;
    if (little_endian_arch)
      {
	  /* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 3) = convert.byte[0];
		*(p + 2) = convert.byte[1];
		*(p + 1) = convert.byte[2];
		*(p + 0) = convert.byte[3];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
		*(p + 2) = convert.byte[2];
		*(p + 3) = convert.byte[3];
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
		*(p + 2) = convert.byte[2];
		*(p + 3) = convert.byte[3];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 3) = convert.byte[0];
		*(p + 2) = convert.byte[1];
		*(p + 1) = convert.byte[2];
		*(p + 0) = convert.byte[3];
	    }
      }
}

GAIAGEO_DECLARE void
gaiaExport64 (unsigned char *p, double value, int little_endian,
	      int little_endian_arch)
{
/* stores a 64bit double into a BLOB respecting declared endiannes */
    union cvt
    {
	unsigned char byte[8];
	double double_value;
    } convert;
    convert.double_value = value;
    if (little_endian_arch)
      {
/* Litte-Endian architecture [e.g. x86] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 7) = convert.byte[0];
		*(p + 6) = convert.byte[1];
		*(p + 5) = convert.byte[2];
		*(p + 4) = convert.byte[3];
		*(p + 3) = convert.byte[4];
		*(p + 2) = convert.byte[5];
		*(p + 1) = convert.byte[6];
		*(p + 0) = convert.byte[7];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
		*(p + 2) = convert.byte[2];
		*(p + 3) = convert.byte[3];
		*(p + 4) = convert.byte[4];
		*(p + 5) = convert.byte[5];
		*(p + 6) = convert.byte[6];
		*(p + 7) = convert.byte[7];
	    }
      }
    else
      {
	  /* Big Endian architecture [e.g. PPC] */
	  if (!little_endian)
	    {
		/* Big Endian data */
		*(p + 0) = convert.byte[0];
		*(p + 1) = convert.byte[1];
		*(p + 2) = convert.byte[2];
		*(p + 3) = convert.byte[3];
		*(p + 4) = convert.byte[4];
		*(p + 5) = convert.byte[5];
		*(p + 6) = convert.byte[6];
		*(p + 7) = convert.byte[7];
	    }
	  else
	    {
		/* Little Endian data */
		*(p + 7) = convert.byte[0];
		*(p + 6) = convert.byte[1];
		*(p + 5) = convert.byte[2];
		*(p + 4) = convert.byte[3];
		*(p + 3) = convert.byte[4];
		*(p + 2) = convert.byte[5];
		*(p + 1) = convert.byte[6];
		*(p + 0) = convert.byte[7];
	    }
      }
}
/**************** End file: gg_endian.c **********/


/**************** Begin file: gg_geometries.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <memory.h> */
/* #include <math.h> */
/* #include <float.h> */

/* #include <spatialite/sqlite3ext.h> */
/* #include <spatialite/gaiageo.h> */

GAIAGEO_DECLARE gaiaPointPtr
gaiaAllocPoint (double x, double y)
{
/* POINT object constructor */
    gaiaPointPtr p = malloc (sizeof (gaiaPoint));
    p->X = x;
    p->Y = y;
    p->Next = NULL;
    p->Prev = NULL;
    return p;
}

GAIAGEO_DECLARE void
gaiaFreePoint (gaiaPointPtr ptr)
{
/* POINT object destructor */
    if (ptr != NULL)
	free (ptr);
}

GAIAGEO_DECLARE gaiaLinestringPtr
gaiaAllocLinestring (int vert)
{
/* LINESTRING object constructor */
    gaiaLinestringPtr p = malloc (sizeof (gaiaLinestring));
    p->Coords = malloc (sizeof (double) * (vert * 2));
    p->Points = vert;
    p->MinX = DBL_MAX;
    p->MinY = DBL_MAX;
    p->MaxX = -DBL_MAX;
    p->MaxY = -DBL_MAX;
    p->Next = NULL;
    return p;
}

GAIAGEO_DECLARE void
gaiaFreeLinestring (gaiaLinestringPtr ptr)
{
/* LINESTRING object desctructror */
    if (ptr)
      {
	  if (ptr->Coords)
	      free (ptr->Coords);
	  free (ptr);
      }
}

GAIAGEO_DECLARE gaiaLinestringPtr
gaiaCloneLinestring (gaiaLinestringPtr line)
{
/* clones a LINESTRING */
    int iv;
    double x;
    double y;
    gaiaLinestringPtr new_line;
    if (!line)
	return NULL;
    new_line = gaiaAllocLinestring (line->Points);
    for (iv = 0; iv < new_line->Points; iv++)
      {
	  gaiaGetPoint (line->Coords, iv, &x, &y);
	  gaiaSetPoint (new_line->Coords, iv, x, y);
      }
    return new_line;
}

GAIAGEO_DECLARE gaiaRingPtr
gaiaAllocRing (int vert)
{
/* ring object constructor */
    gaiaRingPtr p = malloc (sizeof (gaiaRing));
    p->Coords = malloc (sizeof (double) * (vert * 2));
    p->Points = vert;
    p->Link = NULL;
    p->Clockwise = 0;
    p->MinX = DBL_MAX;
    p->MinY = DBL_MAX;
    p->MaxX = -DBL_MAX;
    p->MaxY = -DBL_MAX;
    p->Next = NULL;
    return p;
}

GAIAGEO_DECLARE void
gaiaFreeRing (gaiaRingPtr ptr)
{
/* ring object destructor */
    if (ptr)
      {
	  if (ptr->Coords)
	      free (ptr->Coords);
	  free (ptr);
      }
}

GAIAGEO_DECLARE gaiaRingPtr
gaiaCloneRing (gaiaRingPtr ring)
{
/* clones a RING */
    int iv;
    double x;
    double y;
    gaiaRingPtr new_ring;
    if (!ring)
	return NULL;
    new_ring = gaiaAllocRing (ring->Points);
    for (iv = 0; iv < new_ring->Points; iv++)
      {
	  gaiaGetPoint (ring->Coords, iv, &x, &y);
	  gaiaSetPoint (new_ring->Coords, iv, x, y);
      }
    return new_ring;
}

GAIAGEO_DECLARE gaiaPolygonPtr
gaiaClonePolygon (gaiaPolygonPtr polyg)
{
/* clones a POLYGON */
    int ib;
    int iv;
    double x;
    double y;
    gaiaPolygonPtr new_polyg;
    gaiaRingPtr i_ring;
    gaiaRingPtr o_ring;
    if (!polyg)
	return NULL;
    i_ring = polyg->Exterior;
    new_polyg = gaiaAllocPolygon (i_ring->Points, polyg->NumInteriors);
    o_ring = new_polyg->Exterior;
    for (iv = 0; iv < o_ring->Points; iv++)
      {
	  /* copying points for the EXTERIOR RING */
	  gaiaGetPoint (i_ring->Coords, iv, &x, &y);
	  gaiaSetPoint (o_ring->Coords, iv, x, y);
      }
    for (ib = 0; ib < new_polyg->NumInteriors; ib++)
      {
	  /* copying each INTERIOR RING [if any] */
	  i_ring = polyg->Interiors + ib;
	  o_ring = gaiaAddInteriorRing (new_polyg, ib, i_ring->Points);
	  for (iv = 0; iv < o_ring->Points; iv++)
	    {
		gaiaGetPoint (i_ring->Coords, iv, &x, &y);
		gaiaSetPoint (o_ring->Coords, iv, x, y);
	    }
      }
    return new_polyg;
}

GAIAGEO_DECLARE gaiaPolygonPtr
gaiaAllocPolygon (int vert, int excl)
{
/* POLYGON object constructor */
    gaiaPolygonPtr p;
    gaiaRingPtr pP;
    int ind;
    p = malloc (sizeof (gaiaPolygon));
    p->Exterior = gaiaAllocRing (vert);
    p->NumInteriors = excl;
    p->NextInterior = 0;
    p->Next = NULL;
    if (excl == 0)
	p->Interiors = NULL;
    else
	p->Interiors = malloc (sizeof (gaiaRing) * excl);
    for (ind = 0; ind < p->NumInteriors; ind++)
      {
	  pP = p->Interiors + ind;
	  pP->Points = 0;
	  pP->Coords = NULL;
	  pP->Next = NULL;
	  pP->Link = 0;
      }
    p->MinX = DBL_MAX;
    p->MinY = DBL_MAX;
    p->MaxX = -DBL_MAX;
    p->MaxY = -DBL_MAX;
    return p;
}

GAIAGEO_DECLARE gaiaPolygonPtr
gaiaCreatePolygon (gaiaRingPtr ring)
{
/* POLYGON object constructor */
    gaiaPolygonPtr p;
    int iv;
    double x;
    double y;
    double *coords;
    p = malloc (sizeof (gaiaPolygon));
    p->Exterior = gaiaAllocRing (ring->Points);
    p->NumInteriors = 0;
    p->NextInterior = 0;
    p->Next = NULL;
    p->Interiors = NULL;
    coords = p->Exterior->Coords;
    for (iv = 0; iv < ring->Points; iv++)
      {
	  gaiaGetPoint (ring->Coords, iv, &x, &y);
	  gaiaSetPoint (coords, iv, x, y);
      }
    p->MinX = DBL_MAX;
    p->MinY = DBL_MAX;
    p->MaxX = -DBL_MAX;
    p->MaxY = -DBL_MAX;
    return p;
}

GAIAGEO_DECLARE void
gaiaFreePolygon (gaiaPolygonPtr p)
{
/* POLYGON object destructor */
    gaiaRingPtr pP;
    int ind;
    if (p->Exterior)
	gaiaFreeRing (p->Exterior);
    for (ind = 0; ind < p->NumInteriors; ind++)
      {
	  pP = p->Interiors + ind;
	  if (pP->Coords)
	      free (pP->Coords);
      }
    if (p->Interiors)
	free (p->Interiors);
    free (p);
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaCloneGeomColl (gaiaGeomCollPtr geom)
{
/* clones a GEOMETRYCOLLECTION */
    int ib;
    int iv;
    double x;
    double y;
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaLinestringPtr new_line;
    gaiaPolygonPtr polyg;
    gaiaPolygonPtr new_polyg;
    gaiaGeomCollPtr new_geom;
    gaiaRingPtr i_ring;
    gaiaRingPtr o_ring;
    if (!geom)
	return NULL;
    new_geom = gaiaAllocGeomColl ();
    new_geom->Srid = geom->Srid;
    point = geom->FirstPoint;
    while (point)
      {
	  /* copying POINTs */
	  gaiaAddPointToGeomColl (new_geom, point->X, point->Y);
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* copying LINESTRINGs */
	  new_line = gaiaAddLinestringToGeomColl (new_geom, line->Points);
	  for (iv = 0; iv < new_line->Points; iv++)
	    {
		gaiaGetPoint (line->Coords, iv, &x, &y);
		gaiaSetPoint (new_line->Coords, iv, x, y);
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* copying POLYGONs */
	  i_ring = polyg->Exterior;
	  new_polyg =
	      gaiaAddPolygonToGeomColl (new_geom, i_ring->Points,
					polyg->NumInteriors);
	  o_ring = new_polyg->Exterior;
	  for (iv = 0; iv < o_ring->Points; iv++)
	    {
		/* copying points for the EXTERIOR RING */
		gaiaGetPoint (i_ring->Coords, iv, &x, &y);
		gaiaSetPoint (o_ring->Coords, iv, x, y);
	    }
	  for (ib = 0; ib < new_polyg->NumInteriors; ib++)
	    {
		/* copying each INTERIOR RING [if any] */
		i_ring = polyg->Interiors + ib;
		o_ring = gaiaAddInteriorRing (new_polyg, ib, i_ring->Points);
		for (iv = 0; iv < o_ring->Points; iv++)
		  {
		      gaiaGetPoint (i_ring->Coords, iv, &x, &y);
		      gaiaSetPoint (o_ring->Coords, iv, x, y);
		  }
	    }
	  polyg = polyg->Next;
      }
    return new_geom;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaAllocGeomColl ()
{
/* GEOMETRYCOLLECTION object constructor */
    gaiaGeomCollPtr p = malloc (sizeof (gaiaGeomColl));
    p->Srid = -1;
    p->endian = ' ';
    p->offset = 0;
    p->FirstPoint = NULL;
    p->LastPoint = NULL;
    p->FirstLinestring = NULL;
    p->LastLinestring = NULL;
    p->FirstPolygon = NULL;
    p->LastPolygon = NULL;
    p->MinX = DBL_MAX;
    p->MinY = DBL_MAX;
    p->MaxX = -DBL_MAX;
    p->MaxY = -DBL_MAX;
    p->DeclaredType = GAIA_UNKNOWN;
    return p;
}

GAIAGEO_DECLARE void
gaiaFreeGeomColl (gaiaGeomCollPtr p)
{
/* GEOMETRYCOLLECTION object destructor */
    gaiaPointPtr pP;
    gaiaPointPtr pPn;
    gaiaLinestringPtr pL;
    gaiaLinestringPtr pLn;
    gaiaPolygonPtr pA;
    gaiaPolygonPtr pAn;
    if (!p)
	return;
    pP = p->FirstPoint;
    while (pP != NULL)
      {
	  pPn = pP->Next;
	  gaiaFreePoint (pP);
	  pP = pPn;
      }
    pL = p->FirstLinestring;
    while (pL != NULL)
      {
	  pLn = pL->Next;
	  gaiaFreeLinestring (pL);
	  pL = pLn;
      }
    pA = p->FirstPolygon;
    while (pA != NULL)
      {
	  pAn = pA->Next;
	  gaiaFreePolygon (pA);
	  pA = pAn;
      }
    free (p);
}

GAIAGEO_DECLARE void
gaiaAddPointToGeomColl (gaiaGeomCollPtr p, double x, double y)
{
/* adding a POINT to this GEOMETRYCOLLECTION */
    gaiaPointPtr point = gaiaAllocPoint (x, y);
    if (p->FirstPoint == NULL)
	p->FirstPoint = point;
    if (p->LastPoint != NULL)
	p->LastPoint->Next = point;
    p->LastPoint = point;
}

GAIAGEO_DECLARE gaiaLinestringPtr
gaiaAddLinestringToGeomColl (gaiaGeomCollPtr p, int vert)
{
/* adding a LINESTRING to this GEOMETRYCOLLECTION */
    gaiaLinestringPtr line = gaiaAllocLinestring (vert);
    if (p->FirstLinestring == NULL)
	p->FirstLinestring = line;
    if (p->LastLinestring != NULL)
	p->LastLinestring->Next = line;
    p->LastLinestring = line;
    return line;
}

GAIAGEO_DECLARE void
gaiaInsertLinestringInGeomColl (gaiaGeomCollPtr p, gaiaLinestringPtr line)
{
/* adding an existing LINESTRING to this GEOMETRYCOLLECTION */
    if (p->FirstLinestring == NULL)
	p->FirstLinestring = line;
    if (p->LastLinestring != NULL)
	p->LastLinestring->Next = line;
    p->LastLinestring = line;
}

GAIAGEO_DECLARE gaiaPolygonPtr
gaiaAddPolygonToGeomColl (gaiaGeomCollPtr p, int vert, int interiors)
{
/* adding a POLYGON to this GEOMETRYCOLLECTION */
    gaiaPolygonPtr polyg = gaiaAllocPolygon (vert, interiors);
    if (p->FirstPolygon == NULL)
	p->FirstPolygon = polyg;
    if (p->LastPolygon != NULL)
	p->LastPolygon->Next = polyg;
    p->LastPolygon = polyg;
    return polyg;
}

GAIAGEO_DECLARE gaiaPolygonPtr
gaiaInsertPolygonInGeomColl (gaiaGeomCollPtr p, gaiaRingPtr ring)
{
/* adding a POLYGON to this GEOMETRYCOLLECTION */
    gaiaPolygonPtr polyg;
    polyg = malloc (sizeof (gaiaPolygon));
    polyg->Exterior = ring;
    polyg->NumInteriors = 0;
    polyg->NextInterior = 0;
    polyg->Next = NULL;
    polyg->Interiors = NULL;
    polyg->MinX = DBL_MAX;
    polyg->MinY = DBL_MAX;
    polyg->MaxX = -DBL_MAX;
    polyg->MaxY = -DBL_MAX;
    if (p->FirstPolygon == NULL)
	p->FirstPolygon = polyg;
    if (p->LastPolygon != NULL)
	p->LastPolygon->Next = polyg;
    p->LastPolygon = polyg;
    return polyg;
}

GAIAGEO_DECLARE gaiaRingPtr
gaiaAddInteriorRing (gaiaPolygonPtr p, int pos, int vert)
{
/* adding an interior ring to some polygon */
    gaiaRingPtr pP = p->Interiors + pos;
    pP->Points = vert;
    pP->Coords = malloc (sizeof (double) * (vert * 2));
    return pP;
}

GAIAGEO_DECLARE void
gaiaInsertInteriorRing (gaiaPolygonPtr p, gaiaRingPtr ring)
{
/* adding an interior ring to some polygon */
    int iv;
    gaiaRingPtr hole;
    double x;
    double y;
    if (p->NumInteriors == 0)
      {
	  /* this one is the first interior ring */
	  p->NumInteriors++;
	  p->Interiors = malloc (sizeof (gaiaRing));
	  hole = p->Interiors;
      }
    else
      {
	  /* some interior ring is already defined */
	  gaiaRingPtr save = p->Interiors;
	  p->Interiors = malloc (sizeof (gaiaRing) * (p->NumInteriors + 1));
	  memcpy (p->Interiors, save, (sizeof (gaiaRing) * p->NumInteriors));
	  free (save);
	  hole = p->Interiors + p->NumInteriors;
	  p->NumInteriors++;
      }
    hole->Points = ring->Points;
    hole->Coords = malloc (sizeof (double) * (hole->Points * 2));
    for (iv = 0; iv < ring->Points; iv++)
      {
	  /* copying points */
	  gaiaGetPoint (ring->Coords, iv, &x, &y);
	  gaiaSetPoint (hole->Coords, iv, x, y);
      }
}

GAIAGEO_DECLARE void
gaiaAddRingToPolyg (gaiaPolygonPtr polyg, gaiaRingPtr ring)
{
/* adds an interior ring to this POLYGON  */
    gaiaRingPtr old_interiors = NULL;
    if (!(polyg->Interiors))
      {
	  /* this one is the first interior ring */
	  polyg->Interiors = ring;
	  polyg->NumInteriors = 1;
      }
    else
      {
	  /* adding another interior ring */
	  old_interiors = polyg->Interiors;
	  polyg->Interiors =
	      malloc (sizeof (gaiaRing) * (polyg->NumInteriors + 1));
	  memcpy (polyg->Interiors, old_interiors,
		  (sizeof (gaiaRing) * polyg->NumInteriors));
	  memcpy (polyg->Interiors + polyg->NumInteriors, ring,
		  sizeof (gaiaRing));
	  (polyg->NumInteriors)++;
	  free (old_interiors);
      }
}

GAIAGEO_DECLARE gaiaDynamicLinePtr
gaiaAllocDynamicLine ()
{
/* DYNAMIC LINE object constructor */
    gaiaDynamicLinePtr p = malloc (sizeof (gaiaDynamicLine));
    p->First = NULL;
    p->Last = NULL;
    return p;
}

GAIAGEO_DECLARE void
gaiaFreeDynamicLine (gaiaDynamicLinePtr p)
{
/* DYNAMIC LINE object destructor */
    gaiaPointPtr pP;
    gaiaPointPtr pPn;
    pP = p->First;
    while (pP != NULL)
      {
	  pPn = pP->Next;
	  gaiaFreePoint (pP);
	  pP = pPn;
      }
    free (p);
}

GAIAGEO_DECLARE gaiaPointPtr
gaiaAppendPointToDynamicLine (gaiaDynamicLinePtr p, double x, double y)
{
/* inserts a new POINT to this DYNAMIC LINE after the last one */
    gaiaPointPtr point = gaiaAllocPoint (x, y);
    point->Prev = p->Last;
    if (p->First == NULL)
	p->First = point;
    if (p->Last != NULL)
	p->Last->Next = point;
    p->Last = point;
    return point;
}

GAIAGEO_DECLARE gaiaPointPtr
gaiaPrependPointToDynamicLine (gaiaDynamicLinePtr p, double x, double y)
{
/* inserts a new POINT to this DYNAMIC LINE before the first one */
    gaiaPointPtr point = gaiaAllocPoint (x, y);
    point->Next = p->First;
    if (p->Last == NULL)
	p->Last = point;
    if (p->First != NULL)
	p->First->Prev = point;
    p->First = point;
    return point;
}

GAIAGEO_DECLARE gaiaPointPtr
gaiaDynamicLineInsertAfter (gaiaDynamicLinePtr p, gaiaPointPtr pt, double x,
			    double y)
{
/* inserts a new POINT to this DYNAMIC LINE after the referenced POINT */
    gaiaPointPtr point = gaiaAllocPoint (x, y);
    point->Prev = pt;
    point->Next = pt->Next;
    if (pt->Next)
	pt->Next->Prev = point;
    pt->Next = point;
    if (pt == p->Last)
	p->Last = point;
    return point;
}

GAIAGEO_DECLARE gaiaPointPtr
gaiaDynamicLineInsertBefore (gaiaDynamicLinePtr p, gaiaPointPtr pt, double x,
			     double y)
{
/* inserts a new POINT to this DYNAMIC LINE before the referenced POINT */
    gaiaPointPtr point = gaiaAllocPoint (x, y);
    point->Next = pt;
    point->Prev = pt->Prev;
    if (pt->Prev)
	pt->Prev->Next = point;
    pt->Prev = point;
    if (pt == p->First)
	p->First = point;
    return point;
}

GAIAGEO_DECLARE void
gaiaDynamicLineDeletePoint (gaiaDynamicLinePtr p, gaiaPointPtr pt)
{
/* deletes a POINT from this DYNAMIC LINE */
    if (pt->Prev)
	pt->Prev->Next = pt->Next;
    if (pt->Next)
	pt->Next->Prev = pt->Prev;
    if (pt == p->First)
	p->First = pt->Next;
    if (pt == p->Last)
	p->Last = pt->Prev;
    gaiaFreePoint (pt);
}

GAIAGEO_DECLARE gaiaDynamicLinePtr
gaiaCloneDynamicLine (gaiaDynamicLinePtr org)
{
/* creates a new line obtained by simply copying the current one */
    gaiaPointPtr pt;
    gaiaDynamicLinePtr dst = gaiaAllocDynamicLine ();
    pt = org->First;
    while (pt)
      {
	  gaiaAppendPointToDynamicLine (dst, pt->X, pt->Y);
	  pt = pt->Next;
      }
    return dst;
}

GAIAGEO_DECLARE gaiaDynamicLinePtr
gaiaReverseDynamicLine (gaiaDynamicLinePtr org)
{
/* creates a new line obtained by inverting the current one */
    gaiaPointPtr pt;
    gaiaDynamicLinePtr dst = gaiaAllocDynamicLine ();
    pt = org->Last;
    while (pt)
      {
	  gaiaAppendPointToDynamicLine (dst, pt->X, pt->Y);
	  pt = pt->Prev;
      }
    return dst;
}

GAIAGEO_DECLARE gaiaDynamicLinePtr
gaiaDynamicLineSplitBefore (gaiaDynamicLinePtr org, gaiaPointPtr point)
{
/* creates a new line obtained by cutting the current one in two */
    gaiaDynamicLinePtr dst = gaiaAllocDynamicLine ();
    dst->First = org->First;
    dst->Last = point->Prev;
    point->Prev->Next = NULL;
    org->First = point;
    point->Prev = NULL;
    return dst;
}

GAIAGEO_DECLARE gaiaDynamicLinePtr
gaiaDynamicLineSplitAfter (gaiaDynamicLinePtr org, gaiaPointPtr point)
{
/* creates a new line obtained by cutting the current one in two */
    gaiaDynamicLinePtr dst = gaiaAllocDynamicLine ();
    dst->First = point->Next;
    dst->Last = org->Last;
    point->Next->Prev = NULL;
    org->Last = point;
    point->Next = NULL;
    return dst;
}

GAIAGEO_DECLARE gaiaDynamicLinePtr
gaiaDynamicLineJoinAfter (gaiaDynamicLinePtr org, gaiaPointPtr point,
			  gaiaDynamicLinePtr toJoin)
{
/* creates a new line obtained by joining the current one with another one */
    gaiaPointPtr pt;
    gaiaDynamicLinePtr dst = gaiaAllocDynamicLine ();
    pt = org->First;
    while (pt)
      {
	  /* inserting the first slice since the delimiting POINT included */
	  gaiaAppendPointToDynamicLine (dst, pt->X, pt->Y);
	  if (pt == point)
	      break;
	  pt = pt->Next;
      }
    pt = toJoin->First;
    while (pt)
      {
	  /* inserting the other line */
	  gaiaAppendPointToDynamicLine (dst, pt->X, pt->Y);
	  pt = pt->Next;
      }
    pt = point->Next;
    while (pt)
      {
	  /* inserting the second slice after the delimiting POINT */
	  gaiaAppendPointToDynamicLine (dst, pt->X, pt->Y);
	  pt = pt->Next;
      }
    return dst;
}

GAIAGEO_DECLARE gaiaDynamicLinePtr
gaiaDynamicLineJoinBefore (gaiaDynamicLinePtr org, gaiaPointPtr point,
			   gaiaDynamicLinePtr toJoin)
{
/* creates a new line obtained by joining the current one with another one */
    gaiaPointPtr pt;
    gaiaDynamicLinePtr dst = gaiaAllocDynamicLine ();
    pt = org->First;
    while (pt)
      {
	  /* inserting the first slice since the delimiting POINT excluded */
	  if (pt == point)
	      break;
	  gaiaAppendPointToDynamicLine (dst, pt->X, pt->Y);
	  pt = pt->Next;
      }
    pt = toJoin->First;
    while (pt)
      {
	  /* inserting the other line */
	  gaiaAppendPointToDynamicLine (dst, pt->X, pt->Y);
	  pt = pt->Next;
      }
    pt = point;
    while (pt)
      {
	  /* inserting the second slice beginning from the delimiting POINT */
	  gaiaAppendPointToDynamicLine (dst, pt->X, pt->Y);
	  pt = pt->Next;
      }
    return dst;
}

GAIAGEO_DECLARE gaiaPointPtr
gaiaDynamicLineFindByCoords (gaiaDynamicLinePtr p, double x, double y)
{
/* finds a POINT inside this DYNAMIC LINE */
    gaiaPointPtr pP;
    pP = p->First;
    while (pP != NULL)
      {
	  if (pP->X == x && pP->Y == y)
	      return pP;
	  pP = pP->Next;
      }
    return NULL;
}

GAIAGEO_DECLARE gaiaPointPtr
gaiaDynamicLineFindByPos (gaiaDynamicLinePtr p, int pos)
{
/* finds a POINT inside this DYNAMIC LINE */
    int n = 0;
    gaiaPointPtr pP;
    pP = p->First;
    while (pP != NULL)
      {
	  if (pos == n)
	      return pP;
	  n++;
	  pP = pP->Next;
      }
    return NULL;
}

GAIAGEO_DECLARE gaiaDynamicLinePtr
gaiaCreateDynamicLine (double *coords, int points)
{
/* creates a DynamicLine from an array of coordinates */
    int iv;
    double x;
    double y;
    gaiaDynamicLinePtr line = gaiaAllocDynamicLine ();
    for (iv = 0; iv < points; iv++)
      {
	  gaiaGetPoint (coords, iv, &x, &y);
	  gaiaAppendPointToDynamicLine (line, x, y);
      }
    return line;
}

GAIAGEO_DECLARE void
gaiaMbrLinestring (gaiaLinestringPtr line)
{
/* computes the MBR for this linestring */
    int iv;
    double x;
    double y;
    line->MinX = DBL_MAX;
    line->MinY = DBL_MAX;
    line->MaxX = -DBL_MAX;
    line->MaxY = -DBL_MAX;
    for (iv = 0; iv < line->Points; iv++)
      {
	  gaiaGetPoint (line->Coords, iv, &x, &y);
	  if (x < line->MinX)
	      line->MinX = x;
	  if (y < line->MinY)
	      line->MinY = y;
	  if (x > line->MaxX)
	      line->MaxX = x;
	  if (y > line->MaxY)
	      line->MaxY = y;
      }
}

GAIAGEO_DECLARE void
gaiaMbrRing (gaiaRingPtr rng)
{
/* computes the MBR for this ring */
    int iv;
    double x;
    double y;
    rng->MinX = DBL_MAX;
    rng->MinY = DBL_MAX;
    rng->MaxX = -DBL_MAX;
    rng->MaxY = -DBL_MAX;
    for (iv = 0; iv < rng->Points; iv++)
      {
	  gaiaGetPoint (rng->Coords, iv, &x, &y);
	  if (x < rng->MinX)
	      rng->MinX = x;
	  if (y < rng->MinY)
	      rng->MinY = y;
	  if (x > rng->MaxX)
	      rng->MaxX = x;
	  if (y > rng->MaxY)
	      rng->MaxY = y;
      }
}

GAIAGEO_DECLARE void
gaiaMbrPolygon (gaiaPolygonPtr polyg)
{
/* computes the MBR for this polygon */
    gaiaRingPtr rng;
    polyg->MinX = DBL_MAX;
    polyg->MinY = DBL_MAX;
    polyg->MaxX = -DBL_MAX;
    polyg->MaxY = -DBL_MAX;
    rng = polyg->Exterior;
    gaiaMbrRing (rng);
    if (rng->MinX < polyg->MinX)
	polyg->MinX = rng->MinX;
    if (rng->MinY < polyg->MinY)
	polyg->MinY = rng->MinY;
    if (rng->MaxX > polyg->MaxX)
	polyg->MaxX = rng->MaxX;
    if (rng->MaxY > polyg->MaxY)
	polyg->MaxY = rng->MaxY;
}

GAIAGEO_DECLARE void
gaiaMbrGeometry (gaiaGeomCollPtr geom)
{
/* computes the MBR for this geometry */
    gaiaPointPtr point = NULL;
    gaiaLinestringPtr line = NULL;
    gaiaPolygonPtr polyg = NULL;
    geom->MinX = DBL_MAX;
    geom->MinY = DBL_MAX;
    geom->MaxX = -DBL_MAX;
    geom->MaxY = -DBL_MAX;
    point = geom->FirstPoint;
    while (point)
      {
	  if (point->X < geom->MinX)
	      geom->MinX = point->X;
	  if (point->Y < geom->MinY)
	      geom->MinY = point->Y;
	  if (point->X > geom->MaxX)
	      geom->MaxX = point->X;
	  if (point->Y > geom->MaxY)
	      geom->MaxY = point->Y;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  gaiaMbrLinestring (line);
	  if (line->MinX < geom->MinX)
	      geom->MinX = line->MinX;
	  if (line->MinY < geom->MinY)
	      geom->MinY = line->MinY;
	  if (line->MaxX > geom->MaxX)
	      geom->MaxX = line->MaxX;
	  if (line->MaxY > geom->MaxY)
	      geom->MaxY = line->MaxY;
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  gaiaMbrPolygon (polyg);
	  if (polyg->MinX < geom->MinX)
	      geom->MinX = polyg->MinX;
	  if (polyg->MinY < geom->MinY)
	      geom->MinY = polyg->MinY;
	  if (polyg->MaxX > geom->MaxX)
	      geom->MaxX = polyg->MaxX;
	  if (polyg->MaxY > geom->MaxY)
	      geom->MaxY = polyg->MaxY;
	  polyg = polyg->Next;
      }
}

GAIAGEO_DECLARE int
gaiaDimension (gaiaGeomCollPtr geom)
{
/* determinates the Dimension for this geometry */
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaPolygonPtr polyg;
    int n_points = 0;
    int n_linestrings = 0;
    int n_polygons = 0;
    if (!geom)
	return -1;
    point = geom->FirstPoint;
    while (point)
      {
	  /* counts how many points are there */
	  n_points++;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* counts how many linestrings are there */
	  n_linestrings++;
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* counts how many polygons are there */
	  n_polygons++;
	  polyg = polyg->Next;
      }
    if (n_points == 0 && n_linestrings == 0 && n_polygons == 0)
	return -1;
    if (n_points > 0 && n_linestrings == 0 && n_polygons == 0)
	return 0;
    if (n_linestrings > 0 && n_polygons == 0)
	return 1;
    return 2;
}

GAIAGEO_DECLARE int
gaiaGeometryType (gaiaGeomCollPtr geom)
{
/* determinates the Class for this geometry */
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaPolygonPtr polyg;
    int n_points = 0;
    int n_linestrings = 0;
    int n_polygons = 0;
    if (!geom)
	return GAIA_UNKNOWN;
    point = geom->FirstPoint;
    while (point)
      {
	  /* counts how many points are there */
	  n_points++;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* counts how many linestrings are there */
	  n_linestrings++;
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* counts how many polygons are there */
	  n_polygons++;
	  polyg = polyg->Next;
      }
    if (n_points == 0 && n_linestrings == 0 && n_polygons == 0)
	return GAIA_UNKNOWN;
    if (n_points == 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOINT)
	      return GAIA_MULTIPOINT;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      return GAIA_GEOMETRYCOLLECTION;
	  else
	      return GAIA_POINT;
      }
    if (n_points > 0 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      return GAIA_GEOMETRYCOLLECTION;
	  else
	      return GAIA_MULTIPOINT;
      }
    if (n_points == 0 && n_linestrings == 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTILINESTRING)
	      return GAIA_MULTILINESTRING;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      return GAIA_GEOMETRYCOLLECTION;
	  else
	      return GAIA_LINESTRING;
      }
    if (n_points == 0 && n_linestrings > 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      return GAIA_GEOMETRYCOLLECTION;
	  else
	      return GAIA_MULTILINESTRING;
      }
    if (n_points == 0 && n_linestrings == 0 && n_polygons == 1)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOLYGON)
	      return GAIA_MULTIPOLYGON;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      return GAIA_GEOMETRYCOLLECTION;
	  else
	      return GAIA_POLYGON;
      }
    if (n_points == 0 && n_linestrings == 0 && n_polygons > 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      return GAIA_GEOMETRYCOLLECTION;
	  else
	      return GAIA_MULTIPOLYGON;
      }
    return GAIA_GEOMETRYCOLLECTION;
}

GAIAGEO_DECLARE int
gaiaGeometryAliasType (gaiaGeomCollPtr geom)
{
/* determinates the AliasClass for this geometry */
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaPolygonPtr polyg;
    int n_points = 0;
    int n_linestrings = 0;
    int n_polygons = 0;
    if (!geom)
	return GAIA_UNKNOWN;
    point = geom->FirstPoint;
    while (point)
      {
	  /* counts how many points are there */
	  n_points++;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* counts how many linestrings are there */
	  n_linestrings++;
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* counts how many polygons are there */
	  n_polygons++;
	  polyg = polyg->Next;
      }
    if (n_points == 0 && n_linestrings == 0 && n_polygons == 0)
	return GAIA_UNKNOWN;
    if (n_points > 0 && n_linestrings == 0 && n_polygons == 0)
	return GAIA_MULTIPOINT;
    if (n_points == 0 && n_linestrings > 0 && n_polygons == 0)
	return GAIA_MULTILINESTRING;
    if (n_points == 0 && n_linestrings == 0 && n_polygons > 0)
	return GAIA_MULTIPOLYGON;
    return GAIA_GEOMETRYCOLLECTION;
}

GAIAGEO_DECLARE int
gaiaIsEmpty (gaiaGeomCollPtr geom)
{
/* checks if this GEOMETRYCOLLECTION is an empty one */
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaPolygonPtr polyg;
    if (!geom)
	return 1;
    point = geom->FirstPoint;
    while (point)
      {
	  /* checks for points */
	  return 0;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* checks for linestrings */
	  return 0;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* checks for polygons */
	  return 0;
      }
    return 1;
}

GAIAGEO_DECLARE int
gaiaIsClosed (gaiaLinestringPtr line)
{
/* checks if this linestring is a closed one */
    double x0;
    double y0;
    double x1;
    double y1;
    if (!line)
	return 0;
    if (line->Points < 3)
	return 0;
    gaiaGetPoint (line->Coords, 0, &x0, &y0);
    gaiaGetPoint (line->Coords, (line->Points - 1), &x1, &y1);
    if (x0 == x1 && y0 == y1)
	return 1;
    return 0;
}

GAIAGEO_DECLARE int
gaiaMbrsEqual (gaiaGeomCollPtr mbr1, gaiaGeomCollPtr mbr2)
{
/* 
/ checks if two MBRs are identicals
/
/ returns 1 if TRUE
/ 0 if FALSE
*/
    if (mbr1->MinX != mbr2->MinX)
	return 0;
    if (mbr1->MinY != mbr2->MinY)
	return 0;
    if (mbr1->MaxX != mbr2->MaxX)
	return 0;
    if (mbr1->MaxY != mbr2->MaxY)
	return 0;
    return 1;
}

GAIAGEO_DECLARE int
gaiaMbrsDisjoint (gaiaGeomCollPtr mbr1, gaiaGeomCollPtr mbr2)
{
/* 
/ checks if two MBRs are disjoined
/
/ returns 1 if TRUE
/ 0 if FALSE
*/
    if (mbr1->MinX >= mbr2->MaxX)
	return 1;
    if (mbr1->MinY >= mbr2->MaxY)
	return 1;
    if (mbr1->MaxX <= mbr2->MinX)
	return 1;
    if (mbr1->MaxY <= mbr2->MinY)
	return 1;
    if (mbr2->MinX >= mbr1->MaxX)
	return 1;
    if (mbr2->MinY >= mbr1->MaxY)
	return 1;
    if (mbr2->MaxX <= mbr1->MinX)
	return 1;
    if (mbr2->MaxY <= mbr1->MinY)
	return 1;
    return 0;
}

GAIAGEO_DECLARE int
gaiaMbrsTouches (gaiaGeomCollPtr mbr1, gaiaGeomCollPtr mbr2)
{
/* 
/ checks if two MBRs touches
/
/ returns 1 if TRUE
/ 0 if FALSE
*/
    if (mbr1->MinX == mbr2->MinX)
	return 1;
    if (mbr1->MinY == mbr2->MinY)
	return 1;
    if (mbr1->MaxX == mbr2->MaxX)
	return 1;
    if (mbr1->MaxY == mbr2->MaxY)
	return 1;
    return 0;
}

GAIAGEO_DECLARE int
gaiaMbrsIntersects (gaiaGeomCollPtr mbr1, gaiaGeomCollPtr mbr2)
{
/* 
/ checks if two MBRs intersects
/
/ returns 1 if TRUE
/ 0 if FALSE
*/
    if (gaiaMbrsDisjoint (mbr1, mbr2))
	return 0;
    return 1;
}

GAIAGEO_DECLARE int
gaiaMbrsOverlaps (gaiaGeomCollPtr mbr1, gaiaGeomCollPtr mbr2)
{
/* 
/ checks if two MBRs overlaps
/
/ returns 1 if TRUE
/ 0 if FALSE
*/
    if (gaiaMbrsDisjoint (mbr1, mbr2))
	return 0;
    if (mbr1->MinX >= mbr2->MinX && mbr1->MinX <= mbr2->MaxX)
	return 1;
    if (mbr1->MaxX >= mbr2->MinX && mbr1->MaxX <= mbr2->MaxX)
	return 1;
    if (mbr1->MinY >= mbr2->MinY && mbr1->MinY <= mbr2->MaxY)
	return 1;
    if (mbr1->MaxY >= mbr2->MinY && mbr1->MaxY <= mbr2->MaxY)
	return 1;
    return 0;
}

GAIAGEO_DECLARE int
gaiaMbrsContains (gaiaGeomCollPtr mbr1, gaiaGeomCollPtr mbr2)
{
/* 
/ checks if MBR-1 completely contains MBR-2
/
/ returns 1 if TRUE
/ 0 if FALSE
*/
    int ok_1 = 0;
    int ok_2 = 0;
    int ok_3 = 0;
    int ok_4 = 0;
    if (mbr2->MinX >= mbr1->MinX && mbr2->MinX <= mbr1->MaxX)
	ok_1 = 1;
    if (mbr2->MaxX >= mbr1->MinX && mbr2->MaxX <= mbr1->MaxX)
	ok_2 = 1;
    if (mbr2->MinY >= mbr1->MinY && mbr2->MinY <= mbr1->MaxY)
	ok_3 = 1;
    if (mbr2->MaxY >= mbr1->MinY && mbr2->MaxY <= mbr1->MaxY)
	ok_4 = 1;
    if (ok_1 && ok_2 && ok_3 && ok_4)
	return 1;
    return 0;
}

GAIAGEO_DECLARE int
gaiaMbrsWithin (gaiaGeomCollPtr mbr1, gaiaGeomCollPtr mbr2)
{
/* 
/ checks if MBR-2 completely contains MBR-1
/
/ returns 1 if TRUE
/ 0 if FALSE
*/
    int ok_1 = 0;
    int ok_2 = 0;
    int ok_3 = 0;
    int ok_4 = 0;
    if (mbr1->MinX >= mbr2->MinX && mbr1->MinX <= mbr2->MaxX)
	ok_1 = 1;
    if (mbr1->MaxX >= mbr2->MinX && mbr1->MaxX <= mbr2->MaxX)
	ok_2 = 1;
    if (mbr1->MinY >= mbr2->MinY && mbr1->MinY <= mbr2->MaxY)
	ok_3 = 1;
    if (mbr1->MaxY >= mbr2->MinY && mbr1->MaxY <= mbr2->MaxY)
	ok_4 = 1;
    if (ok_1 && ok_2 && ok_3 && ok_4)
	return 1;
    return 0;
}

GAIAGEO_DECLARE void
gaiaMakePoint (double x, double y, int srid, unsigned char **result, int *size)
{
/* build a Blob encoded Geometry representing a POINT */
    unsigned char *ptr;
    int endian_arch = gaiaEndianArch ();
/* computing the Blob size and then allocating it */
    *size = 44;			/* header size */
    *size += (sizeof (double) * 2);	/* [x,y] coords */
    *result = malloc (*size);
    ptr = *result;
/* setting the Blob value */
    *ptr = GAIA_MARK_START;	/* START signatue */
    *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
    gaiaExport32 (ptr + 2, srid, 1, endian_arch);	/* the SRID */
    gaiaExport64 (ptr + 6, x, 1, endian_arch);	/* MBR - minimun X */
    gaiaExport64 (ptr + 14, y, 1, endian_arch);	/* MBR - minimun Y */
    gaiaExport64 (ptr + 22, x, 1, endian_arch);	/* MBR - maximun X */
    gaiaExport64 (ptr + 30, y, 1, endian_arch);	/* MBR - maximun Y */
    *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
    gaiaExport32 (ptr + 39, GAIA_POINT, 1, endian_arch);	/* class POINT */
    gaiaExport64 (ptr + 43, x, 1, endian_arch);	/* X */
    gaiaExport64 (ptr + 51, y, 1, endian_arch);	/* Y */
    *(ptr + 59) = GAIA_MARK_END;	/* END signature */
}

GAIAGEO_DECLARE void
gaiaBuildMbr (double x1, double y1, double x2, double y2, int srid,
	      unsigned char **result, int *size)
{
/* build a Blob encoded Geometry representing an MBR */
    unsigned char *ptr;
    double minx;
    double maxx;
    double miny;
    double maxy;
    int endian_arch = gaiaEndianArch ();
/* computing MinMax coords */
    if (x1 > x2)
      {
	  maxx = x1;
	  minx = x2;
      }
    else
      {
	  minx = x1;
	  maxx = x2;
      }
    if (y1 > y2)
      {
	  maxy = y1;
	  miny = y2;
      }
    else
      {
	  miny = y1;
	  maxy = y2;
      }
/* computing the Blob size and then allocating it */
    *size = 44;			/* header size */
    *size += (8 + ((sizeof (double) * 2) * 5));	/* # rings + # points + [x.y] array - exterior ring */
    *result = malloc (*size);
    ptr = *result;
/* setting the Blob value */
    *ptr = GAIA_MARK_START;	/* START signatue */
    *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
    gaiaExport32 (ptr + 2, srid, 1, endian_arch);	/* the SRID */
    gaiaExport64 (ptr + 6, minx, 1, endian_arch);	/* MBR - minimun X */
    gaiaExport64 (ptr + 14, miny, 1, endian_arch);	/* MBR - minimun Y */
    gaiaExport64 (ptr + 22, maxx, 1, endian_arch);	/* MBR - maximun X */
    gaiaExport64 (ptr + 30, maxy, 1, endian_arch);	/* MBR - maximun Y */
    *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
    gaiaExport32 (ptr + 39, GAIA_POLYGON, 1, endian_arch);	/* class POLYGON */
    gaiaExport32 (ptr + 43, 1, 1, endian_arch);	/* # rings */
    gaiaExport32 (ptr + 47, 5, 1, endian_arch);	/* # points - exterior ring */
    ptr += 51;
/* setting Envelope points */
    gaiaExport64 (ptr, minx, 1, endian_arch);
    gaiaExport64 (ptr + 8, miny, 1, endian_arch);
    ptr += 16;
    gaiaExport64 (ptr, maxx, 1, endian_arch);
    gaiaExport64 (ptr + 8, miny, 1, endian_arch);
    ptr += 16;
    gaiaExport64 (ptr, maxx, 1, endian_arch);
    gaiaExport64 (ptr + 8, maxy, 1, endian_arch);
    ptr += 16;
    gaiaExport64 (ptr, minx, 1, endian_arch);
    gaiaExport64 (ptr + 8, maxy, 1, endian_arch);
    ptr += 16;
    gaiaExport64 (ptr, minx, 1, endian_arch);
    gaiaExport64 (ptr + 8, miny, 1, endian_arch);
    ptr += 16;
    *ptr = GAIA_MARK_END;	/* END signature */
}

GAIAGEO_DECLARE void
gaiaBuildCircleMbr (double x, double y, double radius, int srid,
		    unsigned char **result, int *size)
{
/* build a Blob encoded Geometry representing an MBR */
    int sz;
    unsigned char *res = NULL;
    double minx = x - radius;
    double maxx = x + radius;
    double miny = y - radius;
    double maxy = y + radius;
    gaiaBuildMbr (minx, miny, maxx, maxy, srid, &res, &sz);
    if (!res)
      {
	  *result = NULL;
	  *size = 0;
      }
    else
      {
	  *result = res;
	  *size = sz;
      }
}

GAIAGEO_DECLARE void
gaiaBuildFilterMbr (double x1, double y1, double x2, double y2, int mode,
		    unsigned char **result, int *size)
{
/* build a filter for an MBR */
    unsigned char *ptr;
    double minx;
    double maxx;
    double miny;
    double maxy;
    int endian_arch = gaiaEndianArch ();
    char filter = GAIA_FILTER_MBR_WITHIN;
    if (mode == GAIA_FILTER_MBR_CONTAINS)
	filter = GAIA_FILTER_MBR_CONTAINS;
    if (mode == GAIA_FILTER_MBR_INTERSECTS)
	filter = GAIA_FILTER_MBR_INTERSECTS;
    if (mode == GAIA_FILTER_MBR_DECLARE)
	filter = GAIA_FILTER_MBR_DECLARE;
/* computing MinMax coords */
    if (x1 > x2)
      {
	  maxx = x1;
	  minx = x2;
      }
    else
      {
	  minx = x1;
	  maxx = x2;
      }
    if (y1 > y2)
      {
	  maxy = y1;
	  miny = y2;
      }
    else
      {
	  miny = y1;
	  maxy = y2;
      }
/* computing the Blob size and then allocating it */
    *size = 37;			/* MBR filter size */
    *result = malloc (*size);
    ptr = *result;
/* setting the Blob value */
    *ptr = filter;		/* signatue */
    ptr++;
    gaiaExport64 (ptr, minx, 1, endian_arch);	/* MBR - minimun X */
    ptr += 8;
    *ptr = filter;		/* signatue */
    ptr++;
    gaiaExport64 (ptr, miny, 1, endian_arch);	/* MBR - minimun Y */
    ptr += 8;
    *ptr = filter;		/* signatue */
    ptr++;
    gaiaExport64 (ptr, maxx, 1, endian_arch);	/* MBR - maximun X */
    ptr += 8;
    *ptr = filter;		/* signatue */
    ptr++;
    gaiaExport64 (ptr, maxy, 1, endian_arch);	/* MBR - maximun Y */
    ptr += 8;
    *ptr = filter;		/* signatue */
}


GAIAGEO_DECLARE int
gaiaParseFilterMbr (unsigned char *ptr, int size, double *minx, double *miny,
		    double *maxx, double *maxy, int *mode)
{
/* parsing a filter for an MBR */
    char decl_mode;
    int endian_arch = gaiaEndianArch ();
    if (size != 37)
	return 0;		/* cannot be an MBR Filter */
    if (!ptr)
	return 0;		/* cannot be an MBR Filter */
    decl_mode = *(ptr + 0);
    if (decl_mode == GAIA_FILTER_MBR_WITHIN)
	;
    else if (decl_mode == GAIA_FILTER_MBR_CONTAINS)
	;
    else if (decl_mode == GAIA_FILTER_MBR_INTERSECTS)
	;
    else if (decl_mode == GAIA_FILTER_MBR_DECLARE)
	;
    else
	return 0;		/* cannot be an MBR Filter */
    if (*(ptr + 9)
	== decl_mode
	&& *(ptr +
	     18) ==
	decl_mode && *(ptr + 27) == decl_mode && *(ptr + 36) == decl_mode)
	;
    else
	return 0;		/* cannot be an MBR Filter */
    *mode = decl_mode;
    *minx = gaiaImport64 (ptr + 1, 1, endian_arch);
    *miny = gaiaImport64 (ptr + 10, 1, endian_arch);
    *maxx = gaiaImport64 (ptr + 19, 1, endian_arch);
    *maxy = gaiaImport64 (ptr + 28, 1, endian_arch);
    return 1;
}

GAIAGEO_DECLARE int
gaiaGetMbrMinX (const unsigned char *blob, unsigned int size, double *minx)
{
/* returns the MinX coordinate value for a Blob encoded Geometry */
    int little_endian;
    int endian_arch = gaiaEndianArch ();
    if (size < 45)
	return 0;		/* cannot be an internal BLOB WKB geometry */
    if (*(blob + 0) != GAIA_MARK_START)
	return 0;		/* failed to recognize START signature */
    if (*(blob + (size - 1)) != GAIA_MARK_END)
	return 0;		/* failed to recognize END signature */
    if (*(blob + 38) != GAIA_MARK_MBR)
	return 0;		/* failed to recognize MBR signature */
    if (*(blob + 1) == GAIA_LITTLE_ENDIAN)
	little_endian = 1;
    else if (*(blob + 1) == GAIA_BIG_ENDIAN)
	little_endian = 0;
    else
	return 0;		/* unknown encoding; nor litte-endian neither big-endian */
    *minx = gaiaImport64 (blob + 6, little_endian, endian_arch);
    return 1;
}

GAIAGEO_DECLARE int
gaiaGetMbrMaxX (const unsigned char *blob, unsigned int size, double *maxx)
{
/* returns the MaxX coordinate value for a Blob encoded Geometry */
    int little_endian;
    int endian_arch = gaiaEndianArch ();
    if (size < 45)
	return 0;		/* cannot be an internal BLOB WKB geometry */
    if (*(blob + 0) != GAIA_MARK_START)
	return 0;		/* failed to recognize START signature */
    if (*(blob + (size - 1)) != GAIA_MARK_END)
	return 0;		/* failed to recognize END signature */
    if (*(blob + 38) != GAIA_MARK_MBR)
	return 0;		/* failed to recognize MBR signature */
    if (*(blob + 1) == GAIA_LITTLE_ENDIAN)
	little_endian = 1;
    else if (*(blob + 1) == GAIA_BIG_ENDIAN)
	little_endian = 0;
    else
	return 0;		/* unknown encoding; nor litte-endian neither big-endian */
    *maxx = gaiaImport64 (blob + 22, little_endian, endian_arch);
    return 1;
}

GAIAGEO_DECLARE int
gaiaGetMbrMinY (const unsigned char *blob, unsigned int size, double *miny)
{
/* returns the MinY coordinate value for a Blob encoded Geometry */
    int little_endian;
    int endian_arch = gaiaEndianArch ();
    if (size < 45)
	return 0;		/* cannot be an internal BLOB WKB geometry */
    if (*(blob + 0) != GAIA_MARK_START)
	return 0;		/* failed to recognize START signature */
    if (*(blob + (size - 1)) != GAIA_MARK_END)
	return 0;		/* failed to recognize END signature */
    if (*(blob + 38) != GAIA_MARK_MBR)
	return 0;		/* failed to recognize MBR signature */
    if (*(blob + 1) == GAIA_LITTLE_ENDIAN)
	little_endian = 1;
    else if (*(blob + 1) == GAIA_BIG_ENDIAN)
	little_endian = 0;
    else
	return 0;		/* unknown encoding; nor litte-endian neither big-endian */
    *miny = gaiaImport64 (blob + 14, little_endian, endian_arch);
    return 1;
}

GAIAGEO_DECLARE int
gaiaGetMbrMaxY (const unsigned char *blob, unsigned int size, double *maxy)
{
/* returns the MaxY coordinate value for a Blob encoded Geometry */
    int little_endian;
    int endian_arch = gaiaEndianArch ();
    if (size < 45)
	return 0;		/* cannot be an internal BLOB WKB geometry */
    if (*(blob + 0) != GAIA_MARK_START)
	return 0;		/* failed to recognize START signature */
    if (*(blob + (size - 1)) != GAIA_MARK_END)
	return 0;		/* failed to recognize END signature */
    if (*(blob + 38) != GAIA_MARK_MBR)
	return 0;		/* failed to recognize MBR signature */
    if (*(blob + 1) == GAIA_LITTLE_ENDIAN)
	little_endian = 1;
    else if (*(blob + 1) == GAIA_BIG_ENDIAN)
	little_endian = 0;
    else
	return 0;		/* unknown encoding; nor litte-endian neither big-endian */
    *maxy = gaiaImport64 (blob + 30, little_endian, endian_arch);
    return 1;
}
/**************** End file: gg_geometries.c **********/


/**************** Begin file: gg_relations.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */

#if OMIT_GEOS == 0		/* including GEOS */
/* #include <geos_c.h> */
#endif

/* #include <spatialite/sqlite3ext.h> */
/* #include <spatialite/gaiageo.h> */

static int
check_point (double *coords, int points, double x, double y)
{
/* checks if [X,Y] point is defined into this coordinate array [Linestring or Ring] */
    int iv;
    double xx;
    double yy;
    for (iv = 0; iv < points; iv++)
      {
	  gaiaGetPoint (coords, iv, &xx, &yy);
	  if (xx == x && yy == y)
	      return 1;
      }
    return 0;
}

GAIAGEO_DECLARE int
gaiaLinestringEquals (gaiaLinestringPtr line1, gaiaLinestringPtr line2)
{
/* checks if two Linestrings are "spatially equal" */
    int iv;
    double x;
    double y;
    if (line1->Points != line2->Points)
	return 0;
    for (iv = 0; iv < line1->Points; iv++)
      {
	  gaiaGetPoint (line1->Coords, iv, &x, &y);
	  if (!check_point (line2->Coords, line2->Points, x, y))
	      return 0;
      }
    return 1;
}

GAIAGEO_DECLARE int
gaiaPolygonEquals (gaiaPolygonPtr polyg1, gaiaPolygonPtr polyg2)
{
/* checks if two Polygons are "spatially equal" */
    int ib;
    int ib2;
    int iv;
    int ok2;
    double x;
    double y;
    gaiaRingPtr ring1;
    gaiaRingPtr ring2;
    if (polyg1->NumInteriors != polyg2->NumInteriors)
	return 0;
/* checking the EXTERIOR RINGs */
    ring1 = polyg1->Exterior;
    ring2 = polyg2->Exterior;
    if (ring1->Points != ring2->Points)
	return 0;
    for (iv = 0; iv < ring1->Points; iv++)
      {
	  gaiaGetPoint (ring1->Coords, iv, &x, &y);
	  if (!check_point (ring2->Coords, ring2->Points, x, y))
	      return 0;
      }
    for (ib = 0; ib < polyg1->NumInteriors; ib++)
      {
	  /* checking the INTERIOR RINGS */
	  int ok = 0;
	  ring1 = polyg1->Interiors + ib;
	  for (ib2 = 0; ib2 < polyg2->NumInteriors; ib2++)
	    {
		ok2 = 1;
		ring2 = polyg2->Interiors + ib2;
		for (iv = 0; iv < ring1->Points; iv++)
		  {
		      gaiaGetPoint (ring1->Coords, iv, &x, &y);
		      if (!check_point (ring2->Coords, ring2->Points, x, y))
			{
			    ok2 = 0;
			    break;
			}
		  }
		if (ok2)
		  {
		      ok = 1;
		      break;
		  }
	    }
	  if (!ok)
	      return 0;
      }
    return 1;
}

#if OMIT_GEOS == 0		/* including GEOS */

GAIAGEO_DECLARE int
gaiaGeomCollEquals (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if two Geometries are "spatially equal" */
    int ret;
    int len;
    unsigned char *p_result = NULL;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom1 || !geom2)
	return -1;
    gaiaToWkb (geom1, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    gaiaToWkb (geom2, &p_result, &len);
    g2 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    ret = GEOSEquals (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollIntersects (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if two Geometries do "spatially intersects" */
    int ret;
    int len;
    unsigned char *p_result = NULL;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom1 || !geom2)
	return -1;
    gaiaToWkb (geom1, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    gaiaToWkb (geom2, &p_result, &len);
    g2 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    ret = GEOSIntersects (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollDisjoint (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if two Geometries are "spatially disjoint" */
    int ret;
    int len;
    unsigned char *p_result = NULL;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom1 || !geom2)
	return -1;
    gaiaToWkb (geom1, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    gaiaToWkb (geom2, &p_result, &len);
    g2 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    ret = GEOSDisjoint (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollOverlaps (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if two Geometries do "spatially overlaps" */
    int ret;
    int len;
    unsigned char *p_result = NULL;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom1 || !geom2)
	return -1;
    gaiaToWkb (geom1, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    gaiaToWkb (geom2, &p_result, &len);
    g2 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    ret = GEOSOverlaps (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollCrosses (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if two Geometries do "spatially crosses" */
    int ret;
    int len;
    unsigned char *p_result = NULL;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom1 || !geom2)
	return -1;
    gaiaToWkb (geom1, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    gaiaToWkb (geom2, &p_result, &len);
    g2 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    ret = GEOSCrosses (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollTouches (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if two Geometries do "spatially touches" */
    int ret;
    int len;
    unsigned char *p_result = NULL;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom1 || !geom2)
	return -1;
    gaiaToWkb (geom1, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    gaiaToWkb (geom2, &p_result, &len);
    g2 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    ret = GEOSTouches (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollWithin (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if GEOM-1 is completely contained within GEOM-2 */
    int ret;
    int len;
    unsigned char *p_result = NULL;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom1 || !geom2)
	return -1;
    gaiaToWkb (geom1, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    gaiaToWkb (geom2, &p_result, &len);
    g2 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    ret = GEOSWithin (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollContains (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* checks if GEOM-1 completely contains GEOM-2 */
    int ret;
    int len;
    unsigned char *p_result = NULL;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom1 || !geom2)
	return -1;
    gaiaToWkb (geom1, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    gaiaToWkb (geom2, &p_result, &len);
    g2 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    ret = GEOSContains (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollRelate (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2,
		    const char *pattern)
{
/* checks if if GEOM-1 and GEOM-2 have a spatial relationship as specified by the pattern Matrix */
    int ret;
    int len;
    unsigned char *p_result = NULL;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom1 || !geom2)
	return -1;
    gaiaToWkb (geom1, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    gaiaToWkb (geom2, &p_result, &len);
    g2 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    ret = GEOSRelatePattern (g1, g2, pattern);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollLength (gaiaGeomCollPtr geom, double *xlength)
{
/* computes the total length for this Geometry */
    double length;
    int ret;
    int len;
    unsigned char *p_result = NULL;
    GEOSGeometry *g;
    if (!geom)
	return 0;
    gaiaToWkb (geom, &p_result, &len);
    g = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    ret = GEOSLength (g, &length);
    GEOSGeom_destroy (g);
    if (ret)
	*xlength = length;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollArea (gaiaGeomCollPtr geom, double *xarea)
{
/* computes the total area for this Geometry */
    double area;
    int ret;
    int len;
    unsigned char *p_result = NULL;
    GEOSGeometry *g;
    if (!geom)
	return 0;
    gaiaToWkb (geom, &p_result, &len);
    g = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    ret = GEOSArea (g, &area);
    GEOSGeom_destroy (g);
    if (ret)
	*xarea = area;
    return ret;
}

GAIAGEO_DECLARE int
gaiaGeomCollDistance (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2,
		      double *xdist)
{
/* computes the minimum distance intercurring between GEOM-1 and GEOM-2 */
    double dist;
    int ret;
    int len;
    unsigned char *p_result = NULL;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom1 || !geom2)
	return 0;
    gaiaToWkb (geom1, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    gaiaToWkb (geom2, &p_result, &len);
    g2 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    ret = GEOSDistance (g1, g2, &dist);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    if (ret)
	*xdist = dist;
    return ret;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometryIntersection (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* builds a new geometry representing the "spatial intersection" of GEOM-1 and GEOM-2 */
    int len;
    size_t tlen;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    GEOSGeometry *g3;
    if (!geom1 || !geom2)
	return NULL;
    gaiaToWkb (geom1, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    gaiaToWkb (geom2, &p_result, &len);
    g2 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    g3 = GEOSIntersection (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    if (!g3)
	return NULL;
    p_result = GEOSGeomToWKB_buf (g3, &tlen);
    if (!p_result)
      {
	  GEOSGeom_destroy (g3);
	  return NULL;
      }
    geo = gaiaFromWkb (p_result, (int) tlen);
    if (geo == NULL)
      {
	  free (p_result);
	  return NULL;
      }
    geo->Srid = geom1->Srid;
    GEOSGeom_destroy (g3);
    free (p_result);
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometryUnion (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* builds a new geometry representing the "spatial union" of GEOM-1 and GEOM-2 */
    int len;
    size_t tlen;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    GEOSGeometry *g3;
    if (!geom1 || !geom2)
	return NULL;
    gaiaToWkb (geom1, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    gaiaToWkb (geom2, &p_result, &len);
    g2 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    g3 = GEOSUnion (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    if (!g3)
	return NULL;
    p_result = GEOSGeomToWKB_buf (g3, &tlen);
    if (!p_result)
      {
	  GEOSGeom_destroy (g3);
	  return NULL;
      }
    geo = gaiaFromWkb (p_result, (int) tlen);
    if (geo == NULL)
      {
	  free (p_result);
	  return NULL;
      }
    geo->Srid = geom1->Srid;
    if (geo->
	DeclaredType == GAIA_POINT && geom1->DeclaredType == GAIA_MULTIPOINT)
	geo->DeclaredType = GAIA_MULTIPOINT;
    if (geo->
	DeclaredType
	== GAIA_LINESTRING && geom1->DeclaredType == GAIA_MULTILINESTRING)
	geo->DeclaredType = GAIA_MULTILINESTRING;
    if (geo->
	DeclaredType
	== GAIA_POLYGON && geom1->DeclaredType == GAIA_MULTIPOLYGON)
	geo->DeclaredType = GAIA_MULTIPOLYGON;
    GEOSGeom_destroy (g3);
    free (p_result);
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometryDifference (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* builds a new geometry representing the "spatial difference" of GEOM-1 and GEOM-2 */
    int len;
    size_t tlen;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    GEOSGeometry *g3;
    if (!geom1 || !geom2)
	return NULL;
    gaiaToWkb (geom1, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    gaiaToWkb (geom2, &p_result, &len);
    g2 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    g3 = GEOSDifference (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    if (!g3)
	return NULL;
    p_result = GEOSGeomToWKB_buf (g3, &tlen);
    if (!p_result)
      {
	  GEOSGeom_destroy (g3);
	  return NULL;
      }
    geo = gaiaFromWkb (p_result, (int) tlen);
    if (geo == NULL)
      {
	  free (p_result);
	  return NULL;
      }
    geo->Srid = geom1->Srid;
    GEOSGeom_destroy (g3);
    free (p_result);
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeometrySymDifference (gaiaGeomCollPtr geom1, gaiaGeomCollPtr geom2)
{
/* builds a new geometry representing the "spatial symmetric difference" of GEOM-1 and GEOM-2 */
    int len;
    size_t tlen;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    GEOSGeometry *g3;
    if (!geom1 || !geom2)
	return NULL;
    gaiaToWkb (geom1, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    gaiaToWkb (geom2, &p_result, &len);
    g2 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    g3 = GEOSSymDifference (g1, g2);
    GEOSGeom_destroy (g1);
    GEOSGeom_destroy (g2);
    if (!g3)
	return NULL;
    p_result = GEOSGeomToWKB_buf (g3, &tlen);
    if (!p_result)
      {
	  GEOSGeom_destroy (g3);
	  return NULL;
      }
    geo = gaiaFromWkb (p_result, (int) tlen);
    if (geo == NULL)
      {
	  free (p_result);
	  return NULL;
      }
    geo->Srid = geom1->Srid;
    GEOSGeom_destroy (g3);
    free (p_result);
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaBoundary (gaiaGeomCollPtr geom)
{
/* builds a new geometry representing the conbinatorial boundary of GEOM */
    int len;
    size_t tlen;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom)
	return NULL;
    gaiaToWkb (geom, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    g2 = GEOSBoundary (g1);
    GEOSGeom_destroy (g1);
    if (!g2)
	return NULL;
    p_result = GEOSGeomToWKB_buf (g2, &tlen);
    if (!p_result)
      {
	  GEOSGeom_destroy (g2);
	  return NULL;
      }
    geo = gaiaFromWkb (p_result, (int) tlen);
    if (geo == NULL)
      {
	  free (p_result);
	  return NULL;
      }
    geo->Srid = geom->Srid;
    GEOSGeom_destroy (g2);
    free (p_result);
    return geo;
}

GAIAGEO_DECLARE int
gaiaGeomCollCentroid (gaiaGeomCollPtr geom, double *x, double *y)
{
/* returns a Point representing the centroid for this Geometry */
    int len;
    size_t tlen;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr result;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom)
	return 0;
    gaiaToWkb (geom, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    g2 = GEOSGetCentroid (g1);
    GEOSGeom_destroy (g1);
    if (!g2)
	return 0;
    p_result = GEOSGeomToWKB_buf (g2, &tlen);
    if (!p_result)
      {
	  GEOSGeom_destroy (g2);
	  return 0;
      }
    GEOSGeom_destroy (g2);
    result = gaiaFromWkb (p_result, (int) tlen);
    if (!result)
      {
	  free (p_result);
	  return 0;
      }
    free (p_result);
    if (result->FirstPoint)
      {
	  *x = result->FirstPoint->X;
	  *y = result->FirstPoint->Y;
	  gaiaFreeGeomColl (result);
	  return 1;
      }
    gaiaFreeGeomColl (result);
    return 0;
}

GAIAGEO_DECLARE int
gaiaGetPointOnSurface (gaiaGeomCollPtr geom, double *x, double *y)
{
/* returns a Point guaranteed to lie on the Surface */
    int len;
    size_t tlen;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr result;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom)
	return 0;
    gaiaToWkb (geom, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    g2 = GEOSPointOnSurface (g1);
    GEOSGeom_destroy (g1);
    if (!g2)
	return 0;
    p_result = GEOSGeomToWKB_buf (g2, &tlen);
    if (!p_result)
      {
	  GEOSGeom_destroy (g2);
	  return 0;
      }
    GEOSGeom_destroy (g2);
    result = gaiaFromWkb (p_result, (int) tlen);
    if (!result)
      {
	  free (p_result);
	  return 0;
      }
    free (p_result);
    if (result->FirstPoint)
      {
	  *x = result->FirstPoint->X;
	  *y = result->FirstPoint->Y;
	  gaiaFreeGeomColl (result);
	  return 1;
      }
    gaiaFreeGeomColl (result);
    return 0;
}

GAIAGEO_DECLARE int
gaiaIsSimple (gaiaGeomCollPtr geom)
{
/* checks if this GEOMETRYCOLLECTION is a simple one */
    int ret;
    int len;
    unsigned char *p_result = NULL;
    GEOSGeometry *g;
    if (!geom)
	return -1;
    gaiaToWkb (geom, &p_result, &len);
    g = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    ret = GEOSisSimple (g);
    GEOSGeom_destroy (g);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE int
gaiaIsRing (gaiaLinestringPtr line)
{
/* checks if this LINESTRING can be a valid RING */
    gaiaGeomCollPtr geo;
    gaiaLinestringPtr line2;
    int ret;
    int len;
    int iv;
    double x;
    double y;
    unsigned char *p_result = NULL;
    GEOSGeometry *g;
    if (!line)
	return -1;
    geo = gaiaAllocGeomColl ();
    line2 = gaiaAddLinestringToGeomColl (geo, line->Points);
    for (iv = 0; iv < line2->Points; iv++)
      {
	  gaiaGetPoint (line->Coords, iv, &x, &y);
	  gaiaSetPoint (line2->Coords, iv, x, y);
      }
    gaiaToWkb (geo, &p_result, &len);
    gaiaFreeGeomColl (geo);
    g = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    ret = GEOSisRing (g);
    GEOSGeom_destroy (g);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE int
gaiaIsValid (gaiaGeomCollPtr geom)
{
/* checks if this GEOMETRYCOLLECTION is a valid one */
    int ret;
    int len;
    unsigned char *p_result = NULL;
    GEOSGeometry *g;
    if (!geom)
	return -1;
    gaiaToWkb (geom, &p_result, &len);
    g = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    ret = GEOSisValid (g);
    GEOSGeom_destroy (g);
    if (ret == 2)
	return -1;
    return ret;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeomCollSimplify (gaiaGeomCollPtr geom, double tolerance)
{
/* builds a simplified geometry using the Douglas-Peuker algorihtm */
    int len;
    size_t tlen;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr result;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom)
	return NULL;
    gaiaToWkb (geom, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    g2 = GEOSSimplify (g1, tolerance);
    GEOSGeom_destroy (g1);
    if (!g2)
	return NULL;
    p_result = GEOSGeomToWKB_buf (g2, &tlen);
    if (!p_result)
      {
	  GEOSGeom_destroy (g2);
	  return NULL;
      }
    GEOSGeom_destroy (g2);
    result = gaiaFromWkb (p_result, (int) tlen);
    free (p_result);
    result->Srid = geom->Srid;
    return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeomCollSimplifyPreserveTopology (gaiaGeomCollPtr geom, double tolerance)
{
/* builds a simplified geometry using the Douglas-Peuker algorihtm [preserving topology] */
    int len;
    size_t tlen;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr result;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom)
	return NULL;
    gaiaToWkb (geom, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    g2 = GEOSTopologyPreserveSimplify (g1, tolerance);
    GEOSGeom_destroy (g1);
    if (!g2)
	return NULL;
    p_result = GEOSGeomToWKB_buf (g2, &tlen);
    if (!p_result)
      {
	  GEOSGeom_destroy (g2);
	  return NULL;
      }
    GEOSGeom_destroy (g2);
    result = gaiaFromWkb (p_result, (int) tlen);
    free (p_result);
    result->Srid = geom->Srid;
    return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaConvexHull (gaiaGeomCollPtr geom)
{
/* builds a geometry that is the convex hull of GEOM */
    int len;
    size_t tlen;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr result;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom)
	return NULL;
    gaiaToWkb (geom, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    g2 = GEOSConvexHull (g1);
    GEOSGeom_destroy (g1);
    if (!g2)
	return NULL;
    p_result = GEOSGeomToWKB_buf (g2, &tlen);
    if (!p_result)
      {
	  GEOSGeom_destroy (g2);
	  return NULL;
      }
    GEOSGeom_destroy (g2);
    result = gaiaFromWkb (p_result, (int) tlen);
    free (p_result);
    result->Srid = geom->Srid;
    return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaGeomCollBuffer (gaiaGeomCollPtr geom, double radius, int points)
{
/* builds a geometry that is the GIS buffer of GEOM */
    int len;
    size_t tlen;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr result;
    GEOSGeometry *g1;
    GEOSGeometry *g2;
    if (!geom)
	return NULL;
    gaiaToWkb (geom, &p_result, &len);
    g1 = GEOSGeomFromWKB_buf (p_result, len);
    free (p_result);
    g2 = GEOSBuffer (g1, radius, points);
    GEOSGeom_destroy (g1);
    if (!g2)
	return NULL;
    p_result = GEOSGeomToWKB_buf (g2, &tlen);
    if (!p_result)
      {
	  GEOSGeom_destroy (g2);
	  return NULL;
      }
    GEOSGeom_destroy (g2);
    result = gaiaFromWkb (p_result, (int) tlen);
    free (p_result);
    result->Srid = geom->Srid;
    return result;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaPolygonize (gaiaGeomCollPtr geom_org, int force_multipolygon)
{
/* trying to promote a (MULTI)LINESTRING to (MULTI)POLYGON */
    int i;
    int len;
    size_t tlen;
    int n_geoms = 0;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr result;
    gaiaGeomCollPtr geom;
    gaiaLinestringPtr ln1;
    gaiaLinestringPtr ln2;
    gaiaPointPtr pt;
    gaiaPolygonPtr pg;
    GEOSGeometry **g_array;
    GEOSGeometry *g2;
    double x;
    double y;
    int npt;
    int nln;
    int npg;
    if (!geom_org)
	return NULL;
    ln1 = geom_org->FirstLinestring;
    while (ln1)
      {
	  /* counting how many linestrings are there */
	  n_geoms++;
	  ln1 = ln1->Next;
      }
    g_array = malloc (sizeof (GEOSGeometry *) * n_geoms);
    n_geoms = 0;
    ln1 = geom_org->FirstLinestring;
    while (ln1)
      {
	  /* preparing individual LINESTRINGs */
	  geom = gaiaAllocGeomColl ();
	  ln2 = gaiaAddLinestringToGeomColl (geom, ln1->Points);
	  for (i = 0; i < ln1->Points; i++)
	    {
		gaiaGetPoint (ln1->Coords, i, &x, &y);
		gaiaSetPoint (ln2->Coords, i, x, y);
	    }
	  gaiaToWkb (geom, &p_result, &len);
	  gaiaFreeGeomColl (geom);
	  *(g_array + n_geoms) = GEOSGeomFromWKB_buf (p_result, len);
	  free (p_result);
	  n_geoms++;
	  ln1 = ln1->Next;
      }
    g2 = GEOSPolygonize ((const GEOSGeometry ** const) g_array, n_geoms);
    for (i = 0; i < n_geoms; i++)
	GEOSGeom_destroy (*(g_array + i));
    free (g_array);
    if (!g2)
	return NULL;
    p_result = GEOSGeomToWKB_buf (g2, &tlen);
    if (!p_result)
      {
	  GEOSGeom_destroy (g2);
	  return NULL;
      }
    GEOSGeom_destroy (g2);
    result = gaiaFromWkb (p_result, (int) tlen);
    free (p_result);
    npt = 0;
    pt = result->FirstPoint;
    while (pt)
      {
	  npt++;
	  pt = pt->Next;
      }
    nln = 0;
    ln1 = result->FirstLinestring;
    while (ln1)
      {
	  nln++;
	  ln1 = ln1->Next;
      }
    npg = 0;
    pg = result->FirstPolygon;
    while (pg)
      {
	  npg++;
	  pg = pg->Next;
      }
    if (npt || nln)
      {
	  /* invalid geometry; not a (MULTI)POLYGON */
	  gaiaFreeGeomColl (result);
	  return NULL;
      }
    result->Srid = geom_org->Srid;
    if (npg == 1)
      {
	  if (force_multipolygon)
	      result->DeclaredType = GAIA_MULTIPOLYGON;
	  else
	      result->DeclaredType = GAIA_POLYGON;
      }
    else
	result->DeclaredType = GAIA_MULTIPOLYGON;
    return result;
}

#endif /* end including GEOS */
/**************** End file: gg_relations.c **********/


/**************** Begin file: gg_shape.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <string.h> */
/* #include <math.h> */
/* #include <float.h> */
/* #include <errno.h> */

#ifdef __MINGW32__
#define LIBICONV_STATIC
/* #include <iconv.h> */
#define LIBCHARSET_STATIC
/* #include <localcharset.h> */
#else /* not MINGW32 */
#ifdef __APPLE__
/* #include <iconv.h> */
/* #include <localcharset.h> */
#else /* not Mac OsX */
/* #include <iconv.h> */
/* #include <langinfo.h> */
#endif
#endif
/* #include <spatialite/sqlite3ext.h> */
/* #include <spatialite/gaiageo.h> */

GAIAGEO_DECLARE void
gaiaFreeValue (gaiaValuePtr p)
{
/* frees all memory allocations for this DBF Field value */
    if (!p)
	return;
    if (p->TxtValue)
	free (p->TxtValue);
    free (p);
}

GAIAGEO_DECLARE void
gaiaSetNullValue (gaiaDbfFieldPtr field)
{
/* assignes a NULL value to some DBF field */
    if (field->Value)
	gaiaFreeValue (field->Value);
    field->Value = malloc (sizeof (gaiaValue));
    field->Value->Type = GAIA_NULL_VALUE;
    field->Value->TxtValue = NULL;
}

GAIAGEO_DECLARE void
gaiaSetIntValue (gaiaDbfFieldPtr field, sqlite3_int64 value)
{
/* assignes an INTEGER value to some DBF field */
    if (field->Value)
	gaiaFreeValue (field->Value);
    field->Value = malloc (sizeof (gaiaValue));
    field->Value->Type = GAIA_INT_VALUE;
    field->Value->TxtValue = NULL;
    field->Value->IntValue = value;
}

GAIAGEO_DECLARE void
gaiaSetDoubleValue (gaiaDbfFieldPtr field, double value)
{
/* assignes a DOUBLE value to some DBF field */
    if (field->Value)
	gaiaFreeValue (field->Value);
    field->Value = malloc (sizeof (gaiaValue));
    field->Value->Type = GAIA_DOUBLE_VALUE;
    field->Value->TxtValue = NULL;
    field->Value->DblValue = value;
}

GAIAGEO_DECLARE void
gaiaSetStrValue (gaiaDbfFieldPtr field, char *str)
{
/* assignes a STRING value to some DBF field */
    int len = strlen (str);
    if (field->Value)
	gaiaFreeValue (field->Value);
    field->Value = malloc (sizeof (gaiaValue));
    field->Value->Type = GAIA_TEXT_VALUE;
    field->Value->TxtValue = malloc (len + 1);
    strcpy (field->Value->TxtValue, str);
}

GAIAGEO_DECLARE gaiaDbfFieldPtr
gaiaAllocDbfField (char *name, unsigned char type,
		   int offset, unsigned char length, unsigned char decimals)
{
/* allocates and initializes a DBF Field definition */
    gaiaDbfFieldPtr p = malloc (sizeof (gaiaDbfField));
    int len = strlen (name);
    p->Name = malloc (len + 1);
    strcpy (p->Name, name);
    p->Type = type;
    p->Offset = offset;
    p->Length = length;
    p->Decimals = decimals;
    p->Value = NULL;
    p->Next = NULL;
    return p;
}

GAIAGEO_DECLARE void
gaiaFreeDbfField (gaiaDbfFieldPtr p)
{
/* frees all memory allocations for this DBF Field definition */
    if (!p)
	return;
    if (p->Name)
	free (p->Name);
    if (p->Value)
	gaiaFreeValue (p->Value);
    free (p);
}

GAIAGEO_DECLARE gaiaDbfFieldPtr
gaiaCloneDbfField (gaiaDbfFieldPtr org)
{
/* creating a new DBF LIST copied from the original one */
    gaiaDbfFieldPtr p = malloc (sizeof (gaiaDbfField));
    int len = strlen (org->Name);
    p->Name = malloc (len + 1);
    strcpy (p->Name, org->Name);
    p->Type = org->Type;
    p->Offset = org->Offset;
    p->Length = org->Length;
    p->Decimals = org->Decimals;
    p->Value = gaiaCloneValue (org->Value);
    p->Next = NULL;
    return p;
}

GAIAGEO_DECLARE gaiaDbfListPtr
gaiaAllocDbfList ()
{
/* allocates and initializes the DBF Fields list */
    gaiaDbfListPtr list = malloc (sizeof (gaiaDbfList));
    list->RowId = 0;
    list->Geometry = NULL;
    list->First = NULL;
    list->Last = NULL;
    return list;
}

GAIAGEO_DECLARE void
gaiaFreeDbfList (gaiaDbfListPtr list)
{
/* frees all memory allocations related to DBF Fields list */
    gaiaDbfFieldPtr p;
    gaiaDbfFieldPtr pn;
    if (!list)
	return;
    p = list->First;
    while (p)
      {
	  pn = p->Next;
	  gaiaFreeDbfField (p);
	  p = pn;
      }
    free (list);
}

GAIAGEO_DECLARE int
gaiaIsValidDbfList (gaiaDbfListPtr list)
{
/* checks if the DBF fields list contains any invalid data type */
    gaiaDbfFieldPtr p;
    if (!list)
	return 0;
    p = list->First;
    while (p)
      {
	  if (p->Type == 'N' || p->Type == 'C' || p->Type == 'L'
	      || p->Type == 'D')
	      ;
	  else
	      return 0;
	  p = p->Next;
      }
    return 1;
}

GAIAGEO_DECLARE gaiaDbfFieldPtr
gaiaAddDbfField (gaiaDbfListPtr list, char *name, unsigned char type,
		 int offset, unsigned char length, unsigned char decimals)
{
/* inserts a Field in the DBF Fields list */
    gaiaDbfFieldPtr p =
	gaiaAllocDbfField (name, type, offset, length, decimals);
    if (!list)
	return NULL;
    if (!(list->First))
	list->First = p;
    if (list->Last)
	list->Last->Next = p;
    list->Last = p;
    return p;
}

GAIAGEO_DECLARE void
gaiaResetDbfEntity (gaiaDbfListPtr list)
{
/* resets data values */
    gaiaDbfFieldPtr p;
    if (!list)
	return;
    p = list->First;
    while (p)
      {
	  if (p->Value)
	      gaiaFreeValue (p->Value);
	  p->Value = NULL;
	  p = p->Next;
      }
    if (list->Geometry)
	gaiaFreeGeomColl (list->Geometry);
    list->Geometry = NULL;
}

GAIAGEO_DECLARE gaiaValuePtr
gaiaCloneValue (gaiaValuePtr org)
{
/* creating a new VARIANT value copied from the original one */
    gaiaValuePtr value;
    int len;
    value = malloc (sizeof (gaiaValue));
    value->Type = GAIA_NULL_VALUE;
    value->TxtValue = NULL;
    switch (org->Type)
      {
      case GAIA_INT_VALUE:
	  value->Type = GAIA_INT_VALUE;
	  value->IntValue = org->IntValue;
	  break;
      case GAIA_DOUBLE_VALUE:
	  value->Type = GAIA_DOUBLE_VALUE;
	  value->DblValue = org->DblValue;
	  break;
      case GAIA_TEXT_VALUE:
	  value->Type = GAIA_TEXT_VALUE;
	  len = strlen (org->TxtValue);
	  value->TxtValue = malloc (len + 1);
	  strcpy (value->TxtValue, org->TxtValue);
      };
    return value;
}

GAIAGEO_DECLARE gaiaDbfListPtr
gaiaCloneDbfEntity (gaiaDbfListPtr org)
{
/* creating a new DBF LIST copied from the original one */
    gaiaDbfFieldPtr p;
    gaiaDbfFieldPtr newFld;
    gaiaDbfListPtr entity = gaiaAllocDbfList ();
    entity->RowId = org->RowId;
    if (org->Geometry)
	entity->Geometry = gaiaCloneGeomColl (org->Geometry);
    p = org->First;
    while (p)
      {
	  newFld =
	      gaiaAddDbfField (entity, p->Name, p->Type, p->Offset, p->Length,
			       p->Decimals);
	  if (p->Value)
	      newFld->Value = gaiaCloneValue (p->Value);
	  p = p->Next;
      }
    return entity;
}

GAIAGEO_DECLARE gaiaShapefilePtr
gaiaAllocShapefile ()
{
/* allocates and initializes the Shapefile object */
    gaiaShapefilePtr shp = malloc (sizeof (gaiaShapefile));
    shp->endian_arch = 1;
    shp->Path = NULL;
    shp->Shape = -1;
    shp->EffectiveType = GAIA_UNKNOWN;
    shp->flShp = NULL;
    shp->flShx = NULL;
    shp->flDbf = NULL;
    shp->Dbf = NULL;
    shp->BufShp = NULL;
    shp->ShpBfsz = 0;
    shp->BufDbf = NULL;
    shp->DbfHdsz = 0;
    shp->DbfReclen = 0;
    shp->DbfSize = 0;
    shp->DbfRecno = 0;
    shp->ShpSize = 0;
    shp->ShxSize = 0;
    shp->MinX = DBL_MAX;
    shp->MinY = DBL_MAX;
    shp->MaxX = -DBL_MAX;
    shp->MaxY = -DBL_MAX;
    shp->Valid = 0;
    shp->IconvObj = NULL;
    shp->LastError = NULL;
    return shp;
}

GAIAGEO_DECLARE void
gaiaFreeShapefile (gaiaShapefilePtr shp)
{
/* frees all memory allocations related to the Shapefile object */
    if (shp->Path)
	free (shp->Path);
    if (shp->flShp)
	fclose (shp->flShp);
    if (shp->flShx)
	fclose (shp->flShx);
    if (shp->flDbf)
	fclose (shp->flDbf);
    if (shp->Dbf)
	gaiaFreeDbfList (shp->Dbf);
    if (shp->BufShp)
	free (shp->BufShp);
    if (shp->BufDbf)
	free (shp->BufDbf);
    if (shp->IconvObj)
	iconv_close ((iconv_t) shp->IconvObj);
    if (shp->LastError)
	free (shp->LastError);
    free (shp);
}

GAIAGEO_DECLARE void
gaiaOpenShpRead (gaiaShapefilePtr shp, const char *path, const char *charFrom,
		 const char *charTo)
{
/* trying to open the shapefile and initial checkings */
    FILE *fl_shx = NULL;
    FILE *fl_shp = NULL;
    FILE *fl_dbf = NULL;
    char xpath[1024];
    int rd;
    unsigned char buf_shx[256];
    int size_shp;
    int size_shx;
    unsigned char *buf_shp = NULL;
    int buf_size = 1024;
    int shape;
    unsigned char bf[1024];
    int dbf_size;
    int dbf_reclen = 0;
    int dbf_recno;
    int off_dbf;
    int ind;
    char field_name[16];
    char *sys_err;
    char errMsg[1024];
    int len;
    iconv_t iconv_ret;
    int endian_arch = gaiaEndianArch ();
    gaiaDbfListPtr dbf_list = NULL;
    if (charFrom && charTo)
      {
	  iconv_ret = iconv_open (charTo, charFrom);
	  if (iconv_ret == (iconv_t) - 1)
	    {
		sprintf (errMsg, "conversion from '%s' to '%s' not available\n",
			 charFrom, charTo);
		goto unsupported_conversion;
	    }
	  shp->IconvObj = iconv_ret;
      }
    else
      {
	  sprintf (errMsg, "a NULL charset-name was passed\n");
	  goto unsupported_conversion;
      }
    if (shp->flShp != NULL || shp->flShx != NULL || shp->flDbf != NULL)
      {
	  sprintf (errMsg,
		   "attempting to reopen an already opened Shapefile\n");
	  goto unsupported_conversion;
      }
    sprintf (xpath, "%s.shx", path);
    fl_shx = fopen (xpath, "rb");
    if (!fl_shx)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for reading: %s", xpath,
		   sys_err);
	  goto no_file;
      }
    sprintf (xpath, "%s.shp", path);
    fl_shp = fopen (xpath, "rb");
    if (!fl_shp)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for reading: %s", xpath,
		   sys_err);
	  goto no_file;
      }
    sprintf (xpath, "%s.dbf", path);
    fl_dbf = fopen (xpath, "rb");
    if (!fl_dbf)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for reading: %s", xpath,
		   sys_err);
	  goto no_file;
      }
/* reading SHX file header */
    rd = fread (buf_shx, sizeof (unsigned char), 100, fl_shx);
    if (rd != 100)
	goto error;
    if (gaiaImport32 (buf_shx + 0, GAIA_BIG_ENDIAN, endian_arch) != 9994)	/* checks the SHX magic number */
	goto error;
    size_shx = gaiaImport32 (buf_shx + 24, GAIA_BIG_ENDIAN, endian_arch);
/* reading SHP file header */
    buf_shp = malloc (sizeof (unsigned char) * buf_size);
    rd = fread (buf_shp, sizeof (unsigned char), 100, fl_shp);
    if (rd != 100)
	goto error;
    if (gaiaImport32 (buf_shp + 0, GAIA_BIG_ENDIAN, endian_arch) != 9994)	/* checks the SHP magic number */
	goto error;
    size_shp = gaiaImport32 (buf_shp + 24, GAIA_BIG_ENDIAN, endian_arch);
    shape = gaiaImport32 (buf_shp + 32, GAIA_LITTLE_ENDIAN, endian_arch);
    if (shape == 1
	|| shape ==
	11
	|| shape ==
	21
	|| shape ==
	3
	|| shape ==
	13
	|| shape ==
	23
	|| shape ==
	5
	|| shape ==
	15 || shape == 25 || shape == 8 || shape == 18 || shape == 28)
	;
    else
	goto unsupported;
/* reading DBF file header */
    rd = fread (bf, sizeof (unsigned char), 32, fl_dbf);
    if (rd != 32)
	goto error;
    if (*bf != 0x03)		/* checks the DBF magic number */
	goto error;
    dbf_recno = gaiaImport32 (bf + 4, GAIA_LITTLE_ENDIAN, endian_arch);
    dbf_size = gaiaImport16 (bf + 8, GAIA_LITTLE_ENDIAN, endian_arch);
    dbf_reclen = gaiaImport16 (bf + 10, GAIA_LITTLE_ENDIAN, endian_arch);
    dbf_size--;
    off_dbf = 0;
    dbf_list = gaiaAllocDbfList ();
    for (ind = 32; ind < dbf_size; ind += 32)
      {
	  /* fetches DBF fields definitions */
	  rd = fread (bf, sizeof (unsigned char), 32, fl_dbf);
	  if (rd != 32)
	      goto error;
	  memcpy (field_name, bf, 11);
	  field_name[11] = '\0';
	  gaiaAddDbfField (dbf_list, field_name, *(bf + 11), off_dbf,
			   *(bf + 16), *(bf + 17));
	  off_dbf += *(bf + 16);
      }
    if (!gaiaIsValidDbfList (dbf_list))
      {
	  /* invalid DBF */
	  goto illegal_dbf;
      }
    len = strlen (path);
    shp->Path = malloc (len + 1);
    strcpy (shp->Path, path);
    shp->ReadOnly = 1;
    shp->Shape = shape;
    if (shape == 1 || shape == 11 || shape == 21)
	shp->EffectiveType = GAIA_POINT;
    if (shape == 3 || shape == 13 || shape == 23)
	shp->EffectiveType = GAIA_MULTILINESTRING;
    if (shape == 5 || shape == 15 || shape == 25)
	shp->EffectiveType = GAIA_MULTIPOLYGON;
    if (shape == 8 || shape == 18 || shape == 28)
	shp->EffectiveType = GAIA_MULTIPOINT;
    shp->flShp = fl_shp;
    shp->flShx = fl_shx;
    shp->flDbf = fl_dbf;
    shp->Dbf = dbf_list;
/* saving the SHP buffer */
    shp->BufShp = buf_shp;
    shp->ShpBfsz = buf_size;
/* allocating DBF buffer */
    shp->BufDbf = malloc (sizeof (unsigned char) * dbf_reclen);
    shp->DbfHdsz = dbf_size + 1;
    shp->DbfReclen = dbf_reclen;
    shp->Valid = 1;
    shp->endian_arch = endian_arch;
    return;
  unsupported_conversion:
/* illegal charset */
    if (shp->LastError)
	free (shp->LastError);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    return;
  no_file:
/* one of shapefile's files can't be accessed */
    if (shp->LastError)
	free (shp->LastError);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    if (fl_shx)
	fclose (fl_shx);
    if (fl_shp)
	fclose (fl_shp);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
  error:
/* the shapefile is invalid or corrupted */
    if (shp->LastError)
	free (shp->LastError);
    sprintf (errMsg, "'%s' is corrupted / has invalid format", path);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    gaiaFreeDbfList (dbf_list);
    if (buf_shp)
	free (buf_shp);
    fclose (fl_shx);
    fclose (fl_shp);
    fclose (fl_dbf);
    return;
  unsupported:
/* the shapefile has an unrecognized shape type */
    if (shp->LastError)
	free (shp->LastError);
    sprintf (errMsg, "'%s' shape=%d is not supported", path, shape);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    gaiaFreeDbfList (dbf_list);
    if (buf_shp)
	free (buf_shp);
    fclose (fl_shx);
    fclose (fl_shp);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
  illegal_dbf:
/* the DBF-file contains unsupported data types */
    if (shp->LastError)
	free (shp->LastError);
    sprintf (errMsg, "'%s.dbf' contains unsupported data types", path);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    gaiaFreeDbfList (dbf_list);
    if (buf_shp)
	free (buf_shp);
    fclose (fl_shx);
    fclose (fl_shp);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
}

GAIAGEO_DECLARE void
gaiaOpenShpWrite (gaiaShapefilePtr shp, const char *path, int shape,
		  gaiaDbfListPtr dbf_list, const char *charFrom,
		  const char *charTo)
{
/* trying to create the shapefile */
    FILE *fl_shx = NULL;
    FILE *fl_shp = NULL;
    FILE *fl_dbf = NULL;
    char xpath[1024];
    unsigned char *buf_shp = NULL;
    int buf_size = 1024;
    unsigned char *dbf_buf = NULL;
    gaiaDbfFieldPtr fld;
    char *sys_err;
    char errMsg[1024];
    int len;
    short dbf_reclen = 0;
    int shp_size = 0;
    int shx_size = 0;
    unsigned short dbf_size = 0;
    iconv_t iconv_ret;
    int endian_arch = gaiaEndianArch ();
    if (charFrom && charTo)
      {
	  iconv_ret = iconv_open (charTo, charFrom);
	  if (iconv_ret == (iconv_t) - 1)
	    {
		sprintf (errMsg, "conversion from '%s' to '%s' not available\n",
			 charFrom, charTo);
		goto unsupported_conversion;
	    }
	  shp->IconvObj = iconv_ret;
      }
    else
      {
	  sprintf (errMsg, "a NULL charset-name was passed\n");
	  goto unsupported_conversion;
      }
    if (shp->flShp != NULL || shp->flShx != NULL || shp->flDbf != NULL)
      {
	  sprintf (errMsg,
		   "attempting to reopen an already opened Shapefile\n");
	  goto unsupported_conversion;
      }
    buf_shp = malloc (buf_size);
/* trying to open shapefile files */
    sprintf (xpath, "%s.shx", path);
    fl_shx = fopen (xpath, "wb");
    if (!fl_shx)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for writing: %s", xpath,
		   sys_err);
	  goto no_file;
      }
    sprintf (xpath, "%s.shp", path);
    fl_shp = fopen (xpath, "wb");
    if (!fl_shp)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for writing: %s", xpath,
		   sys_err);
	  goto no_file;
      }
    sprintf (xpath, "%s.dbf", path);
    fl_dbf = fopen (xpath, "wb");
    if (!fl_dbf)
      {
	  sys_err = strerror (errno);
	  sprintf (errMsg, "unable to open '%s' for writing: %s", xpath,
		   sys_err);
	  goto no_file;
      }
/* allocating DBF buffer */
    dbf_reclen = 1;		/* an extra byte is needed because in DBF rows first byte is a marker for deletion */
    fld = dbf_list->First;
    while (fld)
      {
	  /* computing the DBF record length */
	  dbf_reclen += fld->Length;
	  fld = fld->Next;
      }
    dbf_buf = malloc (dbf_reclen);
/* writing an empty SHP file header */
    memset (buf_shp, 0, 100);
    fwrite (buf_shp, 1, 100, fl_shp);
    shp_size = 50;		/* note: shapefile [SHP and SHX] counts sizes in WORDS of 16 bits, not in bytes of 8 bits !!!! */
/* writing an empty SHX file header */
    memset (buf_shp, 0, 100);
    fwrite (buf_shp, 1, 100, fl_shx);
    shx_size = 50;
/* writing the DBF file header */
    memset (buf_shp, '\0', 32);
    fwrite (buf_shp, 1, 32, fl_dbf);
    dbf_size = 32;		/* note: DBF counts sizes in bytes */
    fld = dbf_list->First;
    while (fld)
      {
	  /* exporting DBF Fields specifications */
	  memset (buf_shp, 0, 32);
	  if (strlen (fld->Name) > 10)
	      memcpy (buf_shp, fld->Name, 10);
	  else
	      memcpy (buf_shp, fld->Name, strlen (fld->Name));
	  *(buf_shp + 11) = fld->Type;
	  *(buf_shp + 16) = fld->Length;
	  *(buf_shp + 17) = fld->Decimals;
	  fwrite (buf_shp, 1, 32, fl_dbf);
	  dbf_size += 32;
	  fld = fld->Next;
      }
    fwrite ("\r", 1, 1, fl_dbf);	/* this one is a special DBF delimiter that closes file header */
    dbf_size++;
/* setting up the SHP struct */
    len = strlen (path);
    shp->Path = malloc (len + 1);
    strcpy (shp->Path, path);
    shp->ReadOnly = 0;
    if (shape == GAIA_POINT)
	shp->Shape = 1;
    if (shape == GAIA_MULTIPOINT)
	shp->Shape = 8;
    if (shape == GAIA_LINESTRING)
	shp->Shape = 3;
    if (shape == GAIA_POLYGON)
	shp->Shape = 5;
    shp->flShp = fl_shp;
    shp->flShx = fl_shx;
    shp->flDbf = fl_dbf;
    shp->Dbf = dbf_list;
    shp->BufShp = buf_shp;
    shp->ShpBfsz = buf_size;
    shp->BufDbf = dbf_buf;
    shp->DbfHdsz = dbf_size + 1;
    shp->DbfReclen = dbf_reclen;
    shp->DbfSize = dbf_size;
    shp->DbfRecno = 0;
    shp->ShpSize = shp_size;
    shp->ShxSize = shx_size;
    shp->MinX = DBL_MAX;
    shp->MinY = DBL_MAX;
    shp->MaxX = -DBL_MAX;
    shp->MaxY = -DBL_MAX;
    shp->Valid = 1;
    shp->endian_arch = endian_arch;
    return;
  unsupported_conversion:
/* illegal charset */
    if (shp->LastError)
	free (shp->LastError);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    return;
  no_file:
/* one of shapefile's files can't be created/opened */
    if (shp->LastError)
	free (shp->LastError);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    if (buf_shp)
	free (buf_shp);
    if (fl_shx)
	fclose (fl_shx);
    if (fl_shp)
	fclose (fl_shp);
    if (fl_dbf)
	fclose (fl_dbf);
    return;
}

static double
to_sqlite_julian_date (int year, int month, int day, double *julian)
{
/* trying to convert an 'YYYY-MM-DD' date into a JulianDate [double] */
    int Y;
    int M;
    int D;
    int A;
    int B;
    int X1;
    int X2;
    if (year < 1900 || year > 2400)
	return 0;
    if (month < 1 || month > 12)
	return 0;
    if (day < 1)
	return 0;
    switch (month)
      {
      case 2:
	  if ((year / 4) == 0)
	    {
		if (day > 29)
		    return 0;
	    }
	  else
	    {
		if (day > 28)
		    return 0;
	    }
	  break;
      case 4:
      case 6:
      case 9:
      case 11:
	  if (day > 30)
	      return 0;
	  break;
      default:
	  if (day > 31)
	      return 0;
      };
// computing the Julian date
    Y = year;
    M = month;
    D = day;
    if (M <= 2)
      {
	  Y--;
	  M += 12;
      }
    A = Y / 100;
    B = 2 - A + (A / 4);
    X1 = 36525 * (Y + 4716) / 100;
    X2 = 306001 * (M + 1) / 10000;
    *julian = (double) (X1 + X2 + D + B - 1524.5);
    return 1;
}

GAIAGEO_DECLARE int
gaiaReadShpEntity (gaiaShapefilePtr shp, int current_row, int srid)
{
/* trying to read an entity from shapefile */
    unsigned char buf[512];
    char utf8buf[2048];
#ifdef __MINGW32__
    const char *pBuf;
    int len;
    int utf8len;
#else /* not MINGW32 */
    char *pBuf;
    size_t len;
    size_t utf8len;
#endif
    char *pUtf8buf;
    int rd;
    int skpos;
    int offset;
    int off_shp;
    int i;
    int sz;
    int shape;
    double x;
    double y;
    int points;
    int n;
    int n1;
    int base;
    int start;
    int end;
    int iv;
    int ind;
    char errMsg[1024];
    gaiaGeomCollPtr geom = NULL;
    gaiaLinestringPtr line = NULL;
    gaiaPolygonPtr polyg = NULL;
    gaiaRingPtr ring = NULL;
    gaiaDbfFieldPtr pFld;
/* positioning and reading the SHX file */
    offset = 100 + (current_row * 8);	/* 100 bytes for the header + current row displacement; each SHX row = 8 bytes */
    skpos = fseek (shp->flShx, offset, SEEK_SET);
    if (skpos != 0)
	goto eof;
    rd = fread (buf, sizeof (unsigned char), 8, shp->flShx);
    if (rd != 8)
	goto eof;
    off_shp = gaiaImport32 (buf, GAIA_BIG_ENDIAN, shp->endian_arch);
/* positioning and reading the DBF file */
    offset = shp->DbfHdsz + (current_row * shp->DbfReclen);
    skpos = fseek (shp->flDbf, offset, SEEK_SET);
    if (skpos != 0)
	goto error;
    rd = fread (shp->BufDbf, sizeof (unsigned char), shp->DbfReclen,
		shp->flDbf);
    if (rd != shp->DbfReclen)
	goto error;
/* positioning and reading corresponding SHP entity - geometry */
    offset = off_shp * 2;
    skpos = fseek (shp->flShp, offset, SEEK_SET);
    if (skpos != 0)
	goto error;
    rd = fread (buf, sizeof (unsigned char), 12, shp->flShp);
    if (rd != 12)
	goto error;
    sz = gaiaImport32 (buf + 4, GAIA_BIG_ENDIAN, shp->endian_arch);
    shape = gaiaImport32 (buf + 8, GAIA_LITTLE_ENDIAN, shp->endian_arch);
    if (shape == 0)
      {
	  /* handling a NULL shape */
	  goto null_shape;
      }
    else if (shape != shp->Shape)
	goto error;
    if ((sz * 2) > shp->ShpBfsz)
      {
	  /* current buffer is too small; we need to allocate a bigger buffer */
	  free (shp->BufShp);
	  shp->ShpBfsz = sz * 2;
	  shp->BufShp = malloc (sizeof (unsigned char) * shp->ShpBfsz);
      }
    if (shape == 1 || shape == 11 || shape == 21)
      {
	  /* shape point */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 16, shp->flShp);
	  if (rd != 16)
	      goto error;
	  x = gaiaImport64 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  y = gaiaImport64 (shp->BufShp + 8, GAIA_LITTLE_ENDIAN,
			    shp->endian_arch);
	  geom = gaiaAllocGeomColl ();
	  geom->DeclaredType = GAIA_POINT;
	  gaiaAddPointToGeomColl (geom, x, y);
	  geom->Srid = srid;
      }
    if (shape == 3 || shape == 13 || shape == 23)
      {
	  /* shape polyline */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 32, shp->flShp);
	  if (rd != 32)
	      goto error;
	  rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
		      shp->flShp);
	  if (rd != (sz * 2) - 36)
	      goto error;
	  n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  n1 = gaiaImport32 (shp->BufShp + 4, GAIA_LITTLE_ENDIAN,
			     shp->endian_arch);
	  base = 8 + (n * 4);
	  start = 0;
	  for (ind = 0; ind < n; ind++)
	    {
		if (ind < (n - 1))
		    end =
			gaiaImport32 (shp->BufShp + 8 + ((ind + 1) * 4),
				      GAIA_LITTLE_ENDIAN, shp->endian_arch);
		else
		    end = n1;
		points = end - start;
		line = gaiaAllocLinestring (points);
		points = 0;
		for (iv = start; iv < end; iv++)
		  {
		      x = gaiaImport64 (shp->BufShp + base + (iv * 16),
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      y = gaiaImport64 (shp->BufShp + base + (iv * 16) + 8,
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      gaiaSetPoint (line->Coords, points, x, y);
		      start++;
		      points++;
		  }
		if (!geom)
		  {
		      geom = gaiaAllocGeomColl ();
		      if (shp->EffectiveType == GAIA_LINESTRING)
			  geom->DeclaredType = GAIA_LINESTRING;
		      else
			  geom->DeclaredType = GAIA_MULTILINESTRING;
		      geom->Srid = srid;
		  }
		gaiaInsertLinestringInGeomColl (geom, line);
	    }
      }
    if (shape == 5 || shape == 15 || shape == 25)
      {
	  /* shape polygon */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 32, shp->flShp);
	  if (rd != 32)
	      goto error;
	  rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
		      shp->flShp);
	  if (rd != (sz * 2) - 36)
	      goto error;
	  n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  n1 = gaiaImport32 (shp->BufShp + 4, GAIA_LITTLE_ENDIAN,
			     shp->endian_arch);
	  base = 8 + (n * 4);
	  start = 0;
	  for (ind = 0; ind < n; ind++)
	    {
		if (ind < (n - 1))
		    end =
			gaiaImport32 (shp->BufShp + 8 + ((ind + 1) * 4),
				      GAIA_LITTLE_ENDIAN, shp->endian_arch);
		else
		    end = n1;
		points = end - start;
		ring = gaiaAllocRing (points);
		points = 0;
		for (iv = start; iv < end; iv++)
		  {
		      x = gaiaImport64 (shp->BufShp + base + (iv * 16),
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      y = gaiaImport64 (shp->BufShp + base + (iv * 16) + 8,
					GAIA_LITTLE_ENDIAN, shp->endian_arch);
		      gaiaSetPoint (ring->Coords, points, x, y);
		      start++;
		      points++;
		  }
		if (!geom)
		  {
		      /* new geometry - new need to allocate a new POLYGON */
		      geom = gaiaAllocGeomColl ();
		      if (shp->EffectiveType == GAIA_POLYGON)
			  geom->DeclaredType = GAIA_POLYGON;
		      else
			  geom->DeclaredType = GAIA_MULTIPOLYGON;
		      geom->Srid = srid;
		      polyg = gaiaInsertPolygonInGeomColl (geom, ring);
		  }
		else
		  {
		      gaiaClockwise (ring);
		      if (ring->Clockwise)
			{
			    /* this one is a POLYGON exterior ring - we need to allocate e new POLYGON */
			    polyg = gaiaInsertPolygonInGeomColl (geom, ring);
			}
		      else
			{
			    /* adding an interior ring to current POLYGON */
			    gaiaAddRingToPolyg (polyg, ring);
			}
		  }
	    }
      }
    if (shape == 8 || shape == 18 || shape == 28)
      {
	  /* shape multipoint */
	  rd = fread (shp->BufShp, sizeof (unsigned char), 32, shp->flShp);
	  if (rd != 32)
	      goto error;
	  rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
		      shp->flShp);
	  if (rd != (sz * 2) - 36)
	      goto error;
	  n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  geom = gaiaAllocGeomColl ();
	  geom->DeclaredType = GAIA_MULTIPOINT;
	  geom->Srid = srid;
	  for (iv = 0; iv < n; iv++)
	    {
		x = gaiaImport64 (shp->BufShp + 4 + (iv * 16),
				  GAIA_LITTLE_ENDIAN, shp->endian_arch);
		y = gaiaImport64 (shp->BufShp + 4 + (iv * 16) + 8,
				  GAIA_LITTLE_ENDIAN, shp->endian_arch);
		gaiaAddPointToGeomColl (geom, x, y);
	    }
      }
/* setting up the current SHP ENTITY */
  null_shape:
    gaiaResetDbfEntity (shp->Dbf);
    shp->Dbf->RowId = current_row;
    shp->Dbf->Geometry = geom;
/* fetching the DBF values */
    pFld = shp->Dbf->First;
    while (pFld)
      {
	  memcpy (buf, shp->BufDbf + pFld->Offset + 1, pFld->Length);
	  buf[pFld->Length] = '\0';
	  if (*buf == '\0')
	      gaiaSetNullValue (pFld);
	  else
	    {
		if (pFld->Type == 'N')
		  {
		      // NUMERIC value
		      if (pFld->Decimals > 0 || pFld->Length > 18)
			  gaiaSetDoubleValue (pFld, atof ((char *) buf));
		      else
			  gaiaSetIntValue (pFld, atoll ((char *) buf));
		  }
		else if (pFld->Type == 'D')
		  {
		      // DATE value
		      if (strlen ((char *) buf) != 8)
			  gaiaSetNullValue (pFld);
		      else
			{
			    // converting into a Julian Date
			    double julian;
			    char date[5];
			    int year = 0;
			    int month = 0;
			    int day = 0;
			    date[0] = buf[0];
			    date[1] = buf[1];
			    date[2] = buf[2];
			    date[3] = buf[3];
			    date[4] = '\0';
			    year = atoi (date);
			    date[0] = buf[4];
			    date[1] = buf[5];
			    date[2] = '\0';
			    month = atoi (date);
			    date[0] = buf[6];
			    date[1] = buf[7];
			    date[2] = '\0';
			    day = atoi (date);
			    if (to_sqlite_julian_date
				(year, month, day, &julian))
				gaiaSetDoubleValue (pFld, julian);
			    else
				gaiaSetNullValue (pFld);
			}
		  }
		else if (pFld->Type == 'L')
		  {
		      // LOGICAL [aka Boolean] value
		      if (*buf == '1' || *buf == 't' || *buf == 'T'
			  || *buf == 'Y' || *buf == 'y')
			  gaiaSetIntValue (pFld, 1);
		      else
			  gaiaSetIntValue (pFld, 0);
		  }
		else
		  {
		      // CHARACTER [aka String, Text] value
		      for (i = strlen ((char *) buf) - 1; i > 1; i--)
			{
			    /* cleaning up trailing spaces */
			    if (buf[i] == ' ')
				buf[i] = '\0';
			    else
				break;
			}
		      len = strlen ((char *) buf);
		      utf8len = 2048;
		      pBuf = (char *) buf;
		      pUtf8buf = utf8buf;
		      if (iconv
			  ((iconv_t) (shp->IconvObj), &pBuf, &len, &pUtf8buf,
			   &utf8len) < 0)
			  goto conversion_error;
		      memcpy (buf, utf8buf, 2048 - utf8len);
		      buf[2048 - utf8len] = '\0';
		      gaiaSetStrValue (pFld, (char *) buf);
		  }
	    }
	  pFld = pFld->Next;
      }
    if (shp->LastError)
	free (shp->LastError);
    shp->LastError = NULL;
    return 1;
  eof:
    if (shp->LastError)
	free (shp->LastError);
    shp->LastError = NULL;
    return 0;
  error:
    if (shp->LastError)
	free (shp->LastError);
    sprintf (errMsg, "'%s' is corrupted / has invalid format", shp->Path);
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    return 0;
  conversion_error:
    if (shp->LastError)
	free (shp->LastError);
    sprintf (errMsg, "Invalid character sequence");
    len = strlen (errMsg);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, errMsg);
    return 0;
}

static void
gaiaSaneClockwise (gaiaPolygonPtr polyg)
{
/*
/ when exporting POLYGONs to SHAPEFILE, we must guarantee that:
/ - all EXTERIOR RING must be clockwise
/ - all INTERIOR RING must be anti-clockwise
/
/ this function checks for the above conditions,
/ and if needed inverts the rings
*/
    int ib;
    int iv;
    int iv2;
    double x;
    double y;
    gaiaRingPtr new_ring;
    gaiaRingPtr ring = polyg->Exterior;
    gaiaClockwise (ring);
    if (!(ring->Clockwise))
      {
	  /* exterior ring needs inversion */
	  new_ring = gaiaAllocRing (ring->Points);
	  iv2 = 0;
	  for (iv = ring->Points - 1; iv >= 0; iv--)
	    {
		gaiaGetPoint (ring->Coords, iv, &x, &y);
		gaiaSetPoint (new_ring->Coords, iv2, x, y);
		iv2++;
	    }
	  polyg->Exterior = new_ring;
	  gaiaFreeRing (ring);
      }
    for (ib = 0; ib < polyg->NumInteriors; ib++)
      {
	  ring = polyg->Interiors + ib;
	  gaiaClockwise (ring);
	  if (ring->Clockwise)
	    {
		/* interior ring needs inversion */
		new_ring = gaiaAllocRing (ring->Points);
		iv2 = 0;
		for (iv = ring->Points - 1; iv >= 0; iv--)
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		      gaiaSetPoint (new_ring->Coords, iv2, x, y);
		      iv2++;
		  }
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      gaiaGetPoint (new_ring->Coords, iv, &x, &y);
		      gaiaSetPoint (ring->Coords, iv, x, y);
		  }
		gaiaFreeRing (new_ring);
	    }
      }
}

GAIAGEO_DECLARE int
gaiaWriteShpEntity (gaiaShapefilePtr shp, gaiaDbfListPtr entity)
{
/* trying to write an entity into shapefile */
    char dummy[128];
    char fmt[16];
    int endian_arch = shp->endian_arch;
    gaiaDbfFieldPtr fld;
    int iv;
    int tot_ln;
    int tot_v;
    int tot_pts;
    int this_size;
    int ix;
    double x;
    double y;
#ifdef __MINGW32__
    const char *pBuf;
    int len;
    int utf8len;
#else /* not MINGW32 */
    char *pBuf;
    size_t len;
    size_t utf8len;
#endif
    char *pUtf8buf;
    char buf[512];
    char utf8buf[2048];
/* writing the DBF record */
    memset (shp->BufDbf, '\0', shp->DbfReclen);
    *(shp->BufDbf) = ' ';	/* in DBF first byte of each row marks for validity or deletion */
    fld = entity->First;
    while (fld)
      {
	  /* transferring field values */
	  switch (fld->Type)
	    {
	    case 'L':
		if (!(fld->Value))
		    *(shp->BufDbf + fld->Offset) = '?';
		else if (fld->Value->Type != GAIA_INT_VALUE)
		    *(shp->BufDbf + fld->Offset + 1) = '?';
		else
		  {
		      if (fld->Value->IntValue == 0)
			  *(shp->BufDbf + fld->Offset + 1) = 'N';
		      else
			  *(shp->BufDbf + fld->Offset + 1) = 'Y';
		  }
		break;
	    case 'D':
		memset (shp->BufDbf + fld->Offset + 1, '0', 8);
		if (fld->Value)
		  {
		      if (fld->Value->Type == GAIA_TEXT_VALUE)
			{
			    if (strlen (fld->Value->TxtValue) == 8)
				memcpy (shp->BufDbf + fld->Offset + 1,
					fld->Value->TxtValue, 8);
			}
		  }
		break;
	    case 'C':
		memset (shp->BufDbf + fld->Offset + 1, ' ', fld->Length);
		if (fld->Value)
		  {
		      if (fld->Value->Type == GAIA_TEXT_VALUE)
			{
			    strcpy (buf, fld->Value->TxtValue);
			    len = strlen (buf);
			    utf8len = 2048;
			    pBuf = buf;
			    pUtf8buf = utf8buf;
			    if (iconv
				((iconv_t) (shp->IconvObj), &pBuf, &len,
				 &pUtf8buf, &utf8len) < 0)
				goto conversion_error;
			    memcpy (buf, utf8buf, 2048 - utf8len);
			    buf[2048 - utf8len] = '\0';
			    if (strlen (buf) < fld->Length)
				memcpy (shp->BufDbf + fld->Offset + 1, buf,
					strlen (buf));
			    else
				memcpy (shp->BufDbf + fld->Offset + 1, buf,
					fld->Length);
			}
		  }
		break;
	    case 'N':
		memset (shp->BufDbf + fld->Offset + 1, '\0', fld->Length);
		if (fld->Value)
		  {
		      if (fld->Value->Type == GAIA_INT_VALUE)
			{
#if defined(_WIN32) || defined(__MINGW32__)
/* CAVEAT - M$ runtime doesn't supports %lld for 64 bits */
			    sprintf (dummy, "%I64d", fld->Value->IntValue);
#else
			    sprintf (dummy, "%lld", fld->Value->IntValue);
#endif
			    if (strlen (dummy) <= fld->Length)
				memcpy (shp->BufDbf + fld->Offset + 1, dummy,
					strlen (dummy));
			}
		      if (fld->Value->Type == GAIA_DOUBLE_VALUE)
			{
			    sprintf (fmt, "%%1.%dlf", fld->Decimals);
			    sprintf (dummy, fmt, fld->Value->DblValue);
			    if (strlen (dummy) <= fld->Length)
				memcpy (shp->BufDbf + fld->Offset + 1, dummy,
					strlen (dummy));
			}
		  }
		break;
	    };
	  fld = fld->Next;
      }
    if (!(entity->Geometry))
      {
	  /* exporting a NULL Shape */
	  gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
	  gaiaExport32 (shp->BufShp + 4, 2, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
	  fwrite (shp->BufShp, 1, 8, shp->flShx);
	  (shp->ShxSize) += 4;	/* updating current SHX file poisition [in 16 bits words !!!] */
	  gaiaExport32 (shp->BufShp, shp->DbfRecno, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
	  gaiaExport32 (shp->BufShp + 4, 2, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
	  gaiaExport32 (shp->BufShp + 8, 0, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = NULL */
	  fwrite (shp->BufShp, 1, 12, shp->flShp);
	  (shp->ShpSize) += 6;	/* updating current SHP file poisition [in 16 bits words !!!] */
      }
    else
      {
	  /* updates the shapefile main MBR-BBOX */
	  gaiaMbrGeometry (entity->Geometry);
	  if (entity->Geometry->MinX < shp->MinX)
	      shp->MinX = entity->Geometry->MinX;
	  if (entity->Geometry->MaxX > shp->MaxX)
	      shp->MaxX = entity->Geometry->MaxX;
	  if (entity->Geometry->MinY < shp->MinY)
	      shp->MinY = entity->Geometry->MinY;
	  if (entity->Geometry->MaxY > shp->MaxY)
	      shp->MaxY = entity->Geometry->MaxY;
	  if (shp->Shape == 1)
	    {
		/* this one is expected to be a POINT */
		gaiaPointPtr pt = entity->Geometry->FirstPoint;
		if (!pt)
		  {
		      strcpy (dummy,
			      "a POINT is expected, but there is no POINT in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		/* inserting POINT entity into SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, 10, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;	/* updating current SHX file poisition [in 16 bits words !!!] */
		/* inserting POINT into SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, 10, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, 1, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = POINT */
		gaiaExport64 (shp->BufShp + 12, pt->X, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports X coordinate */
		gaiaExport64 (shp->BufShp + 20, pt->Y, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports Y coordinate */
		fwrite (shp->BufShp, 1, 28, shp->flShp);
		(shp->ShpSize) += 14;	/* updating current SHP file poisition [in 16 bits words !!!] */
	    }
	  if (shp->Shape == 3)
	    {
		/* this one is expected to be a LINESTRING / MULTILINESTRING */
		gaiaLinestringPtr line;
		tot_ln = 0;
		tot_v = 0;
		line = entity->Geometry->FirstLinestring;
		while (line)
		  {
		      /* computes # lines and total # points */
		      tot_v += line->Points;
		      tot_ln++;
		      line = line->Next;
		  }
		if (!tot_ln)
		  {
		      strcpy (dummy,
			      "a LINESTRING is expected, but there is no LINESTRING in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		this_size = 22 + (2 * tot_ln) + (tot_v * 8);	/* size [in 16 bits words !!!] for this SHP entity */
		if ((this_size * 2) + 1024 > shp->ShpBfsz)
		  {
		      /* current buffer is too small; we need to allocate a bigger one */
		      free (shp->BufShp);
		      shp->ShpBfsz = (this_size * 2) + 1024;
		      shp->BufShp = malloc (shp->ShpBfsz);
		  }
		/* inserting LINESTRING or MULTILINESTRING in SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;
		/* inserting LINESTRING or MULTILINESTRING in SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, 3, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = POLYLINE */
		gaiaExport64 (shp->BufShp + 12, entity->Geometry->MinX, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the MBR for this geometry */
		gaiaExport64 (shp->BufShp + 20, entity->Geometry->MinY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 28, entity->Geometry->MaxX,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 36, entity->Geometry->MaxY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport32 (shp->BufShp + 44, tot_ln, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports # lines in this polyline */
		gaiaExport32 (shp->BufShp + 48, tot_v, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports total # points */
		tot_v = 0;	/* resets points counter */
		ix = 52;	/* sets current buffer offset */
		line = entity->Geometry->FirstLinestring;
		while (line)
		  {
		      /* exports start point index for each line */
		      gaiaExport32 (shp->BufShp + ix, tot_v, GAIA_LITTLE_ENDIAN,
				    endian_arch);
		      tot_v += line->Points;
		      ix += 4;
		      line = line->Next;
		  }
		line = entity->Geometry->FirstLinestring;
		while (line)
		  {
		      /* exports points for each line */
		      for (iv = 0; iv < line->Points; iv++)
			{
			    /* exports a POINT [x,y] */
			    gaiaGetPoint (line->Coords, iv, &x, &y);
			    gaiaExport64 (shp->BufShp + ix, x,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			    gaiaExport64 (shp->BufShp + ix, y,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			}
		      line = line->Next;
		  }
		fwrite (shp->BufShp, 1, ix, shp->flShp);
		(shp->ShpSize) += (ix / 2);	/* updating current SHP file poisition [in 16 bits words !!!] */
	    }
	  if (shp->Shape == 5)
	    {
		/* this one is expected to be a POLYGON or a MULTIPOLYGON */
		gaiaPolygonPtr polyg;
		gaiaRingPtr ring;
		int ib;
		tot_ln = 0;
		tot_v = 0;
		polyg = entity->Geometry->FirstPolygon;
		while (polyg)
		  {
		      /* computes # rings and total # points */
		      gaiaSaneClockwise (polyg);	/* we must assure that exterior ring is clockwise, and interior rings are anti-clockwise */
		      ring = polyg->Exterior;	/* this one is the exterior ring */
		      tot_v += ring->Points;
		      tot_ln++;
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    /* that ones are the interior rings */
			    ring = polyg->Interiors + ib;
			    tot_v += ring->Points;
			    tot_ln++;
			}
		      polyg = polyg->Next;
		  }
		if (!tot_ln)
		  {
		      strcpy (dummy,
			      "a POLYGON is expected, but there is no POLYGON in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		this_size = 22 + (2 * tot_ln) + (tot_v * 8);	/* size [in 16 bits words !!!] for this SHP entity */
		if ((this_size * 2) + 1024 > shp->ShpBfsz)
		  {
		      /* current buffer is too small; we need to allocate a bigger one */
		      free (shp->BufShp);
		      shp->ShpBfsz = (this_size * 2) + 1024;
		      shp->BufShp = malloc (shp->ShpBfsz);
		  }
		/* inserting POLYGON or MULTIPOLYGON in SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;
		/* inserting POLYGON or MULTIPOLYGON in SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, 5, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = POLYGON */
		gaiaExport64 (shp->BufShp + 12, entity->Geometry->MinX, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the MBR for this geometry */
		gaiaExport64 (shp->BufShp + 20, entity->Geometry->MinY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 28, entity->Geometry->MaxX,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 36, entity->Geometry->MaxY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport32 (shp->BufShp + 44, tot_ln, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports # rings in this polygon */
		gaiaExport32 (shp->BufShp + 48, tot_v, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports total # points */
		tot_v = 0;	/* resets points counter */
		ix = 52;	/* sets current buffer offset */
		polyg = entity->Geometry->FirstPolygon;
		while (polyg)
		  {
		      /* exports start point index for each line */
		      ring = polyg->Exterior;	/* this one is the exterior ring */
		      gaiaExport32 (shp->BufShp + ix, tot_v, GAIA_LITTLE_ENDIAN,
				    endian_arch);
		      tot_v += ring->Points;
		      ix += 4;
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    /* that ones are the interior rings */
			    ring = polyg->Interiors + ib;
			    gaiaExport32 (shp->BufShp + ix, tot_v,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    tot_v += ring->Points;
			    ix += 4;
			}
		      polyg = polyg->Next;
		  }
		polyg = entity->Geometry->FirstPolygon;
		while (polyg)
		  {
		      /* exports points for each ring */
		      ring = polyg->Exterior;	/* this one is the exterior ring */
		      for (iv = 0; iv < ring->Points; iv++)
			{
			    /* exports a POINT [x,y] - exterior ring */
			    gaiaGetPoint (ring->Coords, iv, &x, &y);
			    gaiaExport64 (shp->BufShp + ix, x,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			    gaiaExport64 (shp->BufShp + ix, y,
					  GAIA_LITTLE_ENDIAN, endian_arch);
			    ix += 8;
			}
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    /* that ones are the interior rings */
			    ring = polyg->Interiors + ib;
			    for (iv = 0; iv < ring->Points; iv++)
			      {
				  /* exports a POINT [x,y] - interior ring */
				  gaiaGetPoint (ring->Coords, iv, &x, &y);
				  gaiaExport64 (shp->BufShp + ix, x,
						GAIA_LITTLE_ENDIAN,
						endian_arch);
				  ix += 8;
				  gaiaExport64 (shp->BufShp + ix, y,
						GAIA_LITTLE_ENDIAN,
						endian_arch);
				  ix += 8;
			      }
			}
		      polyg = polyg->Next;
		  }
		fwrite (shp->BufShp, 1, ix, shp->flShp);
		(shp->ShpSize) += (ix / 2);
	    }
	  if (shp->Shape == 8)
	    {
		/* this one is expected to be a MULTIPOINT */
		gaiaPointPtr pt;
		tot_pts = 0;
		pt = entity->Geometry->FirstPoint;
		while (pt)
		  {
		      /* computes # points */
		      tot_pts++;
		      pt = pt->Next;
		  }
		if (!tot_pts)
		  {
		      strcpy (dummy,
			      "a MULTIPOINT is expected, but there is no POINT/MULTIPOINT in geometry");
		      if (shp->LastError)
			  free (shp->LastError);
		      len = strlen (dummy);
		      shp->LastError = malloc (len + 1);
		      strcpy (shp->LastError, dummy);
		      return 0;
		  }
		this_size = 20 + (tot_pts * 8);	/* size [in 16 bits words !!!] for this SHP entity */
		if ((this_size * 2) + 1024 > shp->ShpBfsz)
		  {
		      /* current buffer is too small; we need to allocate a bigger one */
		      free (shp->BufShp);
		      shp->ShpBfsz = (this_size * 2) + 1024;
		      shp->BufShp = malloc (shp->ShpBfsz);
		  }
		/* inserting MULTIPOINT in SHX file */
		gaiaExport32 (shp->BufShp, shp->ShpSize, GAIA_BIG_ENDIAN, endian_arch);	/* exports current SHP file position */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entitiy size [in 16 bits words !!!] */
		fwrite (shp->BufShp, 1, 8, shp->flShx);
		(shp->ShxSize) += 4;
		/* inserting MULTIPOINT in SHP file */
		gaiaExport32 (shp->BufShp, shp->DbfRecno, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity ID */
		gaiaExport32 (shp->BufShp + 4, this_size, GAIA_BIG_ENDIAN, endian_arch);	/* exports entity size [in 16 bits words !!!] */
		gaiaExport32 (shp->BufShp + 8, 8, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports geometry type = MULTIPOINT */
		gaiaExport64 (shp->BufShp + 12, entity->Geometry->MinX, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the MBR for this geometry */
		gaiaExport64 (shp->BufShp + 20, entity->Geometry->MinY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 28, entity->Geometry->MaxX,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport64 (shp->BufShp + 36, entity->Geometry->MaxY,
			      GAIA_LITTLE_ENDIAN, endian_arch);
		gaiaExport32 (shp->BufShp + 44, tot_pts, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports total # points */
		ix = 48;	/* sets current buffer offset */
		pt = entity->Geometry->FirstPoint;
		while (pt)
		  {
		      /* exports each point */
		      gaiaExport64 (shp->BufShp + ix, pt->X, GAIA_LITTLE_ENDIAN,
				    endian_arch);
		      ix += 8;
		      gaiaExport64 (shp->BufShp + ix, pt->Y, GAIA_LITTLE_ENDIAN,
				    endian_arch);
		      ix += 8;
		      pt = pt->Next;
		  }
		fwrite (shp->BufShp, 1, ix, shp->flShp);
		(shp->ShpSize) += (ix / 2);	/* updating current SHP file poisition [in 16 bits words !!!] */
	    }
      }
/* inserting entity in DBF file */
    fwrite (shp->BufDbf, 1, shp->DbfReclen, shp->flDbf);
    (shp->DbfRecno)++;
    return 1;
  conversion_error:
    if (shp->LastError)
	free (shp->LastError);
    sprintf (dummy, "Invalid character sequence");
    len = strlen (dummy);
    shp->LastError = malloc (len + 1);
    strcpy (shp->LastError, dummy);
    return 0;
}

GAIAGEO_DECLARE void
gaiaFlushShpHeaders (gaiaShapefilePtr shp)
{
/* updates the various file headers */
    FILE *fl_shp = shp->flShp;
    FILE *fl_shx = shp->flShx;
    FILE *fl_dbf = shp->flDbf;
    int shp_size = shp->ShpSize;
    int shx_size = shp->ShxSize;
    int dbf_size = shp->DbfSize;
    int dbf_reclen = shp->DbfReclen;
    int dbf_recno = shp->DbfRecno;
    int endian_arch = shp->endian_arch;
    double minx = shp->MinX;
    double miny = shp->MinY;
    double maxx = shp->MaxX;
    double maxy = shp->MaxY;
    unsigned char *buf_shp = shp->BufShp;
/* writing the SHP file header */
    fseek (fl_shp, 0, SEEK_SET);	/* repositioning at SHP file start */
    gaiaExport32 (buf_shp, 9994, GAIA_BIG_ENDIAN, endian_arch);	/* SHP magic number */
    gaiaExport32 (buf_shp + 4, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 8, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 12, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 16, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 20, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 24, shp_size, GAIA_BIG_ENDIAN, endian_arch);	/* SHP file size - measured in 16 bits words !!! */
    gaiaExport32 (buf_shp + 28, 1000, GAIA_LITTLE_ENDIAN, endian_arch);	/* version */
    gaiaExport32 (buf_shp + 32, shp->Shape, GAIA_LITTLE_ENDIAN, endian_arch);	/* ESRI shape */
    gaiaExport64 (buf_shp + 36, minx, GAIA_LITTLE_ENDIAN, endian_arch);	/* the MBR/BBOX for the whole shapefile */
    gaiaExport64 (buf_shp + 44, miny, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 52, maxx, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 60, maxy, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 68, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 76, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 84, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 92, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    fwrite (buf_shp, 1, 100, fl_shp);
/* writing the SHX file header */
    fseek (fl_shx, 0, SEEK_SET);	/* repositioning at SHX file start */
    gaiaExport32 (buf_shp, 9994, GAIA_BIG_ENDIAN, endian_arch);	/* SHP magic number */
    gaiaExport32 (buf_shp + 4, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 8, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 12, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 16, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 20, 0, GAIA_BIG_ENDIAN, endian_arch);
    gaiaExport32 (buf_shp + 24, shx_size, GAIA_BIG_ENDIAN, endian_arch);	/* SHXfile size - measured in 16 bits words !!! */
    gaiaExport32 (buf_shp + 28, 1000, GAIA_LITTLE_ENDIAN, endian_arch);	/* version */
    gaiaExport32 (buf_shp + 32, shp->Shape, GAIA_LITTLE_ENDIAN, endian_arch);	/* ESRI shape */
    gaiaExport64 (buf_shp + 36, minx, GAIA_LITTLE_ENDIAN, endian_arch);	/* the MBR for the whole shapefile */
    gaiaExport64 (buf_shp + 44, miny, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 52, maxx, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 60, maxy, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 68, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 76, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 84, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaExport64 (buf_shp + 92, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
    fwrite (buf_shp, 1, 100, fl_shx);
/* writing the DBF file header */
    *buf_shp = 0x1a;		/* DBF - this is theEOF marker */
    fwrite (buf_shp, 1, 1, fl_dbf);
    fseek (fl_dbf, 0, SEEK_SET);	/* repositioning at DBF file start */
    memset (buf_shp, '\0', 32);
    *buf_shp = 0x03;		/* DBF magic number */
    *(buf_shp + 1) = 1;		/* this is supposed to be the last update date [Year, Month, Day], but we ignore it at all */
    *(buf_shp + 2) = 1;
    *(buf_shp + 3) = 1;
    gaiaExport32 (buf_shp + 4, dbf_recno, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports # records in this DBF */
    gaiaExport16 (buf_shp + 8, (short) dbf_size, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the file header size */
    gaiaExport16 (buf_shp + 10, (short) dbf_reclen, GAIA_LITTLE_ENDIAN, endian_arch);	/* exports the record length */
    fwrite (buf_shp, 1, 32, fl_dbf);
}

GAIAGEO_DECLARE void
gaiaShpAnalyze (gaiaShapefilePtr shp)
{
/* analyzing the SHP content, in order to detect if there are LINESTRINGS or MULTILINESTRINGS 
/ the same check is needed in order to detect if there are POLYGONS or MULTIPOLYGONS 
 */
    unsigned char buf[512];
    int rd;
    int skpos;
    int offset;
    int off_shp;
    int sz;
    int shape;
    int points;
    int n;
    int n1;
    int base;
    int start;
    int end;
    int iv;
    int ind;
    double x;
    double y;
    int polygons;
    int multi = 0;
    int current_row = 0;
    gaiaRingPtr ring = NULL;
    while (1)
      {
	  /* positioning and reading the SHX file */
	  offset = 100 + (current_row * 8);	/* 100 bytes for the header + current row displacement; each SHX row = 8 bytes */
	  skpos = fseek (shp->flShx, offset, SEEK_SET);
	  if (skpos != 0)
	      goto exit;
	  rd = fread (buf, sizeof (unsigned char), 8, shp->flShx);
	  if (rd != 8)
	      goto exit;
	  off_shp = gaiaImport32 (buf, GAIA_BIG_ENDIAN, shp->endian_arch);
	  /* positioning and reading corresponding SHP entity - geometry */
	  offset = off_shp * 2;
	  skpos = fseek (shp->flShp, offset, SEEK_SET);
	  if (skpos != 0)
	      goto exit;
	  rd = fread (buf, sizeof (unsigned char), 12, shp->flShp);
	  if (rd != 12)
	      goto exit;
	  sz = gaiaImport32 (buf + 4, GAIA_BIG_ENDIAN, shp->endian_arch);
	  shape = gaiaImport32 (buf + 8, GAIA_LITTLE_ENDIAN, shp->endian_arch);
	  if ((sz * 2) > shp->ShpBfsz)
	    {
		/* current buffer is too small; we need to allocate a bigger buffer */
		free (shp->BufShp);
		shp->ShpBfsz = sz * 2;
		shp->BufShp = malloc (sizeof (unsigned char) * shp->ShpBfsz);
	    }
	  if (shape == 3 || shape == 13 || shape == 23)
	    {
		/* shape polyline */
		rd = fread (shp->BufShp, sizeof (unsigned char), 32,
			    shp->flShp);
		if (rd != 32)
		    goto exit;
		rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
			    shp->flShp);
		if (rd != (sz * 2) - 36)
		    goto exit;
		n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN,
				  shp->endian_arch);
		if (n > 1)
		    multi++;
	    }
	  if (shape == 5 || shape == 15 || shape == 25)
	    {
		/* shape polygon */
		polygons = 0;
		rd = fread (shp->BufShp, sizeof (unsigned char), 32,
			    shp->flShp);
		if (rd != 32)
		    goto exit;
		rd = fread (shp->BufShp, sizeof (unsigned char), (sz * 2) - 36,
			    shp->flShp);
		if (rd != (sz * 2) - 36)
		    goto exit;
		n = gaiaImport32 (shp->BufShp, GAIA_LITTLE_ENDIAN,
				  shp->endian_arch);
		n1 = gaiaImport32 (shp->BufShp + 4, GAIA_LITTLE_ENDIAN,
				   shp->endian_arch);
		base = 8 + (n * 4);
		start = 0;
		for (ind = 0; ind < n; ind++)
		  {
		      if (ind < (n - 1))
			  end =
			      gaiaImport32 (shp->BufShp + 8 + ((ind + 1) * 4),
					    GAIA_LITTLE_ENDIAN,
					    shp->endian_arch);
		      else
			  end = n1;
		      points = end - start;
		      ring = gaiaAllocRing (points);
		      points = 0;
		      for (iv = start; iv < end; iv++)
			{
			    x = gaiaImport64 (shp->BufShp + base + (iv * 16),
					      GAIA_LITTLE_ENDIAN,
					      shp->endian_arch);
			    y = gaiaImport64 (shp->BufShp + base + (iv * 16) +
					      8, GAIA_LITTLE_ENDIAN,
					      shp->endian_arch);
			    gaiaSetPoint (ring->Coords, points, x, y);
			    start++;
			    points++;
			}
		      if (!polygons)
			{
			    /* this one is the first POLYGON */
			    polygons = 1;
			}
		      else
			{
			    gaiaClockwise (ring);
			    if (ring->Clockwise)
			      {
				  /* this one is a different POLYGON exterior ring - we need to allocate e new POLYGON */
				  polygons++;
			      }
			}
		      gaiaFreeRing (ring);
		      ring = NULL;
		  }
		if (polygons > 1)
		    multi++;
	    }
	  current_row++;
      }
  exit:
    if (ring)
	gaiaFreeRing (ring);
    if (shp->LastError)
	free (shp->LastError);
    shp->LastError = NULL;
/* setting the EffectiveType, as determined by this analysis */
    if (shp->Shape == 3 || shp->Shape == 13 || shp->Shape == 23)
      {
	  /* SHAPE polyline */
	  if (multi)
	      shp->EffectiveType = GAIA_MULTILINESTRING;
	  else
	      shp->EffectiveType = GAIA_LINESTRING;
      }
    if (shp->Shape == 5 || shp->Shape == 15 || shp->Shape == 25)
      {
	  /* SHAPE polygon */
	  if (multi)
	      shp->EffectiveType = GAIA_MULTIPOLYGON;
	  else
	      shp->EffectiveType = GAIA_POLYGON;
      }
}
/**************** End file: gg_shape.c **********/


/**************** Begin file: gg_transform.c **********/

/* #include <stdio.h> */
/* #include <string.h> */

#if OMIT_PROJ == 0		/* including PROJ.4 */
/* #include <proj_api.h> */
#endif

/* #include <spatialite/sqlite3ext.h> */
/* #include <spatialite/gaiageo.h> */

GAIAGEO_DECLARE void
gaiaShiftCoords (gaiaGeomCollPtr geom, double shift_x, double shift_y)
{
/* returns a geometry that is the old old geometry with required shifting applied to coordinates */
    int ib;
    int iv;
    double x;
    double y;
    gaiaPointPtr point;
    gaiaPolygonPtr polyg;
    gaiaLinestringPtr line;
    gaiaRingPtr ring;
    if (!geom)
	return;
    point = geom->FirstPoint;
    while (point)
      {
	  /* shifting POINTs */
	  point->X += shift_x;
	  point->Y += shift_y;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* shifting LINESTRINGs */
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPoint (line->Coords, iv, &x, &y);
		x += shift_x;
		y += shift_y;
		gaiaSetPoint (line->Coords, iv, x, y);
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* shifting POLYGONs */
	  ring = polyg->Exterior;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* shifting the EXTERIOR RING */
		gaiaGetPoint (ring->Coords, iv, &x, &y);
		x += shift_x;
		y += shift_y;
		gaiaSetPoint (ring->Coords, iv, x, y);
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		/* shifting the INTERIOR RINGs */
		ring = polyg->Interiors + ib;
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		      x += shift_x;
		      y += shift_y;
		      gaiaSetPoint (ring->Coords, iv, x, y);
		  }
	    }
	  polyg = polyg->Next;
      }
    gaiaMbrGeometry (geom);
}

GAIAGEO_DECLARE void
gaiaScaleCoords (gaiaGeomCollPtr geom, double scale_x, double scale_y)
{
/* returns a geometry that is the old old geometry with required scaling applied to coordinates */
    int ib;
    int iv;
    double x;
    double y;
    gaiaPointPtr point;
    gaiaPolygonPtr polyg;
    gaiaLinestringPtr line;
    gaiaRingPtr ring;
    if (!geom)
	return;
    point = geom->FirstPoint;
    while (point)
      {
	  /* scaling POINTs */
	  point->X *= scale_x;
	  point->Y *= scale_y;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* scaling LINESTRINGs */
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPoint (line->Coords, iv, &x, &y);
		x *= scale_x;
		y *= scale_y;
		gaiaSetPoint (line->Coords, iv, x, y);
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* scaling POLYGONs */
	  ring = polyg->Exterior;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* scaling the EXTERIOR RING */
		gaiaGetPoint (ring->Coords, iv, &x, &y);
		x *= scale_x;
		y *= scale_y;
		gaiaSetPoint (ring->Coords, iv, x, y);
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		/* scaling the INTERIOR RINGs */
		ring = polyg->Interiors + ib;
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		      x *= scale_x;
		      y *= scale_y;
		      gaiaSetPoint (ring->Coords, iv, x, y);
		  }
	    }
	  polyg = polyg->Next;
      }
    gaiaMbrGeometry (geom);
}

GAIAGEO_DECLARE void
gaiaRotateCoords (gaiaGeomCollPtr geom, double angle)
{
/* returns a geometry that is the old old geometry with required rotation applied to coordinates */
    int ib;
    int iv;
    double x;
    double y;
    double nx;
    double ny;
    double rad = angle * 0.0174532925199432958;
    double cosine = cos (rad);
    double sine = sin (rad);
    gaiaPointPtr point;
    gaiaPolygonPtr polyg;
    gaiaLinestringPtr line;
    gaiaRingPtr ring;
    if (!geom)
	return;
    point = geom->FirstPoint;
    while (point)
      {
	  /* shifting POINTs */
	  x = point->X;
	  y = point->Y;
	  point->X = (x * cosine) + (y * sine);
	  point->Y = (y * cosine) - (x * sine);
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* rotating LINESTRINGs */
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPoint (line->Coords, iv, &x, &y);
		nx = (x * cosine) + (y * sine);
		ny = (y * cosine) - (x * sine);
		gaiaSetPoint (line->Coords, iv, nx, ny);
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* rotating POLYGONs */
	  ring = polyg->Exterior;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* rotating the EXTERIOR RING */
		gaiaGetPoint (ring->Coords, iv, &x, &y);
		nx = (x * cosine) + (y * sine);
		ny = (y * cosine) - (x * sine);
		gaiaSetPoint (ring->Coords, iv, nx, ny);
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		/* rotating the INTERIOR RINGs */
		ring = polyg->Interiors + ib;
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		      nx = (x * cosine) + (y * sine);
		      ny = (y * cosine) - (x * sine);
		      gaiaSetPoint (ring->Coords, iv, nx, ny);
		  }
	    }
	  polyg = polyg->Next;
      }
    gaiaMbrGeometry (geom);
}

GAIAGEO_DECLARE void
gaiaReflectCoords (gaiaGeomCollPtr geom, int x_axis, int y_axis)
{
/* returns a geometry that is the old old geometry with required reflection applied to coordinates */
    int ib;
    int iv;
    double x;
    double y;
    gaiaPointPtr point;
    gaiaPolygonPtr polyg;
    gaiaLinestringPtr line;
    gaiaRingPtr ring;
    if (!geom)
	return;
    point = geom->FirstPoint;
    while (point)
      {
	  /* reflecting POINTs */
	  if (x_axis)
	      point->X *= -1.0;
	  if (y_axis)
	      point->Y *= -1.0;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* reflecting LINESTRINGs */
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPoint (line->Coords, iv, &x, &y);
		if (x_axis)
		    x *= -1.0;
		if (y_axis)
		    y *= -1.0;
		gaiaSetPoint (line->Coords, iv, x, y);
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* reflecting POLYGONs */
	  ring = polyg->Exterior;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* reflecting the EXTERIOR RING */
		gaiaGetPoint (ring->Coords, iv, &x, &y);
		if (x_axis)
		    x *= -1.0;
		if (y_axis)
		    y *= -1.0;
		gaiaSetPoint (ring->Coords, iv, x, y);
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		/* reflecting the INTERIOR RINGs */
		ring = polyg->Interiors + ib;
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		      if (x_axis)
			  x *= -1.0;
		      if (y_axis)
			  y *= -1.0;
		      gaiaSetPoint (ring->Coords, iv, x, y);
		  }
	    }
	  polyg = polyg->Next;
      }
    gaiaMbrGeometry (geom);
}

GAIAGEO_DECLARE void
gaiaSwapCoords (gaiaGeomCollPtr geom)
{
/* returns a geometry that is the old old geometry with swapped x- and y-coordinates */
    int ib;
    int iv;
    double x;
    double y;
    double sv;
    gaiaPointPtr point;
    gaiaPolygonPtr polyg;
    gaiaLinestringPtr line;
    gaiaRingPtr ring;
    if (!geom)
	return;
    point = geom->FirstPoint;
    while (point)
      {
	  /* swapping POINTs */
	  sv = point->X;
	  point->X = point->Y;
	  point->Y = sv;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* swapping LINESTRINGs */
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPoint (line->Coords, iv, &x, &y);
		sv = x;
		x = y;
		y = sv;
		gaiaSetPoint (line->Coords, iv, x, y);
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* swapping POLYGONs */
	  ring = polyg->Exterior;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* shifting the EXTERIOR RING */
		gaiaGetPoint (ring->Coords, iv, &x, &y);
		sv = x;
		x = y;
		y = sv;
		gaiaSetPoint (ring->Coords, iv, x, y);
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		/* swapping the INTERIOR RINGs */
		ring = polyg->Interiors + ib;
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		      sv = x;
		      x = y;
		      y = sv;
		      gaiaSetPoint (ring->Coords, iv, x, y);
		  }
	    }
	  polyg = polyg->Next;
      }
    gaiaMbrGeometry (geom);
}

#if OMIT_PROJ == 0		/* including PROJ.4 */

static int
gaiaIsLongLat (char *str)
{
/* checks if we have to do with ANGLES if +proj=longlat is defined */
    if (strstr (str, "+proj=longlat") != NULL)
	return 1;
    return 0;
}

GAIAGEO_DECLARE double
gaiaRadsToDegs (double rads)
{
/* converts an ANGLE from radians to degrees */
    return rads * RAD_TO_DEG;
}

GAIAGEO_DECLARE double
gaiaDegsToRads (double degs)
{
/* converts an ANGLE from degrees to radians */
    return degs * DEG_TO_RAD;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaTransform (gaiaGeomCollPtr org, char *proj_from, char *proj_to)
{
/* creates a new GEOMETRY reprojecting coordinates from the original one */
    int ib;
    int cnt;
    int i;
    double *xx;
    double *yy;
    double *zz;
    double x;
    double y;
    int error = 0;
    int from_angle;
    int to_angle;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaLinestringPtr dst_ln;
    gaiaPolygonPtr pg;
    gaiaPolygonPtr dst_pg;
    gaiaRingPtr rng;
    gaiaRingPtr dst_rng;
    projPJ from_cs = pj_init_plus (proj_from);
    projPJ to_cs = pj_init_plus (proj_to);
    gaiaGeomCollPtr dst = gaiaAllocGeomColl ();
/* setting up projection parameters */
    from_angle = gaiaIsLongLat (proj_from);
    to_angle = gaiaIsLongLat (proj_to);
    if (!from_cs)
	return dst;
    if (!to_cs)
	return dst;
    cnt = 0;
    pt = org->FirstPoint;
    while (pt)
      {
	  /* counting POINTs */
	  cnt++;
	  pt = pt->Next;
      }
    if (cnt)
      {
	  /* reprojecting POINTs */
	  xx = malloc (sizeof (double) * cnt);
	  yy = malloc (sizeof (double) * cnt);
	  zz = malloc (sizeof (double) * cnt);
	  i = 0;
	  pt = org->FirstPoint;
	  while (pt)
	    {
		/* inserting points to be converted in temporary arrays */
		if (from_angle)
		  {
		      xx[i] = gaiaDegsToRads (pt->X);
		      yy[i] = gaiaDegsToRads (pt->Y);
		  }
		else
		  {
		      xx[i] = pt->X;
		      yy[i] = pt->Y;
		  }
		zz[i] = 0.0;
		i++;
		pt = pt->Next;
	    }
	  /* applying reprojection        */
	  if (pj_transform (from_cs, to_cs, cnt, 0, xx, yy, zz) == 0)
	    {
		/* inserting the reprojected POINTs in the new GEOMETRY */
		for (i = 0; i < cnt; i++)
		  {
		      if (to_angle)
			{
			    x = gaiaRadsToDegs (xx[i]);
			    y = gaiaRadsToDegs (yy[i]);
			}
		      else
			{
			    x = xx[i];
			    y = yy[i];
			}
		      gaiaAddPointToGeomColl (dst, x, y);
		  }
	    }
	  else
	      error = 1;
	  free (xx);
	  free (yy);
	  free (zz);
      }
    if (error)
	goto stop;
    ln = org->FirstLinestring;
    while (ln)
      {
	  /* reprojecting LINESTRINGs */
	  cnt = ln->Points;
	  xx = malloc (sizeof (double) * cnt);
	  yy = malloc (sizeof (double) * cnt);
	  zz = malloc (sizeof (double) * cnt);
	  for (i = 0; i < cnt; i++)
	    {
		/* inserting points to be converted in temporary arrays */
		gaiaGetPoint (ln->Coords, i, &x, &y);
		if (from_angle)
		  {
		      xx[i] = gaiaDegsToRads (x);
		      yy[i] = gaiaDegsToRads (y);
		  }
		else
		  {
		      xx[i] = x;
		      yy[i] = y;
		  }
		zz[i] = 0.0;
	    }
	  /* applying reprojection        */
	  if (pj_transform (from_cs, to_cs, cnt, 0, xx, yy, zz) == 0)
	    {
		/* inserting the reprojected LINESTRING in the new GEOMETRY */
		dst_ln = gaiaAddLinestringToGeomColl (dst, cnt);
		for (i = 0; i < cnt; i++)
		  {
		      /* setting LINESTRING points */
		      if (to_angle)
			{
			    x = gaiaRadsToDegs (xx[i]);
			    y = gaiaRadsToDegs (yy[i]);
			}
		      else
			{
			    x = xx[i];
			    y = yy[i];
			}
		      gaiaSetPoint (dst_ln->Coords, i, x, y);
		  }
	    }
	  else
	      error = 1;
	  free (xx);
	  free (yy);
	  free (zz);
	  if (error)
	      goto stop;
	  ln = ln->Next;
      }
    pg = org->FirstPolygon;
    while (pg)
      {
	  /* reprojecting POLYGONs */
	  rng = pg->Exterior;
	  cnt = rng->Points;
	  dst_pg = gaiaAddPolygonToGeomColl (dst, cnt, pg->NumInteriors);
	  xx = malloc (sizeof (double) * cnt);
	  yy = malloc (sizeof (double) * cnt);
	  zz = malloc (sizeof (double) * cnt);
	  for (i = 0; i < cnt; i++)
	    {
		/* inserting points to be converted in temporary arrays [EXTERIOR RING] */
		gaiaGetPoint (rng->Coords, i, &x, &y);
		if (from_angle)
		  {
		      xx[i] = gaiaDegsToRads (x);
		      yy[i] = gaiaDegsToRads (y);
		  }
		else
		  {
		      xx[i] = x;
		      yy[i] = y;
		  }
		zz[i] = 0.0;
	    }
	  /* applying reprojection        */
	  if (pj_transform (from_cs, to_cs, cnt, 0, xx, yy, zz) == 0)
	    {
		/* inserting the reprojected POLYGON in the new GEOMETRY */
		dst_rng = dst_pg->Exterior;
		for (i = 0; i < cnt; i++)
		  {
		      /* setting EXTERIOR RING points */
		      if (to_angle)
			{
			    x = gaiaRadsToDegs (xx[i]);
			    y = gaiaRadsToDegs (yy[i]);
			}
		      else
			{
			    x = xx[i];
			    y = yy[i];
			}
		      gaiaSetPoint (dst_rng->Coords, i, x, y);
		  }
	    }
	  else
	      error = 1;
	  free (xx);
	  free (yy);
	  free (zz);
	  if (error)
	      goto stop;
	  for (ib = 0; ib < pg->NumInteriors; ib++)
	    {
		/* processing INTERIOR RINGS */
		rng = pg->Interiors + ib;
		cnt = rng->Points;
		xx = malloc (sizeof (double) * cnt);
		yy = malloc (sizeof (double) * cnt);
		zz = malloc (sizeof (double) * cnt);
		for (i = 0; i < cnt; i++)
		  {
		      /* inserting points to be converted in temporary arrays [INTERIOR RING] */
		      gaiaGetPoint (rng->Coords, i, &x, &y);
		      if (from_angle)
			{
			    xx[i] = gaiaDegsToRads (x);
			    yy[i] = gaiaDegsToRads (y);
			}
		      else
			{
			    xx[i] = x;
			    yy[i] = y;
			}
		      zz[i] = 0.0;
		  }
		/* applying reprojection        */
		if (pj_transform (from_cs, to_cs, cnt, 0, xx, yy, zz) == 0)
		  {
		      /* inserting the reprojected POLYGON in the new GEOMETRY */
		      dst_rng = dst_pg->Interiors + ib;
		      dst_rng->Points = cnt;
		      dst_rng->Coords =
			  malloc (sizeof (double) * (dst_rng->Points * 2));
		      for (i = 0; i < cnt; i++)
			{
			    /* setting INTERIOR RING points */
			    if (to_angle)
			      {
				  x = gaiaRadsToDegs (xx[i]);
				  y = gaiaRadsToDegs (yy[i]);
			      }
			    else
			      {
				  x = xx[i];
				  y = yy[i];
			      }
			    gaiaSetPoint (dst_rng->Coords, i, x, y);
			}
		  }
		else
		    error = 1;
		free (xx);
		free (yy);
		free (zz);
		if (error)
		    goto stop;
	    }
	  pg = pg->Next;
      }
/* destroying the PROJ4 params */
  stop:
    pj_free (from_cs);
    pj_free (to_cs);
    if (error)
      {
	  /* some error occurred */
	  gaiaPointPtr pP;
	  gaiaPointPtr pPn;
	  gaiaLinestringPtr pL;
	  gaiaLinestringPtr pLn;
	  gaiaPolygonPtr pA;
	  gaiaPolygonPtr pAn;
	  pP = dst->FirstPoint;
	  while (pP != NULL)
	    {
		pPn = pP->Next;
		gaiaFreePoint (pP);
		pP = pPn;
	    }
	  pL = dst->FirstLinestring;
	  while (pL != NULL)
	    {
		pLn = pL->Next;
		gaiaFreeLinestring (pL);
		pL = pLn;
	    }
	  pA = dst->FirstPolygon;
	  while (pA != NULL)
	    {
		pAn = pA->Next;
		gaiaFreePolygon (pA);
		pA = pAn;
	    }
	  dst->FirstPoint = NULL;
	  dst->LastPoint = NULL;
	  dst->FirstLinestring = NULL;
	  dst->LastLinestring = NULL;
	  dst->FirstPolygon = NULL;
	  dst->LastPolygon = NULL;
      }
    if (dst)
	dst->DeclaredType = org->DeclaredType;
    return dst;
}

#endif /* end including PROJ.4 */
/**************** End file: gg_transform.c **********/


/**************** Begin file: gg_wkb.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <float.h> */

/* #include <spatialite/sqlite3ext.h> */
/* #include <spatialite/gaiageo.h> */

static void
ParseWkbPoint (gaiaGeomCollPtr geo)
{
/* decodes a POINT from WKB */
    double x;
    double y;
    if (geo->size < geo->offset + 16)
	return;
    x = gaiaImport64 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
		      geo->endian_arch);
    geo->offset += 16;
    gaiaAddPointToGeomColl (geo, x, y);
}

static void
ParseWkbLine (gaiaGeomCollPtr geo)
{
/* decodes a LINESTRING from WKB */
    int points;
    int iv;
    double x;
    double y;
    gaiaLinestringPtr line;
    if (geo->size < geo->offset + 4)
	return;
    points =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    if (geo->size < geo->offset + (16 * points))
	return;
    line = gaiaAddLinestringToGeomColl (geo, points);
    for (iv = 0; iv < points; iv++)
      {
	  x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
			    geo->endian_arch);
	  gaiaSetPoint (line->Coords, iv, x, y);
	  geo->offset += 16;
      }
}

static void
ParseWkbPolygon (gaiaGeomCollPtr geo)
{
/* decodes a POLYGON from WKB */
    int rings;
    int nverts;
    int iv;
    int ib;
    double x;
    double y;
    gaiaPolygonPtr polyg = NULL;
    gaiaRingPtr ring;
    if (geo->size < geo->offset + 4)
	return;
    rings =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    for (ib = 0; ib < rings; ib++)
      {
	  if (geo->size < geo->offset + 4)
	      return;
	  nverts =
	      gaiaImport32 (geo->blob + geo->offset, geo->endian,
			    geo->endian_arch);
	  geo->offset += 4;
	  if (geo->size < geo->offset + (16 * nverts))
	      return;
	  if (ib == 0)
	    {
		polyg = gaiaAddPolygonToGeomColl (geo, nverts, rings - 1);
		ring = polyg->Exterior;
	    }
	  else
	      ring = gaiaAddInteriorRing (polyg, ib - 1, nverts);
	  for (iv = 0; iv < nverts; iv++)
	    {
		x = gaiaImport64 (geo->blob + geo->offset, geo->endian,
				  geo->endian_arch);
		y = gaiaImport64 (geo->blob + (geo->offset + 8), geo->endian,
				  geo->endian_arch);
		geo->offset += 16;
		gaiaSetPoint (ring->Coords, iv, x, y);
	    }
      }
}

static void
ParseWkbGeometry (gaiaGeomCollPtr geo)
{
/* decodes a MULTIxx or GEOMETRYCOLLECTION from SpatiaLite BLOB */
    int entities;
    int type;
    int ie;
    if (geo->size < geo->offset + 4)
	return;
    entities =
	gaiaImport32 (geo->blob + geo->offset, geo->endian, geo->endian_arch);
    geo->offset += 4;
    for (ie = 0; ie < entities; ie++)
      {
	  if (geo->size < geo->offset + 5)
	      return;
	  type =
	      gaiaImport32 (geo->blob + geo->offset + 1, geo->endian,
			    geo->endian_arch);
	  geo->offset += 5;
	  switch (type)
	    {
	    case GAIA_POINT:
		ParseWkbPoint (geo);
		break;
	    case GAIA_LINESTRING:
		ParseWkbLine (geo);
		break;
	    case GAIA_POLYGON:
		ParseWkbPolygon (geo);
		break;
	    default:
		break;
	    };
      }
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaFromSpatiaLiteBlobWkb (const unsigned char *blob, unsigned int size)
{
/* decoding from SpatiaLite BLOB to GEOMETRY */
    int type;
    int little_endian;
    int endian_arch = gaiaEndianArch ();
    gaiaGeomCollPtr geo = NULL;
    if (size < 45)
	return NULL;		/* cannot be an internal BLOB WKB geometry */
    if (*(blob + 0) != GAIA_MARK_START)
	return NULL;		/* failed to recognize START signature */
    if (*(blob + (size - 1)) != GAIA_MARK_END)
	return NULL;		/* failed to recognize END signature */
    if (*(blob + 38) != GAIA_MARK_MBR)
	return NULL;		/* failed to recognize MBR signature */
    if (*(blob + 1) == GAIA_LITTLE_ENDIAN)
	little_endian = 1;
    else if (*(blob + 1) == GAIA_BIG_ENDIAN)
	little_endian = 0;
    else
	return NULL;		/* unknown encoding; nor litte-endian neither big-endian */
    type = gaiaImport32 (blob + 39, little_endian, endian_arch);
    geo = gaiaAllocGeomColl ();
    geo->Srid = gaiaImport32 (blob + 2, little_endian, endian_arch);
    geo->endian_arch = (char) endian_arch;
    geo->endian = (char) little_endian;
    geo->blob = blob;
    geo->size = size;
    geo->offset = 43;
    switch (type)
      {
      case GAIA_POINT:
	  ParseWkbPoint (geo);
	  break;
      case GAIA_LINESTRING:
	  ParseWkbLine (geo);
	  break;
      case GAIA_POLYGON:
	  ParseWkbPolygon (geo);
	  break;
      case GAIA_MULTIPOINT:
      case GAIA_MULTILINESTRING:
      case GAIA_MULTIPOLYGON:
      case GAIA_GEOMETRYCOLLECTION:
	  ParseWkbGeometry (geo);
	  break;
      default:
	  break;
      };
    geo->MinX = gaiaImport64 (blob + 6, little_endian, endian_arch);
    geo->MinY = gaiaImport64 (blob + 14, little_endian, endian_arch);
    geo->MaxX = gaiaImport64 (blob + 22, little_endian, endian_arch);
    geo->MaxY = gaiaImport64 (blob + 30, little_endian, endian_arch);
    switch (type)
      {
      case GAIA_POINT:
	  geo->DeclaredType = GAIA_POINT;
	  break;
      case GAIA_LINESTRING:
	  geo->DeclaredType = GAIA_LINESTRING;
	  break;
      case GAIA_POLYGON:
	  geo->DeclaredType = GAIA_POLYGON;
	  break;
      case GAIA_MULTIPOINT:
	  geo->DeclaredType = GAIA_MULTIPOINT;
	  break;
      case GAIA_MULTILINESTRING:
	  geo->DeclaredType = GAIA_MULTILINESTRING;
	  break;
      case GAIA_MULTIPOLYGON:
	  geo->DeclaredType = GAIA_MULTIPOLYGON;
	  break;
      case GAIA_GEOMETRYCOLLECTION:
	  geo->DeclaredType = GAIA_GEOMETRYCOLLECTION;
	  break;
      default:
	  geo->DeclaredType = GAIA_UNKNOWN;
	  break;
      };
    return geo;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaFromSpatiaLiteBlobMbr (const unsigned char *blob, unsigned int size)
{
/* decoding from SpatiaLite BLOB to GEOMETRY [MBR only] */
    int type;
    int little_endian;
    int endian_arch = gaiaEndianArch ();
    double minx;
    double miny;
    double maxx;
    double maxy;
    gaiaGeomCollPtr geo = NULL;
    gaiaPolygonPtr polyg;
    gaiaRingPtr ring;
    if (size < 45)
	return NULL;		/* cannot be an internal BLOB WKB geometry */
    if (*(blob + 0) != GAIA_MARK_START)
	return NULL;		/* failed to recognize START signature */
    if (*(blob + (size - 1)) != GAIA_MARK_END)
	return NULL;		/* failed to recognize END signature */
    if (*(blob + 38) != GAIA_MARK_MBR)
	return NULL;		/* failed to recognize MBR signature */
    if (*(blob + 1) == GAIA_LITTLE_ENDIAN)
	little_endian = 1;
    else if (*(blob + 1) == GAIA_BIG_ENDIAN)
	little_endian = 0;
    else
	return NULL;		/* unknown encoding; nor litte-endian neither big-endian */
    type = gaiaImport32 (blob + 39, little_endian, endian_arch);
    geo = gaiaAllocGeomColl ();
    polyg = gaiaAddPolygonToGeomColl (geo, 5, 0);
    ring = polyg->Exterior;
    minx = gaiaImport64 (blob + 6, little_endian, endian_arch);
    miny = gaiaImport64 (blob + 14, little_endian, endian_arch);
    maxx = gaiaImport64 (blob + 22, little_endian, endian_arch);
    maxy = gaiaImport64 (blob + 30, little_endian, endian_arch);
    gaiaSetPoint (ring->Coords, 0, minx, miny);	/* vertex # 1 */
    gaiaSetPoint (ring->Coords, 1, maxx, miny);	/* vertex # 2 */
    gaiaSetPoint (ring->Coords, 2, maxx, maxy);	/* vertex # 3 */
    gaiaSetPoint (ring->Coords, 3, minx, maxy);	/* vertex # 4 */
    gaiaSetPoint (ring->Coords, 4, minx, miny);	/* vertex # 5 [same as vertex # 1 to close the polygon] */
    return geo;
}

GAIAGEO_DECLARE void
gaiaToSpatiaLiteBlobWkb (gaiaGeomCollPtr geom, unsigned char **result,
			 int *size)
{
/* builds the SpatiaLite BLOB representation for this GEOMETRY */
    int ib;
    int iv;
    double x;
    double y;
    int entities = 0;
    int n_points = 0;
    int n_linestrings = 0;
    int n_polygons = 0;
    int type;
    unsigned char *ptr;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    gaiaPointPtr point = NULL;
    gaiaLinestringPtr line = NULL;
    gaiaPolygonPtr polyg = NULL;
    int endian_arch = gaiaEndianArch ();
    gaiaMbrGeometry (geom);
/* how many entities, and of what kind, do we have ? */
    pt = geom->FirstPoint;
    while (pt)
      {
	  point = pt;
	  entities++;
	  n_points++;
	  pt = pt->Next;
      }
    ln = geom->FirstLinestring;
    while (ln)
      {
	  line = ln;
	  entities++;
	  n_linestrings++;
	  ln = ln->Next;
      }
    pg = geom->FirstPolygon;
    while (pg)
      {
	  polyg = pg;
	  entities++;
	  n_polygons++;
	  pg = pg->Next;
      }
    *size = 0;
    *result = NULL;
    if (n_points == 0 && n_polygons == 0 && n_linestrings == 0)
	return;
/* ok, we can determine the geometry class */
    if (n_points == 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOINT)
	      type = GAIA_MULTIPOINT;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_POINT;
      }
    else if (n_points > 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_MULTIPOINT;
      }
    else if (n_points == 0 && n_linestrings == 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTILINESTRING)
	      type = GAIA_MULTILINESTRING;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_LINESTRING;
      }
    else if (n_points == 0 && n_linestrings > 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_MULTILINESTRING;
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons == 1)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOLYGON)
	      type = GAIA_MULTIPOLYGON;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_POLYGON;
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons > 1)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_MULTIPOLYGON;
      }
    else
	type = GAIA_GEOMETRYCOLLECTION;
/* and now we compute the size of BLOB */
    *size = 44;			/* header size */
    if (type == GAIA_POINT)
	*size += (sizeof (double) * 2);	/* [x,y] coords */
    else if (type == GAIA_LINESTRING)
	*size += (4 + ((sizeof (double) * 2) * line->Points));	/* # points + [x,y] for each vertex */
    else if (type == GAIA_POLYGON)
      {
	  rng = polyg->Exterior;
	  *size += (8 + ((sizeof (double) * 2) * rng->Points));	/* # rings + # points + [x.y] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		*size += (4 + ((sizeof (double) * 2) * rng->Points));	/* # points + [x,y] array - interior ring */
	    }
      }
    else
      {
	  /* this one is not a simple geometry; should be a MULTIxxxx or a GEOMETRYCOLLECTION */
	  *size += 4;		/* # entities */
	  point = geom->FirstPoint;
	  while (point)
	    {
		*size += 5;	/* entity header */
		*size += (sizeof (double) * 2);	/* two doubles for each POINT */
		point = point->Next;
	    }
	  line = geom->FirstLinestring;
	  while (line)
	    {
		*size += 5;	/* entity header */
		*size += (4 + ((sizeof (double) * 2) * line->Points));	/* # points + [x,y] for each vertex */
		line = line->Next;
	    }
	  polyg = geom->FirstPolygon;
	  while (polyg)
	    {
		*size += 5;	/* entity header */
		rng = polyg->Exterior;
		*size += (8 + ((sizeof (double) * 2) * rng->Points));	/* # rings + # points + [x.y] array - exterior ring */
		for (ib = 0; ib < polyg->NumInteriors; ib++)
		  {
		      rng = polyg->Interiors + ib;
		      *size += (4 + ((sizeof (double) * 2) * rng->Points));	/* # points + [x,y] array - interior ring */
		  }
		polyg = polyg->Next;
	    }
      }
    *result = malloc (*size);
    ptr = *result;
/* and finally we build the BLOB */
    if (type == GAIA_POINT)
      {
	  *ptr = GAIA_MARK_START;	/* START signature */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimun X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimun Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximun X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximun Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POINT, 1, endian_arch);	/* class POINT */
	  gaiaExport64 (ptr + 43, point->X, 1, endian_arch);	/* X */
	  gaiaExport64 (ptr + 51, point->Y, 1, endian_arch);	/* Y */
	  *(ptr + 59) = GAIA_MARK_END;	/* END signature */
      }
    else if (type == GAIA_LINESTRING)
      {
	  *ptr = GAIA_MARK_START;	/* START signatue */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimun X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimun Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximun X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximun Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_LINESTRING, 1, endian_arch);	/* class LINESTRING */
	  gaiaExport32 (ptr + 43, line->Points, 1, endian_arch);	/* # points */
	  ptr += 47;
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPoint (line->Coords, iv, &x, &y);
		gaiaExport64 (ptr, x, 1, endian_arch);
		gaiaExport64 (ptr + 8, y, 1, endian_arch);
		ptr += 16;
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
      }
    else if (type == GAIA_POLYGON)
      {
	  *ptr = GAIA_MARK_START;	/* START signatue */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimun X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimun Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximun X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximun Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, GAIA_POLYGON, 1, endian_arch);	/* class POLYGON */
	  gaiaExport32 (ptr + 43, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr + 47, rng->Points, 1, endian_arch);	/* # points - exterior ring */
	  ptr += 51;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPoint (rng->Coords, iv, &x, &y);
		gaiaExport64 (ptr, x, 1, endian_arch);	/* X - exterior ring */
		gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - exterior ring */
		ptr += 16;
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		ptr += 4;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X - interior ring */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - interior ring */
		      ptr += 16;
		  }
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
      }
    else
      {
	  /* this one is a MULTIxxxx or a GEOMETRYCOLLECTION - building the main header */
	  *ptr = GAIA_MARK_START;	/* START signatue */
	  *(ptr + 1) = GAIA_LITTLE_ENDIAN;	/* byte ordering */
	  gaiaExport32 (ptr + 2, geom->Srid, 1, endian_arch);	/* the SRID */
	  gaiaExport64 (ptr + 6, geom->MinX, 1, endian_arch);	/* MBR - minimun X */
	  gaiaExport64 (ptr + 14, geom->MinY, 1, endian_arch);	/* MBR - minimun Y */
	  gaiaExport64 (ptr + 22, geom->MaxX, 1, endian_arch);	/* MBR - maximun X */
	  gaiaExport64 (ptr + 30, geom->MaxY, 1, endian_arch);	/* MBR - maximun Y */
	  *(ptr + 38) = GAIA_MARK_MBR;	/* MBR signature */
	  gaiaExport32 (ptr + 39, type, 1, endian_arch);	/* geometric class */
	  gaiaExport32 (ptr + 43, entities, 1, endian_arch);	/* # entities */
	  ptr += 47;
	  point = geom->FirstPoint;
	  while (point)
	    {
		*ptr = GAIA_MARK_ENTITY;	/* ENTITY signature */
		gaiaExport32 (ptr + 1, GAIA_POINT, 1, endian_arch);	/* class POINT */
		gaiaExport64 (ptr + 5, point->X, 1, endian_arch);	/* X */
		gaiaExport64 (ptr + 13, point->Y, 1, endian_arch);	/* Y */
		ptr += 21;
		point = point->Next;
	    }
	  line = geom->FirstLinestring;
	  while (line)
	    {
		*ptr = GAIA_MARK_ENTITY;	/* ENTITY signature */
		gaiaExport32 (ptr + 1, GAIA_LINESTRING, 1, endian_arch);	/* class LINESTRING */
		gaiaExport32 (ptr + 5, line->Points, 1, endian_arch);	/* # points */
		ptr += 9;
		for (iv = 0; iv < line->Points; iv++)
		  {
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y */
		      ptr += 16;
		  }
		line = line->Next;
	    }
	  polyg = geom->FirstPolygon;
	  while (polyg)
	    {
		*ptr = GAIA_MARK_ENTITY;	/* ENTITY signature */
		gaiaExport32 (ptr + 1, GAIA_POLYGON, 1, endian_arch);	/* class POLYGON */
		gaiaExport32 (ptr + 5, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
		rng = polyg->Exterior;
		gaiaExport32 (ptr + 9, rng->Points, 1, endian_arch);	/* # points - exterior ring */
		ptr += 13;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X - exterior ring */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - exterior ring */
		      ptr += 16;
		  }
		for (ib = 0; ib < polyg->NumInteriors; ib++)
		  {
		      rng = polyg->Interiors + ib;
		      gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		      ptr += 4;
		      for (iv = 0; iv < rng->Points; iv++)
			{
			    gaiaGetPoint (rng->Coords, iv, &x, &y);
			    gaiaExport64 (ptr, x, 1, endian_arch);	/* X - interior ring */
			    gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - interior ring */
			    ptr += 16;
			}
		  }
		*ptr = GAIA_MARK_END;	/* END signature */
		polyg = polyg->Next;
	    }
	  *ptr = GAIA_MARK_END;	/* END signature */
      }
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaFromWkb (const unsigned char *blob, unsigned int size)
{
/* decoding from WKB to GEOMETRY  */
    int type;
    int little_endian;
    gaiaGeomCollPtr geo = NULL;
    int endian_arch = gaiaEndianArch ();
    if (size < 5)
	return NULL;
    if (*(blob + 0) == 0x01)
	little_endian = GAIA_LITTLE_ENDIAN;
    else
	little_endian = GAIA_BIG_ENDIAN;
    type = gaiaImport32 (blob + 1, little_endian, endian_arch);
    geo = gaiaAllocGeomColl ();
    geo->Srid = -1;
    geo->endian_arch = (char) endian_arch;
    geo->endian = (char) little_endian;
    geo->blob = blob;
    geo->size = size;
    geo->offset = 5;
    switch (type)
      {
      case GAIA_POINT:
	  ParseWkbPoint (geo);
	  break;
      case GAIA_LINESTRING:
	  ParseWkbLine (geo);
	  break;
      case GAIA_POLYGON:
	  ParseWkbPolygon (geo);
	  break;
      case GAIA_MULTIPOINT:
      case GAIA_MULTILINESTRING:
      case GAIA_MULTIPOLYGON:
      case GAIA_GEOMETRYCOLLECTION:
	  ParseWkbGeometry (geo);
	  break;
      default:
	  break;
      };
    gaiaMbrGeometry (geo);
    geo->DeclaredType = type;
    return geo;
}

GAIAGEO_DECLARE char *
gaiaToHexWkb (gaiaGeomCollPtr geom)
{
/* builds the hexadecimal WKB representation for this GEOMETRY */
    unsigned char *wkb = NULL;
    int size = 0;
    char *hexbuf = NULL;
    int i;
    char hex[16];
    char *p;
    gaiaToWkb (geom, &wkb, &size);
    if (!wkb)
	return NULL;
    hexbuf = malloc ((size * 2) + 1);
    p = hexbuf;
    for (i = 0; i < size; i++)
      {
	  sprintf (hex, "%02X", *(wkb + i));
	  *p++ = hex[0];
	  *p++ = hex[1];
      }
    *p = '\0';
    return hexbuf;
}

GAIAGEO_DECLARE void
gaiaToWkb (gaiaGeomCollPtr geom, unsigned char **result, int *size)
{
/* builds the WKB representation for this GEOMETRY */
    int ib;
    int iv;
    double x;
    double y;
    int entities = 0;
    int n_points = 0;
    int n_linestrings = 0;
    int n_polygons = 0;
    int type;
    unsigned char *ptr;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    gaiaPointPtr point = NULL;
    gaiaLinestringPtr line = NULL;
    gaiaPolygonPtr polyg = NULL;
    int endian_arch = gaiaEndianArch ();
    gaiaMbrGeometry (geom);
/* how many entities, and of what kind, do we have ? */
    pt = geom->FirstPoint;
    while (pt)
      {
	  point = pt;
	  entities++;
	  n_points++;
	  pt = pt->Next;
      }
    ln = geom->FirstLinestring;
    while (ln)
      {
	  line = ln;
	  entities++;
	  n_linestrings++;
	  ln = ln->Next;
      }
    pg = geom->FirstPolygon;
    while (pg)
      {
	  polyg = pg;
	  entities++;
	  n_polygons++;
	  pg = pg->Next;
      }
    *size = 0;
    *result = NULL;
    if (n_points == 0 && n_polygons == 0 && n_linestrings == 0)
	return;
/* ok, we can determine the geometry class */
    if (n_points == 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOINT)
	      type = GAIA_MULTIPOINT;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_POINT;
      }
    else if (n_points > 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_MULTIPOINT;
      }
    else if (n_points == 0 && n_linestrings == 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTILINESTRING)
	      type = GAIA_MULTILINESTRING;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_LINESTRING;
      }
    else if (n_points == 0 && n_linestrings > 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_MULTILINESTRING;
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons == 1)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOLYGON)
	      type = GAIA_MULTIPOLYGON;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_POLYGON;
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons > 1)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_MULTIPOLYGON;
      }
    else
	type = GAIA_GEOMETRYCOLLECTION;
/* and now we compute the size of WKB */
    *size = 5;			/* header size */
    if (type ==
	GAIA_MULTIPOINT
	|| type ==
	GAIA_MULTILINESTRING
	|| type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	*size += 4;
    point = geom->FirstPoint;
    while (point)
      {
	  if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING
	      || type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	      *size += 5;
	  *size += (sizeof (double) * 2);	/* two doubles for each POINT */
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING
	      || type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	      *size += 5;
	  *size += (4 + ((sizeof (double) * 2) * line->Points));	/* # points + [x,y] for each vertex */
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  if (type == GAIA_MULTIPOINT || type == GAIA_MULTILINESTRING
	      || type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	      *size += 5;
	  rng = polyg->Exterior;
	  *size += (8 + ((sizeof (double) * 2) * rng->Points));	/* # rings + # points + [x.y] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		*size += (4 + ((sizeof (double) * 2) * rng->Points));	/* # points + [x,y] array - interior ring */
	    }
	  polyg = polyg->Next;
      }
    *result = malloc (*size);
    ptr = *result;
/* and finally we build the WKB */
    *ptr = 0x01;		/* little endian byte order */
    gaiaExport32 (ptr + 1, type, 1, endian_arch);	/* the main CLASS TYPE */
    ptr += 5;
    if (type ==
	GAIA_MULTIPOINT
	|| type ==
	GAIA_MULTILINESTRING
	|| type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
      {
	  gaiaExport32 (ptr, entities, 1, endian_arch);	/* it's a collection; # entities */
	  ptr += 4;
      }
    point = geom->FirstPoint;
    while (point)
      {
	  if (type == GAIA_MULTIPOINT || type == GAIA_GEOMETRYCOLLECTION)
	    {
		*ptr = 0x01;
		gaiaExport32 (ptr + 1, GAIA_POINT, 1, endian_arch);	/* it's a collection: the CLASS TYPE for this element */
		ptr += 5;
	    }
	  gaiaExport64 (ptr, point->X, 1, endian_arch);	/* X */
	  gaiaExport64 (ptr + 8, point->Y, 1, endian_arch);	/* Y */
	  ptr += 16;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  if (type == GAIA_MULTILINESTRING || type == GAIA_GEOMETRYCOLLECTION)
	    {
		*ptr = 0x01;
		gaiaExport32 (ptr + 1, GAIA_LINESTRING, 1, endian_arch);	/* it's a collection: the CLASS TYPE for this element */
		ptr += 5;
	    }
	  gaiaExport32 (ptr, line->Points, 1, endian_arch);	/* # points */
	  ptr += 4;
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPoint (line->Coords, iv, &x, &y);
		gaiaExport64 (ptr, x, 1, endian_arch);	/* X */
		gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y */
		ptr += 16;
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  if (type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	    {
		*ptr = 0x01;
		gaiaExport32 (ptr + 1, GAIA_POLYGON, 1, endian_arch);	/* it's a collection: the CLASS TYPE for this element */
		ptr += 5;
	    }
	  gaiaExport32 (ptr, polyg->NumInteriors + 1, 1, endian_arch);	/* # rings */
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr + 4, rng->Points, 1, endian_arch);	/* # points - exterior ring */
	  ptr += 8;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPoint (rng->Coords, iv, &x, &y);
		gaiaExport64 (ptr, x, 1, endian_arch);	/* X - exterior ring */
		gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - exterior ring */
		ptr += 16;
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		ptr += 4;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		      gaiaExport64 (ptr, x, 1, endian_arch);	/* X - interior ring */
		      gaiaExport64 (ptr + 8, y, 1, endian_arch);	/* Y - interior ring */
		      ptr += 16;
		  }
	    }
	  polyg = polyg->Next;
      }
}

static int
coordDimsFromFgf (int endian_arch, const unsigned char *blob, unsigned int size)
{
/* decoding the coordinate Dimensions for an FGF Geometry */
    int coord_dims;
    if (size < 4)
	return 0;
    coord_dims = gaiaImport32 (blob, GAIA_LITTLE_ENDIAN, endian_arch);
    switch (coord_dims)
      {
      case GAIA_XY:
	  return 2;
      case GAIA_XY_M:
      case GAIA_XY_Z:
	  return 3;
      case GAIA_XY_Z_M:
	  return 4;
      default:
	  return 0;
      }
}

static int
pointFromFgf (gaiaGeomCollPtr geom, int endian_arch, const unsigned char *blob,
	      unsigned int size, unsigned int *consumed)
{
/* decoding a POINT Geometry from FGF  */
    double x;
    double y;
    unsigned int sz = size;
    const unsigned char *ptr = blob;
    int coord_dims;
/* checking Geometry Type */
    if (sz < 4)
	return 0;
    if (gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch) != GAIA_POINT)
	return 0;
    ptr += 4;
    sz -= 4;
/* checking size */
    if (sz < 4)
	return 0;
    coord_dims = coordDimsFromFgf (endian_arch, ptr, size);
    if (!coord_dims)
	return 0;
    ptr += 4;
    sz -= 4;
    if (sz < (coord_dims * sizeof (double)))
	return 0;
    if (consumed)
	*consumed = coord_dims * sizeof (double);
/* building the POINT */
    x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
    y = gaiaImport64 (ptr + 8, GAIA_LITTLE_ENDIAN, endian_arch);
    gaiaAddPointToGeomColl (geom, x, y);
    return 1;
}

static int
linestringFromFgf (gaiaGeomCollPtr geom, int endian_arch,
		   const unsigned char *blob, unsigned int size,
		   unsigned int *consumed)
{
/* decoding a LINESTRING Geometry from FGF  */
    gaiaLinestringPtr ln;
    int pts;
    int iv;
    double x;
    double y;
    unsigned int ln_sz;
    unsigned int sz = size;
    const unsigned char *ptr = blob;
    int coord_dims;
/* checking Geometry Type */
    if (sz < 4)
	return 0;
    if (gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch) != GAIA_LINESTRING)
	return 0;
    ptr += 4;
    sz -= 4;
/* checking size */
    coord_dims = coordDimsFromFgf (endian_arch, ptr, size);
    if (!coord_dims)
	return 0;
    ptr += 4;
    sz -= 4;
/* how many points are there ? */
    if (sz < 4)
	return 0;
    pts = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
    ptr += 4;
    sz -= 4;
    if (pts < 2)
	return 0;
    ln_sz = pts * coord_dims * sizeof (double);
    if (sz < ln_sz)
	return 0;
    if (consumed)
	*consumed = (12 + ln_sz);
/* building the LINESTRING */
    ln = gaiaAddLinestringToGeomColl (geom, pts);
    for (iv = 0; iv < pts; iv++)
      {
	  /* inserting vertices into the linestring */
	  x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
	  y = gaiaImport64 (ptr + sizeof (double), GAIA_LITTLE_ENDIAN,
			    endian_arch);
	  ptr += (coord_dims * sizeof (double));
	  gaiaSetPoint (ln->Coords, iv, x, y);
      }
    return 1;
}

static int
polygonFromFgf (gaiaGeomCollPtr geom, int endian_arch,
		const unsigned char *blob, unsigned int size,
		unsigned int *consumed)
{
/* decoding a POLYGON Geometry from FGF  */
    gaiaPolygonPtr pg = NULL;
    gaiaRingPtr rng;
    int rings;
    int ir;
    int pts;
    int iv;
    double x;
    double y;
    unsigned int rng_sz;
    unsigned int sz = size;
    const unsigned char *ptr = blob;
    int coord_dims;
    unsigned int bytes = 0;
/* checking Geometry Type */
    if (sz < 4)
	return 0;
    if (gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch) != GAIA_POLYGON)
	return 0;
    ptr += 4;
    sz -= 4;
    bytes += 4;
/* checking size */
    coord_dims = coordDimsFromFgf (endian_arch, ptr, size);
    if (!coord_dims)
	return 0;
    ptr += 4;
    sz -= 4;
    bytes += 4;
/* how many rings are there ? */
    if (sz < 4)
	return 0;
    rings = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
    ptr += 4;
    sz -= 4;
    bytes += 4;
    if (rings < 1)
	return 0;
    for (ir = 0; ir < rings; ir++)
      {
	  /* fetching Polygon's rings */
	  if (sz < 4)
	      return 0;
	  pts = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
	  ptr += 4;
	  sz -= 4;
	  bytes += 4;
	  if (pts < 4)
	      return 0;
	  rng_sz = pts * coord_dims * sizeof (double);
	  if (sz < rng_sz)
	      return 0;
	  bytes += rng_sz;
	  if (ir == 0)
	    {
		/* building the EXTERIOR RING */
		pg = gaiaAddPolygonToGeomColl (geom, pts, rings - 1);
		rng = pg->Exterior;
		for (iv = 0; iv < pts; iv++)
		  {
		      /* inserting vertices into the EXTERIOR Ring */
		      x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
		      y = gaiaImport64 (ptr + sizeof (double),
					GAIA_LITTLE_ENDIAN, endian_arch);
		      ptr += (coord_dims * sizeof (double));
		      gaiaSetPoint (rng->Coords, iv, x, y);
		  }
	    }
	  else
	    {
		/* building an INTERIOR RING */
		rng = gaiaAddInteriorRing (pg, ir - 1, pts);
		for (iv = 0; iv < pts; iv++)
		  {
		      /* inserting vertices into some INTERIOR Ring */
		      x = gaiaImport64 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
		      y = gaiaImport64 (ptr + sizeof (double),
					GAIA_LITTLE_ENDIAN, endian_arch);
		      ptr += (coord_dims * sizeof (double));
		      gaiaSetPoint (rng->Coords, iv, x, y);
		  }
	    }
	  sz -= rng_sz;
      }
    if (consumed)
	*consumed = bytes;
    return 1;
}

static int
multiPointFromFgf (gaiaGeomCollPtr geom, int endian_arch,
		   const unsigned char *blob, unsigned int size)
{
/* decoding a MULTIPOINT Geometry from FGF  */
    int pts;
    int ipt;
    unsigned int sz = size;
    const unsigned char *ptr = blob;
    unsigned int consumed;
/* checking Geometry Type */
    if (sz < 4)
	return 0;
    if (gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch) != GAIA_MULTIPOINT)
	return 0;
    ptr += 4;
    sz -= 4;
/* how many points are there ? */
    if (sz < 4)
	return 0;
    pts = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
    ptr += 4;
    sz -= 4;
    if (pts < 1)
	return 0;
    for (ipt = 0; ipt < pts; ipt++)
      {
	  /* fetching individual Points from FGF */
	  if (!pointFromFgf (geom, endian_arch, ptr, sz, &consumed))
	      return 0;
	  ptr += consumed;
	  sz -= consumed;
      }
    return 1;
}

static int
multiLinestringFromFgf (gaiaGeomCollPtr geom, int endian_arch,
			const unsigned char *blob, unsigned int size)
{
/* decoding a MULTILINESTRING Geometry from FGF  */
    int lns;
    int iln;
    unsigned int sz = size;
    const unsigned char *ptr = blob;
    unsigned int consumed;
/* checking Geometry Type */
    if (sz < 4)
	return 0;
    if (gaiaImport32
	(ptr, GAIA_LITTLE_ENDIAN, endian_arch) != GAIA_MULTILINESTRING)
	return 0;
    ptr += 4;
    sz -= 4;
/* how many linestrings are there ? */
    if (sz < 4)
	return 0;
    lns = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
    ptr += 4;
    sz -= 4;
    if (lns < 1)
	return 0;
    for (iln = 0; iln < lns; iln++)
      {
	  /* fetching individual Linestrings from FGF */
	  if (!linestringFromFgf (geom, endian_arch, ptr, sz, &consumed))
	      return 0;
	  ptr += consumed;
	  sz -= consumed;
      }
    return 1;
}

static int
multiPolygonFromFgf (gaiaGeomCollPtr geom, int endian_arch,
		     const unsigned char *blob, unsigned int size)
{
/* decoding a MULTIPOLYGON Geometry from FGF  */
    int pgs;
    int ipg;
    unsigned int sz = size;
    const unsigned char *ptr = blob;
    unsigned int consumed;
/* checking Geometry Type */
    if (sz < 4)
	return 0;
    if (gaiaImport32
	(ptr, GAIA_LITTLE_ENDIAN, endian_arch) != GAIA_MULTIPOLYGON)
	return 0;
    ptr += 4;
    sz -= 4;
/* how many polygons are there ? */
    if (sz < 4)
	return 0;
    pgs = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
    ptr += 4;
    sz -= 4;
    if (pgs < 1)
	return 0;
    for (ipg = 0; ipg < pgs; ipg++)
      {
	  /* fetching individual Polygons from FGF */
	  if (!polygonFromFgf (geom, endian_arch, ptr, sz, &consumed))
	      return 0;
	  ptr += consumed;
	  sz -= consumed;
      }
    return 1;
}

static int
geomCollectionFromFgf (gaiaGeomCollPtr geom, int endian_arch,
		       const unsigned char *blob, unsigned int size)
{
/* decoding a  GEOMETRYCOLLECTION Geometry from FGF  */
    int geoms;
    int ig;
    int geom_type;
    unsigned int sz = size;
    const unsigned char *ptr = blob;
    unsigned int consumed;
/* checking Geometry Type */
    if (sz < 4)
	return 0;
    if (gaiaImport32
	(ptr, GAIA_LITTLE_ENDIAN, endian_arch) != GAIA_GEOMETRYCOLLECTION)
	return 0;
    ptr += 4;
    sz -= 4;
/* how many individual Geometries are there ? */
    if (sz < 4)
	return 0;
    geoms = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
    ptr += 4;
    sz -= 4;
    if (geoms < 1)
	return 0;
    for (ig = 0; ig < geoms; ig++)
      {
	  /* fetching individual Geometries from FGF */
	  if (sz < 4)
	      return 0;
	  geom_type = gaiaImport32 (ptr, GAIA_LITTLE_ENDIAN, endian_arch);
	  switch (geom_type)
	    {
	    case GAIA_POINT:
		if (!pointFromFgf (geom, endian_arch, ptr, sz, &consumed))
		    return 0;
		break;
	    case GAIA_LINESTRING:
		if (!linestringFromFgf (geom, endian_arch, ptr, sz, &consumed))
		    return 0;
		break;
	    case GAIA_POLYGON:
		if (!polygonFromFgf (geom, endian_arch, ptr, sz, &consumed))
		    return 0;
		break;
	    default:		/* unsupported geometry type */
		return 0;
		break;
	    };
	  ptr += consumed;
	  sz -= consumed;
      }
    return 1;
}

GAIAGEO_DECLARE gaiaGeomCollPtr
gaiaFromFgf (const unsigned char *blob, unsigned int size)
{
/* decoding from FGF to GEOMETRY  */
    gaiaGeomCollPtr geom = NULL;
    int geom_type;
    int endian_arch = gaiaEndianArch ();
    if (size < 4)
	return NULL;
/* checking FGF type */
    geom_type = gaiaImport32 (blob, GAIA_LITTLE_ENDIAN, endian_arch);
    geom = gaiaAllocGeomColl ();
    geom->DeclaredType = geom_type;
    switch (geom_type)
      {
      case GAIA_POINT:
	  if (pointFromFgf (geom, endian_arch, blob, size, NULL))
	      return geom;
	  break;
      case GAIA_LINESTRING:
	  if (linestringFromFgf (geom, endian_arch, blob, size, NULL))
	      return geom;
	  break;
      case GAIA_POLYGON:
	  if (polygonFromFgf (geom, endian_arch, blob, size, NULL))
	      return geom;
	  break;
      case GAIA_MULTIPOINT:
	  if (multiPointFromFgf (geom, endian_arch, blob, size))
	      return geom;
	  break;
      case GAIA_MULTILINESTRING:
	  if (multiLinestringFromFgf (geom, endian_arch, blob, size))
	      return geom;
	  break;
      case GAIA_MULTIPOLYGON:
	  if (multiPolygonFromFgf (geom, endian_arch, blob, size))
	      return geom;
	  break;
      case GAIA_GEOMETRYCOLLECTION:
	  if (geomCollectionFromFgf (geom, endian_arch, blob, size))
	      return geom;
	  break;
      default:			/* unsupported geometry type */
	  break;
      };
    gaiaFreeGeomColl (geom);
    return NULL;
}

GAIAGEO_DECLARE void
gaiaToFgf (gaiaGeomCollPtr geom, unsigned char **result, int *size,
	   int coord_dims)
{
/* builds the FGF representation for this GEOMETRY */
    int ib;
    int iv;
    double x;
    double y;
    int entities = 0;
    int n_points = 0;
    int n_linestrings = 0;
    int n_polygons = 0;
    int type;
    int n_coords;
    unsigned char *ptr;
    int sz = 0;
    gaiaPointPtr pt;
    gaiaLinestringPtr ln;
    gaiaPolygonPtr pg;
    gaiaRingPtr rng;
    gaiaPointPtr point = NULL;
    gaiaLinestringPtr line = NULL;
    gaiaPolygonPtr polyg = NULL;
    int endian_arch = gaiaEndianArch ();
    gaiaMbrGeometry (geom);
    switch (coord_dims)
      {
      case GAIA_XY:
	  n_coords = 2;
	  break;
      case GAIA_XY_M:
      case GAIA_XY_Z:
	  n_coords = 3;
	  break;
      case GAIA_XY_Z_M:
	  n_coords = 4;
	  break;
      default:
	  n_coords = 0;
	  break;
      }
/* how many entities, and of what kind, do we have ? */
    pt = geom->FirstPoint;
    while (pt)
      {
	  point = pt;
	  entities++;
	  n_points++;
	  pt = pt->Next;
      }
    ln = geom->FirstLinestring;
    while (ln)
      {
	  line = ln;
	  entities++;
	  n_linestrings++;
	  ln = ln->Next;
      }
    pg = geom->FirstPolygon;
    while (pg)
      {
	  polyg = pg;
	  entities++;
	  n_polygons++;
	  pg = pg->Next;
      }
    *size = 0;
    sz = 0;
    *result = NULL;
    if (n_points == 0 && n_polygons == 0 && n_linestrings == 0)
	return;
/* ok, we can determine the geometry class */
    if (n_points == 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOINT)
	      type = GAIA_MULTIPOINT;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_POINT;
      }
    else if (n_points > 1 && n_linestrings == 0 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_MULTIPOINT;
      }
    else if (n_points == 0 && n_linestrings == 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_MULTILINESTRING)
	      type = GAIA_MULTILINESTRING;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_LINESTRING;
      }
    else if (n_points == 0 && n_linestrings > 1 && n_polygons == 0)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_MULTILINESTRING;
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons == 1)
      {
	  if (geom->DeclaredType == GAIA_MULTIPOLYGON)
	      type = GAIA_MULTIPOLYGON;
	  else if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_POLYGON;
      }
    else if (n_points == 0 && n_linestrings == 0 && n_polygons > 1)
      {
	  if (geom->DeclaredType == GAIA_GEOMETRYCOLLECTION)
	      type = GAIA_GEOMETRYCOLLECTION;
	  else
	      type = GAIA_MULTIPOLYGON;
      }
    else
	type = GAIA_GEOMETRYCOLLECTION;
/* and now we compute the size of FGF */
    if (type ==
	GAIA_MULTIPOINT
	|| type ==
	GAIA_MULTILINESTRING
	|| type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	sz += 8;
    point = geom->FirstPoint;
    while (point)
      {
	  sz += (8 + (n_coords * sizeof (double)));	/* the size of each POINT */
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  sz += (12 + ((n_coords * sizeof (double)) * line->Points));	/* # points + [x,y] for each vertex */
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  rng = polyg->Exterior;
	  sz += (16 + ((n_coords * sizeof (double)) * rng->Points));	/* # rings + # points + [x.y] array - exterior ring */
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		sz += (4 + ((n_coords * sizeof (double)) * rng->Points));	/* # points + [x,y] array - interior ring */
	    }
	  polyg = polyg->Next;
      }
    *size = sz;
    ptr = malloc (sz);
    *result = ptr;
/* and finally we build the FGF */
    if (type ==
	GAIA_MULTIPOINT
	|| type ==
	GAIA_MULTILINESTRING
	|| type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
      {
	  gaiaExport32 (ptr, type, GAIA_LITTLE_ENDIAN, endian_arch);	/* Geometry Type */
	  ptr += 4;
	  gaiaExport32 (ptr, entities, GAIA_LITTLE_ENDIAN, endian_arch);	/* it's a collection; # entities */
	  ptr += 4;
      }
    point = geom->FirstPoint;
    while (point)
      {
	  gaiaExport32 (ptr, GAIA_POINT, GAIA_LITTLE_ENDIAN, endian_arch);	/* the CLASS TYPE for this element */
	  ptr += 4;
	  gaiaExport32 (ptr, coord_dims, GAIA_LITTLE_ENDIAN, endian_arch);	/* the CoordDimension */
	  ptr += 4;
	  gaiaExport64 (ptr, point->X, GAIA_LITTLE_ENDIAN, endian_arch);	/* X */
	  ptr += 8;
	  gaiaExport64 (ptr, point->Y, GAIA_LITTLE_ENDIAN, endian_arch);	/* Y */
	  ptr += 8;
	  if (n_coords > 2)
	    {
		/* the third coordinate [Z or M]; defaulting to ZERO */
		gaiaExport64 (ptr, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
		ptr += 8;
	    }
	  if (n_coords > 3)
	    {
		/* the fourth coordinate [ M]; defaulting to ZERO */
		gaiaExport64 (ptr, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
		ptr += 8;
	    }
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  gaiaExport32 (ptr, GAIA_LINESTRING, GAIA_LITTLE_ENDIAN, endian_arch);	/* the CLASS TYPE for this element */
	  ptr += 4;
	  gaiaExport32 (ptr, coord_dims, GAIA_LITTLE_ENDIAN, endian_arch);	/* the CoordDimension */
	  ptr += 4;
	  gaiaExport32 (ptr, line->Points, GAIA_LITTLE_ENDIAN, endian_arch);	/* # points */
	  ptr += 4;
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPoint (line->Coords, iv, &x, &y);
		gaiaExport64 (ptr, x, GAIA_LITTLE_ENDIAN, endian_arch);	/* X */
		ptr += 8;
		gaiaExport64 (ptr, y, GAIA_LITTLE_ENDIAN, endian_arch);	/* Y */
		ptr += 8;
		if (n_coords > 2)
		  {
		      /* the third coordinate [Z or M]; defaulting to ZERO */
		      gaiaExport64 (ptr, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
		      ptr += 8;
		  }
		if (n_coords > 3)
		  {
		      /* the fourth coordinate [ M]; defaulting to ZERO */
		      gaiaExport64 (ptr, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
		      ptr += 8;
		  }
	    }
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  gaiaExport32 (ptr, GAIA_POLYGON, GAIA_LITTLE_ENDIAN, endian_arch);	/* the CLASS TYPE for this element */
	  ptr += 4;
	  gaiaExport32 (ptr, coord_dims, GAIA_LITTLE_ENDIAN, endian_arch);	/* the CoordDimension */
	  ptr += 4;
	  gaiaExport32 (ptr, polyg->NumInteriors + 1, GAIA_LITTLE_ENDIAN, endian_arch);	/* # rings */
	  ptr += 4;
	  rng = polyg->Exterior;
	  gaiaExport32 (ptr, rng->Points, GAIA_LITTLE_ENDIAN, endian_arch);	/* # points - exterior ring */
	  ptr += 4;
	  for (iv = 0; iv < rng->Points; iv++)
	    {
		gaiaGetPoint (rng->Coords, iv, &x, &y);
		gaiaExport64 (ptr, x, GAIA_LITTLE_ENDIAN, endian_arch);	/* X - exterior ring */
		ptr += 8;
		gaiaExport64 (ptr, y, GAIA_LITTLE_ENDIAN, endian_arch);	/* Y - exterior ring */
		ptr += 8;
		if (n_coords > 2)
		  {
		      /* the third coordinate [Z or M]; defaulting to ZERO */
		      gaiaExport64 (ptr, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
		      ptr += 8;
		  }
		if (n_coords > 3)
		  {
		      /* the fourth coordinate [ M]; defaulting to ZERO */
		      gaiaExport64 (ptr, 0.0, GAIA_LITTLE_ENDIAN, endian_arch);
		      ptr += 8;
		  }
	    }
	  for (ib = 0; ib < polyg->NumInteriors; ib++)
	    {
		rng = polyg->Interiors + ib;
		gaiaExport32 (ptr, rng->Points, 1, endian_arch);	/* # points - interior ring */
		ptr += 4;
		for (iv = 0; iv < rng->Points; iv++)
		  {
		      gaiaGetPoint (rng->Coords, iv, &x, &y);
		      gaiaExport64 (ptr, x, GAIA_LITTLE_ENDIAN, endian_arch);	/* X - interior ring */
		      ptr += 8;
		      gaiaExport64 (ptr, y, GAIA_LITTLE_ENDIAN, endian_arch);	/* Y - interior ring */
		      ptr += 8;
		      if (n_coords > 2)
			{
			    /* the third coordinate [Z or M]; defaulting to ZERO */
			    gaiaExport64 (ptr, 0.0, GAIA_LITTLE_ENDIAN,
					  endian_arch);
			    ptr += 8;
			}
		      if (n_coords > 3)
			{
			    /* the fourth coordinate [ M]; defaulting to ZERO */
			    gaiaExport64 (ptr, 0.0, GAIA_LITTLE_ENDIAN,
					  endian_arch);
			    ptr += 8;
			}
		  }
	    }
	  polyg = polyg->Next;
      }
}
/**************** End file: gg_wkb.c **********/


/**************** Begin file: gg_wkt.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <string.h> */

/* #include <spatialite/sqlite3ext.h> */
/* #include <spatialite/gaiageo.h> */

typedef struct gaiaTokenStruct
{
/* linked list of pre-parsed tokens - used in WKT parsing */
    int type;			/* token code */
    double coord;		/* a coordinate [if any] */
    struct gaiaTokenStruct *next;	/* reference to next element in linked list */
} gaiaToken;
typedef gaiaToken *gaiaTokenPtr;

typedef struct gaiaListTokenStruct
{
/* 
 / a block of consecutive WKT tokens that identify a LINESTRING or RING, 
 / in the form: (1 2, 3 4, 5 6, 7 8)
 */
    gaiaTokenPtr first;		/* reference to first element in linked list - has always to be a '(' */
    gaiaTokenPtr last;		/* reference to last element in linked list - has always to be a ')' */
    int points;			/* number of POINTS contained in the linkek list */
    gaiaLinestringPtr line;	/* the LINESTRING builded after successful parsing */
    struct gaiaListTokenStruct *next;	/* points to next element in linked list [if any] */
} gaiaListToken;
typedef gaiaListToken *gaiaListTokenPtr;

typedef struct gaiaMultiListTokenStruct
{
/*
 / a group of the above token lists, that identify a POLYGON or a MULTILINESTRING
 / in the form: ((...),(...),(...))
 */
    gaiaListTokenPtr first;	/* reference to first element in linked list - has always to be a '(' */
    gaiaListTokenPtr last;	/* reference to last element in linked list - has always to be a ')' */
    struct gaiaMultiListTokenStruct *next;	/* points to next element in linked list [if any] */
} gaiaMultiListToken;
typedef gaiaMultiListToken *gaiaMultiListTokenPtr;

typedef struct gaiaMultiMultiListTokenStruct
{
/*
 / a group of the above token multi-lists, that identify a MULTIPOLYGON 
 / in the form: (((...),(...),(...)),((...),(...)),((...)))
 */
    gaiaMultiListTokenPtr first;	/* reference to first element in linked list - has always to be a '(' */
    gaiaMultiListTokenPtr last;	/* reference to last element in linked list - has always to be a ')' */
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
    int type;			/* may be GAIA_POINT, GAIA_LINESTRING or GAIA_POLYGON */
    void *pointer;		/* 
				   / to be casted as *sple_multi_list_token_ptr* if type is GAIA_LINESTRING/ or as *gaiaMultiMultiListTokenPtr* if type is GAIA_POLYGON
				 */
    double x;			/* X,Y are valids only if type is GAIA_POINT */
    double y;
    struct gaiaVarListTokenStruct *next;	/* points to next element in linked list */
} gaiaVarListToken;
typedef gaiaVarListToken *gaiaVarListTokenPtr;

typedef struct gaiaGeomCollListTokenStruct
{
/*
/ a group of lists and multi-lists that identifies a GEOMETRYCOLLECTION 
/ in the form: (ELEM(...),ELEM(...),ELEM(....))
*/
    gaiaVarListTokenPtr first;	/* reference to first element in linked list - has always to be a '(' */
    gaiaVarListTokenPtr last;	/* reference to last element in linked list - has always to be a ')' */
} gaiaGeomCollListToken;
typedef gaiaGeomCollListToken *gaiaGeomCollListTokenPtr;

static char *
gaiaCleanWkt (const unsigned char *old)
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
    len = strlen ((char *) old);
    if (len == 0)
	return NULL;
    buf = malloc (len + 1);
    pn = buf;
    po = old;
    while (*po != '\0')
      {
	  if (*po >= '0' && *po <= '9')
	    {
		*pn = *po;
		pn++;
		space = 0;
	    }
	  else if (*po == '+' || *po == '-')
	    {
		*pn = *po;
		pn++;
		space = 0;
	    }
	  else if ((*po >= 'A' && *po <= 'Z') || (*po >= 'a' && *po <= 'z')
		   || *po == ',' || *po == '.' || *po == '(' || *po == ')')
	    {
		if (pn > buf)
		  {
		      if (*(pn - 1) == ' ')
			  *(pn - 1) = *po;
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
		if (*po == '(')
		    opened++;
		if (*po == ')')
		    closed++;
		space = 1;
	    }
	  else if (*po == ' ' || *po == '\t' || *po == '\n' || *po == '\r')
	    {
		if (!space)
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
    if (opened != closed)
	error = 1;
    *pn = '\0';
    len = strlen (buf);
    if (buf[len - 1] != ')')
	error = 1;
    ok = 0;
    if (!error)
      {
	  if (len > 6 && strncasecmp (buf, "POINT(", 6) == 0)
	      ok = 1;
	  if (len > 11 && strncasecmp (buf, "LINESTRING(", 11) == 0)
	      ok = 1;
	  if (len > 8 && strncasecmp (buf, "POLYGON(", 8) == 0)
	      ok = 1;
	  if (len > 11 && strncasecmp (buf, "MULTIPOINT(", 11) == 0)
	      ok = 1;
	  if (len > 16 && strncasecmp (buf, "MULTILINESTRING(", 16) == 0)
	      ok = 1;
	  if (len > 13 && strncasecmp (buf, "MULTIPOLYGON(", 13) == 0)
	      ok = 1;
	  if (len > 19 && strncasecmp (buf, "GEOMETRYCOLLECTION(", 19) == 0)
	      ok = 1;
	  if (!ok)
	      error = 1;
      }
    if (error)
      {
	  free (buf);
	  return NULL;
      }
    return buf;
}

static void
gaiaFreeListToken (gaiaListTokenPtr p)
{
/* cleans all memory allocations for list token */
    if (!p)
	return;
    if (p->line)
	gaiaFreeLinestring (p->line);
    free (p);
}

static void
gaiaFreeMultiListToken (gaiaMultiListTokenPtr p)
{
/* cleans all memory allocations for multi list token */
    gaiaListTokenPtr pt;
    gaiaListTokenPtr ptn;
    if (!p)
	return;
    pt = p->first;
    while (pt)
      {
	  ptn = pt->next;
	  gaiaFreeListToken (pt);
	  pt = ptn;
      }
    free (p);
}

static void
gaiaFreeMultiMultiListToken (gaiaMultiMultiListTokenPtr p)
{
/* cleans all memory allocations for multi-multi list token */
    gaiaMultiListTokenPtr pt;
    gaiaMultiListTokenPtr ptn;
    if (!p)
	return;
    pt = p->first;
    while (pt)
      {
	  ptn = pt->next;
	  gaiaFreeMultiListToken (pt);
	  pt = ptn;
      }
    free (p);
}

static void
gaiaFreeGeomCollListToken (gaiaGeomCollListTokenPtr p)
{
/* cleans all memory allocations for geocoll list token */
    gaiaVarListTokenPtr pt;
    gaiaVarListTokenPtr ptn;
    if (!p)
	return;
    pt = p->first;
    while (pt)
      {
	  ptn = pt->next;
	  if (pt->type == GAIA_LINESTRING)
	      gaiaFreeListToken ((gaiaListTokenPtr) (pt->pointer));
	  if (pt->type == GAIA_POLYGON)
	      gaiaFreeMultiListToken ((gaiaMultiListTokenPtr) (pt->pointer));
	  pt = ptn;
      }
    free (p);
}

static int
gaiaParseDouble (char *token, double *coord)
{
/* checks if this token is a valid double */
    int i;
    int digits = 0;
    int errs = 0;
    int commas = 0;
    int signs = 0;
    *coord = 0.0;
    for (i = 0; i < (int) strlen (token); i++)
      {
	  if (token[i] == '+' || token[i] == '-')
	      signs++;
	  else if (token[i] == '.')
	      commas++;
	  else if (token[i] >= '0' && token[i] <= '9')
	      digits++;
	  else
	      errs++;
      }
    if (errs > 0)
	return 0;
    if (digits == 0)
	return 0;
    if (commas > 1)
	return 0;
    if (signs > 1)
	return 0;
    if (signs)
      {
	  switch (token[0])
	    {
	    case '-':
	    case '+':
		break;
	    default:
		return 0;
	    };
      }
    *coord = atof (token);
    return 1;
}

static void
gaiaAddToken (char *token, gaiaTokenPtr * first, gaiaTokenPtr * last)
{
/* inserts a token at the end of the linked list */
    double coord;
    gaiaTokenPtr p;
    if (strlen (token) == 0)
	return;
    p = malloc (sizeof (gaiaToken));
    p->type = GAIA_UNKNOWN;
    p->coord = 0.0;
    if (strcasecmp (token, "POINT") == 0)
	p->type = GAIA_POINT;
    if (strcasecmp (token, "LINESTRING") == 0)
	p->type = GAIA_LINESTRING;
    if (strcasecmp (token, "POLYGON") == 0)
	p->type = GAIA_POLYGON;
    if (strcasecmp (token, "MULTIPOINT") == 0)
	p->type = GAIA_MULTIPOINT;
    if (strcasecmp (token, "MULTILINESTRING") == 0)
	p->type = GAIA_MULTILINESTRING;
    if (strcasecmp (token, "MULTIPOLYGON") == 0)
	p->type = GAIA_MULTIPOLYGON;
    if (strcasecmp (token, "GEOMETRYCOLLECTION") == 0)
	p->type = GAIA_GEOMETRYCOLLECTION;
    if (strcmp (token, "(") == 0)
	p->type = GAIA_OPENED;
    if (strcmp (token, ")") == 0)
	p->type = GAIA_CLOSED;
    if (strcmp (token, ",") == 0)
	p->type = GAIA_COMMA;
    if (strcmp (token, " ") == 0)
	p->type = GAIA_SPACE;
    if (p->type == GAIA_UNKNOWN)
      {
	  if (gaiaParseDouble (token, &coord))
	    {
		p->type = GAIA_COORDINATE;
		p->coord = coord;
	    }
      }
    p->next = NULL;
    if (*first == NULL)
	*first = p;
    if (*last != NULL)
	(*last)->next = p;
    *last = p;
}

static gaiaListTokenPtr
gaiaBuildListToken (gaiaTokenPtr first, gaiaTokenPtr last)
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
    while (pt != NULL)
      {
	  /* check if this one is a valid list of POINTS */
	  if (i == 0)
	    {
		if (pt->type != GAIA_OPENED)
		    err = 1;
	    }
	  else if (pt == last)
	    {
		if (pt->type != GAIA_CLOSED)
		    err = 1;
	    }
	  else
	    {
		if (ip == 0)
		  {
		      if (pt->type != GAIA_COORDINATE)
			  err = 1;
		      else
			  nx++;
		  }
		else if (ip == 1)
		  {
		      if (pt->type != GAIA_SPACE)
			  err = 1;
		  }
		else if (ip == 2)
		  {
		      if (pt->type != GAIA_COORDINATE)
			  err = 1;
		      else
			  ny++;
		  }
		else if (ip == 3)
		  {
		      if (pt->type != GAIA_COMMA)
			  err = 1;
		  }
		ip++;
		if (ip > 3)
		    ip = 0;
	    }
	  i++;
	  pt = pt->next;
	  if (pt == last)
	      break;
      }
    if (nx != ny)
	err = 1;
    if (nx < 1)
	err = 1;
    if (err)
	return NULL;
/* ok, there is no erorr. finally we can build the POINTS list */
    list = malloc (sizeof (gaiaListToken));
    list->points = nx;
    list->line = gaiaAllocLinestring (nx);
    list->next = NULL;
    iv = 0;
    ip = 0;
    i = 0;
    pt = first;
    while (pt != NULL)
      {
	  /* sets coords for all POINTS */
	  if (i == 0)
	      ;
	  else if (pt == last)
	      ;
	  else
	    {
		if (ip == 0)
		    x = pt->coord;
		else if (ip == 2)
		  {
		      y = pt->coord;
		      gaiaSetPoint (list->line->Coords, iv, x, y);
		      iv++;
		  }
		ip++;
		if (ip > 3)
		    ip = 0;
	    }
	  i++;
	  pt = pt->next;
	  if (pt == last)
	      break;
      }
    return list;
}

static gaiaMultiListTokenPtr
gaiaBuildMultiListToken (gaiaTokenPtr first, gaiaTokenPtr last)
{
/* builds a multi list of tokens representing an array of elementar lists in the form ((...),(....),(...)), as required by MULTILINESTRING and POLYGON */
    gaiaMultiListTokenPtr multi_list = NULL;
    gaiaTokenPtr pt;
    gaiaTokenPtr p_first = NULL;
    gaiaListTokenPtr list;
    int opened = 0;
    pt = first;
    while (pt != NULL)
      {
	  /* identifies the sub-lists contained in this multi list */
	  if (pt->type == GAIA_OPENED)
	    {
		opened++;
		if (opened == 2)
		    p_first = pt;
	    }
	  if (pt->type == GAIA_CLOSED)
	    {
		if (p_first)
		  {
		      list = gaiaBuildListToken (p_first, pt);
		      if (!multi_list)
			{
			    multi_list = malloc (sizeof (gaiaMultiListToken));
			    multi_list->first = NULL;
			    multi_list->last = NULL;
			    multi_list->next = NULL;
			}
		      if (multi_list->first == NULL)
			  multi_list->first = list;
		      if (multi_list->last != NULL)
			  multi_list->last->next = list;
		      multi_list->last = list;
		      p_first = NULL;
		  }
		opened--;
	    }
	  pt = pt->next;
	  if (pt == last)
	      break;
      }
    return multi_list;
}

static gaiaMultiMultiListTokenPtr
gaiaBuildMultiMultiListToken (gaiaTokenPtr first, gaiaTokenPtr last)
{
/* builds a multi list of tokens representing an array of complex lists in the form (((...),(....),(...)),((...),(...)),((...))), as required by MULTIPOLYGON */
    gaiaMultiMultiListTokenPtr multi_multi_list = NULL;
    gaiaTokenPtr pt;
    gaiaTokenPtr p_first = NULL;
    gaiaMultiListTokenPtr multi_list;
    int opened = 0;
    pt = first;
    while (pt != NULL)
      {
	  /* identifies the sub-lists contained in this multi list */
	  if (pt->type == GAIA_OPENED)
	    {
		opened++;
		if (opened == 2)
		    p_first = pt;
	    }
	  if (pt->type == GAIA_CLOSED)
	    {
		if (p_first && opened == 2)
		  {
		      multi_list = gaiaBuildMultiListToken (p_first, pt);
		      if (!multi_multi_list)
			{
			    multi_multi_list =
				malloc (sizeof (gaiaMultiMultiListToken));
			    multi_multi_list->first = NULL;
			    multi_multi_list->last = NULL;
			}
		      if (multi_multi_list->first == NULL)
			  multi_multi_list->first = multi_list;
		      if (multi_multi_list->last != NULL)
			  multi_multi_list->last->next = multi_list;
		      multi_multi_list->last = multi_list;
		      p_first = NULL;
		  }
		opened--;
	    }
	  pt = pt->next;
	  if (pt == last)
	      break;
      }
    return multi_multi_list;
}

static gaiaGeomCollListTokenPtr
gaiaBuildGeomCollListToken (gaiaTokenPtr first, gaiaTokenPtr last)
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
    while (pt != NULL)
      {
	  /* identifies the sub-lists contained in this complex list */
	  if (pt->type == GAIA_POINT)
	    {
		/* parsing a POINT list */
		err = 0;
		i = 0;
		pt2 = pt->next;
		while (pt2 != NULL)
		  {
		      /* check if this one is a valid POINT */
		      switch (i)
			{
			case 0:
			    if (pt2->type != GAIA_OPENED)
				err = 1;
			    break;
			case 1:
			    if (pt2->type != GAIA_COORDINATE)
				err = 1;
			    else
				x = pt2->coord;
			    break;
			case 2:
			    if (pt2->type != GAIA_SPACE)
				err = 1;
			    break;
			case 3:
			    if (pt2->type != GAIA_COORDINATE)
				err = 1;
			    else
				y = pt2->coord;
			    break;
			case 4:
			    if (pt2->type != GAIA_CLOSED)
				err = 1;
			    break;
			};
		      i++;
		      if (i > 4)
			  break;
		      pt2 = pt2->next;
		  }
		if (err)
		    goto error;
		var_list = malloc (sizeof (gaiaVarListToken));
		var_list->type = GAIA_POINT;
		var_list->x = x;
		var_list->y = y;
		var_list->next = NULL;
		if (!geocoll_list)
		  {
		      geocoll_list = malloc (sizeof (gaiaGeomCollListToken));
		      geocoll_list->first = NULL;
		      geocoll_list->last = NULL;
		  }
		if (geocoll_list->first == NULL)
		    geocoll_list->first = var_list;
		if (geocoll_list->last != NULL)
		    geocoll_list->last->next = var_list;
		geocoll_list->last = var_list;
	    }
	  else if (pt->type == GAIA_LINESTRING)
	    {
		/* parsing a LINESTRING list */
		p_first = NULL;
		pt2 = pt->next;
		while (pt2 != NULL)
		  {
		      if (pt2->type == GAIA_OPENED)
			  p_first = pt2;
		      if (pt2->type == GAIA_CLOSED)
			{
			    list = gaiaBuildListToken (p_first, pt2);
			    if (list)
			      {
				  var_list = malloc (sizeof (gaiaVarListToken));
				  var_list->type = GAIA_LINESTRING;
				  var_list->pointer = list;
				  var_list->next = NULL;
				  if (!geocoll_list)
				    {
					geocoll_list =
					    malloc (sizeof
						    (gaiaGeomCollListToken));
					geocoll_list->first = NULL;
					geocoll_list->last = NULL;
				    }
				  if (geocoll_list->first == NULL)
				      geocoll_list->first = var_list;
				  if (geocoll_list->last != NULL)
				      geocoll_list->last->next = var_list;
				  geocoll_list->last = var_list;
				  break;
			      }
			    else
				goto error;
			}
		      pt2 = pt2->next;
		      if (pt2 == last)
			  break;
		  }
	    }
	  else if (pt->type == GAIA_POLYGON)
	    {
		/* parsing a POLYGON list */
		opened = 0;
		p_first = NULL;
		pt2 = pt->next;
		while (pt2 != NULL)
		  {
		      if (pt2->type == GAIA_OPENED)
			{
			    opened++;
			    if (opened == 1)
				p_first = pt2;
			}
		      if (pt2->type == GAIA_CLOSED)
			{
			    if (p_first && opened == 1)
			      {
				  multi_list =
				      gaiaBuildMultiListToken (p_first, pt2);
				  if (multi_list)
				    {
					var_list =
					    malloc (sizeof (gaiaVarListToken));
					var_list->type = GAIA_POLYGON;
					var_list->pointer = multi_list;
					var_list->next = NULL;
					if (!geocoll_list)
					  {
					      geocoll_list =
						  malloc (sizeof
							  (gaiaGeomCollListToken));
					      geocoll_list->first = NULL;
					      geocoll_list->last = NULL;
					  }
					if (geocoll_list->first == NULL)
					    geocoll_list->first = var_list;
					if (geocoll_list->last != NULL)
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
		      if (pt2 == last)
			  break;
		  }
	    }
	  pt = pt->next;
      }
    return geocoll_list;
  error:
    gaiaFreeGeomCollListToken (geocoll_list);
    return NULL;
}

static gaiaPointPtr
gaiaBuildPoint (gaiaTokenPtr first)
{
/* builds a POINT, if this token's list contains a valid POINT */
    gaiaPointPtr point = NULL;
    gaiaTokenPtr pt = first;
    int i = 0;
    int err = 0;
    double x = 0.0;
    double y = 0.0;
    while (pt != NULL)
      {
	  /* check if this one is a valid POINT */
	  switch (i)
	    {
	    case 0:
		if (pt->type != GAIA_OPENED)
		    err = 1;
		break;
	    case 1:
		if (pt->type != GAIA_COORDINATE)
		    err = 1;
		else
		    x = pt->coord;
		break;
	    case 2:
		if (pt->type != GAIA_SPACE)
		    err = 1;
		break;
	    case 3:
		if (pt->type != GAIA_COORDINATE)
		    err = 1;
		else
		    y = pt->coord;
		break;
	    case 4:
		if (pt->type != GAIA_CLOSED)
		    err = 1;
		break;
	    default:
		err = 1;
		break;
	    };
	  i++;
	  pt = pt->next;
      }
    if (err)
	return NULL;
    point = gaiaAllocPoint (x, y);
    return point;
}

static gaiaGeomCollPtr
gaiaGeometryFromPoint (gaiaPointPtr point)
{
/* builds a GEOMETRY containing a POINT */
    gaiaGeomCollPtr geom = NULL;
    geom = gaiaAllocGeomColl ();
    geom->DeclaredType = GAIA_POINT;
    gaiaAddPointToGeomColl (geom, point->X, point->Y);
    gaiaFreePoint (point);
    return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromLinestring (gaiaLinestringPtr line)
{
/* builds a GEOMETRY containing a LINESTRING */
    gaiaGeomCollPtr geom = NULL;
    gaiaLinestringPtr line2;
    int iv;
    double x;
    double y;
    geom = gaiaAllocGeomColl ();
    geom->DeclaredType = GAIA_LINESTRING;
    line2 = gaiaAddLinestringToGeomColl (geom, line->Points);
    for (iv = 0; iv < line2->Points; iv++)
      {
	  /* sets the POINTS for the exterior ring */
	  gaiaGetPoint (line->Coords, iv, &x, &y);
	  gaiaSetPoint (line2->Coords, iv, x, y);
      }
    gaiaFreeLinestring (line);
    return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromPolygon (gaiaMultiListTokenPtr polygon)
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
    while (pt != NULL)
      {
	  /* counts how many rings are in the list */
	  borders++;
	  pt = pt->next;
      }
    if (!borders)
	return NULL;
    geom = gaiaAllocGeomColl ();
    geom->DeclaredType = GAIA_POLYGON;
/* builds the polygon */
    line = polygon->first->line;
    pg = gaiaAddPolygonToGeomColl (geom, line->Points, borders - 1);
    ring = pg->Exterior;
    for (iv = 0; iv < ring->Points; iv++)
      {
	  /* sets the POINTS for the exterior ring */
	  gaiaGetPoint (line->Coords, iv, &x, &y);
	  gaiaSetPoint (ring->Coords, iv, x, y);
      }
    ib = 0;
    pt = polygon->first->next;
    while (pt != NULL)
      {
	  /* builds the interior rings [if any] */
	  line = pt->line;
	  ring = gaiaAddInteriorRing (pg, ib, line->Points);
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* sets the POINTS for some interior ring */
		gaiaGetPoint (line->Coords, iv, &x, &y);
		gaiaSetPoint (ring->Coords, iv, x, y);
	    }
	  ib++;
	  pt = pt->next;
      }
    return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromMPoint (gaiaLinestringPtr mpoint)
{
/* builds a GEOMETRY containing a MULTIPOINT */
    int ie;
    double x;
    double y;
    gaiaGeomCollPtr geom = NULL;
    geom = gaiaAllocGeomColl ();
    geom->DeclaredType = GAIA_MULTIPOINT;
    for (ie = 0; ie < mpoint->Points; ie++)
      {
	  gaiaGetPoint (mpoint->Coords, ie, &x, &y);
	  gaiaAddPointToGeomColl (geom, x, y);
      }
    return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromMLine (gaiaMultiListTokenPtr mline)
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
    while (pt != NULL)
      {
/* counts how many linestrings are in the list */
	  lines++;
	  pt = pt->next;
      }
    if (!lines)
	return NULL;
    geom = gaiaAllocGeomColl ();
    geom->DeclaredType = GAIA_MULTILINESTRING;
    pt = mline->first;
    while (pt != NULL)
      {
	  /* creates and initializes one linestring for each iteration */
	  line = pt->line;
	  line2 = gaiaAddLinestringToGeomColl (geom, line->Points);
	  for (iv = 0; iv < line->Points; iv++)
	    {
		gaiaGetPoint (line->Coords, iv, &x, &y);
		gaiaSetPoint (line2->Coords, iv, x, y);
	    }
	  pt = pt->next;
      }
    return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromMPoly (gaiaMultiMultiListTokenPtr mpoly)
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
    while (multi != NULL)
      {
	  /* counts how many polygons are in the list */
	  entities++;
	  multi = multi->next;
      }
    if (!entities)
	return NULL;
/* allocates and initializes the geometry to be returned */
    geom = gaiaAllocGeomColl ();
    geom->DeclaredType = GAIA_MULTIPOLYGON;
    multi = mpoly->first;
    while (multi != NULL)
      {
	  borders = 0;
	  pt = multi->first;
	  while (pt != NULL)
	    {
		/* counts how many rings are in the list */
		borders++;
		pt = pt->next;
	    }
	  /* builds one polygon */
	  line = multi->first->line;
	  pg = gaiaAddPolygonToGeomColl (geom, line->Points, borders - 1);
	  ring = pg->Exterior;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		/* sets the POINTS for the exterior ring */
		gaiaGetPoint (line->Coords, iv, &x, &y);
		gaiaSetPoint (ring->Coords, iv, x, y);
	    }
	  ib = 0;
	  pt = multi->first->next;
	  while (pt != NULL)
	    {
		/* builds the interior rings [if any] */
		line = pt->line;
		ring = gaiaAddInteriorRing (pg, ib, line->Points);
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      /* sets the POINTS for the exterior ring */
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		      gaiaSetPoint (ring->Coords, iv, x, y);
		  }
		ib++;
		pt = pt->next;
	    }
	  multi = multi->next;
      }
    return geom;
}

static gaiaGeomCollPtr
gaiaGeometryFromGeomColl (gaiaGeomCollListTokenPtr geocoll)
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
    while (multi != NULL)
      {
	  /* counts how many polygons are in the list */
	  entities++;
	  multi = multi->next;
      }
    if (!entities)
	return NULL;
/* allocates and initializes the geometry to be returned */
    geom = gaiaAllocGeomColl ();
    geom->DeclaredType = GAIA_GEOMETRYCOLLECTION;
    multi = geocoll->first;
    while (multi != NULL)
      {
	  switch (multi->type)
	    {
	    case GAIA_POINT:
		gaiaAddPointToGeomColl (geom, multi->x, multi->y);
		break;
	    case GAIA_LINESTRING:
		linestring = (gaiaListTokenPtr) (multi->pointer);
		line = linestring->line;
		line2 = gaiaAddLinestringToGeomColl (geom, line->Points);
		for (iv = 0; iv < line2->Points; iv++)
		  {
		      /* sets the POINTS for the LINESTRING */
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		      gaiaSetPoint (line2->Coords, iv, x, y);
		  }
		break;
	    case GAIA_POLYGON:
		polyg = multi->pointer;
		borders = 0;
		pt = polyg->first;
		while (pt != NULL)
		  {
		      /* counts how many rings are in the list */
		      borders++;
		      pt = pt->next;
		  }
		/* builds one polygon */
		line = polyg->first->line;
		pg = gaiaAddPolygonToGeomColl (geom, line->Points, borders - 1);
		ring = pg->Exterior;
		for (iv = 0; iv < ring->Points; iv++)
		  {
		      /* sets the POINTS for the exterior ring */
		      gaiaGetPoint (line->Coords, iv, &x, &y);
		      gaiaSetPoint (ring->Coords, iv, x, y);
		  }
		ib = 0;
		pt = polyg->first->next;
		while (pt != NULL)
		  {
		      /* builds the interior rings [if any] */
		      line = pt->line;
		      ring = gaiaAddInteriorRing (pg, ib, line->Points);
		      for (iv = 0; iv < ring->Points; iv++)
			{
			    /* sets the POINTS for the exterior ring */
			    gaiaGetPoint (line->Coords, iv, &x, &y);
			    gaiaSetPoint (ring->Coords, iv, x, y);
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
gaiaParseWkt (const unsigned char *dirty_buffer, short type)
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
    buffer = gaiaCleanWkt (dirty_buffer);
    if (!buffer)
	return NULL;
    p = buffer;
    while (*p != '\0')
      {
	  /* breaking the WKT string into tokens */
	  if (*p == '(')
	    {
		*po = '\0';
		gaiaAddToken (dummy, &first, &last);
		gaiaAddToken ("(", &first, &last);
		po = dummy;
		p++;
		continue;
	    }
	  if (*p == ')')
	    {
		*po = '\0';
		gaiaAddToken (dummy, &first, &last);
		gaiaAddToken (")", &first, &last);
		po = dummy;
		p++;
		continue;
	    }
	  if (*p == ' ')
	    {
		*po = '\0';
		gaiaAddToken (dummy, &first, &last);
		gaiaAddToken (" ", &first, &last);
		po = dummy;
		p++;
		continue;
	    }
	  if (*p == ',')
	    {
		*po = '\0';
		gaiaAddToken (dummy, &first, &last);
		gaiaAddToken (",", &first, &last);
		po = dummy;
		p++;
		continue;
	    }
	  *po++ = *p++;
      }
    if (first == NULL)
	goto err;
    max_opened = 0;
    opened = 0;
    closed = 0;
    pt = first;
    while (pt != NULL)
      {
	  /* checks for absence of serious errors */
	  if (pt->type == GAIA_UNKNOWN)
	      goto err;
	  if (pt->type == GAIA_OPENED)
	    {
		opened++;
		if (opened > 3)
		    goto err;
		if (opened > max_opened)
		    max_opened = opened;
	    }
	  if (pt->type == GAIA_CLOSED)
	    {
		closed++;
		if (closed > opened)
		    goto err;
		opened--;
		closed--;
	    }
	  pt = pt->next;
      }
    if (opened == 0 && closed == 0)
	;
    else
	goto err;
    if (type < 0)
	;			/* no restrinction about GEOMETRY CLASS TYPE */
    else
      {
	  if (first->type != type)
	      goto err;		/* invalid CLASS TYPE for request */
      }
/* preparing the organized token-list structures */
    switch (first->type)
      {
      case GAIA_POINT:
	  if (max_opened != 1)
	      goto err;
	  break;
      case GAIA_LINESTRING:
	  if (max_opened != 1)
	      goto err;
	  list_token = gaiaBuildListToken (first->next, last);
	  break;
      case GAIA_POLYGON:
	  if (max_opened != 2)
	      goto err;
	  multi_list_token = gaiaBuildMultiListToken (first->next, last);
	  break;
      case GAIA_MULTIPOINT:
	  if (max_opened != 1)
	      goto err;
	  list_token = gaiaBuildListToken (first->next, last);
	  break;
      case GAIA_MULTILINESTRING:
	  if (max_opened != 2)
	      goto err;
	  multi_list_token = gaiaBuildMultiListToken (first->next, last);
	  break;
      case GAIA_MULTIPOLYGON:
	  if (max_opened != 3)
	      goto err;
	  multi_multi_list_token =
	      gaiaBuildMultiMultiListToken (first->next, last);
	  break;
      case GAIA_GEOMETRYCOLLECTION:
	  if (max_opened == 2 || max_opened == 3)
	      ;
	  else
	      goto err;
	  geocoll_list_token = gaiaBuildGeomCollListToken (first->next, last);
	  break;
      }
    switch (first->type)
      {
      case GAIA_POINT:
	  point = gaiaBuildPoint (first->next);
	  if (point)
	      geo = gaiaGeometryFromPoint (point);
	  break;
      case GAIA_LINESTRING:
	  line = list_token->line;
	  if (line)
	      geo = gaiaGeometryFromLinestring (line);
	  list_token->line = NULL;
	  break;
      case GAIA_POLYGON:
	  if (multi_list_token)
	      geo = gaiaGeometryFromPolygon (multi_list_token);
	  break;
      case GAIA_MULTIPOINT:
	  line = list_token->line;
	  if (line)
	      geo = gaiaGeometryFromMPoint (line);
	  list_token->line = NULL;
	  break;
      case GAIA_MULTILINESTRING:
	  if (multi_list_token)
	      geo = gaiaGeometryFromMLine (multi_list_token);
	  break;
      case GAIA_MULTIPOLYGON:
	  if (multi_multi_list_token)
	      geo = gaiaGeometryFromMPoly (multi_multi_list_token);
	  break;
      case GAIA_GEOMETRYCOLLECTION:
	  if (geocoll_list_token)
	      geo = gaiaGeometryFromGeomColl (geocoll_list_token);
	  break;
      }
    if (buffer)
	free (buffer);
    gaiaFreeListToken (list_token);
    gaiaFreeMultiListToken (multi_list_token);
    gaiaFreeMultiMultiListToken (multi_multi_list_token);
    gaiaFreeGeomCollListToken (geocoll_list_token);
    pt = first;
    while (pt != NULL)
      {
	  /* cleans the token's list */
	  ptn = pt->next;
	  free (pt);
	  pt = ptn;
      }
    gaiaMbrGeometry (geo);
    return geo;
  err:
    if (buffer)
	free (buffer);
    gaiaFreeListToken (list_token);
    gaiaFreeMultiListToken (multi_list_token);
    gaiaFreeMultiMultiListToken (multi_multi_list_token);
    gaiaFreeGeomCollListToken (geocoll_list_token);
    pt = first;
    while (pt != NULL)
      {
	  /* cleans the token's list */
	  ptn = pt->next;
	  free (pt);
	  pt = ptn;
      }
    return NULL;
}

static void
gaiaOutClean (char *buffer)
{
/* cleans unneeded trailing zeros */
    int i;
    for (i = strlen (buffer) - 1; i > 0; i--)
      {
	  if (buffer[i] == '0')
	      buffer[i] = '\0';
	  else
	      break;
      }
    if (buffer[i] == '.')
	buffer[i] = '\0';
}

static void
gaiaOutCheckBuffer (char **buffer, int *size)
{
/* checks if the receiving buffer has enough free room, and in case reallocates it */
    char *old = *buffer;
    int len = strlen (*buffer);
    if ((*size - len) < 256)
      {
	  *size += 4096;
	  *buffer = realloc (old, *size);
      }
}

static void
gaiaOutText (char *text, char **buffer, int *size)
{
/* formats a WKT generic text */
    gaiaOutCheckBuffer (buffer, size);
    strcat (*buffer, text);
}

static void
gaiaOutPoint (gaiaPointPtr point, char **buffer, int *size)
{
/* formats a WKT POINT */
    char buf_x[128];
    char buf_y[128];
    char buf[256];
    gaiaOutCheckBuffer (buffer, size);
    sprintf (buf_x, "%1.6lf", point->X);
    gaiaOutClean (buf_x);
    sprintf (buf_y, "%1.6lf", point->Y);
    gaiaOutClean (buf_y);
    sprintf (buf, "%s %s", buf_x, buf_y);
    strcat (*buffer, buf);
}

static void
gaiaOutLinestring (gaiaLinestringPtr line, char **buffer, int *size)
{
/* formats a WKT LINESTRING */
    char buf_x[128];
    char buf_y[128];
    char buf[256];
    double x;
    double y;
    int iv;
    for (iv = 0; iv < line->Points; iv++)
      {
	  gaiaGetPoint (line->Coords, iv, &x, &y);
	  gaiaOutCheckBuffer (buffer, size);
	  sprintf (buf_x, "%1.6lf", x);
	  gaiaOutClean (buf_x);
	  sprintf (buf_y, "%1.6lf", y);
	  gaiaOutClean (buf_y);
	  if (iv > 0)
	      sprintf (buf, ", %s %s", buf_x, buf_y);
	  else
	      sprintf (buf, "%s %s", buf_x, buf_y);
	  strcat (*buffer, buf);
      }
}

static void
gaiaOutPolygon (gaiaPolygonPtr polyg, char **buffer, int *size)
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
    for (iv = 0; iv < ring->Points; iv++)
      {
	  gaiaGetPoint (ring->Coords, iv, &x, &y);
	  gaiaOutCheckBuffer (buffer, size);
	  sprintf (buf_x, "%1.6lf", x);
	  gaiaOutClean (buf_x);
	  sprintf (buf_y, "%1.6lf", y);
	  gaiaOutClean (buf_y);
	  if (iv == 0)
	      sprintf (buf, "(%s %s", buf_x, buf_y);
	  else if (iv == (ring->Points - 1))
	      sprintf (buf, ", %s %s)", buf_x, buf_y);
	  else
	      sprintf (buf, ", %s %s", buf_x, buf_y);
	  strcat (*buffer, buf);
      }
    for (ib = 0; ib < polyg->NumInteriors; ib++)
      {
	  ring = polyg->Interiors + ib;
	  for (iv = 0; iv < ring->Points; iv++)
	    {
		gaiaGetPoint (ring->Coords, iv, &x, &y);
		gaiaOutCheckBuffer (buffer, size);
		sprintf (buf_x, "%1.6lf", x);
		gaiaOutClean (buf_x);
		sprintf (buf_y, "%1.6lf", y);
		gaiaOutClean (buf_y);
		if (iv == 0)
		    sprintf (buf, ", (%s %s", buf_x, buf_y);
		else if (iv == (ring->Points - 1))
		    sprintf (buf, ", %s %s)", buf_x, buf_y);
		else
		    sprintf (buf, ", %s %s", buf_x, buf_y);
		strcat (*buffer, buf);
	    }
      }
}

GAIAGEO_DECLARE void
gaiaOutWkt (gaiaGeomCollPtr geom, char **result)
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
    if (!geom)
      {
	  *result = NULL;
	  return;
      }
    *result = malloc (txt_size);
    memset (*result, '\0', txt_size);
    point = geom->FirstPoint;
    while (point)
      {
	  /* counting how many POINTs are there */
	  pts++;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* counting how many LINESTRINGs are there */
	  lns++;
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* counting how many POLYGONs are there */
	  pgs++;
	  polyg = polyg->Next;
      }
    if ((pts + lns +
	 pgs) == 1
	&& (geom->
	    DeclaredType
	    ==
	    GAIA_POINT
	    ||
	    geom->
	    DeclaredType
	    == GAIA_LINESTRING || geom->DeclaredType == GAIA_POLYGON))
      {
	  /* we have only one elementary geometry */
	  point = geom->FirstPoint;
	  while (point)
	    {
		/* processing POINT */
		strcpy (*result, "POINT(");
		gaiaOutPoint (point, result, &txt_size);
		gaiaOutText (")", result, &txt_size);
		point = point->Next;
	    }
	  line = geom->FirstLinestring;
	  while (line)
	    {
		/* processing LINESTRING */
		strcpy (*result, "LINESTRING(");
		gaiaOutLinestring (line, result, &txt_size);
		gaiaOutText (")", result, &txt_size);
		line = line->Next;
	    }
	  polyg = geom->FirstPolygon;
	  while (polyg)
	    {
		/* counting how many POLYGON */
		strcpy (*result, "POLYGON(");
		gaiaOutPolygon (polyg, result, &txt_size);
		gaiaOutText (")", result, &txt_size);
		polyg = polyg->Next;
	    }
      }
    else
      {
	  /* we have some kind of complex geometry */
	  if (pts > 0 && lns == 0 && pgs == 0
	      && geom->DeclaredType == GAIA_MULTIPOINT)
	    {
		/* this one is a MULTIPOINT */
		strcpy (*result, "MULTIPOINT(");
		point = geom->FirstPoint;
		while (point)
		  {
		      /* processing POINTs */
		      if (point != geom->FirstPoint)
			  gaiaOutText (", ", result, &txt_size);
		      gaiaOutPoint (point, result, &txt_size);
		      point = point->Next;
		  }
		gaiaOutText (")", result, &txt_size);
	    }
	  else if (pts == 0 && lns > 0 && pgs == 0
		   && geom->DeclaredType == GAIA_MULTILINESTRING)
	    {
		/* this one is a MULTILINESTRING */
		strcpy (*result, "MULTILINESTRING(");
		line = geom->FirstLinestring;
		while (line)
		  {
		      /* processing LINESTRINGs */
		      if (line != geom->FirstLinestring)
			  gaiaOutText (", (", result, &txt_size);
		      else
			  gaiaOutText ("(", result, &txt_size);
		      gaiaOutLinestring (line, result, &txt_size);
		      gaiaOutText (")", result, &txt_size);
		      line = line->Next;
		  }
		gaiaOutText (")", result, &txt_size);
	    }
	  else if (pts == 0 && lns == 0 && pgs > 0
		   && geom->DeclaredType == GAIA_MULTIPOLYGON)
	    {
		/* this one is a MULTIPOLYGON */
		strcpy (*result, "MULTIPOLYGON(");
		polyg = geom->FirstPolygon;
		while (polyg)
		  {
		      /* processing POLYGONs */
		      if (polyg != geom->FirstPolygon)
			  gaiaOutText (", (", result, &txt_size);
		      else
			  gaiaOutText ("(", result, &txt_size);
		      gaiaOutPolygon (polyg, result, &txt_size);
		      gaiaOutText (")", result, &txt_size);
		      polyg = polyg->Next;
		  }
		gaiaOutText (")", result, &txt_size);
	    }
	  else
	    {
		/* this one is a GEOMETRYCOLLECTION */
		int ie = 0;
		strcpy (*result, "GEOMETRYCOLLECTION(");
		point = geom->FirstPoint;
		while (point)
		  {
		      /* processing POINTs */
		      if (ie > 0)
			  gaiaOutText (", ", result, &txt_size);
		      ie++;
		      strcat (*result, "POINT(");
		      gaiaOutPoint (point, result, &txt_size);
		      gaiaOutText (")", result, &txt_size);
		      point = point->Next;
		  }
		line = geom->FirstLinestring;
		while (line)
		  {
		      /* processing LINESTRINGs */
		      if (ie > 0)
			  gaiaOutText (", ", result, &txt_size);
		      ie++;
		      strcat (*result, "LINESTRING(");
		      gaiaOutLinestring (line, result, &txt_size);
		      gaiaOutText (")", result, &txt_size);
		      line = line->Next;
		  }
		polyg = geom->FirstPolygon;
		while (polyg)
		  {
		      /* processing POLYGONs */
		      if (ie > 0)
			  gaiaOutText (", ", result, &txt_size);
		      ie++;
		      strcat (*result, "POLYGON(");
		      gaiaOutPolygon (polyg, result, &txt_size);
		      gaiaOutText (")", result, &txt_size);
		      polyg = polyg->Next;
		  }
		gaiaOutText (")", result, &txt_size);
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
SvgCoords (gaiaPointPtr point, char **buffer, int *size, int relative,
	   int precision)
{
/* formats POINT as SVG-attributes x,y */
    char buf_x[128];
    char buf_y[128];
    char buf[256];
    gaiaOutCheckBuffer (buffer, size);
    sprintf (buf_x, "%.*f", precision, point->X);
    gaiaOutClean (buf_x);
    sprintf (buf_y, "%.*f", precision, point->Y * -1);
    gaiaOutClean (buf_y);
    sprintf (buf, "x=\"%s\" y=\"%s\"", buf_x, buf_y);
    strcat (*buffer, buf);
}

static void
SvgCircle (gaiaPointPtr point, char **buffer, int *size, int relative,
	   int precision)
{
/* formats POINT as SVG-attributes cx,cy */
    char buf_x[128];
    char buf_y[128];
    char buf[256];
    gaiaOutCheckBuffer (buffer, size);
    sprintf (buf_x, "%.*f", precision, point->X);
    gaiaOutClean (buf_x);
    sprintf (buf_y, "%.*f", precision, point->Y * -1);
    gaiaOutClean (buf_y);
    sprintf (buf, "cx=\"%s\" cy=\"%s\"", buf_x, buf_y);
    strcat (*buffer, buf);
}

static void
SvgPathRelative (int points, double *coords, char **buffer, int *size,
		 int relative, int precision, int closePath)
{
/* formats LINESTRING as SVG-path d-attribute with relative coordinate moves */
    char buf_x[128];
    char buf_y[128];
    char buf[256];
    double x;
    double y;
    double lastX = 0.0;
    double lastY = 0.0;
    int iv;
    for (iv = 0; iv < points; iv++)
      {
	  gaiaGetPoint (coords, iv, &x, &y);
	  gaiaOutCheckBuffer (buffer, size);

	  sprintf (buf_x, "%.*f", precision, x - lastX);
	  gaiaOutClean (buf_x);
	  sprintf (buf_y, "%.*f", precision, lastY - y);
	  gaiaOutClean (buf_y);

	  if (iv == 0)
	    {
		sprintf (buf, "M %s %s l ", buf_x, buf_y);
	    }
	  else
	    {
		sprintf (buf, "%s %s ", buf_x, buf_y);
	    }
	  lastX = x;
	  lastY = y;
	  if (iv == points - 1 && closePath == 1)
	      sprintf (buf, "z ");
	  strcat (*buffer, buf);
      }
}

static void
SvgPathAbsolute (int points, double *coords, char **buffer, int *size,
		 int relative, int precision, int closePath)
{
/* formats LINESTRING as SVG-path d-attribute with relative coordinate moves */
    char buf_x[128];
    char buf_y[128];
    char buf[256];
    double x;
    double y;
    int iv;
    for (iv = 0; iv < points; iv++)
      {
	  gaiaGetPoint (coords, iv, &x, &y);
	  gaiaOutCheckBuffer (buffer, size);
	  sprintf (buf_x, "%.*f", precision, x);
	  gaiaOutClean (buf_x);
	  sprintf (buf_y, "%.*f", precision, y * -1);
	  gaiaOutClean (buf_y);
	  if (iv == 0)
	      sprintf (buf, "M %s %s ", buf_x, buf_y);
	  else
	      sprintf (buf, "%s %s ", buf_x, buf_y);
	  if (iv == points - 1 && closePath == 1)
	      sprintf (buf, "z ");
	  strcat (*buffer, buf);
      }
}

GAIAGEO_DECLARE void
gaiaOutSvg (gaiaGeomCollPtr geom, char **result, int relative, int precision)
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
    gaiaRingPtr ring;
    if (!geom)
      {
	  *result = NULL;
	  return;
      }
    *result = malloc (txt_size);
    memset (*result, '\0', txt_size);
    point = geom->FirstPoint;
    while (point)
      {
	  /* counting how many POINTs are there */
	  pts++;
	  point = point->Next;
      }
    line = geom->FirstLinestring;
    while (line)
      {
	  /* counting how many LINESTRINGs are there */
	  lns++;
	  line = line->Next;
      }
    polyg = geom->FirstPolygon;
    while (polyg)
      {
	  /* counting how many POLYGONs are there */
	  pgs++;
	  polyg = polyg->Next;
      }

    if ((pts + lns + pgs) == 1)
      {
	  /* we have only one elementary geometry */
	  point = geom->FirstPoint;
	  while (point)
	    {
		/* processing POINT */
		if (relative == 1)
		    SvgCoords (point, result, &txt_size, relative, precision);
		else
		    SvgCircle (point, result, &txt_size, relative, precision);
		point = point->Next;
	    }
	  line = geom->FirstLinestring;
	  while (line)
	    {
		/* processing LINESTRING */
		if (relative == 1)
		    SvgPathRelative (line->Points, line->Coords, result,
				     &txt_size, relative, precision, 0);
		else
		    SvgPathAbsolute (line->Points, line->Coords, result,
				     &txt_size, relative, precision, 0);
		line = line->Next;
	    }
	  polyg = geom->FirstPolygon;
	  while (polyg)
	    {
		/* process exterior and interior rings */
		ring = polyg->Exterior;
		if (relative == 1)
		  {
		      SvgPathRelative (ring->Points, ring->Coords, result,
				       &txt_size, relative, precision, 1);
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    ring = polyg->Interiors + ib;
			    SvgPathRelative (ring->Points, ring->Coords, result,
					     &txt_size, relative, precision, 1);
			}
		  }
		else
		  {
		      SvgPathAbsolute (ring->Points, ring->Coords, result,
				       &txt_size, relative, precision, 1);
		      for (ib = 0; ib < polyg->NumInteriors; ib++)
			{
			    ring = polyg->Interiors + ib;
			    SvgPathAbsolute (ring->Points, ring->Coords, result,
					     &txt_size, relative, precision, 1);
			}
		  }
		polyg = polyg->Next;
	    }
      }
    else
      {
	  /* we have some kind of complex geometry */
	  if (pts > 0 && lns == 0 && pgs == 0)
	    {
		/* this one is a MULTIPOINT */
		point = geom->FirstPoint;
		while (point)
		  {
		      /* processing POINTs */
		      if (point != geom->FirstPoint)
			  gaiaOutText (",", result, &txt_size);
		      if (relative == 1)
			  SvgCoords (point, result, &txt_size, relative,
				     precision);
		      else
			  SvgCircle (point, result, &txt_size, relative,
				     precision);
		      point = point->Next;
		  }
	    }
	  else if (pts == 0 && lns > 0 && pgs == 0)
	    {
		/* this one is a MULTILINESTRING */
		line = geom->FirstLinestring;
		while (line)
		  {
		      /* processing LINESTRINGs */
		      if (relative == 1)
			  SvgPathRelative (line->Points, line->Coords, result,
					   &txt_size, relative, precision, 0);
		      else
			  SvgPathAbsolute (line->Points, line->Coords, result,
					   &txt_size, relative, precision, 0);
		      line = line->Next;
		  }
	    }
	  else if (pts == 0 && lns == 0 && pgs > 0)
	    {
		/* this one is a MULTIPOLYGON */
		polyg = geom->FirstPolygon;
		while (polyg)
		  {
		      /* processing POLYGONs */
		      ring = polyg->Exterior;
		      if (relative == 1)
			{
			    SvgPathRelative (ring->Points, ring->Coords, result,
					     &txt_size, relative, precision, 1);
			    for (ib = 0; ib < polyg->NumInteriors; ib++)
			      {
				  ring = polyg->Interiors + ib;
				  SvgPathRelative (ring->Points, ring->Coords,
						   result, &txt_size, relative,
						   precision, 1);
			      }
			}
		      else
			{
			    SvgPathAbsolute (ring->Points, ring->Coords, result,
					     &txt_size, relative, precision, 1);
			    for (ib = 0; ib < polyg->NumInteriors; ib++)
			      {
				  ring = polyg->Interiors + ib;
				  SvgPathAbsolute (ring->Points, ring->Coords,
						   result, &txt_size, relative,
						   precision, 1);
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
		while (point)
		  {
		      /* processing POINTs */
		      if (ie > 0)
			{
			    gaiaOutText (";", result, &txt_size);
			}
		      ie++;
		      if (relative == 1)
			  SvgCoords (point, result, &txt_size, relative,
				     precision);
		      else
			  SvgCircle (point, result, &txt_size, relative,
				     precision);
		      point = point->Next;
		  }
		line = geom->FirstLinestring;
		while (line)
		  {
		      /* processing LINESTRINGs */
		      if (ie > 0)
			  gaiaOutText (";", result, &txt_size);
		      ie++;
		      if (relative == 1)
			  SvgPathRelative (line->Points, line->Coords, result,
					   &txt_size, relative, precision, 0);
		      else
			  SvgPathAbsolute (line->Points, line->Coords, result,
					   &txt_size, relative, precision, 0);
		      line = line->Next;
		  }
		polyg = geom->FirstPolygon;
		while (polyg)
		  {
		      /* processing POLYGONs */
		      ie++;
		      /* process exterior and interior rings */
		      ring = polyg->Exterior;
		      if (relative == 1)
			{
			    SvgPathRelative (ring->Points, ring->Coords, result,
					     &txt_size, relative, precision, 1);
			    for (ib = 0; ib < polyg->NumInteriors; ib++)
			      {
				  ring = polyg->Interiors + ib;
				  SvgPathRelative (ring->Points, ring->Coords,
						   result, &txt_size, relative,
						   precision, 1);
			      }
			}
		      else
			{
			    SvgPathAbsolute (ring->Points, ring->Coords, result,
					     &txt_size, relative, precision, 1);
			    for (ib = 0; ib < polyg->NumInteriors; ib++)
			      {
				  ring = polyg->Interiors + ib;
				  SvgPathAbsolute (ring->Points, ring->Coords,
						   result, &txt_size, relative,
						   precision, 1);
			      }
			}
		      polyg = polyg->Next;
		  }
	    }
      }
}

/* END of Klaus Foerster SVG implementation */
/**************** End file: gg_wkt.c **********/


/**************** Begin file: spatialite.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <string.h> */
/* #include <math.h> */
/* #include <float.h> */
/* #include <locale.h> */
/* #include <errno.h> */

/* #include <spatialite/sqlite3ext.h> */
/* #include <spatialite/gaiageo.h> */
/* #include <spatialite/gaiaexif.h> */
/* #include <spatialite/spatialite.h> */
/* #include <spatialite.h> */

static SQLITE_EXTENSION_INIT1 struct spatial_index_str
{
/* a struct to implement a linked list of spatial-indexes */
    char ValidRtree;
    char ValidCache;
    char *TableName;
    char *ColumnName;
    struct spatial_index_str *Next;
};

struct stddev_str
{
/* a struct to implement StandardVariation and Variance aggregate functions */
    int cleaned;
    double mean;
    double quot;
    double count;
};

struct fdo_table
{
/* a struct to implement a linked-list for FDO-ORG table names */
    char *table;
    struct fdo_table *next;
};

static void
fnct_GeometryConstraints (sqlite3_context * context, int argc,
			  sqlite3_value ** argv)
{
/* SQL function:
/ GeometryConstraints(BLOBencoded geometry, geometry-type, srid)
/
/ checks geometry constraints, returning:
/
/ -1 - if some error occurred
/ 1 - if geometry constraints validation passes
/ 0 - if geometry constraints validation fails
/
*/
    int little_endian;
    int endian_arch = gaiaEndianArch ();
    unsigned char *p_blob = NULL;
    int n_bytes = 0;
    int srid;
    int geom_srid = -1;
    const unsigned char *type;
    int xtype;
    int geom_type = -1;
    int ret;
    if (sqlite3_value_type (argv[0]) == SQLITE_BLOB
	|| sqlite3_value_type (argv[0]) == SQLITE_NULL)
	;
    else
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_TEXT)
	type = sqlite3_value_text (argv[1]);
    else
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
	srid = sqlite3_value_int (argv[2]);
    else
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[0]) == SQLITE_BLOB)
      {
	  p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
	  n_bytes = sqlite3_value_bytes (argv[0]);
      }
    if (p_blob)
      {
	  /* quick Geometry validation */
	  if (n_bytes < 45)
	      goto illegal_geometry;	/* cannot be an internal BLOB WKB geometry */
	  if (*(p_blob + 0) != GAIA_MARK_START)
	      goto illegal_geometry;	/* failed to recognize START signature */
	  if (*(p_blob + (n_bytes - 1)) != GAIA_MARK_END)
	      goto illegal_geometry;	/* failed to recognize END signature */
	  if (*(p_blob + 38) != GAIA_MARK_MBR)
	      goto illegal_geometry;	/* failed to recognize MBR signature */
	  if (*(p_blob + 1) == GAIA_LITTLE_ENDIAN)
	      little_endian = 1;
	  else if (*(p_blob + 1) == GAIA_BIG_ENDIAN)
	      little_endian = 0;
	  else
	      goto illegal_geometry;	/* unknown encoding; nor litte-endian neither big-endian */
	  geom_type = gaiaImport32 (p_blob + 39, little_endian, endian_arch);
	  geom_srid = gaiaImport32 (p_blob + 2, little_endian, endian_arch);
	  goto valid_geometry;
	illegal_geometry:
	  sqlite3_result_int (context, -1);
	  return;
      }
  valid_geometry:
    xtype = GAIA_UNKNOWN;
    if (strcasecmp ((char *) type, "POINT") == 0)
	xtype = GAIA_POINT;
    if (strcasecmp ((char *) type, "LINESTRING") == 0)
	xtype = GAIA_LINESTRING;
    if (strcasecmp ((char *) type, "POLYGON") == 0)
	xtype = GAIA_POLYGON;
    if (strcasecmp ((char *) type, "MULTIPOINT") == 0)
	xtype = GAIA_MULTIPOINT;
    if (strcasecmp ((char *) type, "MULTILINESTRING") == 0)
	xtype = GAIA_MULTILINESTRING;
    if (strcasecmp ((char *) type, "MULTIPOLYGON") == 0)
	xtype = GAIA_MULTIPOLYGON;
    if (strcasecmp ((char *) type, "GEOMETRYCOLLECTION") == 0)
	xtype = GAIA_GEOMETRYCOLLECTION;
    if (strcasecmp ((char *) type, "GEOMETRY") == 0)
	xtype = -1;
    if (xtype == GAIA_UNKNOWN)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = 1;
	  if (p_blob)
	    {
		/* skipping NULL Geometry; this is assumed to be always good */
		if (geom_srid != srid)
		    ret = 0;
		if (xtype == -1)
		    ;
		else if (xtype != geom_type)
		    ret = 0;
	    }
	  sqlite3_result_int (context, ret);
      }
}

static int
checkSpatialMetaData (sqlite3 * sqlite)
{
/* internal utility function:
/
/ for FDO-OGR interoperability:
/ tests the SpatialMetadata type, returning:
/
/ 0 - if no valid SpatialMetaData where found
/ 1 - if SpatiaLite-like SpatialMetadata where found
/ 2- if FDO-OGR-like SpatialMetadata where found
/
*/
    int spatialite_rs = 0;
    int fdo_rs = 0;
    int spatialite_gc = 0;
    int fdo_gc = 0;
    int rs_srid = 0;
    int auth_name = 0;
    int auth_srid = 0;
    int srtext = 0;
    int ref_sys_name = 0;
    int proj4text = 0;
    int f_table_name = 0;
    int f_geometry_column = 0;
    int geometry_type = 0;
    int coord_dimension = 0;
    int gc_srid = 0;
    int geometry_format = 0;
    int type = 0;
    int spatial_index_enabled = 0;
    char sql[1024];
    int ret;
    const char *name;
    int i;
    char **results;
    int rows;
    int columns;
/* checking the GEOMETRY_COLUMNS table */
    strcpy (sql, "PRAGMA table_info(\"geometry_columns\")");
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	goto unknown;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		if (strcasecmp (name, "f_table_name") == 0)
		    f_table_name = 1;
		if (strcasecmp (name, "f_geometry_column") == 0)
		    f_geometry_column = 1;
		if (strcasecmp (name, "geometry_type") == 0)
		    geometry_type = 1;
		if (strcasecmp (name, "coord_dimension") == 0)
		    coord_dimension = 1;
		if (strcasecmp (name, "srid") == 0)
		    gc_srid = 1;
		if (strcasecmp (name, "geometry_format") == 0)
		    geometry_format = 1;
		if (strcasecmp (name, "type") == 0)
		    type = 1;
		if (strcasecmp (name, "spatial_index_enabled") == 0)
		    spatial_index_enabled = 1;
	    }
      }
    sqlite3_free_table (results);
    if (f_table_name
	&&
	f_geometry_column
	&& type && coord_dimension && gc_srid && spatial_index_enabled)
	spatialite_gc = 1;
    if (f_table_name
	&&
	f_geometry_column
	&& geometry_type && coord_dimension && gc_srid && geometry_format)
	fdo_gc = 1;
/* checking the SPATIAL_REF_SYS table */
    strcpy (sql, "PRAGMA table_info(\"spatial_ref_sys\")");
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	goto unknown;
    if (rows < 1)
	;
    else
      {
	  for (i = 1; i <= rows; i++)
	    {
		name = results[(i * columns) + 1];
		if (strcasecmp (name, "srid") == 0)
		    rs_srid = 1;
		if (strcasecmp (name, "auth_name") == 0)
		    auth_name = 1;
		if (strcasecmp (name, "auth_srid") == 0)
		    auth_srid = 1;
		if (strcasecmp (name, "srtext") == 0)
		    srtext = 1;
		if (strcasecmp (name, "ref_sys_name") == 0)
		    ref_sys_name = 1;
		if (strcasecmp (name, "proj4text") == 0)
		    proj4text = 1;
	    }
      }
    sqlite3_free_table (results);
    if (rs_srid && auth_name && auth_srid && ref_sys_name && proj4text)
	spatialite_rs = 1;
    if (rs_srid && auth_name && auth_srid && srtext)
	fdo_rs = 1;
/* verifying the MetaData format */
    if (spatialite_gc && spatialite_rs)
	return 1;
    if (fdo_gc && fdo_rs)
	return 2;
  unknown:
    return 0;
}

static void
add_fdo_table (struct fdo_table **first, struct fdo_table **last,
	       const char *table, int len)
{
/* adds an FDO-OGR styled Geometry Table to corresponding linked list */
    struct fdo_table *p = malloc (sizeof (struct fdo_table));
    p->table = malloc (len + 1);
    strcpy (p->table, table);
    p->next = NULL;
    if (!(*first))
	(*first) = p;
    if ((*last))
	(*last)->next = p;
    (*last) = p;
}

static void
free_fdo_tables (struct fdo_table *first)
{
/* memory cleanup; destroying the FDO-OGR tables linked list */
    struct fdo_table *p;
    struct fdo_table *pn;
    p = first;
    while (p)
      {
	  pn = p->next;
	  if (p->table)
	      free (p->table);
	  free (p);
	  p = pn;
      }
}

static void
fnct_AutoFDOStart (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ AutoFDOStart(void)
/
/ for FDO-OGR interoperability:
/ tests the SpatialMetadata type, then automatically
/ creating a VirtualFDO table for each FDO-OGR main table 
/ declared within FDO-styled SpatialMetadata
/
*/
    int ret;
    const char *name;
    int i;
    char **results;
    int rows;
    int columns;
    char sql[1024];
    int count = 0;
    struct fdo_table *first = NULL;
    struct fdo_table *last = NULL;
    struct fdo_table *p;
    int len;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (checkSpatialMetaData (sqlite) == 2)
      {
	  /* ok, creating VirtualFDO tables */
	  strcpy (sql, "SELECT DISTINCT f_table_name FROM geometry_columns");
	  ret =
	      sqlite3_get_table (sqlite, sql, &results, &rows, &columns, NULL);
	  if (ret != SQLITE_OK)
	      goto error;
	  if (rows < 1)
	      ;
	  else
	    {
		for (i = 1; i <= rows; i++)
		  {
		      name = results[(i * columns) + 0];
		      if (name)
			{
			    len = strlen (name);
			    add_fdo_table (&first, &last, name, len);
			}
		  }
	    }
	  sqlite3_free_table (results);
	  p = first;
	  while (p)
	    {
		/* destroying the VirtualFDO table [if existing] */
		sprintf (sql, "DROP TABLE IF EXISTS \"fdo_%s\"", p->table);
		ret = sqlite3_exec (sqlite, sql, NULL, 0, NULL);
		if (ret != SQLITE_OK)
		    goto error;
		/* creating the VirtualFDO table */
		sprintf (sql,
			 "CREATE VIRTUAL TABLE \"fdo_%s\" USING VirtualFDO(%s)",
			 p->table, p->table);
		ret = sqlite3_exec (sqlite, sql, NULL, 0, NULL);
		if (ret != SQLITE_OK)
		    goto error;
		count++;
		p = p->next;
	    }
	error:
	  free_fdo_tables (first);
	  sqlite3_result_int (context, count);
	  return;
      }
    sqlite3_result_int (context, 0);
    return;
}

static void
fnct_AutoFDOStop (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ AutoFDOStop(void)
/
/ for FDO-OGR interoperability:
/ tests the SpatialMetadata type, then automatically
/ removes any VirtualFDO table 
/
*/
    int ret;
    const char *name;
    int i;
    char **results;
    int rows;
    int columns;
    char sql[1024];
    int count = 0;
    struct fdo_table *first = NULL;
    struct fdo_table *last = NULL;
    struct fdo_table *p;
    int len;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (checkSpatialMetaData (sqlite) == 2)
      {
	  /* ok, creating VirtualFDO tables */
	  strcpy (sql, "SELECT DISTINCT f_table_name FROM geometry_columns");
	  ret =
	      sqlite3_get_table (sqlite, sql, &results, &rows, &columns, NULL);
	  if (ret != SQLITE_OK)
	      goto error;
	  if (rows < 1)
	      ;
	  else
	    {
		for (i = 1; i <= rows; i++)
		  {
		      name = results[(i * columns) + 0];
		      if (name)
			{
			    len = strlen (name);
			    add_fdo_table (&first, &last, name, len);
			}
		  }
	    }
	  sqlite3_free_table (results);
	  p = first;
	  while (p)
	    {
		/* destroying the VirtualFDO table [if existing] */
		sprintf (sql, "DROP TABLE IF EXISTS \"fdo_%s\"", p->table);
		ret = sqlite3_exec (sqlite, sql, NULL, 0, NULL);
		if (ret != SQLITE_OK)
		    goto error;
		count++;
		p = p->next;
	    }
	error:
	  free_fdo_tables (first);
	  sqlite3_result_int (context, count);
	  return;
      }
    sqlite3_result_int (context, 0);
    return;
}

static void
fnct_CheckSpatialMetaData (sqlite3_context * context, int argc,
			   sqlite3_value ** argv)
{
/* SQL function:
/ CheckSpatialMetaData(void)
/
/ for FDO-OGR interoperability:
/ tests the SpatialMetadata type, returning:
/
/ 0 - if no valid SpatialMetaData where found
/ 1 - if SpatiaLite-like SpatialMetadata where found
/ 2- if FDO-OGR-like SpatialMetadata where found
/
*/
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    int ret = checkSpatialMetaData (sqlite);
    sqlite3_result_int (context, ret);
    return;
}

static void
fnct_InitSpatialMetaData (sqlite3_context * context, int argc,
			  sqlite3_value ** argv)
{
/* SQL function:
/ InitSpatialMetaData(void)
/
/ creates the SPATIAL_REF_SYS and GEOMETRY_COLUMNS tables
/ returns 1 on success
/ 0 on failure
*/
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
/* creating the SPATIAL_REF_SYS tables */
    strcpy (sql, "CREATE TABLE spatial_ref_sys (\n");
    strcat (sql, "srid INTEGER NOT NULL PRIMARY KEY,\n");
    strcat (sql, "auth_name VARCHAR(256) NOT NULL,\n");
    strcat (sql, "auth_srid INTEGER NOT NULL,\n");
    strcat (sql, "ref_sys_name VARCHAR(256),\n");
    strcat (sql, "proj4text VARCHAR(2048) NOT NULL)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/* setting a trigger to ensure referential integrity on DELETE */
    strcpy (sql,
	    "CREATE TRIGGER fkd_refsys_geocols BEFORE DELETE ON spatial_ref_sys\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ROLLBACK, 'delete on table ''spatial_ref_sys'' violates constraint: ''geometry_columns.srid''')\n");
    strcat (sql,
	    "WHERE (SELECT srid FROM geometry_columns WHERE srid = OLD.srid) IS NOT NULL;\n");
    strcat (sql, "END;");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/* creating the GEOMETRY_COLUMN tables */
    strcpy (sql, "CREATE TABLE geometry_columns (\n");
    strcat (sql, "f_table_name VARCHAR(256) NOT NULL,\n");
    strcat (sql, "f_geometry_column VARCHAR(256) NOT NULL,\n");
    strcat (sql, "type VARCHAR(30) NOT NULL,\n");
    strcat (sql, "coord_dimension INTEGER NOT NULL,\n");
    strcat (sql, "srid INTEGER,\n");
    strcat (sql, "spatial_index_enabled INTEGER NOT NULL)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/* setting a trigger to ensure referential integrity on INSERT */
    strcpy (sql,
	    "CREATE TRIGGER fki_geocols_refsys BEFORE INSERT ON geometry_columns\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ROLLBACK, 'insert on table ''geometry_columns'' violates constraint: ''spatial_ref_sys.srid''')\n");
    strcat (sql, "WHERE  NEW.\"srid\" IS NOT NULL\n");
    strcat (sql,
	    "AND (SELECT srid FROM spatial_ref_sys WHERE srid = NEW.srid) IS NULL;\n");
    strcat (sql, "END;");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/* setting a trigger to ensure referential integrity on UPDATE */
    strcpy (sql,
	    "CREATE TRIGGER fku_geocols_refsys BEFORE UPDATE ON geometry_columns\n");
    strcat (sql, "FOR EACH ROW BEGIN\n");
    strcat (sql,
	    "SELECT RAISE(ROLLBACK, 'update on table ''geometry_columns'' violates constraint: ''spatial_ref_sys.srid''')\n");
    strcat (sql, "WHERE  NEW.srid IS NOT NULL\n");
    strcat (sql,
	    "AND (SELECT srid FROM spatial_ref_sys WHERE srid = NEW.srid) IS NULL;\n");
    strcat (sql, "END;");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/* creating an UNIQUE INDEX */
    strcpy (sql, "CREATE UNIQUE INDEX idx_geocols ON geometry_columns\n");
    strcat (sql, "(f_table_name, f_geometry_column)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/* creating the GEOM_COLS_REF_SYS view */
    strcpy (sql, "CREATE VIEW geom_cols_ref_sys AS\n");
    strcat (sql, "SELECT  f_table_name, f_geometry_column, type,\n");
    strcat (sql, "coord_dimension, spatial_ref_sys.srid AS srid,\n");
    strcat (sql, "auth_name, auth_srid, ref_sys_name, proj4text\n");
    strcat (sql, "FROM geometry_columns, spatial_ref_sys\n");
    strcat (sql, "WHERE geometry_columns.srid = spatial_ref_sys.srid");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "InitSpatiaMetaData() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static int
recoverGeomColumn (sqlite3 * sqlite, const unsigned char *table,
		   const unsigned char *column, int xtype, int srid)
{
/* checks if TABLE.COLUMN exists and has the required features */
    int ok = 1;
    char sql[1024];
    int type;
    sqlite3_stmt *stmt;
    gaiaGeomCollPtr geom;
    const void *blob_value;
    int len;
    int ret;
    int i_col;
    sprintf (sql, "SELECT %s FROM \"%s\"", column, table);
/* compiling SQL prepared statement */
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "recoverGeomColumn: error %d \"%s\"\n",
		   sqlite3_errcode (sqlite), sqlite3_errmsg (sqlite));
	  return 0;
      }
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* cecking Geometry features */
		geom = NULL;
		for (i_col = 0; i_col < sqlite3_column_count (stmt); i_col++)
		  {
		      if (sqlite3_column_type (stmt, i_col) != SQLITE_BLOB)
			  ok = 0;
		      else
			{
			    blob_value = sqlite3_column_blob (stmt, i_col);
			    len = sqlite3_column_bytes (stmt, i_col);
			    geom = gaiaFromSpatiaLiteBlobWkb (blob_value, len);
			    if (!geom)
				ok = 0;
			    else
			      {
				  if (geom->Srid != srid)
				      ok = 0;
				  type = gaiaGeometryType (geom);
				  if (xtype == type)
				      ;
				  else
				      ok = 0;
				  gaiaFreeGeomColl (geom);
			      }
			}
		  }
	    }
	  if (!ok)
	      break;
      }
    ret = sqlite3_finalize (stmt);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "recoverGeomColumn: error %d \"%s\"\n",
		   sqlite3_errcode (sqlite), sqlite3_errmsg (sqlite));
	  return 0;
      }
    return ok;
}

static void
buildSpatialIndex (sqlite3 * sqlite, const unsigned char *table, char *col_name)
{
/* loading a SpatialIndex [RTree] */
    char sql[2048];
    char sql2[1024];
    char *errMsg = NULL;
    int ret;
    sprintf (sql,
	     "INSERT INTO \"idx_%s_%s\" (\"pkid\", \"xmin\", \"xmax\", \"ymin\", \"ymax\") ",
	     table, col_name);
    sprintf (sql2,
	     "SELECT ROWID, MbrMinX(\"%s\"), MbrMaxX(\"%s\"), MbrMinY(\"%s\"), MbrMaxY(\"%s\") FROM \"%s\"",
	     col_name, col_name, col_name, col_name, table);
    strcat (sql, sql2);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "buildSpatialIndex error: \"%s\"\n", errMsg);
	  sqlite3_free (errMsg);
      }
}

static void
updateGeometryTriggers (sqlite3 * sqlite, const unsigned char *table,
			const unsigned char *column)
{
/* updates triggers for some Spatial Column */
    char sql[256];
    char trigger[4096];
    char **results;
    int ret;
    int rows;
    int columns;
    int i;
    char tblname[256];
    char colname[256];
    char col_type[32];
    char col_srid[32];
    char col_index[32];
    int srid;
    int index;
    int cached;
    int len;
    char *errMsg = NULL;
    char dummy[512];
    struct spatial_index_str *first_idx = NULL;
    struct spatial_index_str *last_idx = NULL;
    struct spatial_index_str *curr_idx;
    struct spatial_index_str *next_idx;
    sprintf (sql,
	     "SELECT f_table_name, f_geometry_column, type, srid, spatial_index_enabled FROM geometry_columns WHERE f_table_name LIKE '%s' AND f_geometry_column LIKE '%s'",
	     table, column);
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, &errMsg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "updateTableTriggers: \"%s\"\n", errMsg);
	  sqlite3_free (errMsg);
	  return;
      }
    for (i = 1; i <= rows; i++)
      {
	  /* preparing the triggers */
	  strcpy (tblname, results[(i * columns)]);
	  strcpy (colname, results[(i * columns) + 1]);
	  strcpy (col_type, results[(i * columns) + 2]);
	  strcpy (col_srid, results[(i * columns) + 3]);
	  strcpy (col_index, results[(i * columns) + 4]);
	  srid = atoi (col_srid);
	  if (atoi (col_index) == 1)
	      index = 1;
	  else
	      index = 0;
	  if (atoi (col_index) == 2)
	      cached = 1;
	  else
	      cached = 0;

	  /* trying to delete old versions [v2.0, v2.2] triggers[if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS \"gti_%s_%s\"", tblname,
		   colname);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  sprintf (trigger, "DROP TRIGGER IF EXISTS \"gtu_%s_%s\"", tblname,
		   colname);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  sprintf (trigger, "DROP TRIGGER IF EXISTS \"gsi_%s_%s\"", tblname,
		   colname);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  sprintf (trigger, "DROP TRIGGER IF EXISTS \"gsu_%s_%s\"", tblname,
		   colname);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  /* end deletion old versions [v2.0, v2.2] triggers[if any] */

	  /* deleting the old INSERT trigger TYPE [if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS \"ggi_%s_%s\"", tblname,
		   colname);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  /* inserting the new INSERT trigger TYPE */
	  sprintf (trigger,
		   "CREATE TRIGGER \"ggi_%s_%s\" BEFORE INSERT ON \"%s\"\n",
		   tblname, colname, tblname);
	  strcat (trigger, "FOR EACH ROW BEGIN\n");
	  sprintf (dummy,
		   "SELECT RAISE(ROLLBACK, '\"%s\".\"%s\" violates Geometry constraint [geom-type or SRID not allowed]')\n",
		   tblname, colname);
	  strcat (trigger, dummy);
	  strcat (trigger, "WHERE (SELECT type FROM geometry_columns\n");
	  sprintf (dummy,
		   "WHERE f_table_name = '%s' AND f_geometry_column = '%s'\n",
		   tblname, colname);
	  strcat (trigger, dummy);
	  sprintf (dummy,
		   "AND GeometryConstraints(NEW.\"%s\", type, srid) = 1) IS NULL;\n",
		   colname);
	  strcat (trigger, dummy);
	  strcat (trigger, "END;");
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  /* deleting the old UPDATE trigger TYPE [if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS \"ggu_%s_%s\"", tblname,
		   colname);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  /* inserting the new UPDATE trigger TYPE */
	  sprintf (trigger,
		   "CREATE TRIGGER \"ggu_%s_%s\" BEFORE UPDATE ON \"%s\"\n",
		   tblname, colname, tblname);
	  strcat (trigger, "FOR EACH ROW BEGIN\n");
	  sprintf (dummy,
		   "SELECT RAISE(ROLLBACK, '\"%s\".\"%s\" violates Geometry constraint [geom-type or SRID not allowed]')\n",
		   tblname, colname);
	  strcat (trigger, dummy);
	  strcat (trigger,
		  "WHERE (SELECT \"type\" FROM \"geometry_columns\"\n");
	  sprintf (dummy,
		   "WHERE f_table_name = '%s' AND f_geometry_column = '%s'\n",
		   tblname, colname);
	  strcat (trigger, dummy);
	  sprintf (dummy,
		   "AND GeometryConstraints(NEW.\"%s\", type, srid) = 1) IS NULL;\n",
		   colname);
	  strcat (trigger, dummy);
	  strcat (trigger, "END;");
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  /* inserting SpatialIndex infos into the linked list */
	  curr_idx = malloc (sizeof (struct spatial_index_str));
	  len = strlen (tblname);
	  curr_idx->TableName = malloc (len + 1);
	  strcpy (curr_idx->TableName, tblname);
	  len = strlen ((char *) colname);
	  curr_idx->ColumnName = malloc (len + 1);
	  strcpy (curr_idx->ColumnName, (char *) colname);
	  curr_idx->ValidRtree = (char) index;
	  curr_idx->ValidCache = (char) cached;
	  curr_idx->Next = NULL;
	  if (!first_idx)
	      first_idx = curr_idx;
	  if (last_idx)
	      last_idx->Next = curr_idx;
	  last_idx = curr_idx;
	  /* deleting the old INSERT trigger SPATIAL_INDEX [if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS \"gii_%s_%s\"", tblname,
		   colname);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  if (index)
	    {
		/* inserting the new INSERT trigger SRID */
		sprintf (trigger,
			 "CREATE TRIGGER \"gii_%s_%s\" AFTER INSERT ON \"%s\"\n",
			 tblname, colname, tblname);
		strcat (trigger, "FOR EACH ROW BEGIN\n");
		sprintf (dummy,
			 "INSERT INTO \"idx_%s_%s\" (pkid, xmin, xmax, ymin, ymax) VALUES (NEW.ROWID,\n",
			 tblname, colname);
		strcat (trigger, dummy);
		sprintf (dummy, "MbrMinX(NEW.\"%s\"), ", colname);
		strcat (trigger, dummy);
		sprintf (dummy, "MbrMaxX(NEW.\"%s\"), ", colname);
		strcat (trigger, dummy);
		sprintf (dummy, "MbrMinY(NEW.\"%s\"), ", colname);
		strcat (trigger, dummy);
		sprintf (dummy, "MbrMaxY(NEW.\"%s\"));\n", colname);
		strcat (trigger, dummy);
		strcat (trigger, "END;");
		ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
		if (ret != SQLITE_OK)
		    goto error;
	    }
	  /* deleting the old UPDATE trigger SPATIAL_INDEX [if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS \"giu_%s_%s\"", tblname,
		   colname);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  if (index)
	    {
		/* inserting the new UPDATE trigger SRID */
		sprintf (trigger,
			 "CREATE TRIGGER \"giu_%s_%s\" AFTER UPDATE ON \"%s\"\n",
			 tblname, colname, tblname);
		strcat (trigger, "FOR EACH ROW BEGIN\n");
		sprintf (dummy, "UPDATE \"idx_%s_%s\" SET ", tblname, colname);
		strcat (trigger, dummy);
		sprintf (dummy, "\"xmin\" = MbrMinX(NEW.\"%s\"), ", colname);
		strcat (trigger, dummy);
		sprintf (dummy, "\"xmax\" = MbrMaxX(NEW.\"%s\"), ", colname);
		strcat (trigger, dummy);
		sprintf (dummy, "\"ymin\" = MbrMinY(NEW.\"%s\"), ", colname);
		strcat (trigger, dummy);
		sprintf (dummy, "\"ymax\" = MbrMaxY(NEW.\"%s\")\n", colname);
		strcat (trigger, dummy);
		strcat (trigger, "WHERE \"pkid\" = NEW.ROWID;\n");
		strcat (trigger, "END;");
		ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
		if (ret != SQLITE_OK)
		    goto error;
	    }
	  /* deleting the old UPDATE trigger SPATIAL_INDEX [if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS \"gid_%s_%s\"", tblname,
		   colname);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  if (index)
	    {
		/* inserting the new DELETE trigger SRID */
		sprintf (trigger,
			 "CREATE TRIGGER \"gid_%s_%s\" AFTER DELETE ON \"%s\"\n",
			 tblname, colname, tblname);
		strcat (trigger, "FOR EACH ROW BEGIN\n");
		sprintf (dummy,
			 "DELETE FROM \"idx_%s_%s\" WHERE pkid = OLD.ROWID;\n",
			 tblname, colname);
		strcat (trigger, dummy);
		strcat (trigger, "END;");
		ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
		if (ret != SQLITE_OK)
		    goto error;
	    }
	  /* deleting the old INSERT trigger MBR_CACHE [if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS \"gci_%s_%s\"", tblname,
		   colname);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  if (cached)
	    {
		/* inserting the new INSERT trigger SRID */
		sprintf (trigger,
			 "CREATE TRIGGER \"gci_%s_%s\" AFTER INSERT ON \"%s\"\n",
			 tblname, colname, tblname);
		strcat (trigger, "FOR EACH ROW BEGIN\n");
		sprintf (dummy,
			 "INSERT INTO \"cache_%s_%s\" (rowid, mbr) VALUES (NEW.ROWID,\nBuildMbrFilter(",
			 tblname, colname);
		strcat (trigger, dummy);
		sprintf (dummy, "MbrMinX(NEW.\"%s\"), ", colname);
		strcat (trigger, dummy);
		sprintf (dummy, "MbrMinY(NEW.\"%s\"), ", colname);
		strcat (trigger, dummy);
		sprintf (dummy, "MbrMaxX(NEW.\"%s\"), ", colname);
		strcat (trigger, dummy);
		sprintf (dummy, "MbrMaxY(NEW.\"%s\")));\n", colname);
		strcat (trigger, dummy);
		strcat (trigger, "END;");
		ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
		if (ret != SQLITE_OK)
		    goto error;
	    }
	  /* deleting the old UPDATE trigger MBR_CACHE [if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS \"gcu_%s_%s\"", tblname,
		   colname);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  if (cached)
	    {
		/* inserting the new UPDATE trigger SRID */
		sprintf (trigger,
			 "CREATE TRIGGER \"gcu_%s_%s\" AFTER UPDATE ON \"%s\"\n",
			 tblname, colname, tblname);
		strcat (trigger, "FOR EACH ROW BEGIN\n");
		sprintf (dummy, "UPDATE \"cache_%s_%s\" SET ", tblname,
			 colname);
		strcat (trigger, dummy);
		sprintf (dummy,
			 "\"mbr\" = BuildMbrFilter(MbrMinX(NEW.\"%s\"), ",
			 colname);
		strcat (trigger, dummy);
		sprintf (dummy, "MbrMinY(NEW.\"%s\"), ", colname);
		strcat (trigger, dummy);
		sprintf (dummy, "MbrMaxX(NEW.\"%s\"), ", colname);
		strcat (trigger, dummy);
		sprintf (dummy, "MbrMaxY(NEW.\"%s\"))\n", colname);
		strcat (trigger, dummy);
		strcat (trigger, "WHERE \"rowid\" = NEW.ROWID;\n");
		strcat (trigger, "END;");
		ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
		if (ret != SQLITE_OK)
		    goto error;
	    }
	  /* deleting the old UPDATE trigger MBR_CACHE [if any] */
	  sprintf (trigger, "DROP TRIGGER IF EXISTS \"gcd_%s_%s\"", tblname,
		   colname);
	  ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
	  if (ret != SQLITE_OK)
	      goto error;
	  if (cached)
	    {
		/* inserting the new DELETE trigger SRID */
		sprintf (trigger,
			 "CREATE TRIGGER \"gcd_%s_%s\" AFTER DELETE ON \"%s\"\n",
			 tblname, colname, tblname);
		strcat (trigger, "FOR EACH ROW BEGIN\n");
		sprintf (dummy,
			 "DELETE FROM \"cache_%s_%s\" WHERE \"rowid\" = OLD.ROWID;\n",
			 tblname, colname);
		strcat (trigger, dummy);
		strcat (trigger, "END;");
		ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
		if (ret != SQLITE_OK)
		    goto error;
	    }
      }
    sqlite3_free_table (results);
/* now we'll adjust any related SpatialIndex as required */
    curr_idx = first_idx;
    while (curr_idx)
      {
	  if (curr_idx->ValidRtree)
	    {
		/* building RTree SpatialIndex */
		sprintf (trigger,
			 "CREATE VIRTUAL TABLE \"idx_%s_%s\" USING rtree(\n",
			 curr_idx->TableName, curr_idx->ColumnName);
		strcat (trigger, "pkid, xmin, xmax, ymin, ymax)");
		ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
		if (ret != SQLITE_OK)
		    goto error;
		buildSpatialIndex (sqlite,
				   (unsigned char *) (curr_idx->TableName),
				   curr_idx->ColumnName);
	    }
	  if (curr_idx->ValidCache)
	    {
		/* building MbrCache SpatialIndex */
		sprintf (trigger,
			 "CREATE VIRTUAL TABLE \"cache_%s_%s\" USING MbrCache(%s, %s)\n",
			 curr_idx->TableName, curr_idx->ColumnName,
			 curr_idx->TableName, curr_idx->ColumnName);
		ret = sqlite3_exec (sqlite, trigger, NULL, NULL, &errMsg);
		if (ret != SQLITE_OK)
		    goto error;
	    }
	  curr_idx = curr_idx->Next;
      }
    goto index_cleanup;
  error:
    fprintf (stderr, "updateTableTriggers: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
  index_cleanup:
    curr_idx = first_idx;
    while (curr_idx)
      {
	  next_idx = curr_idx->Next;
	  if (curr_idx->TableName)
	      free (curr_idx->TableName);
	  if (curr_idx->ColumnName)
	      free (curr_idx->ColumnName);
	  free (curr_idx);
	  curr_idx = next_idx;
      }
}

static void
fnct_AddGeometryColumn (sqlite3_context * context, int argc,
			sqlite3_value ** argv)
{
/* SQL function:
/ AddGeometryColumn(table, column, srid, type , dimension  [  , not-null ]  )
/
/ creates a new COLUMN of given TYPE into TABLE
/ returns 1 on success
/ 0 on failure
*/
    const unsigned char *table;
    const unsigned char *column;
    const unsigned char *type;
    int xtype;
    int srid = -1;
    int dimension = 2;
    char dummy[32];
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    char **results;
    int rows;
    int columns;
    int i;
    char tblname[256];
    int notNull = 0;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "AddGeometryColumn() error: argument 1 [table_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    table = sqlite3_value_text (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "AddGeometryColumn() error: argument 2 [column_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    column = sqlite3_value_text (argv[1]);
    if (sqlite3_value_type (argv[2]) != SQLITE_INTEGER)
      {
	  fprintf (stderr,
		   "AddGeometryColumn() error: argument 3 [SRID] is not of the Integer type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    srid = sqlite3_value_int (argv[2]);
    if (sqlite3_value_type (argv[3]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "AddGeometryColumn() error: argument 4 [geometry_type] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    type = sqlite3_value_text (argv[3]);
    if (sqlite3_value_type (argv[4]) != SQLITE_INTEGER)
      {
	  fprintf (stderr,
		   "AddGeometryColumn() error: argument 5 [dimension] is not of the Integer type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    dimension = sqlite3_value_int (argv[4]);
    if (argc > 5)
      {
	  /* optional NOT NULL arg */
	  if (sqlite3_value_type (argv[5]) != SQLITE_INTEGER)
	    {
		fprintf (stderr,
			 "AddGeometryColumn() error: argument 6 [not null] is not of the Integer type\n");
		sqlite3_result_int (context, 0);
		return;
	    }
	  notNull = sqlite3_value_int (argv[5]);
      }
    xtype = GAIA_UNKNOWN;
    if (strcasecmp ((char *) type, "POINT") == 0)
	xtype = GAIA_POINT;
    if (strcasecmp ((char *) type, "LINESTRING") == 0)
	xtype = GAIA_LINESTRING;
    if (strcasecmp ((char *) type, "POLYGON") == 0)
	xtype = GAIA_POLYGON;
    if (strcasecmp ((char *) type, "MULTIPOINT") == 0)
	xtype = GAIA_MULTIPOINT;
    if (strcasecmp ((char *) type, "MULTILINESTRING") == 0)
	xtype = GAIA_MULTILINESTRING;
    if (strcasecmp ((char *) type, "MULTIPOLYGON") == 0)
	xtype = GAIA_MULTIPOLYGON;
    if (strcasecmp ((char *) type, "GEOMETRYCOLLECTION") == 0)
	xtype = GAIA_GEOMETRYCOLLECTION;
    if (strcasecmp ((char *) type, "GEOMETRY") == 0)
	xtype = -1;
    if (xtype == GAIA_UNKNOWN)
      {
	  fprintf (stderr,
		   "AddGeometryColumn() error: argument 3 [geometry_type] has an illegal value\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    if (dimension != 2)
      {
	  fprintf (stderr,
		   "AddGeometryColumn() error: argument 5 [dimension] current version only accepts dimension=2\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
/* checking if the table exists */
    sprintf (sql,
	     "SELECT name FROM sqlite_master WHERE type = 'table' AND name LIKE '%s'",
	     table);
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, &errMsg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddGeometryColumn: \"%s\"\n", errMsg);
	  sqlite3_free (errMsg);
	  return;
      }
    *tblname = '\0';
    for (i = 1; i <= rows; i++)
      {
	  /* preparing the triggers */
	  strcpy (tblname, results[(i * columns)]);
      }
    sqlite3_free_table (results);
    if (*tblname == '\0')
      {
	  fprintf (stderr,
		   "AddGeometryColumn() error: table '%s' does not exists\n",
		   table);
	  sqlite3_result_int (context, 0);
	  return;
      }
/* trying to add the column */
    strcpy (sql, "ALTER TABLE \"");
    strcat (sql, (char *) table);
    strcat (sql, "\" ADD COLUMN \"");
    strcat (sql, (char *) column);
    strcat (sql, "\" ");
    switch (xtype)
      {
      case GAIA_POINT:
	  strcat (sql, "POINT");
	  break;
      case GAIA_LINESTRING:
	  strcat (sql, "LINESTRING");
	  break;
      case GAIA_POLYGON:
	  strcat (sql, "POLYGON");
	  break;
      case GAIA_MULTIPOINT:
	  strcat (sql, "MULTIPOINT");
	  break;
      case GAIA_MULTILINESTRING:
	  strcat (sql, "MULTILINESTRING");
	  break;
      case GAIA_MULTIPOLYGON:
	  strcat (sql, "MULTIPOLYGON");
	  break;
      case GAIA_GEOMETRYCOLLECTION:
	  strcat (sql, "GEOMETRYCOLLECTION");
	  break;
      case -1:
	  strcat (sql, "GEOMETRY");
	  break;
      };
    if (notNull)
      {
	  /* adding a NOT NULL clause */
	  strcat (sql, " NOT NULL DEFAULT ''");
      }
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/*ok, inserting into geometry_columns [Spatial Metadata] */
    strcpy (sql,
	    "INSERT INTO geometry_columns (f_table_name, f_geometry_column, type, ");
    strcat (sql, "coord_dimension, srid, spatial_index_enabled) VALUES (");
    strcat (sql, "'");
    strcat (sql, (char *) tblname);
    strcat (sql, "', '");
    strcat (sql, (char *) column);
    strcat (sql, "', '");
    switch (xtype)
      {
      case GAIA_POINT:
	  strcat (sql, "POINT");
	  break;
      case GAIA_LINESTRING:
	  strcat (sql, "LINESTRING");
	  break;
      case GAIA_POLYGON:
	  strcat (sql, "POLYGON");
	  break;
      case GAIA_MULTIPOINT:
	  strcat (sql, "MULTIPOINT");
	  break;
      case GAIA_MULTILINESTRING:
	  strcat (sql, "MULTILINESTRING");
	  break;
      case GAIA_MULTIPOLYGON:
	  strcat (sql, "MULTIPOLYGON");
	  break;
      case GAIA_GEOMETRYCOLLECTION:
	  strcat (sql, "GEOMETRYCOLLECTION");
	  break;
      case -1:
	  strcat (sql, "GEOMETRY");
	  break;
      };
    strcat (sql, "', 2, ");
    if (srid <= 0)
	strcat (sql, "-1");
    else
      {
	  sprintf (dummy, "%d", srid);
	  strcat (sql, dummy);
      }
    strcat (sql, ", 0)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    updateGeometryTriggers (sqlite, table, column);
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "AddGeometryColumn() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static void
fnct_RecoverGeometryColumn (sqlite3_context * context, int argc,
			    sqlite3_value ** argv)
{
/* SQL function:
/ RecoverGeometryColumn(table, column, srid, type , dimension )
/
/ checks if an existing TABLE.COLUMN satisfies the required geometric features
/ if yes adds it to SpatialMetaData and enabling triggers
/ returns 1 on success
/ 0 on failure
*/
    const unsigned char *table;
    const unsigned char *column;
    const unsigned char *type;
    int xtype;
    int srid = -1;
    int dimension = 2;
    char dummy[32];
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    char **results;
    int rows;
    int columns;
    int i;
    char tblname[256];
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "RecoverGeometryColumn() error: argument 1 [table_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    table = sqlite3_value_text (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "RecoverGeometryColumn() error: argument 2 [column_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    column = sqlite3_value_text (argv[1]);
    if (sqlite3_value_type (argv[2]) != SQLITE_INTEGER)
      {
	  fprintf (stderr,
		   "RecoverGeometryColumn() error: argument 3 [SRID] is not of the Integer type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    srid = sqlite3_value_int (argv[2]);
    if (sqlite3_value_type (argv[3]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "RecoverGeometryColumn() error: argument 4 [geometry_type] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    type = sqlite3_value_text (argv[3]);
    if (sqlite3_value_type (argv[4]) != SQLITE_INTEGER)
      {
	  fprintf (stderr,
		   "RecoverGeometryColumn() error: argument 5 [dimension] is not of the Integer type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    dimension = sqlite3_value_int (argv[4]);
    xtype = GAIA_UNKNOWN;
    if (strcasecmp ((char *) type, "POINT") == 0)
	xtype = GAIA_POINT;
    if (strcasecmp ((char *) type, "LINESTRING") == 0)
	xtype = GAIA_LINESTRING;
    if (strcasecmp ((char *) type, "POLYGON") == 0)
	xtype = GAIA_POLYGON;
    if (strcasecmp ((char *) type, "MULTIPOINT") == 0)
	xtype = GAIA_MULTIPOINT;
    if (strcasecmp ((char *) type, "MULTILINESTRING") == 0)
	xtype = GAIA_MULTILINESTRING;
    if (strcasecmp ((char *) type, "MULTIPOLYGON") == 0)
	xtype = GAIA_MULTIPOLYGON;
    if (strcasecmp ((char *) type, "GEOMETRYCOLLECTION") == 0)
	xtype = GAIA_GEOMETRYCOLLECTION;
    if (strcasecmp ((char *) type, "GEOMETRY") == 0)
	xtype = -1;
    if (xtype == GAIA_UNKNOWN)
      {
	  fprintf (stderr,
		   "RecoverGeometryColumn() error: argument 3 [geometry_type] has an illegal value\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    if (dimension != 2)
      {
	  fprintf (stderr,
		   "RecoverGeometryColumn() error: argument 5 [dimension] current version only accepts dimension=2\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
/* checking if the table exists */
    sprintf (sql,
	     "SELECT name FROM sqlite_master WHERE type = 'table' AND name LIKE '%s'",
	     table);
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, &errMsg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "RecoverGeometryColumn: \"%s\"\n", errMsg);
	  sqlite3_free (errMsg);
	  return;
      }
    *tblname = '\0';
    for (i = 1; i <= rows; i++)
      {
	  /* preparing the triggers */
	  strcpy (tblname, results[(i * columns)]);
      }
    sqlite3_free_table (results);
    if (*tblname == '\0')
      {
	  fprintf (stderr,
		   "RecoverGeometryColumn() error: table '%s' does not exists\n",
		   table);
	  sqlite3_result_int (context, 0);
	  return;
      }
    if (!recoverGeomColumn (sqlite, table, column, xtype, srid))
      {
	  fprintf (stderr, "RecoverGeometryColumn(): validation failed\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    strcpy (sql,
	    "INSERT INTO geometry_columns (f_table_name, f_geometry_column, type, ");
    strcat (sql, "coord_dimension, srid, spatial_index_enabled) VALUES (");
    strcat (sql, "'");
    strcat (sql, (char *) tblname);
    strcat (sql, "', '");
    strcat (sql, (char *) column);
    strcat (sql, "', '");
    switch (xtype)
      {
      case GAIA_POINT:
	  strcat (sql, "POINT");
	  break;
      case GAIA_LINESTRING:
	  strcat (sql, "LINESTRING");
	  break;
      case GAIA_POLYGON:
	  strcat (sql, "POLYGON");
	  break;
      case GAIA_MULTIPOINT:
	  strcat (sql, "MULTIPOINT");
	  break;
      case GAIA_MULTILINESTRING:
	  strcat (sql, "MULTILINESTRING");
	  break;
      case GAIA_MULTIPOLYGON:
	  strcat (sql, "MULTIPOLYGON");
	  break;
      case GAIA_GEOMETRYCOLLECTION:
	  strcat (sql, "GEOMETRYCOLLECTION");
	  break;
      case -1:
	  strcat (sql, "GEOMETRY");
	  break;
      };
    strcat (sql, "', 2, ");
    if (srid <= 0)
	strcat (sql, "-1");
    else
      {
	  sprintf (dummy, "%d", srid);
	  strcat (sql, dummy);
      }
    strcat (sql, ", 0)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    updateGeometryTriggers (sqlite, table, column);
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "RecoverGeometryColumn() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static void
fnct_DiscardGeometryColumn (sqlite3_context * context, int argc,
			    sqlite3_value ** argv)
{
/* SQL function:
/ DiscardGeometryColumn(table, column)
/
/ removes TABLE.COLUMN from the Spatial MetaData [thus disablig triggers too]
/ returns 1 on success
/ 0 on failure
*/
    const unsigned char *table;
    const unsigned char *column;
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "DiscardGeometryColumn() error: argument 1 [table_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    table = sqlite3_value_text (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "DiscardGeometryColumn() error: argument 2 [column_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    column = sqlite3_value_text (argv[1]);
    sprintf (sql,
	     "DELETE FROM geometry_columns WHERE f_table_name LIKE '%s' AND f_geometry_column LIKE '%s'",
	     (char *) table, (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/* removing triggers too */
    sprintf (sql,
	     "DROP TRIGGER IF EXISTS \"ggi_%s_%s\"",
	     (char *) table, (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql,
	     "DROP TRIGGER IF EXISTS \"ggu_%s_%s\"",
	     (char *) table, (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql,
	     "DROP TRIGGER IF EXISTS \"gii_%s_%s\"",
	     (char *) table, (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql,
	     "DROP TRIGGER IF EXISTS \"giu_%s_%s\"",
	     (char *) table, (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql,
	     "DROP TRIGGER IF EXISTS \"gid_%s_%s\"",
	     (char *) table, (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql,
	     "DROP TRIGGER IF EXISTS \"gci_%s_%s\"",
	     (char *) table, (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql,
	     "DROP TRIGGER IF EXISTS \"gcu_%s_%s\"",
	     (char *) table, (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql,
	     "DROP TRIGGER IF EXISTS \"gcd_%s_%s\"",
	     (char *) table, (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;

    /* trying to delete old versions [v2.0, v2.2] triggers[if any] */
    sprintf (sql, "DROP TRIGGER IF EXISTS \"gti_%s_%s\"", table, column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql, "DROP TRIGGER IF EXISTS \"gtu_%s_%s\"", table, column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql, "DROP TRIGGER IF EXISTS \"gsi_%s_%s\"", table, column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sprintf (sql, "DROP TRIGGER IF EXISTS \"gsu_%s_%s\"", table, column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    /* end deletion old versions [v2.0, v2.2] triggers[if any] */

    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "DiscardGeometryColumn() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static void
fnct_InitFDOSpatialMetaData (sqlite3_context * context, int argc,
			     sqlite3_value ** argv)
{
/* SQL function:
/ InitFDOSpatialMetaData(void)
/
/ creates the FDO-styled SPATIAL_REF_SYS and GEOMETRY_COLUMNS tables
/ returns 1 on success
/ 0 on failure
*/
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
/* creating the SPATIAL_REF_SYS tables */
    strcpy (sql, "CREATE TABLE spatial_ref_sys (\n");
    strcat (sql, "srid INTEGER PRIMARY KEY,\n");
    strcat (sql, "auth_name TEXT,\n");
    strcat (sql, "auth_srid INTEGER,\n");
    strcat (sql, "srtext TEXT)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/* creating the GEOMETRY_COLUMN tables */
    strcpy (sql, "CREATE TABLE geometry_columns (\n");
    strcat (sql, "f_table_name TEXT,\n");
    strcat (sql, "f_geometry_column TEXT,\n");
    strcat (sql, "geometry_type INTEGER,\n");
    strcat (sql, "coord_dimension INTEGER,\n");
    strcat (sql, "srid INTEGER,\n");
    strcat (sql, "geometry_format TEXT)");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "InitFDOSpatiaMetaData() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static int
recoverFDOGeomColumn (sqlite3 * sqlite, const unsigned char *table,
		      const unsigned char *column, int xtype, int srid,
		      const unsigned char *format)
{
/* checks if TABLE.COLUMN exists and has the required features */
    int ok = 1;
    char sql[1024];
    int type;
    sqlite3_stmt *stmt;
    gaiaGeomCollPtr geom;
    const void *blob_value;
    int len;
    int ret;
    int i_col;
    sprintf (sql, "SELECT \"%s\" FROM \"%s\"", column, table);
/* compiling SQL prepared statement */
    ret = sqlite3_prepare_v2 (sqlite, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "recoverFDOGeomColumn: error %d \"%s\"\n",
		   sqlite3_errcode (sqlite), sqlite3_errmsg (sqlite));
	  return 0;
      }
    while (1)
      {
	  /* scrolling the result set rows */
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;		/* end of result set */
	  if (ret == SQLITE_ROW)
	    {
		/* cecking Geometry features */
		geom = NULL;
		for (i_col = 0; i_col < sqlite3_column_count (stmt); i_col++)
		  {
		      if (sqlite3_column_type (stmt, i_col) != SQLITE_BLOB)
			  ok = 0;
		      else
			{
			    blob_value = sqlite3_column_blob (stmt, i_col);
			    len = sqlite3_column_bytes (stmt, i_col);
			    geom = gaiaFromSpatiaLiteBlobWkb (blob_value, len);
			    if (!geom)
				ok = 0;
			    else
			      {
				  if (geom->Srid != srid)
				      ok = 0;
				  type = gaiaGeometryType (geom);
				  if (xtype == type)
				      ;
				  else
				      ok = 0;
				  gaiaFreeGeomColl (geom);
			      }
			}
		  }
	    }
	  if (!ok)
	      break;
      }
    ret = sqlite3_finalize (stmt);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "recoverFDOGeomColumn: error %d \"%s\"\n",
		   sqlite3_errcode (sqlite), sqlite3_errmsg (sqlite));
	  return 0;
      }
    return ok;
}

static void
fnct_AddFDOGeometryColumn (sqlite3_context * context, int argc,
			   sqlite3_value ** argv)
{
/* SQL function:
/ AddFDOGeometryColumn(table, column, srid, geometry_type , dimension, geometry_format )
/
/ creates a new COLUMN of given TYPE into TABLE
/ returns 1 on success
/ 0 on failure
*/
    const char *table;
    const char *column;
    const char *format;
    char xformat[64];
    int type;
    int srid = -1;
    int dimension = 2;
    char dummy[32];
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    char **results;
    int rows;
    int columns;
    int i;
    char tblname[256];
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "AddFDOGeometryColumn() error: argument 1 [table_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    table = (const char *) sqlite3_value_text (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "AddFDOGeometryColumn() error: argument 2 [column_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    column = (const char *) sqlite3_value_text (argv[1]);
    if (sqlite3_value_type (argv[2]) != SQLITE_INTEGER)
      {
	  fprintf (stderr,
		   "AddFDOGeometryColumn() error: argument 3 [SRID] is not of the Integer type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    srid = sqlite3_value_int (argv[2]);
    if (sqlite3_value_type (argv[3]) != SQLITE_INTEGER)
      {
	  fprintf (stderr,
		   "AddFDOGeometryColumn() error: argument 4 [geometry_type] is not of the Integer type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    type = sqlite3_value_int (argv[3]);
    if (sqlite3_value_type (argv[4]) != SQLITE_INTEGER)
      {
	  fprintf (stderr,
		   "AddFDOGeometryColumn() error: argument 5 [dimension] is not of the Integer type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    dimension = sqlite3_value_int (argv[4]);
    if (sqlite3_value_type (argv[5]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "AddFDOGeometryColumn() error: argument 6 [geometry_format] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    format = (const char *) sqlite3_value_text (argv[5]);
    if (type ==
	GAIA_POINT
	|| type ==
	GAIA_LINESTRING
	|| type ==
	GAIA_POLYGON
	|| type ==
	GAIA_MULTIPOINT
	|| type ==
	GAIA_MULTILINESTRING
	|| type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	;
    else
      {
	  fprintf (stderr,
		   "AddFDOGeometryColumn() error: argument 4 [geometry_type] has an illegal value\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    if (dimension < 2 || dimension > 4)
      {
	  fprintf (stderr,
		   "AddFDOGeometryColumn() error: argument 5 [dimension] current version only accepts dimension=2,3,4\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    if (strcasecmp (format, "WKT") == 0)
	strcpy (xformat, "WKT");
    else if (strcasecmp (format, "WKB") == 0)
	strcpy (xformat, "WKB");
    else if (strcasecmp (format, "FGF") == 0)
	strcpy (xformat, "FGF");
    else
      {
	  fprintf (stderr,
		   "AddFDOGeometryColumn() error: argument 6 [geometry_format] has to be one of: WKT,WKB,FGF\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
/* checking if the table exists */
    sprintf (sql,
	     "SELECT name FROM sqlite_master WHERE type = 'table' AND name LIKE '%s'",
	     table);
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, &errMsg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "AddFDOGeometryColumn: \"%s\"\n", errMsg);
	  sqlite3_free (errMsg);
	  return;
      }
    *tblname = '\0';
    for (i = 1; i <= rows; i++)
      {
	  strcpy (tblname, results[(i * columns)]);
      }
    sqlite3_free_table (results);
    if (*tblname == '\0')
      {
	  fprintf (stderr,
		   "AddFDOGeometryColumn() error: table '%s' does not exists\n",
		   table);
	  sqlite3_result_int (context, 0);
	  return;
      }
/* trying to add the column */
    strcpy (sql, "ALTER TABLE ");
    strcat (sql, (char *) table);
    strcat (sql, " ADD COLUMN ");
    strcat (sql, (char *) column);
    strcat (sql, " BLOB");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
/*ok, inserting into geometry_columns [FDO Spatial Metadata] */
    strcpy (sql,
	    "INSERT INTO geometry_columns (f_table_name, f_geometry_column, geometry_type, ");
    strcat (sql, "coord_dimension, srid, geometry_format) VALUES (");
    strcat (sql, "'");
    strcat (sql, (char *) tblname);
    strcat (sql, "', '");
    strcat (sql, (char *) column);
    strcat (sql, "', ");
    sprintf (dummy, "%d, %d, ", type, dimension);
    strcat (sql, dummy);
    if (srid <= 0)
	strcat (sql, "-1");
    else
      {
	  sprintf (dummy, "%d", srid);
	  strcat (sql, dummy);
      }
    strcat (sql, ", '");
    strcat (sql, xformat);
    strcat (sql, "')");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "AddFDOGeometryColumn() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static void
fnct_RecoverFDOGeometryColumn (sqlite3_context * context, int argc,
			       sqlite3_value ** argv)
{
/* SQL function:
/ RecoverFDOGeometryColumn(table, column, srid, geometry_type , dimension, geometry_format )
/
/ checks if an existing TABLE.COLUMN satisfies the required geometric features
/ if yes adds it to FDO-styled SpatialMetaData 
/ returns 1 on success
/ 0 on failure
*/
    const char *table;
    const char *column;
    const char *format;
    char xformat[64];
    int type;
    int srid = -1;
    int dimension = 2;
    char dummy[32];
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    char **results;
    int rows;
    int columns;
    int i;
    char tblname[256];
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "RecoverFDOGeometryColumn() error: argument 1 [table_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    table = (const char *) sqlite3_value_text (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "RecoverFDOGeometryColumn() error: argument 2 [column_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    column = (const char *) sqlite3_value_text (argv[1]);
    if (sqlite3_value_type (argv[2]) != SQLITE_INTEGER)
      {
	  fprintf (stderr,
		   "RecoverFDOGeometryColumn() error: argument 3 [SRID] is not of the Integer type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    srid = sqlite3_value_int (argv[2]);
    if (sqlite3_value_type (argv[3]) != SQLITE_INTEGER)
      {
	  fprintf (stderr,
		   "RecoverFDOGeometryColumn() error: argument 4 [geometry_type] is not of the Integer type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    type = sqlite3_value_int (argv[3]);
    if (sqlite3_value_type (argv[4]) != SQLITE_INTEGER)
      {
	  fprintf (stderr,
		   "RecoverFDOGeometryColumn() error: argument 5 [dimension] is not of the Integer type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    dimension = sqlite3_value_int (argv[4]);
    if (sqlite3_value_type (argv[5]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "RecoverFDOGeometryColumn() error: argument 6 [geometry_format] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    format = (const char *) sqlite3_value_text (argv[5]);
    if (type ==
	GAIA_POINT
	|| type ==
	GAIA_LINESTRING
	|| type ==
	GAIA_POLYGON
	|| type ==
	GAIA_MULTIPOINT
	|| type ==
	GAIA_MULTILINESTRING
	|| type == GAIA_MULTIPOLYGON || type == GAIA_GEOMETRYCOLLECTION)
	;
    else
      {
	  fprintf (stderr,
		   "RecoverFDOGeometryColumn() error: argument 4 [geometry_type] has an illegal value\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    if (dimension < 2 || dimension > 4)
      {
	  fprintf (stderr,
		   "RecoverFDOGeometryColumn() error: argument 5 [dimension] current version only accepts dimension=2,3,4\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    if (strcasecmp (format, "WKT") == 0)
	strcpy (xformat, "WKT");
    else if (strcasecmp (format, "WKB") == 0)
	strcpy (xformat, "WKB");
    else if (strcasecmp (format, "FGF") == 0)
	strcpy (xformat, "FGF");
    else
      {
	  fprintf (stderr,
		   "RecoverFDOGeometryColumn() error: argument 6 [geometry_format] has to be one of: WKT,WKB,FGF\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
/* checking if the table exists */
    sprintf (sql,
	     "SELECT name FROM sqlite_master WHERE type = 'table' AND name LIKE '%s'",
	     table);
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, &errMsg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "RecoverFDOGeometryColumn: \"%s\"\n", errMsg);
	  sqlite3_free (errMsg);
	  return;
      }
    *tblname = '\0';
    for (i = 1; i <= rows; i++)
      {
	  strcpy (tblname, results[(i * columns)]);
      }
    sqlite3_free_table (results);
    if (*tblname == '\0')
      {
	  fprintf (stderr,
		   "RecoverFDOGeometryColumn() error: table '%s' does not exists\n",
		   table);
	  sqlite3_result_int (context, 0);
	  return;
      }
    if (!recoverFDOGeomColumn
	(sqlite, (const unsigned char *) table, (const unsigned char *) column,
	 type, srid, (const unsigned char *) xformat))
      {
	  fprintf (stderr, "RecoverFDOGeometryColumn(): validation failed\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    strcpy (sql,
	    "INSERT INTO geometry_columns (f_table_name, f_geometry_column, geometry_type, ");
    strcat (sql, "coord_dimension, srid, geometry_format) VALUES (");
    strcat (sql, "'");
    strcat (sql, (char *) tblname);
    strcat (sql, "', '");
    strcat (sql, (char *) column);
    strcat (sql, "', ");
    sprintf (dummy, "%d, %d, ", type, dimension);
    strcat (sql, dummy);
    if (srid <= 0)
	strcat (sql, "-1");
    else
      {
	  sprintf (dummy, "%d", srid);
	  strcat (sql, dummy);
      }
    strcat (sql, ", '");
    strcat (sql, xformat);
    strcat (sql, "')");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "RecoverFDOGeometryColumn() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static void
fnct_DiscardFDOGeometryColumn (sqlite3_context * context, int argc,
			       sqlite3_value ** argv)
{
/* SQL function:
/ DiscardFDOGeometryColumn(table, column)
/
/ removes TABLE.COLUMN from the Spatial MetaData
/ returns 1 on success
/ 0 on failure
*/
    const unsigned char *table;
    const unsigned char *column;
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "DiscardFDOGeometryColumn() error: argument 1 [table_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    table = sqlite3_value_text (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "DiscardFDOGeometryColumn() error: argument 2 [column_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    column = sqlite3_value_text (argv[1]);
    sprintf (sql,
	     "DELETE FROM geometry_columns WHERE f_table_name LIKE '%s' AND f_geometry_column LIKE '%s'",
	     (char *) table, (char *) column);
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "DiscardFDOGeometryColumn() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static void
fnct_CreateSpatialIndex (sqlite3_context * context, int argc,
			 sqlite3_value ** argv)
{
/* SQL function:
/ CreateSpatialIndex(table, column )
/
/ creates a SpatialIndex based on Column and Table
/ returns 1 on success
/ 0 on failure
*/
    const unsigned char *table;
    const unsigned char *column;
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "CreateSpatialIndex() error: argument 1 [table_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    table = sqlite3_value_text (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "CreateSpatialIndex() error: argument 2 [column_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    column = sqlite3_value_text (argv[1]);
    strcpy (sql,
	    "UPDATE geometry_columns SET spatial_index_enabled = 1 WHERE f_table_name LIKE '");
    strcat (sql, (char *) table);
    strcat (sql, "' AND f_geometry_column LIKE '");
    strcat (sql, (char *) column);
    strcat (sql, "' AND spatial_index_enabled = 0");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    if (sqlite3_changes (sqlite) == 0)
      {
	  fprintf (stderr,
		   "CreateSpatialIndex() error: either \"%s\".\"%s\" isn't a Geometry column or a SpatialIndex is already defined\n",
		   table, column);
	  sqlite3_result_int (context, 0);
	  return;
      }
    updateGeometryTriggers (sqlite, table, column);
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "CreateSpatialIndex() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static void
fnct_CreateMbrCache (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ CreateMbrCache(table, column )
/
/ creates an MBR Cache based on Column and Table
/ returns 1 on success
/ 0 on failure
*/
    const unsigned char *table;
    const unsigned char *column;
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "CreateMbrCache() error: argument 1 [table_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    table = sqlite3_value_text (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "CreateMbrCache() error: argument 2 [column_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    column = sqlite3_value_text (argv[1]);
    strcpy (sql,
	    "UPDATE geometry_columns SET spatial_index_enabled = 2 WHERE f_table_name LIKE '");
    strcat (sql, (char *) table);
    strcat (sql, "' AND f_geometry_column LIKE '");
    strcat (sql, (char *) column);
    strcat (sql, "' AND spatial_index_enabled = 0");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    if (sqlite3_changes (sqlite) == 0)
      {
	  fprintf (stderr,
		   "CreateMbrCache() error: either \"%s\".\"%s\" isn't a Geometry column or a SpatialIndex is already defined\n",
		   table, column);
	  sqlite3_result_int (context, 0);
	  return;
      }
    updateGeometryTriggers (sqlite, table, column);
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "CreateMbrCache() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static void
fnct_DisableSpatialIndex (sqlite3_context * context, int argc,
			  sqlite3_value ** argv)
{
/* SQL function:
/ DisableSpatialIndex(table, column )
/
/ disables a SpatialIndex based on Column and Table
/ returns 1 on success
/ 0 on failure
*/
    const unsigned char *table;
    const unsigned char *column;
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "DisableSpatialIndex() error: argument 1 [table_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    table = sqlite3_value_text (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "DisableSpatialIndex() error: argument 2 [column_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    column = sqlite3_value_text (argv[1]);
    strcpy (sql,
	    "UPDATE geometry_columns SET spatial_index_enabled = 0 WHERE f_table_name LIKE '");
    strcat (sql, (char *) table);
    strcat (sql, "' AND f_geometry_column LIKE '");
    strcat (sql, (char *) column);
    strcat (sql, "' AND spatial_index_enabled <> 0");
    ret = sqlite3_exec (sqlite, sql, NULL, NULL, &errMsg);
    if (ret != SQLITE_OK)
	goto error;
    if (sqlite3_changes (sqlite) == 0)
      {
	  fprintf (stderr,
		   "DisableSpatialIndex() error: either \"%s\".\"%s\" isn't a Geometry column or no SpatialIndex is defined\n",
		   table, column);
	  sqlite3_result_int (context, 0);
	  return;
      }
    updateGeometryTriggers (sqlite, table, column);
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "DisableSpatialIndex() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static void
fnct_RebuildGeometryTriggers (sqlite3_context * context, int argc,
			      sqlite3_value ** argv)
{
/* SQL function:
/ RebuildGeometryTriggers(table, column )
/
/ rebuilds Geometry Triggers (constraints)  based on Column and Table
/ returns 1 on success
/ 0 on failure
*/
    const unsigned char *table;
    const unsigned char *column;
    char sql[1024];
    char *errMsg = NULL;
    int ret;
    char **results;
    int rows;
    int columns;
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "RebuildGeometryTriggers() error: argument 1 [table_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    table = sqlite3_value_text (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_TEXT)
      {
	  fprintf (stderr,
		   "RebuildGeometryTriggers() error: argument 2 [column_name] is not of the String type\n");
	  sqlite3_result_int (context, 0);
	  return;
      }
    column = sqlite3_value_text (argv[1]);
    strcpy (sql,
	    "SELECT f_table_name FROM geometry_columns WHERE f_table_name LIKE '");
    strcat (sql, (char *) table);
    strcat (sql, "' AND f_geometry_column LIKE '");
    strcat (sql, (char *) column);
    strcat (sql, "'");
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, NULL);
    if (ret != SQLITE_OK)
	goto error;
    sqlite3_free_table (results);
    if (rows <= 0)
      {
	  fprintf (stderr,
		   "RebuildGeometryTriggers() error: \"%s\".\"%s\" isn't a Geometry column\n",
		   table, column);
	  sqlite3_result_int (context, 0);
	  return;
      }
    updateGeometryTriggers (sqlite, table, column);
    sqlite3_result_int (context, 1);
    return;
  error:
    fprintf (stderr, "RebuildGeometryTriggers() error: \"%s\"\n", errMsg);
    sqlite3_free (errMsg);
    sqlite3_result_int (context, 0);
    return;
}

static gaiaPointPtr
simplePoint (gaiaGeomCollPtr geo)
{
/* helper function
/ if this GEOMETRY contains only one POINT, and no other elementary geometry
/ the POINT address will be returned
/ otherwise NULL will be returned
*/
    int cnt = 0;
    gaiaPointPtr point;
    gaiaPointPtr this_point = NULL;
    if (!geo)
	return NULL;
    if (geo->FirstLinestring || geo->FirstPolygon)
	return NULL;
    point = geo->FirstPoint;
    while (point)
      {
	  /* counting how many POINTs are there */
	  cnt++;
	  this_point = point;
	  point = point->Next;
      }
    if (cnt == 1 && this_point)
	return this_point;
    return NULL;
}

static gaiaLinestringPtr
simpleLinestring (gaiaGeomCollPtr geo)
{
/* helper function
/ if this GEOMETRY contains only one LINESTRING, and no other elementary geometry
/ the LINESTRING address will be returned
/ otherwise NULL will be returned
*/
    int cnt = 0;
    gaiaLinestringPtr line;
    gaiaLinestringPtr this_line = NULL;
    if (!geo)
	return NULL;
    if (geo->FirstPoint || geo->FirstPolygon)
	return NULL;
    line = geo->FirstLinestring;
    while (line)
      {
	  /* counting how many LINESTRINGs are there */
	  cnt++;
	  this_line = line;
	  line = line->Next;
      }
    if (cnt == 1 && this_line)
	return this_line;
    return NULL;
}

static gaiaPolygonPtr
simplePolygon (gaiaGeomCollPtr geo)
{
/* helper function
/ if this GEOMETRY contains only one POLYGON, and no other elementary geometry
/ the POLYGON address will be returned
/ otherwise NULL will be returned
*/
    int cnt = 0;
    gaiaPolygonPtr polyg;
    gaiaPolygonPtr this_polyg = NULL;
    if (!geo)
	return NULL;
    if (geo->FirstPoint || geo->FirstLinestring)
	return NULL;
    polyg = geo->FirstPolygon;
    while (polyg)
      {
	  /* counting how many POLYGONs are there */
	  cnt++;
	  this_polyg = polyg;
	  polyg = polyg->Next;
      }
    if (cnt == 1 && this_polyg)
	return this_polyg;
    return NULL;
}

static void
fnct_AsText (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ AsText(BLOB encoded geometry)
/
/ returns the corresponding WKT encoded value
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaOutWkt (geo, &p_result);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	    {
		len = strlen (p_result);
		//if (len > 65536)
		//sqlite3_result_error_toobig(context);
		//else
		sqlite3_result_text (context, p_result, len, free);
	    }
      }
    gaiaFreeGeomColl (geo);
}

/*
/
/ AsSvg(geometry,[relative], [precision]) implementation
/
////////////////////////////////////////////////////////////
/
/ Author: Klaus Foerster klaus.foerster@svg.cc
/ version 0.9. 2008 September 21
 /
 */

static void
fnct_AsSvg (sqlite3_context * context, int argc, sqlite3_value ** argv,
	    int relative, int precision)
{
/* SQL function:
   AsSvg(BLOB encoded geometry, [int relative], [int precision])
   returns the corresponding SVG encoded value or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  /* make sure relative is 0 or 1 */
	  if (relative > 0)
	      relative = 1;
	  else
	      relative = 0;
	  /* make sure precision is between 0 and 15 - default to 6 if absent */
	  if (precision > GAIA_SVG_DEFAULT_MAX_PRECISION)
	      precision = GAIA_SVG_DEFAULT_MAX_PRECISION;
	  if (precision < 0)
	      precision = 0;
	  /* produce SVG-notation - actual work is done in gaiageo/gg_wkt.c */
	  gaiaOutSvg (geo, &p_result, relative, precision);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	    {
		len = strlen (p_result);
		sqlite3_result_text (context, p_result, len, free);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_AsSvg1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* called without additional arguments */
    fnct_AsSvg (context, argc, argv, GAIA_SVG_DEFAULT_RELATIVE,
		GAIA_SVG_DEFAULT_PRECISION);
}

static void
fnct_AsSvg2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* called with relative-switch */
    if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
	fnct_AsSvg (context, argc, argv, sqlite3_value_int (argv[1]),
		    GAIA_SVG_DEFAULT_PRECISION);
    else
	sqlite3_result_null (context);
}

static void
fnct_AsSvg3 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* called with relative-switch and precision-argument */
    if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER
	&& sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
	fnct_AsSvg (context, argc, argv, sqlite3_value_int (argv[1]),
		    sqlite3_value_int (argv[2]));
    else
	sqlite3_result_null (context);
}

/* END of Klaus Foerster AsSvg() implementation */

static void
fnct_AsBinary (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ AsBinary(BLOB encoded geometry)
/
/ returns the corresponding WKB encoded value
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaToWkb (geo, &p_result, &len);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_blob (context, p_result, len, free);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_AsFGF (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ AsFGF(BLOB encoded geometry)
/
/ returns the corresponding FGF encoded value
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    int coord_dims;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  fprintf (stderr,
		   "AsFGF() error: argument 2 [geom_coords] is not of the Integer type\n");
	  sqlite3_result_null (context);
	  return;
      }
    coord_dims = sqlite3_value_int (argv[1]);
    if (coord_dims
	== 0 || coord_dims == 1 || coord_dims == 2 || coord_dims == 3)
	;
    else
      {
	  fprintf (stderr,
		   "AsFGF() error: argument 2 [geom_coords] out of range [0,1,2,3]\n");
	  sqlite3_result_null (context);
	  return;
      }
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaToFgf (geo, &p_result, &len, coord_dims);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_blob (context, p_result, len, free);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_MakePoint1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ MakePoint(double X, double Y)
/
/ builds a POINT 
/ or NULL if any error is encountered
*/
    int len;
    int int_value;
    unsigned char *p_result = NULL;
    double x;
    double y;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	y = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  y = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    gaiaMakePoint (x, y, -1, &p_result, &len);
    if (!p_result)
	sqlite3_result_null (context);
    else
	sqlite3_result_blob (context, p_result, len, free);
}

static void
fnct_MakePoint2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ MakePoint(double X, double Y, int SRID)
/
/ builds a POINT 
/ or NULL if any error is encountered
*/
    int len;
    int int_value;
    unsigned char *p_result = NULL;
    double x;
    double y;
    int srid;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	y = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  y = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
	srid = sqlite3_value_int (argv[2]);
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    gaiaMakePoint (x, y, srid, &p_result, &len);
    if (!p_result)
	sqlite3_result_null (context);
    else
	sqlite3_result_blob (context, p_result, len, free);
}

static void
geom_from_text1 (sqlite3_context * context, int argc, sqlite3_value ** argv,
		 short type)
{
/* SQL function:
/ GeomFromText(WKT encoded geometry)
/
/ returns the current geometry by parsing WKT encoded string 
/ or NULL if any error is encountered
/
/ if *type* is a negative value can accept any GEOMETRY CLASS
/ otherwise only requests conforming with required CLASS are valid
*/
    int len;
    unsigned char *p_result = NULL;
    const unsigned char *text;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  sqlite3_result_null (context);
	  return;
      }
    text = sqlite3_value_text (argv[0]);
    geo = gaiaParseWkt (text, type);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
    gaiaFreeGeomColl (geo);
    sqlite3_result_blob (context, p_result, len, free);
}

static void
geom_from_text2 (sqlite3_context * context, int argc, sqlite3_value ** argv,
		 short type)
{
/* SQL function:
/ GeomFromText(WKT encoded geometry, SRID)
/
/ returns the current geometry by parsing WKT encoded string 
/ or NULL if any error is encountered
/
/ if *type* is a negative value can accept any GEOMETRY CLASS
/ otherwise only requests conforming with required CLASS are valid
*/
    int len;
    unsigned char *p_result = NULL;
    const unsigned char *text;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_null (context);
	  return;
      }
    text = sqlite3_value_text (argv[0]);
    geo = gaiaParseWkt (text, type);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    geo->Srid = sqlite3_value_int (argv[1]);
    gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
    gaiaFreeGeomColl (geo);
    sqlite3_result_blob (context, p_result, len, free);
}

static int
check_wkb (const unsigned char *wkb, int size, short type)
{
/* checking type coherency for WKB encoded GEOMETRY */
    int little_endian;
    int wkb_type;
    int endian_arch = gaiaEndianArch ();
    if (size < 5)
	return 0;		/* too short to be a WKB */
    if (*(wkb + 0) == 0x01)
	little_endian = GAIA_LITTLE_ENDIAN;
    else if (*(wkb + 0) == 0x00)
	little_endian = GAIA_BIG_ENDIAN;
    else
	return 0;		/* illegal byte ordering; neither BIG-ENDIAN nor LITTLE-ENDIAN */
    wkb_type = gaiaImport32 (wkb + 1, little_endian, endian_arch);
    if (wkb_type ==
	GAIA_POINT
	|| wkb_type
	==
	GAIA_LINESTRING
	|| wkb_type
	==
	GAIA_POLYGON
	|| wkb_type
	==
	GAIA_MULTIPOINT
	|| wkb_type
	==
	GAIA_MULTILINESTRING
	|| wkb_type == GAIA_MULTIPOLYGON || wkb_type == GAIA_GEOMETRYCOLLECTION)
	;
    else
	return 0;		/* illegal GEOMETRY CLASS */
    if (type < 0)
	;			/* no restrinction about GEOMETRY CLASS TYPE */
    else
      {
	  if (wkb_type != type)
	      return 0;		/* invalid CLASS TYPE for request */
      }
    return 1;
}

static void
geom_from_wkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv,
		short type)
{
/* SQL function:
/ GeomFromWKB(WKB encoded geometry)
/
/ returns the current geometry by parsing a WKB encoded blob 
/ or NULL if any error is encountered
/
/ if *type* is a negative value can accept any GEOMETRY CLASS
/ otherwise only requests conforming with required CLASS are valid
*/
    int len;
    int n_bytes;
    unsigned char *p_result = NULL;
    const unsigned char *wkb;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    wkb = sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!check_wkb (wkb, n_bytes, type))
	return;
    geo = gaiaFromWkb (wkb, n_bytes);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
    gaiaFreeGeomColl (geo);
    sqlite3_result_blob (context, p_result, len, free);
}

static void
geom_from_wkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv,
		short type)
{
/* SQL function:
/ GeomFromWKB(WKB encoded geometry, SRID)
/
/ returns the current geometry by parsing a WKB encoded blob
/ or NULL if any error is encountered
/
/ if *type* is a negative value can accept any GEOMETRY CLASS
/ otherwise only requests conforming with required CLASS are valid
*/
    int len;
    int n_bytes;
    unsigned char *p_result = NULL;
    const unsigned char *wkb;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_null (context);
	  return;
      }
    wkb = sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!check_wkb (wkb, n_bytes, type))
	return;
    geo = gaiaFromWkb (wkb, n_bytes);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    geo->Srid = sqlite3_value_int (argv[1]);
    gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
    gaiaFreeGeomColl (geo);
    sqlite3_result_blob (context, p_result, len, free);
}

static void
fnct_GeometryFromFGF1 (sqlite3_context * context, int argc,
		       sqlite3_value ** argv)
{
/* SQL function:
/ GeomFromFGF(FGF encoded geometry)
/
/ returns the current geometry by parsing an FGF encoded blob 
/ or NULL if any error is encountered
/
/ if *type* is a negative value can accept any GEOMETRY CLASS
/ otherwise only requests conforming with required CLASS are valid
*/
    int len;
    int n_bytes;
    unsigned char *p_result = NULL;
    const unsigned char *fgf;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    fgf = sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromFgf (fgf, n_bytes);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
    gaiaFreeGeomColl (geo);
    sqlite3_result_blob (context, p_result, len, free);
}

static void
fnct_GeometryFromFGF2 (sqlite3_context * context, int argc,
		       sqlite3_value ** argv)
{
/* SQL function:
/ GeomFromFGF(FGF encoded geometry, SRID)
/
/ returns the current geometry by parsing an FGF encoded string 
/ or NULL if any error is encountered
/
/ if *type* is a negative value can accept any GEOMETRY CLASS
/ otherwise only requests conforming with required CLASS are valid
*/
    int len;
    int n_bytes;
    unsigned char *p_result = NULL;
    const unsigned char *fgf;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_null (context);
	  return;
      }
    fgf = sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromFgf (fgf, n_bytes);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    geo->Srid = sqlite3_value_int (argv[1]);
    gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
    gaiaFreeGeomColl (geo);
    sqlite3_result_blob (context, p_result, len, free);
}

/*
/ the following functions simply readdress the request to geom_from_text?()
/ setting the appropriate GEOMETRY CLASS TYPE
*/

static void
fnct_GeomFromText1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) -1);
}

static void
fnct_GeomFromText2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) -1);
}

static void
fnct_GeomCollFromText1 (sqlite3_context * context, int argc,
			sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) GAIA_GEOMETRYCOLLECTION);
}

static void
fnct_GeomCollFromText2 (sqlite3_context * context, int argc,
			sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) GAIA_GEOMETRYCOLLECTION);
}

static void
fnct_LineFromText1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) GAIA_LINESTRING);
}

static void
fnct_LineFromText2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) GAIA_LINESTRING);
}

static void
fnct_PointFromText1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) GAIA_POINT);
}

static void
fnct_PointFromText2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) GAIA_POINT);
}

static void
fnct_PolyFromText1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) GAIA_POLYGON);
}

static void
fnct_PolyFromText2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) GAIA_POLYGON);
}

static void
fnct_MLineFromText1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) GAIA_MULTILINESTRING);
}

static void
fnct_MLineFromText2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) GAIA_MULTILINESTRING);
}

static void
fnct_MPointFromText1 (sqlite3_context * context, int argc,
		      sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) GAIA_MULTIPOINT);
}

static void
fnct_MPointFromText2 (sqlite3_context * context, int argc,
		      sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) GAIA_MULTIPOINT);
}

static void
fnct_MPolyFromText1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text1 (context, argc, argv, (short) GAIA_MULTIPOLYGON);
}

static void
fnct_MPolyFromText2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_text2 (context, argc, argv, (short) GAIA_MULTIPOLYGON);
}

/*
/ the following functions simply readdress the request to geom_from_wkb?()
/ setting the appropriate GEOMETRY CLASS TYPE
*/

static void
fnct_GeomFromWkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) -1);
}

static void
fnct_GeomFromWkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) -1);
}

static void
fnct_GeomCollFromWkb1 (sqlite3_context * context, int argc,
		       sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) GAIA_GEOMETRYCOLLECTION);
}

static void
fnct_GeomCollFromWkb2 (sqlite3_context * context, int argc,
		       sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) GAIA_GEOMETRYCOLLECTION);
}

static void
fnct_LineFromWkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) GAIA_LINESTRING);
}

static void
fnct_LineFromWkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) GAIA_LINESTRING);
}

static void
fnct_PointFromWkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) GAIA_POINT);
}

static void
fnct_PointFromWkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) GAIA_POINT);
}

static void
fnct_PolyFromWkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) GAIA_POLYGON);
}

static void
fnct_PolyFromWkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) GAIA_POLYGON);
}

static void
fnct_MLineFromWkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) GAIA_MULTILINESTRING);
}

static void
fnct_MLineFromWkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) GAIA_MULTILINESTRING);
}

static void
fnct_MPointFromWkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) GAIA_MULTIPOINT);
}

static void
fnct_MPointFromWkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) GAIA_MULTIPOINT);
}

static void
fnct_MPolyFromWkb1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb1 (context, argc, argv, (short) GAIA_MULTIPOLYGON);
}

static void
fnct_MPolyFromWkb2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    geom_from_wkb2 (context, argc, argv, (short) GAIA_MULTIPOLYGON);
}

static void
fnct_Dimension (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Dimension(BLOB encoded geometry)
/
/ returns:
/ 0 if geometry is a POINT or MULTIPOINT
/ 1 if geometry is a LINESTRING or MULTILINESTRING
/ 2 if geometry is a POLYGON or MULTIPOLYGON
/ 0, 1, 2, for GEOMETRYCOLLECTIONS according to geometries contained inside
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int dim;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  dim = gaiaDimension (geo);
	  sqlite3_result_int (context, dim);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_GeometryType (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ GeometryType(BLOB encoded geometry)
/
/ returns the class for current geometry:
/ 'POINT' or 'MULTIPOINT'
/ 'LINESTRING' or 'MULTILINESTRING'
/ 'POLYGON' or 'MULTIPOLYGON'
/ 'GEOMETRYCOLLECTION' 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    int type;
    char *p_type = NULL;
    char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  type = gaiaGeometryType (geo);
	  switch (type)
	    {
	    case GAIA_POINT:
		p_type = "POINT";
		break;
	    case GAIA_MULTIPOINT:
		p_type = "MULTIPOINT";
		break;
	    case GAIA_LINESTRING:
		p_type = "LINESTRING";
		break;
	    case GAIA_MULTILINESTRING:
		p_type = "MULTILINESTRING";
		break;
	    case GAIA_POLYGON:
		p_type = "POLYGON";
		break;
	    case GAIA_MULTIPOLYGON:
		p_type = "MULTIPOLYGON";
		break;
	    case GAIA_GEOMETRYCOLLECTION:
		p_type = "GEOMETRYCOLLECTION";
		break;
	    };
	  if (p_type)
	    {
		len = strlen (p_type);
		p_result = malloc (len + 1);
		strcpy (p_result, p_type);
	    }
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	    {
		len = strlen (p_result);
		sqlite3_result_text (context, p_result, len, free);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_GeometryAliasType (sqlite3_context * context, int argc,
			sqlite3_value ** argv)
{
/* SQL function:
/ GeometryAliasType(BLOB encoded geometry)
/
/ returns the alias-class for current geometry:
/ 'MULTIPOINT'
/ 'MULTILINESTRING'
/ 'MULTIPOLYGON'
/ 'GEOMETRYCOLLECTION' 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    int type;
    char *p_type = NULL;
    char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  type = gaiaGeometryAliasType (geo);
	  switch (type)
	    {
	    case GAIA_POINT:
		p_type = "POINT";
		break;
	    case GAIA_MULTIPOINT:
		p_type = "MULTIPOINT";
		break;
	    case GAIA_LINESTRING:
		p_type = "LINESTRING";
		break;
	    case GAIA_MULTILINESTRING:
		p_type = "MULTILINESTRING";
		break;
	    case GAIA_POLYGON:
		p_type = "POLYGON";
		break;
	    case GAIA_MULTIPOLYGON:
		p_type = "MULTIPOLYGON";
		break;
	    case GAIA_GEOMETRYCOLLECTION:
		p_type = "GEOMETRYCOLLECTION";
		break;
	    };
	  if (p_type)
	    {
		len = strlen (p_type);
		p_result = malloc (len + 1);
		strcpy (p_result, p_type);
	    }
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	    {
		len = strlen (p_result);
		sqlite3_result_text (context, p_result, len, free);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_SRID (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Srid(BLOB encoded geometry)
/
/ returns the SRID
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
	sqlite3_result_int (context, geo->Srid);
    gaiaFreeGeomColl (geo);
}

static void
fnct_SetSRID (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ SetSrid(BLOBencoded geometry, srid)
/
/ returns a new geometry that is the original one received, but with the new SRID [no coordinates translation is applied]
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    int srid;
    unsigned char *p_result = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
	srid = sqlite3_value_int (argv[1]);
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  geo->Srid = srid;
	  gaiaToSpatiaLiteBlobWkb (geo, &p_result, &n_bytes);
	  sqlite3_result_blob (context, p_result, n_bytes, free);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_IsEmpty (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ IsEmpty(BLOB encoded geometry)
/
/ returns:
/ 1 if this geometry contains no elementary geometries
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_int (context, 1);
    else
	sqlite3_result_int (context, gaiaIsEmpty (geo));
    gaiaFreeGeomColl (geo);
}

static void
fnct_Envelope (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Envelope(BLOB encoded geometry)
/
/ returns the MBR for current geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr bbox;
    gaiaPolygonPtr polyg;
    gaiaRingPtr rect;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  if (gaiaIsEmpty (geo))
	      sqlite3_result_null (context);
	  else
	    {
		gaiaMbrGeometry (geo);
		bbox = gaiaAllocGeomColl ();
		polyg = gaiaAddPolygonToGeomColl (bbox, 5, 0);
		rect = polyg->Exterior;
		gaiaSetPoint (rect->Coords, 0, geo->MinX, geo->MinY);	/* vertex # 1 */
		gaiaSetPoint (rect->Coords, 1, geo->MaxX, geo->MinY);	/* vertex # 2 */
		gaiaSetPoint (rect->Coords, 2, geo->MaxX, geo->MaxY);	/* vertex # 3 */
		gaiaSetPoint (rect->Coords, 3, geo->MinX, geo->MaxY);	/* vertex # 4 */
		gaiaSetPoint (rect->Coords, 4, geo->MinX, geo->MinY);	/* vertex # 5 [same as vertex # 1 to close the polygon] */
		gaiaToSpatiaLiteBlobWkb (bbox, &p_result, &len);
		gaiaFreeGeomColl (bbox);
		sqlite3_result_blob (context, p_result, len, free);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
build_filter_mbr (sqlite3_context * context, int argc, sqlite3_value ** argv,
		  int mode)
{
/* SQL functions:
/ BuildMbrFilter(double X1, double Y1, double X2, double Y2)
/ FilterMBRWithin(double X1, double Y1, double X2, double Y2)
/ FilterMBRContain(double X1, double Y1, double X2, double Y2)
/ FilterMBRIntersects(double X1, double Y1, double X2, double Y2)
/
/ builds a generic filter for MBR from two points (identifying a rectangle's diagonal) 
/ or NULL if any error is encountered
*/
    int len;
    unsigned char *p_result = NULL;
    double x1;
    double y1;
    double x2;
    double y2;
    int int_value;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x1 = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x1 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	y1 = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  y1 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[2]) == SQLITE_FLOAT)
	x2 = sqlite3_value_double (argv[2]);
    else if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[2]);
	  x2 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[3]) == SQLITE_FLOAT)
	y2 = sqlite3_value_double (argv[3]);
    else if (sqlite3_value_type (argv[3]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[3]);
	  y2 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    gaiaBuildFilterMbr (x1, y1, x2, y2, mode, &p_result, &len);
    if (!p_result)
	sqlite3_result_null (context);
    else
	sqlite3_result_blob (context, p_result, len, free);
}

/*
/ the following functions simply readdress the request to build_filter_mbr()
/ setting the appropriate MODe
*/

static void
fnct_BuildMbrFilter (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    build_filter_mbr (context, argc, argv, GAIA_FILTER_MBR_DECLARE);
}

static void
fnct_FilterMbrWithin (sqlite3_context * context, int argc,
		      sqlite3_value ** argv)
{
    build_filter_mbr (context, argc, argv, GAIA_FILTER_MBR_WITHIN);
}

static void
fnct_FilterMbrContains (sqlite3_context * context, int argc,
			sqlite3_value ** argv)
{
    build_filter_mbr (context, argc, argv, GAIA_FILTER_MBR_CONTAINS);
}

static void
fnct_FilterMbrIntersects (sqlite3_context * context, int argc,
			  sqlite3_value ** argv)
{
    build_filter_mbr (context, argc, argv, GAIA_FILTER_MBR_INTERSECTS);
}

static void
fnct_BuildMbr1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ BuildMBR(double X1, double Y1, double X2, double Y2)
/
/ builds an MBR from two points (identifying a rectangle's diagonal) 
/ or NULL if any error is encountered
*/
    int len;
    unsigned char *p_result = NULL;
    double x1;
    double y1;
    double x2;
    double y2;
    int int_value;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x1 = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x1 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	y1 = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  y1 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[2]) == SQLITE_FLOAT)
	x2 = sqlite3_value_double (argv[2]);
    else if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[2]);
	  x2 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[3]) == SQLITE_FLOAT)
	y2 = sqlite3_value_double (argv[3]);
    else if (sqlite3_value_type (argv[3]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[3]);
	  y2 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    gaiaBuildMbr (x1, y1, x2, y2, -1, &p_result, &len);
    if (!p_result)
	sqlite3_result_null (context);
    else
	sqlite3_result_blob (context, p_result, len, free);
}

static void
fnct_BuildMbr2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ BuildMBR(double X1, double Y1, double X2, double Y2, int SRID)
/
/ builds an MBR from two points (identifying a rectangle's diagonal) 
/ or NULL if any error is encountered
*/
    int len;
    unsigned char *p_result = NULL;
    double x1;
    double y1;
    double x2;
    double y2;
    int int_value;
    int srid;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x1 = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x1 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	y1 = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  y1 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[2]) == SQLITE_FLOAT)
	x2 = sqlite3_value_double (argv[2]);
    else if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[2]);
	  x2 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[3]) == SQLITE_FLOAT)
	y2 = sqlite3_value_double (argv[3]);
    else if (sqlite3_value_type (argv[3]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[3]);
	  y2 = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[4]) == SQLITE_INTEGER)
	srid = sqlite3_value_int (argv[4]);
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    gaiaBuildMbr (x1, y1, x2, y2, srid, &p_result, &len);
    if (!p_result)
	sqlite3_result_null (context);
    else
	sqlite3_result_blob (context, p_result, len, free);
}

static void
fnct_BuildCircleMbr1 (sqlite3_context * context, int argc,
		      sqlite3_value ** argv)
{
/* SQL function:
/ BuildCircleMBR(double X, double Y, double radius)
/
/ builds an MBR from two points (identifying a rectangle's diagonal) 
/ or NULL if any error is encountered
*/
    int len;
    unsigned char *p_result = NULL;
    double x;
    double y;
    double radius;
    int int_value;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	y = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  y = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[2]) == SQLITE_FLOAT)
	radius = sqlite3_value_double (argv[2]);
    else if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[2]);
	  radius = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    gaiaBuildCircleMbr (x, y, radius, -1, &p_result, &len);
    if (!p_result)
	sqlite3_result_null (context);
    else
	sqlite3_result_blob (context, p_result, len, free);
}

static void
fnct_BuildCircleMbr2 (sqlite3_context * context, int argc,
		      sqlite3_value ** argv)
{
/* SQL function:
/ BuildCircleMBR(double X, double Y, double radius, int SRID)
/
/ builds an MBR from two points (identifying a rectangle's diagonal) 
/ or NULL if any error is encountered
*/
    int len;
    unsigned char *p_result = NULL;
    double x;
    double y;
    double radius;
    int int_value;
    int srid;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	y = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  y = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[2]) == SQLITE_FLOAT)
	radius = sqlite3_value_double (argv[2]);
    else if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[2]);
	  radius = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[3]) == SQLITE_INTEGER)
	srid = sqlite3_value_int (argv[3]);
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    gaiaBuildCircleMbr (x, y, radius, srid, &p_result, &len);
    if (!p_result)
	sqlite3_result_null (context);
    else
	sqlite3_result_blob (context, p_result, len, free);
}

static void
fnct_MbrMinX (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ MbrMinX(BLOB encoded GEMETRY)
/
/ returns the MinX coordinate for current geometry's MBR 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    double coord;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!gaiaGetMbrMinX (p_blob, n_bytes, &coord))
	sqlite3_result_null (context);
    else
	sqlite3_result_double (context, coord);
}

static void
fnct_MbrMaxX (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ MbrMaxX(BLOB encoded GEMETRY)
/
/ returns the MaxX coordinate for current geometry's MBR 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    double coord;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!gaiaGetMbrMaxX (p_blob, n_bytes, &coord))
	sqlite3_result_null (context);
    else
	sqlite3_result_double (context, coord);
}

static void
fnct_MbrMinY (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ MbrMinY(BLOB encoded GEMETRY)
/
/ returns the MinY coordinate for current geometry's MBR 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    double coord;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!gaiaGetMbrMinY (p_blob, n_bytes, &coord))
	sqlite3_result_null (context);
    else
	sqlite3_result_double (context, coord);
}

static void
fnct_MbrMaxY (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ MbrMaxY(BLOB encoded GEMETRY)
/
/ returns the MaxY coordinate for current geometry's MBR 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    double coord;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!gaiaGetMbrMaxY (p_blob, n_bytes, &coord))
	sqlite3_result_null (context);
    else
	sqlite3_result_double (context, coord);
}

static void
fnct_BuildRings (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ BuildRings(BLOBencoded geometry)
/
/ returns a new geometry [set of closed RINGs] obtained by 
/ reassembling a set of  LINESTRINGs
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr line_geom = NULL;
    gaiaGeomCollPtr result = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    line_geom = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (line_geom == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
/* one or more LINESTINGs are expected */
    if (line_geom->FirstPoint || line_geom->FirstPolygon)
	goto invalid;
    if (!line_geom->FirstLinestring)
	goto invalid;
    result = gaiaBuildRings (line_geom);
    if (!result)
	goto invalid;
    gaiaFreeGeomColl (line_geom);
    gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
    gaiaFreeGeomColl (result);
    sqlite3_result_blob (context, p_result, len, free);
    return;
  invalid:
    gaiaFreeGeomColl (line_geom);
    sqlite3_result_null (context);
}

static void
fnct_X (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ X(BLOB encoded POINT)
/
/ returns the X coordinate for current POINT geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaPointPtr point;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  point = simplePoint (geo);
	  if (!point)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_double (context, point->X);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_Y (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Y(BLOB encoded POINT)
/
/ returns the Y coordinate for current POINT geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaPointPtr point;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  point = simplePoint (geo);
	  if (!point)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_double (context, point->Y);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_NumPoints (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ NumPoints(BLOB encoded LINESTRING)
/
/ returns the numer of vertices for current LINESTRING geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaLinestringPtr line;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  line = simpleLinestring (geo);
	  if (!line)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_int (context, line->Points);
      }
    gaiaFreeGeomColl (geo);
}

static void
point_n (sqlite3_context * context, int argc, sqlite3_value ** argv,
	 int request)
{
/* SQL functions:
/ StartPoint(BLOB encoded LINESTRING geometry)
/ EndPoint(BLOB encoded LINESTRING geometry)
/ PointN(BLOB encoded LINESTRING geometry, integer point_no)
/
/ returns the Nth POINT for current LINESTRING geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int vertex;
    int len;
    double x;
    double y;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    gaiaLinestringPtr line;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (request == GAIA_POINTN)
      {
	  /* PointN() requires point index to be defined as an SQL function argument */
	  if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
	    {
		sqlite3_result_null (context);
		return;
	    }
	  vertex = sqlite3_value_int (argv[1]);
      }
    else if (request == GAIA_END_POINT)
	vertex = -1;		/* EndPoint() specifies a negative point index */
    else
	vertex = 1;		/* StartPoint() */
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  line = simpleLinestring (geo);
	  if (!line)
	      sqlite3_result_null (context);
	  else
	    {
		if (vertex < 0)
		    vertex = line->Points - 1;
		else
		    vertex -= 1;	/* decreasing the point index by 1, because PointN counts starting at index 1 */
		if (vertex >= 0 && vertex < line->Points)
		  {
		      gaiaGetPoint (line->Coords, vertex, &x, &y);
		      result = gaiaAllocGeomColl ();
		      gaiaAddPointToGeomColl (result, x, y);
		  }
		else
		    result = NULL;
		if (!result)
		    sqlite3_result_null (context);
		else
		  {
		      gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		      gaiaFreeGeomColl (result);
		      sqlite3_result_blob (context, p_result, len, free);
		  }
	    }
      }
    gaiaFreeGeomColl (geo);
}

/*
/ the following functions simply readdress the request to point_n()
/ setting the appropriate request mode
*/

static void
fnct_StartPoint (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    point_n (context, argc, argv, GAIA_START_POINT);
}

static void
fnct_EndPoint (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    point_n (context, argc, argv, GAIA_END_POINT);
}

static void
fnct_PointN (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    point_n (context, argc, argv, GAIA_POINTN);
}

static void
fnct_ExteriorRing (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL functions:
/ ExteriorRing(BLOB encoded POLYGON geometry)
/
/ returns the EXTERIOR RING for current POLYGON geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int iv;
    double x;
    double y;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    gaiaPolygonPtr polyg;
    gaiaRingPtr ring;
    gaiaLinestringPtr line;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  polyg = simplePolygon (geo);
	  if (!polyg)
	      sqlite3_result_null (context);
	  else
	    {
		ring = polyg->Exterior;
		result = gaiaAllocGeomColl ();
		line = gaiaAddLinestringToGeomColl (result, ring->Points);
		for (iv = 0; iv < line->Points; iv++)
		  {
		      gaiaGetPoint (ring->Coords, iv, &x, &y);
		      gaiaSetPoint (line->Coords, iv, x, y);
		  }
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		gaiaFreeGeomColl (result);
		sqlite3_result_blob (context, p_result, len, free);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_NumInteriorRings (sqlite3_context * context, int argc,
		       sqlite3_value ** argv)
{
/* SQL function:
/ NumInteriorRings(BLOB encoded POLYGON)
/
/ returns the number of INTERIOR RINGS for current POLYGON geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaPolygonPtr polyg;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  polyg = simplePolygon (geo);
	  if (!polyg)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_int (context, polyg->NumInteriors);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_InteriorRingN (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL functions:
/ InteriorRingN(BLOB encoded POLYGON geometry)
/
/ returns the Nth INTERIOR RING for current POLYGON geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int border;
    int iv;
    double x;
    double y;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    gaiaPolygonPtr polyg;
    gaiaRingPtr ring;
    gaiaLinestringPtr line;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    border = sqlite3_value_int (argv[1]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  polyg = simplePolygon (geo);
	  if (!polyg)
	      sqlite3_result_null (context);
	  else
	    {
		if (border >= 1 && border <= polyg->NumInteriors)
		  {
		      ring = polyg->Interiors + (border - 1);
		      result = gaiaAllocGeomColl ();
		      line = gaiaAddLinestringToGeomColl (result, ring->Points);
		      for (iv = 0; iv < line->Points; iv++)
			{
			    gaiaGetPoint (ring->Coords, iv, &x, &y);
			    gaiaSetPoint (line->Coords, iv, x, y);
			}
		      gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		      gaiaFreeGeomColl (result);
		      sqlite3_result_blob (context, p_result, len, free);
		  }
		else
		    sqlite3_result_null (context);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_NumGeometries (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ NumGeometries(BLOB encoded GEOMETRYCOLLECTION)
/
/ returns the number of elementary geometries for current geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int cnt = 0;
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaPolygonPtr polyg;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  point = geo->FirstPoint;
	  while (point)
	    {
		/* counts how many points are there */
		cnt++;
		point = point->Next;
	    }
	  line = geo->FirstLinestring;
	  while (line)
	    {
		/* counts how many linestrings are there */
		cnt++;
		line = line->Next;
	    }
	  polyg = geo->FirstPolygon;
	  while (polyg)
	    {
		/* counts how many polygons are there */
		cnt++;
		polyg = polyg->Next;
	    }
	  sqlite3_result_int (context, cnt);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_GeometryN (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ GeometryN(BLOB encoded GEOMETRYCOLLECTION geometry)
/
/ returns the Nth geometry for current GEOMETRYCOLLECTION or MULTIxxxx geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int entity;
    int len;
    int cnt = 0;
    int iv;
    int ib;
    double x;
    double y;
    gaiaPointPtr point;
    gaiaLinestringPtr line;
    gaiaLinestringPtr line2;
    gaiaPolygonPtr polyg;
    gaiaPolygonPtr polyg2;
    gaiaRingPtr ring_in;
    gaiaRingPtr ring_out;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    entity = sqlite3_value_int (argv[1]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  point = geo->FirstPoint;
	  while (point)
	    {
		/* counts how many points are there */
		cnt++;
		if (cnt == entity)
		  {
		      /* ok, required elementary geometry is this POINT */
		      result = gaiaAllocGeomColl ();
		      gaiaAddPointToGeomColl (result, point->X, point->Y);
		      goto skip;
		  }
		point = point->Next;
	    }
	  line = geo->FirstLinestring;
	  while (line)
	    {
		/* counts how many linestrings are there */
		cnt++;
		if (cnt == entity)
		  {
		      /* ok, required elementary geometry is this LINESTRING */
		      result = gaiaAllocGeomColl ();
		      line2 =
			  gaiaAddLinestringToGeomColl (result, line->Points);
		      for (iv = 0; iv < line2->Points; iv++)
			{
			    gaiaGetPoint (line->Coords, iv, &x, &y);
			    gaiaSetPoint (line2->Coords, iv, x, y);
			}
		      goto skip;
		  }
		line = line->Next;
	    }
	  polyg = geo->FirstPolygon;
	  while (polyg)
	    {
		/* counts how many polygons are there */
		cnt++;
		if (cnt == entity)
		  {
		      /* ok, required elementary geometry is this POLYGON */
		      result = gaiaAllocGeomColl ();
		      ring_in = polyg->Exterior;
		      polyg2 =
			  gaiaAddPolygonToGeomColl (result, ring_in->Points,
						    polyg->NumInteriors);
		      ring_out = polyg2->Exterior;
		      for (iv = 0; iv < ring_out->Points; iv++)
			{
			    /* copying the exterior ring POINTs */
			    gaiaGetPoint (ring_in->Coords, iv, &x, &y);
			    gaiaSetPoint (ring_out->Coords, iv, x, y);
			}
		      for (ib = 0; ib < polyg2->NumInteriors; ib++)
			{
			    /* processing the interior rings */
			    ring_in = polyg->Interiors + ib;
			    ring_out =
				gaiaAddInteriorRing (polyg2, ib,
						     ring_in->Points);
			    for (iv = 0; iv < ring_out->Points; iv++)
			      {
				  gaiaGetPoint (ring_in->Coords, iv, &x, &y);
				  gaiaSetPoint (ring_out->Coords, iv, x, y);
			      }
			}
		      goto skip;
		  }
		polyg = polyg->Next;
	    }
	skip:
	  if (result)
	    {
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		gaiaFreeGeomColl (result);
		sqlite3_result_blob (context, p_result, len, free);
	    }
	  else
	      sqlite3_result_null (context);
      }
    gaiaFreeGeomColl (geo);
}

static void
mbrs_eval (sqlite3_context * context, int argc, sqlite3_value ** argv,
	   int request)
{
/* SQL function:
/ MBRsomething(BLOB encoded GEOMETRY-1, BLOB encoded GEOMETRY-2)
/
/ returns:
/ 1 if the required spatial relationship between the two MBRs is TRUE
/ 0 otherwise
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int ret;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobMbr (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobMbr (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_null (context);
    else
      {
	  ret = 0;
	  gaiaMbrGeometry (geo1);
	  gaiaMbrGeometry (geo2);
	  switch (request)
	    {
	    case GAIA_MBR_CONTAINS:
		ret = gaiaMbrsContains (geo1, geo2);
		break;
	    case GAIA_MBR_DISJOINT:
		ret = gaiaMbrsDisjoint (geo1, geo2);
		break;
	    case GAIA_MBR_EQUAL:
		ret = gaiaMbrsEqual (geo1, geo2);
		break;
	    case GAIA_MBR_INTERSECTS:
		ret = gaiaMbrsIntersects (geo1, geo2);
		break;
	    case GAIA_MBR_OVERLAPS:
		ret = gaiaMbrsOverlaps (geo1, geo2);
		break;
	    case GAIA_MBR_TOUCHES:
		ret = gaiaMbrsTouches (geo1, geo2);
		break;
	    case GAIA_MBR_WITHIN:
		ret = gaiaMbrsWithin (geo1, geo2);
		break;
	    }
	  if (ret < 0)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

/*
/ the following functions simply readdress the mbr_eval()
/ setting the appropriate request mode
*/

static void
fnct_MbrContains (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    mbrs_eval (context, argc, argv, GAIA_MBR_CONTAINS);
}

static void
fnct_MbrDisjoint (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    mbrs_eval (context, argc, argv, GAIA_MBR_DISJOINT);
}

static void
fnct_MbrEqual (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    mbrs_eval (context, argc, argv, GAIA_MBR_EQUAL);
}

static void
fnct_MbrIntersects (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    mbrs_eval (context, argc, argv, GAIA_MBR_INTERSECTS);
}

static void
fnct_MbrOverlaps (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    mbrs_eval (context, argc, argv, GAIA_MBR_OVERLAPS);
}

static void
fnct_MbrTouches (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    mbrs_eval (context, argc, argv, GAIA_MBR_TOUCHES);
}

static void
fnct_MbrWithin (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    mbrs_eval (context, argc, argv, GAIA_MBR_WITHIN);
}

static void
fnct_ShiftCoords (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ ShiftCoords(BLOBencoded geometry, shiftX, shiftY)
/
/ returns a new geometry that is the original one received, but with shifted coordinates
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    double shift_x;
    double shift_y;
    int int_value;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	shift_x = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  shift_x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[2]) == SQLITE_FLOAT)
	shift_y = sqlite3_value_double (argv[2]);
    else if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[2]);
	  shift_y = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaShiftCoords (geo, shift_x, shift_y);
	  gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_blob (context, p_result, len, free);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_ScaleCoords (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ ScaleCoords(BLOBencoded geometry, scale_factor_x [, scale_factor_y])
/
/ returns a new geometry that is the original one received, but with scaled coordinates
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    double scale_x;
    double scale_y;
    int int_value;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	scale_x = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  scale_x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (argc == 2)
	scale_y = scale_x;	/* this one is an isotropic scaling request */
    else
      {
	  /* an anisotropic scaling is requested */
	  if (sqlite3_value_type (argv[2]) == SQLITE_FLOAT)
	      scale_y = sqlite3_value_double (argv[2]);
	  else if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
	    {
		int_value = sqlite3_value_int (argv[2]);
		scale_y = int_value;
	    }
	  else
	    {
		sqlite3_result_null (context);
		return;
	    }
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaScaleCoords (geo, scale_x, scale_y);
	  gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_blob (context, p_result, len, free);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_RotateCoords (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ RotateCoords(BLOBencoded geometry, angle)
/
/ returns a new geometry that is the original one received, but with rotated coordinates
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    double angle;
    int int_value;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	angle = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  angle = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaRotateCoords (geo, angle);
	  gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_blob (context, p_result, len, free);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_ReflectCoords (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ ReflectCoords(BLOBencoded geometry, x_axis,  y_axis)
/
/ returns a new geometry that is the original one received, but with mirrored coordinates
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    int x_axis;
    int y_axis;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
	x_axis = sqlite3_value_int (argv[1]);
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER)
	y_axis = sqlite3_value_int (argv[2]);
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaReflectCoords (geo, x_axis, y_axis);
	  gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_blob (context, p_result, len, free);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_SwapCoords (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ SwapCoords(BLOBencoded geometry)
/
/ returns a new geometry that is the original one received, but with swapped x- and y-coordinate
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  gaiaSwapCoords (geo);
	  gaiaToSpatiaLiteBlobWkb (geo, &p_result, &len);
	  if (!p_result)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_blob (context, p_result, len, free);
      }
    gaiaFreeGeomColl (geo);
}

static void
proj_params (sqlite3 * sqlite, int srid, char *proj_params)
{
/* retrives the PROJ params from SPATIAL_SYS_REF table, if possible */
    char sql[256];
    char **results;
    int rows;
    int columns;
    int i;
    int ret;
    char *errMsg = NULL;
    *proj_params = '\0';
    sprintf (sql,
	     "SELECT proj4text FROM spatial_ref_sys WHERE srid = %d", srid);
    ret = sqlite3_get_table (sqlite, sql, &results, &rows, &columns, &errMsg);
    if (ret != SQLITE_OK)
      {
	  fprintf (stderr, "unknown SRID: %d\t<%s>\n", srid, errMsg);
	  sqlite3_free (errMsg);
	  return;
      }
    for (i = 1; i <= rows; i++)
	strcpy (proj_params, results[(i * columns)]);
    if (*proj_params == '\0')
	fprintf (stderr, "unknown SRID: %d\n", srid);
    sqlite3_free_table (results);
}

#if OMIT_PROJ == 0		/* including PROJ.4 */

static void
fnct_Transform (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Transform(BLOBencoded geometry, srid)
/
/ returns a new geometry that is the original one received, but with the new SRID [no coordinates translation is applied]
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    int srid_from;
    int srid_to;
    char proj_from[2048];
    char proj_to[2048];
    sqlite3 *sqlite = sqlite3_context_db_handle (context);
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
	srid_to = sqlite3_value_int (argv[1]);
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  *proj_from = '\0';
	  *proj_to = '\0';
	  srid_from = geo->Srid;
	  proj_params (sqlite, srid_from, proj_from);
	  proj_params (sqlite, srid_to, proj_to);
	  if (*proj_to == '\0' || *proj_from == '\0')
	    {
		gaiaFreeGeomColl (geo);
		sqlite3_result_null (context);
		return;
	    }
	  result = gaiaTransform (geo, proj_from, proj_to);
	  if (!result)
	      sqlite3_result_null (context);
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		result->Srid = srid_to;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo);
}

#endif /* end including PROJ.4 */

#if OMIT_GEOS == 0		/* including GEOS */

static void
fnct_Boundary (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Boundary(BLOB encoded geometry)
/
/ returns the combinatioral boundary for current geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr boundary;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  if (gaiaIsEmpty (geo))
	      sqlite3_result_null (context);
	  else
	    {
		boundary = gaiaBoundary (geo);
		if (!boundary)
		    sqlite3_result_null (context);
		else
		  {
		      gaiaToSpatiaLiteBlobWkb (boundary, &p_result, &len);
		      gaiaFreeGeomColl (boundary);
		      sqlite3_result_blob (context, p_result, len, free);
		  }
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_IsClosed (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ IsClosed(BLOB encoded LINESTRING or MULTILINESTRING geometry)
/
/ returns:
/ 1 if this LINESTRING is closed [or if this is a MULTILINESTRING and every LINESTRINGs are closed] 
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaLinestringPtr line;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_int (context, -1);
    else
      {
	  line = simpleLinestring (geo);
	  if (!line < 0)
	      sqlite3_result_int (context, -1);
	  else
	      sqlite3_result_int (context, gaiaIsClosed (line));
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_IsSimple (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ IsSimple(BLOB encoded GEOMETRY)
/
/ returns:
/ 1 if this GEOMETRY is simple
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int ret;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaIsSimple (geo);
	  if (ret < 0)
	      sqlite3_result_int (context, -1);
	  else
	      sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_IsRing (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ IsRing(BLOB encoded LINESTRING geometry)
/
/ returns:
/ 1 if this LINESTRING is a valid RING
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int ret;
    gaiaGeomCollPtr geo = NULL;
    gaiaLinestringPtr line;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_int (context, -1);
    else
      {
	  line = simpleLinestring (geo);
	  if (!line < 0)
	      sqlite3_result_int (context, -1);
	  else
	    {
		ret = gaiaIsRing (line);
		sqlite3_result_int (context, ret);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_IsValid (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ IsValid(BLOB encoded GEOMETRY)
/
/ returns:
/ 1 if this GEOMETRY is a valid one
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int ret;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaIsValid (geo);
	  if (ret < 0)
	      sqlite3_result_int (context, -1);
	  else
	      sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_Length (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Length(BLOB encoded GEOMETRYCOLLECTION)
/
/ returns  the total length for current geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    double length = 0.0;
    int ret;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  ret = gaiaGeomCollLength (geo, &length);
	  if (!ret)
	      sqlite3_result_null (context);
	  sqlite3_result_double (context, length);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_Area (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Area(BLOB encoded GEOMETRYCOLLECTION)
/
/ returns the total area for current geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    double area = 0.0;
    int ret;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  ret = gaiaGeomCollArea (geo, &area);
	  if (!ret)
	      sqlite3_result_null (context);
	  sqlite3_result_double (context, area);
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_Centroid (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Centroid(BLOBencoded POLYGON or MULTIPOLYGON geometry)
/
/ returns a POINT representing the centroid for current POLYGON / MULTIPOLYGON geometry 
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    int ret;
    double x;
    double y;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  if (gaiaIsEmpty (geo))
	      sqlite3_result_null (context);
	  else
	    {
		ret = gaiaGeomCollCentroid (geo, &x, &y);
		if (!ret)
		    sqlite3_result_null (context);
		else
		  {
		      result = gaiaAllocGeomColl ();
		      gaiaAddPointToGeomColl (result, x, y);
		      gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		      gaiaFreeGeomColl (result);
		      sqlite3_result_blob (context, p_result, len, free);
		  }
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_PointOnSurface (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ PointOnSurface(BLOBencoded POLYGON or MULTIPOLYGON geometry)
/
/ returns a POINT guaranteed to lie on the Surface
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    double x;
    double y;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  if (!gaiaGetPointOnSurface (geo, &x, &y))
	      sqlite3_result_null (context);
	  else
	    {
		result = gaiaAllocGeomColl ();
		gaiaAddPointToGeomColl (result, x, y);
		result->Srid = geo->Srid;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		gaiaFreeGeomColl (result);
		sqlite3_result_blob (context, p_result, len, free);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_Simplify (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Simplify(BLOBencoded geometry, tolerance)
/
/ returns a new geometry that is a caricature of the original one received, but simplified using the Douglas-Peuker algorihtm
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    int int_value;
    double tolerance;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	tolerance = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  tolerance = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaGeomCollSimplify (geo, tolerance);
	  if (!result)
	      sqlite3_result_null (context);
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_SimplifyPreserveTopology (sqlite3_context * context, int argc,
			       sqlite3_value ** argv)
{
/* SQL function:
/ SimplifyPreserveTopology(BLOBencoded geometry, tolerance)
/
/ returns a new geometry that is a caricature of the original one received, but simplified using the Douglas-Peuker algorihtm [preserving topology]
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    int int_value;
    double tolerance;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	tolerance = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  tolerance = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaGeomCollSimplifyPreserveTopology (geo, tolerance);
	  if (!result)
	      sqlite3_result_null (context);
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_ConvexHull (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ ConvexHull(BLOBencoded geometry)
/
/ returns a new geometry representing the CONVEX HULL for current geometry
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int len;
    unsigned char *p_result = NULL;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaConvexHull (geo);
	  if (!result)
	      sqlite3_result_null (context);
	  else
	    {
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_Buffer (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Buffer(BLOBencoded geometry, radius)
/
/ returns a new geometry representing the BUFFER for current geometry
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    gaiaGeomCollPtr result;
    double radius;
    int int_value;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	radius = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  radius = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaGeomCollBuffer (geo, radius, 30);
	  if (!result)
	      sqlite3_result_null (context);
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		result->Srid = geo->Srid;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo);
}

static void
fnct_Intersection (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Intersection(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns a new geometry representing the INTERSECTION of both geometries
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    gaiaGeomCollPtr result;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaGeometryIntersection (geo1, geo2);
	  if (!result)
	      sqlite3_result_null (context);
	  else if (gaiaIsEmpty (result))
	    {
		gaiaFreeGeomColl (result);
		sqlite3_result_null (context);
	    }
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Union_step (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Union(BLOBencoded geom)
/
/ aggregate function - STEP
/
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geom;
    gaiaGeomCollPtr result;
    gaiaGeomCollPtr *p;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geom = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geom)
	return;
    p = sqlite3_aggregate_context (context, sizeof (gaiaGeomCollPtr));
    if (!(*p))
      {
	  /* this is the first row */
	  *p = geom;
      }
    else
      {
	  /* subsequent rows */
	  result = gaiaGeometryUnion (*p, geom);
	  gaiaFreeGeomColl (*p);
	  *p = result;
	  gaiaFreeGeomColl (geom);
      }
}

static void
fnct_Union_final (sqlite3_context * context)
{
/* SQL function:
/ Union(BLOBencoded geom)
/
/ aggregate function - FINAL
/
*/
    gaiaGeomCollPtr result;
    gaiaGeomCollPtr *p = sqlite3_aggregate_context (context, 0);
    if (!p)
      {
	  sqlite3_result_null (context);
	  return;
      }
    result = *p;
    if (!result)
	sqlite3_result_null (context);
    else if (gaiaIsEmpty (result))
      {
	  gaiaFreeGeomColl (result);
	  sqlite3_result_null (context);
      }
    else
      {
	  /* builds the BLOB geometry to be returned */
	  int len;
	  unsigned char *p_result = NULL;
	  gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
	  sqlite3_result_blob (context, p_result, len, free);
	  gaiaFreeGeomColl (result);
      }
}

static void
fnct_Union (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Union(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns a new geometry representing the UNION of both geometries
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    gaiaGeomCollPtr result;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaGeometryUnion (geo1, geo2);
	  if (!result)
	      sqlite3_result_null (context);
	  else if (gaiaIsEmpty (result))
	    {
		gaiaFreeGeomColl (result);
		sqlite3_result_null (context);
	    }
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Difference (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Difference(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns a new geometry representing the DIFFERENCE of both geometries
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    gaiaGeomCollPtr result;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaGeometryDifference (geo1, geo2);
	  if (!result)
	      sqlite3_result_null (context);
	  else if (gaiaIsEmpty (result))
	    {
		gaiaFreeGeomColl (result);
		sqlite3_result_null (context);
	    }
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_SymDifference (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ SymDifference(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns a new geometry representing the SYMMETRIC DIFFERENCE of both geometries
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    gaiaGeomCollPtr result;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_null (context);
    else
      {
	  result = gaiaGeometrySymDifference (geo1, geo2);
	  if (!result)
	      sqlite3_result_null (context);
	  else if (gaiaIsEmpty (result))
	    {
		gaiaFreeGeomColl (result);
		sqlite3_result_null (context);
	    }
	  else
	    {
		/* builds the BLOB geometry to be returned */
		int len;
		unsigned char *p_result = NULL;
		gaiaToSpatiaLiteBlobWkb (result, &p_result, &len);
		sqlite3_result_blob (context, p_result, len, free);
		gaiaFreeGeomColl (result);
	    }
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Equals (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Equals(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if the two geometries are "spatially equal"
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollEquals (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Intersects (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Intersects(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if the two geometries do "spatially intersects"
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollIntersects (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Disjoint (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Disjoint(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if the two geometries are "spatially disjoint"
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollDisjoint (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Overlaps (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Overlaps(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if the two geometries do "spatially overlaps"
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollOverlaps (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Crosses (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Crosses(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if the two geometries do "spatially crosses"
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollCrosses (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Touches (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Touches(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if the two geometries do "spatially touches"
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollTouches (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Within (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Within(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if GEOM-1 is completely contained within GEOM-2
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollWithin (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Contains (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Contains(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns:
/ 1 if GEOM-1 completely contains GEOM-2
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollContains (geo1, geo2);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Relate (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Relate(BLOBencoded geom1, BLOBencoded geom2, string pattern)
/
/ returns:
/ 1 if GEOM-1 and GEOM-2 have a spatial relationship as specified by the patternMatrix 
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    int ret;
    const unsigned char *pattern;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    if (sqlite3_value_type (argv[2]) != SQLITE_TEXT)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    pattern = sqlite3_value_text (argv[2]);
    if (!geo1 || !geo2)
	sqlite3_result_int (context, -1);
    else
      {
	  ret = gaiaGeomCollRelate (geo1, geo2, (char *) pattern);
	  sqlite3_result_int (context, ret);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
fnct_Distance (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Distance(BLOBencoded geom1, BLOBencoded geom2)
/
/ returns the distance between GEOM-1 and GEOM-2
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo1 = NULL;
    gaiaGeomCollPtr geo2 = NULL;
    double dist;
    int ret;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo1 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    p_blob = (unsigned char *) sqlite3_value_blob (argv[1]);
    n_bytes = sqlite3_value_bytes (argv[1]);
    geo2 = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (!geo1 || !geo2)
	sqlite3_result_null (context);
    else
      {
	  ret = gaiaGeomCollDistance (geo1, geo2, &dist);
	  if (!ret)
	      sqlite3_result_null (context);
	  sqlite3_result_double (context, dist);
      }
    gaiaFreeGeomColl (geo1);
    gaiaFreeGeomColl (geo2);
}

static void
geos_error (const char *fmt, ...)
{
/* reporting some GEOS warning/error */
    va_list ap;
    fprintf (stderr, "GEOS: ");
    va_start (ap, fmt);
    vfprintf (stdout, fmt, ap);
    va_end (ap);
    fprintf (stdout, "\n");
}

static void
fnct_polygonize (sqlite3_context * context, gaiaGeomCollPtr geom_org,
		 int force_multipolygon)
{
/* a  common function performing any kind of polygonization op */
    gaiaGeomCollPtr geom_new = NULL;
    int len;
    unsigned char *p_result = NULL;
    if (!geom_org)
	goto invalid;
    geom_new = gaiaPolygonize (geom_org, force_multipolygon);
    if (!geom_new)
	goto invalid;
    gaiaFreeGeomColl (geom_org);
    gaiaToSpatiaLiteBlobWkb (geom_new, &p_result, &len);
    gaiaFreeGeomColl (geom_new);
    sqlite3_result_blob (context, p_result, len, free);
    return;
  invalid:
    if (geom_org)
	gaiaFreeGeomColl (geom_org);
    sqlite3_result_null (context);
}

/*
/ the following functions performs initial argument checking, 
/ and then readdressing the request to fnct_polygonize()
/ for actual processing
*/

static void
fnct_BdPolyFromText1 (sqlite3_context * context, int argc,
		      sqlite3_value ** argv)
{
/* SQL function:
/ BdPolyFromText(WKT encoded  LINESTRING)
/
/ returns the current geometry [POLYGON] by parsing a WKT encoded LINESTRING 
/ or NULL if any error is encountered
/
*/
    const unsigned char *text;
    gaiaGeomCollPtr geo = NULL;
    gaiaLinestringPtr ln;
    double x0;
    double y0;
    double xn;
    double yn;
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  sqlite3_result_null (context);
	  return;
      }
    text = sqlite3_value_text (argv[0]);
    geo = gaiaParseWkt (text, -1);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    geo->Srid = -1;
/* one or more closed LINESTINGs are expected */
    if (geo->FirstPoint || geo->FirstPolygon)
	goto invalid;
    if (!geo->FirstLinestring)
	goto invalid;
    ln = geo->FirstLinestring;
    while (ln)
      {
	  gaiaGetPoint (ln->Coords, 0, &x0, &y0);
	  gaiaGetPoint (ln->Coords, ln->Points - 1, &xn, &yn);
	  if (x0 != xn || y0 != yn)
	      goto invalid;
	  ln = ln->Next;
      }
    fnct_polygonize (context, geo, 0);
    return;
  invalid:
    gaiaFreeGeomColl (geo);
    sqlite3_result_null (context);
}

static void
fnct_BdPolyFromText2 (sqlite3_context * context, int argc,
		      sqlite3_value ** argv)
{
/* SQL function:
/ BdPolyFromText(WKT encoded LINESTRING, SRID)
/
/ returns the current geometry [POLYGON] by parsing a WKT encoded LINESTRING 
/ or NULL if any error is encountered
/
*/
    const unsigned char *text;
    gaiaGeomCollPtr geo = NULL;
    gaiaLinestringPtr ln;
    double x0;
    double y0;
    double xn;
    double yn;
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_null (context);
	  return;
      }
    text = sqlite3_value_text (argv[0]);
    geo = gaiaParseWkt (text, -1);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    geo->Srid = sqlite3_value_int (argv[1]);
/* one or more closed LINESTINGs are expected */
    if (geo->FirstPoint || geo->FirstPolygon)
	goto invalid;
    if (!geo->FirstLinestring)
	goto invalid;
    ln = geo->FirstLinestring;
    while (ln)
      {
	  gaiaGetPoint (ln->Coords, 0, &x0, &y0);
	  gaiaGetPoint (ln->Coords, ln->Points - 1, &xn, &yn);
	  if (x0 != xn || y0 != yn)
	      goto invalid;
	  ln = ln->Next;
      }
    fnct_polygonize (context, geo, 0);
    return;
  invalid:
    gaiaFreeGeomColl (geo);
    sqlite3_result_null (context);
}

static void
fnct_BdMPolyFromText1 (sqlite3_context * context, int argc,
		       sqlite3_value ** argv)
{
/* SQL function:
/ BdMPolyFromText(WKT encoded  MULTILINESTRING)
/
/ returns the current geometry [MULTIPOLYGON] by parsing a WKT encoded MULTILINESTRING 
/ or NULL if any error is encountered
/
*/
    const unsigned char *text;
    gaiaGeomCollPtr geo = NULL;
    gaiaLinestringPtr ln;
    double x0;
    double y0;
    double xn;
    double yn;
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  sqlite3_result_null (context);
	  return;
      }
    text = sqlite3_value_text (argv[0]);
    geo = gaiaParseWkt (text, -1);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    geo->Srid = -1;
/* one or more closed LINESTINGs are expected */
    if (geo->FirstPoint || geo->FirstPolygon)
	goto invalid;
    if (!geo->FirstLinestring)
	goto invalid;
    ln = geo->FirstLinestring;
    while (ln)
      {
	  gaiaGetPoint (ln->Coords, 0, &x0, &y0);
	  gaiaGetPoint (ln->Coords, ln->Points - 1, &xn, &yn);
	  if (x0 != xn || y0 != yn)
	      goto invalid;
	  ln = ln->Next;
      }
    fnct_polygonize (context, geo, 1);
    return;
  invalid:
    gaiaFreeGeomColl (geo);
    sqlite3_result_null (context);
}

static void
fnct_BdMPolyFromText2 (sqlite3_context * context, int argc,
		       sqlite3_value ** argv)
{
/* SQL function:
/ BdMPolyFromText(WKT encoded MULTILINESTRING, SRID)
/
/ returns the current geometry [MULTIPOLYGON] by parsing a WKT encoded MULTILINESTRING 
/ or NULL if any error is encountered
/
*/
    const unsigned char *text;
    gaiaGeomCollPtr geo = NULL;
    gaiaLinestringPtr ln;
    double x0;
    double y0;
    double xn;
    double yn;
    if (sqlite3_value_type (argv[0]) != SQLITE_TEXT)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_null (context);
	  return;
      }
    text = sqlite3_value_text (argv[0]);
    geo = gaiaParseWkt (text, -1);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    geo->Srid = sqlite3_value_int (argv[1]);
/* one or more closed LINESTINGs are expected */
    if (geo->FirstPoint || geo->FirstPolygon)
	goto invalid;
    if (!geo->FirstLinestring)
	goto invalid;
    ln = geo->FirstLinestring;
    while (ln)
      {
	  gaiaGetPoint (ln->Coords, 0, &x0, &y0);
	  gaiaGetPoint (ln->Coords, ln->Points - 1, &xn, &yn);
	  if (x0 != xn || y0 != yn)
	      goto invalid;
	  ln = ln->Next;
      }
    fnct_polygonize (context, geo, 1);
    return;
  invalid:
    gaiaFreeGeomColl (geo);
    sqlite3_result_null (context);
}

static void
fnct_BdPolyFromWKB1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ BdPolyFromWKB(WKB encoded  LINESTRING)
/
/ returns the current geometry [POLYGON] by parsing a WKB encoded LINESTRING 
/ or NULL if any error is encountered
/
*/
    int n_bytes;
    const unsigned char *wkb;
    gaiaGeomCollPtr geo = NULL;
    gaiaLinestringPtr ln;
    double x0;
    double y0;
    double xn;
    double yn;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    wkb = sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!check_wkb (wkb, n_bytes, -1))
	return;
    geo = gaiaFromWkb (wkb, n_bytes);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    geo->Srid = -1;
/* one or more closed LINESTINGs are expected */
    if (geo->FirstPoint || geo->FirstPolygon)
	goto invalid;
    if (!geo->FirstLinestring)
	goto invalid;
    ln = geo->FirstLinestring;
    while (ln)
      {
	  gaiaGetPoint (ln->Coords, 0, &x0, &y0);
	  gaiaGetPoint (ln->Coords, ln->Points - 1, &xn, &yn);
	  if (x0 != xn || y0 != yn)
	      goto invalid;
	  ln = ln->Next;
      }
    fnct_polygonize (context, geo, 0);
    return;
  invalid:
    gaiaFreeGeomColl (geo);
    sqlite3_result_null (context);
}

static void
fnct_BdPolyFromWKB2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ BdPolyFromWKB(WKB encoded  LINESTRING)
/
/ returns the current geometry [POLYGON] by parsing a WKB encoded LINESTRING 
/ or NULL if any error is encountered
/
*/
    int n_bytes;
    const unsigned char *wkb;
    gaiaGeomCollPtr geo = NULL;
    gaiaLinestringPtr ln;
    double x0;
    double y0;
    double xn;
    double yn;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_null (context);
	  return;
      }
    wkb = sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!check_wkb (wkb, n_bytes, -1))
	return;
    geo = gaiaFromWkb (wkb, n_bytes);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    geo->Srid = sqlite3_value_int (argv[1]);
/* one or more closed LINESTINGs are expected */
    if (geo->FirstPoint || geo->FirstPolygon)
	goto invalid;
    if (!geo->FirstLinestring)
	goto invalid;
    ln = geo->FirstLinestring;
    while (ln)
      {
	  gaiaGetPoint (ln->Coords, 0, &x0, &y0);
	  gaiaGetPoint (ln->Coords, ln->Points - 1, &xn, &yn);
	  if (x0 != xn || y0 != yn)
	      goto invalid;
	  ln = ln->Next;
      }
    fnct_polygonize (context, geo, 0);
    return;
  invalid:
    gaiaFreeGeomColl (geo);
    sqlite3_result_null (context);
}

static void
fnct_BdMPolyFromWKB1 (sqlite3_context * context, int argc,
		      sqlite3_value ** argv)
{
/* SQL function:
/ BdMPolyFromWKB(WKB encoded  MULTILINESTRING)
/
/ returns the current geometry [MULTIPOLYGON] by parsing a WKB encoded MULTILINESTRING 
/ or NULL if any error is encountered
/
*/
    int n_bytes;
    const unsigned char *wkb;
    gaiaGeomCollPtr geo = NULL;
    gaiaLinestringPtr ln;
    double x0;
    double y0;
    double xn;
    double yn;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    wkb = sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!check_wkb (wkb, n_bytes, -1))
	return;
    geo = gaiaFromWkb (wkb, n_bytes);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    geo->Srid = -1;
/* one or more closed LINESTINGs are expected */
    if (geo->FirstPoint || geo->FirstPolygon)
	goto invalid;
    if (!geo->FirstLinestring)
	goto invalid;
    ln = geo->FirstLinestring;
    while (ln)
      {
	  gaiaGetPoint (ln->Coords, 0, &x0, &y0);
	  gaiaGetPoint (ln->Coords, ln->Points - 1, &xn, &yn);
	  if (x0 != xn || y0 != yn)
	      goto invalid;
	  ln = ln->Next;
      }
    fnct_polygonize (context, geo, 1);
    return;
  invalid:
    gaiaFreeGeomColl (geo);
    sqlite3_result_null (context);
}

static void
fnct_BdMPolyFromWKB2 (sqlite3_context * context, int argc,
		      sqlite3_value ** argv)
{
/* SQL function:
/ BdMPolyFromWKB(WKB encoded  MULTILINESTRING)
/
/ returns the current geometry [MULTIPOLYGON] by parsing a WKB encoded MULTILINESTRING 
/ or NULL if any error is encountered
/
*/
    int n_bytes;
    const unsigned char *wkb;
    gaiaGeomCollPtr geo = NULL;
    gaiaLinestringPtr ln;
    double x0;
    double y0;
    double xn;
    double yn;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_null (context);
	  return;
      }
    wkb = sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (!check_wkb (wkb, n_bytes, -1))
	return;
    geo = gaiaFromWkb (wkb, n_bytes);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    geo->Srid = sqlite3_value_int (argv[1]);
/* one or more closed LINESTINGs are expected */
    if (geo->FirstPoint || geo->FirstPolygon)
	goto invalid;
    if (!geo->FirstLinestring)
	goto invalid;
    ln = geo->FirstLinestring;
    while (ln)
      {
	  gaiaGetPoint (ln->Coords, 0, &x0, &y0);
	  gaiaGetPoint (ln->Coords, ln->Points - 1, &xn, &yn);
	  if (x0 != xn || y0 != yn)
	      goto invalid;
	  ln = ln->Next;
      }
    fnct_polygonize (context, geo, 1);
    return;
  invalid:
    gaiaFreeGeomColl (geo);
    sqlite3_result_null (context);
}

static void
fnct_Polygonize1 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Polygonize(BLOBencoded geometry)
/
/ returns a new geometry [POLYGON or MULTIPOLYGON] representing 
/ the polygonization for current LINESTRING or MULTILINESTRING geometry
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
/* one or more LINESTINGs are expected */
    if (geo->FirstPoint || geo->FirstPolygon)
	goto invalid;
    if (!geo->FirstLinestring)
	goto invalid;
    fnct_polygonize (context, geo, 0);
    return;
  invalid:
    gaiaFreeGeomColl (geo);
    sqlite3_result_null (context);
}

static void
fnct_Polygonize2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ Polygonize(BLOBencoded geometry, BOOL force_multipolygon)
/
/ returns a new geometry [POLYGON or MULTIPOLYGON] representing 
/ the polygonization for current LINESTRING or MULTILINESTRING geometry
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geo = NULL;
    int force_multipolygon;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) != SQLITE_INTEGER)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    geo = gaiaFromSpatiaLiteBlobWkb (p_blob, n_bytes);
    if (geo == NULL)
      {
	  sqlite3_result_null (context);
	  return;
      }
    force_multipolygon = sqlite3_value_int (argv[1]);
/* one or more LINESTINGs are expected */
    if (geo->FirstPoint || geo->FirstPolygon)
	goto invalid;
    if (!geo->FirstLinestring)
	goto invalid;
    fnct_polygonize (context, geo, force_multipolygon);
    return;
  invalid:
    gaiaFreeGeomColl (geo);
    sqlite3_result_null (context);
}

#endif /* end including GEOS */

#if OMIT_MATHSQL == 0		/* supporting SQL math functions */

static void
fnct_math_abs (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ abs(double X)
/
/ Returns the absolute value of X
/ or NULL if any error is encountered
*/
    sqlite3_int64 int_value;
    double x;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
      {
	  x = fabs (sqlite3_value_double (argv[0]));
	  sqlite3_result_double (context, x);
      }
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = math_llabs (sqlite3_value_int64 (argv[0]));
	  sqlite3_result_int64 (context, int_value);
      }
    else
	sqlite3_result_null (context);
}

static void
fnct_math_acos (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ acos(double X)
/
/ Returns the arc cosine of X, that is, the value whose cosine is X
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    errno = 0;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
      {
	  x = acos (sqlite3_value_double (argv[0]));
	  if (errno == EDOM)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_double (context, x);
      }
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
	  x = acos (x);
	  if (errno == EDOM)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_double (context, x);
      }
    else
	sqlite3_result_null (context);
}

static void
fnct_math_asin (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ asin(double X)
/
/ Returns the arc sine of X, that is, the value whose sine is X
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    errno = 0;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
      {
	  x = asin (sqlite3_value_double (argv[0]));
	  if (errno == EDOM)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_double (context, x);
      }
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
	  x = asin (x);
	  if (errno == EDOM)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_double (context, x);
      }
    else
	sqlite3_result_null (context);
}

static void
fnct_math_atan (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ atan(double X)
/
/ Returns the arc tangent of X, that is, the value whose tangent is X
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
      {
	  x = atan (sqlite3_value_double (argv[0]));
	  sqlite3_result_double (context, x);
      }
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
	  x = atan (x);
	  sqlite3_result_double (context, x);
      }
    else
	sqlite3_result_null (context);
}

static void
fnct_math_ceil (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ ceil(double X)
/
/ Returns the smallest integer value not less than X
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
      {
	  x = ceil (sqlite3_value_double (argv[0]));
	  sqlite3_result_double (context, x);
      }
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
	  x = ceil (x);
	  sqlite3_result_double (context, x);
      }
    else
	sqlite3_result_null (context);
}

static void
fnct_math_cos (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ cos(double X)
/
/ Returns the cosine of X, where X is given in radians
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
      {
	  x = cos (sqlite3_value_double (argv[0]));
	  sqlite3_result_double (context, x);
      }
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
	  x = cos (x);
	  sqlite3_result_double (context, x);
      }
    else
	sqlite3_result_null (context);
}

static void
fnct_math_cot (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ cot(double X)
/
/ Returns the cotangent of X
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    double tang;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
      }
    else {
	sqlite3_result_null (context);
	return;
    }
    tang = tan (x);
    if (tang == 0.0)
      {
	  sqlite3_result_null (context);
	  return;
      }
    x = 1.0 / tang;
    sqlite3_result_double (context, x);
}

static void
fnct_math_degrees (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ degrees(double X)
/
/ Returns the argument X, converted from radians to degrees
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    x *= 57.29577951308232;
    sqlite3_result_double (context, x);
}

static void
fnct_math_exp (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ exp(double X)
/
/ Returns the value of e (the base of natural logarithms) raised to the power of X
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
      {
	  x = exp (sqlite3_value_double (argv[0]));
	  sqlite3_result_double (context, x);
      }
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
	  x = exp (x);
	  sqlite3_result_double (context, x);
      }
    else
	sqlite3_result_null (context);
}

static void
fnct_math_floor (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ floor(double X)
/
/ Returns the largest integer value not greater than X
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
      {
	  x = floor (sqlite3_value_double (argv[0]));
	  sqlite3_result_double (context, x);
      }
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
	  x = floor (x);
	  sqlite3_result_double (context, x);
      }
    else
	sqlite3_result_null (context);
}

static void
fnct_math_logn (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ log(double X)
/
/ Returns the natural logarithm of X; that is, the base-e logarithm of X
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    errno = 0;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
      {
	  x = log (sqlite3_value_double (argv[0]));
	  if (errno == EDOM || errno == ERANGE)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_double (context, x);
      }
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
	  x = log (x);
	  if (errno == EDOM || errno == ERANGE)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_double (context, x);
      }
    else
	sqlite3_result_null (context);
}

static void
fnct_math_logn2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ log(double B, double X)
/
/ Returns the logarithm of X to the base B
/ or NULL if any error is encountered
*/
    int int_value;
    double x = 0.0;
    double b = 1.0;
    double log1;
    double log2;
    errno = 0;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  b = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	b = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (x <= 0.0 || b <= 1.0)
      {
	  sqlite3_result_null (context);
	  return;
      }
    log1 = log (x);
    if (errno == EDOM || errno == ERANGE)
      {
	  sqlite3_result_null (context);
	  return;
      }
    log2 = log (b);
    if (errno == EDOM || errno == ERANGE)
      {
	  sqlite3_result_null (context);
	  return;
      }
    sqlite3_result_double (context, log1 / log2);
}

static void
fnct_math_log_2 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ log2(double X)
/
/ Returns the base-2 logarithm of X
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    double log1;
    double log2;
    errno = 0;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    log1 = log (x);
    if (errno == EDOM || errno == ERANGE)
      {
	  sqlite3_result_null (context);
	  return;
      }
    log2 = log (2.0);
    sqlite3_result_double (context, log1 / log2);
}

static void
fnct_math_log_10 (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ log10(double X)
/
/ Returns the base-10 logarithm of X
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    double log1;
    double log2;
    errno = 0;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    log1 = log (x);
    if (errno == EDOM || errno == ERANGE)
      {
	  sqlite3_result_null (context);
	  return;
      }
    log2 = log (10.0);
    sqlite3_result_double (context, log1 / log2);
}

static void
fnct_math_pi (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ pi(void)
/
/ Returns the value of (pi)
*/
    sqlite3_result_double (context, 3.14159265358979323846);
}

static void
fnct_math_pow (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ pow(double X, double Y)
/
/ Returns the value of X raised to the power of Y.
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    double y;
    double p;
    errno = 0;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (sqlite3_value_type (argv[1]) == SQLITE_FLOAT)
	y = sqlite3_value_double (argv[1]);
    else if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[1]);
	  y = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    p = pow (x, y);
    if (errno == EDOM)
	sqlite3_result_null (context);
    else
	sqlite3_result_double (context, p);
}

static void
fnct_math_stddev_step (sqlite3_context * context, int argc,
		       sqlite3_value ** argv)
{
/* SQL function:
/ stddev_pop(double X)
/ stddev_samp(double X)
/ var_pop(double X)
/ var_samp(double X)
/
/ aggregate function - STEP
/
*/
    struct stddev_str *p;
    int int_value;
    double x;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
      }
    else
	return;
    p = sqlite3_aggregate_context (context, sizeof (struct stddev_str));
    if (!(p->cleaned))
      {
	  p->cleaned = 1;
	  p->mean = x;
	  p->quot = 0.0;
	  p->count = 0.0;
      }
    p->count += 1.0;
    p->quot =
	p->quot +
	(((p->count - 1.0) * ((x - p->mean) * (x - p->mean))) / p->count);
    p->mean = p->mean + ((x - p->mean) / p->count);
}

static void
fnct_math_stddev_pop_final (sqlite3_context * context)
{
/* SQL function:
/ stddev_pop(double X)
/ aggregate function -  FINAL
/
*/
    double x;
    struct stddev_str *p = sqlite3_aggregate_context (context, 0);
    if (!p)
      {
	  sqlite3_result_null (context);
	  return;
      }
    x = sqrt (p->quot / (p->count - 1.0));
    sqlite3_result_double (context, x);
}

static void
fnct_math_stddev_samp_final (sqlite3_context * context)
{
/* SQL function:
/ stddev_samp(double X)
/ aggregate function -  FINAL
/
*/
    double x;
    struct stddev_str *p = sqlite3_aggregate_context (context, 0);
    if (!p)
      {
	  sqlite3_result_null (context);
	  return;
      }
    x = sqrt (p->quot / p->count);
    sqlite3_result_double (context, x);
}

static void
fnct_math_var_pop_final (sqlite3_context * context)
{
/* SQL function:
/ var_pop(double X)
/ aggregate function -  FINAL
/
*/
    double x;
    struct stddev_str *p = sqlite3_aggregate_context (context, 0);
    if (!p)
      {
	  sqlite3_result_null (context);
	  return;
      }
    x = p->quot / (p->count - 1.0);
    sqlite3_result_double (context, x);
}

static void
fnct_math_var_samp_final (sqlite3_context * context)
{
/* SQL function:
/ var_samp(double X)
/ aggregate function -  FINAL
/
*/
    double x;
    struct stddev_str *p = sqlite3_aggregate_context (context, 0);
    if (!p)
      {
	  sqlite3_result_null (context);
	  return;
      }
    x = p->quot / p->count;
    sqlite3_result_double (context, x);
}

static void
fnct_math_radians (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ radians(double X)
/
/ Returns the argument X, converted from degrees to radians
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    x = x * .0174532925199432958;
    sqlite3_result_double (context, x);
}


static void
fnct_math_round (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ round(double X)
/
/ Returns the the nearest integer, but round halfway cases away from zero
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
      {
	  x = math_round (sqlite3_value_double (argv[0]));
	  sqlite3_result_double (context, x);
      }
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
	  x = math_round (x);
	  sqlite3_result_double (context, x);
      }
    else
	sqlite3_result_null (context);
}

static void
fnct_math_sign (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ sign(double X)
/
/ Returns the sign of the argument as -1, 0, or 1, depending on whether X is negative, zero, or positive
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
	x = sqlite3_value_double (argv[0]);
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
      }
    else
      {
	  sqlite3_result_null (context);
	  return;
      }
    if (x > 0.0)
	sqlite3_result_double (context, 1.0);
    else if (x < 0.0)
	sqlite3_result_double (context, -1.0);
    else
	sqlite3_result_double (context, 0.0);
}

static void
fnct_math_sin (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ sin(double X)
/
/ Returns the sine of X, where X is given in radians
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
      {
	  x = sin (sqlite3_value_double (argv[0]));
	  sqlite3_result_double (context, x);
      }
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
	  x = sin (x);
	  sqlite3_result_double (context, x);
      }
    else
	sqlite3_result_null (context);
}

static void
fnct_math_sqrt (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ sqrt(double X)
/
/ Returns the square root of a non-negative number X
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    errno = 0;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
      {
	  x = sqrt (sqlite3_value_double (argv[0]));
	  if (errno)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_double (context, x);
      }
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
	  x = sqrt (x);
	  if (errno == EDOM)
	      sqlite3_result_null (context);
	  else
	      sqlite3_result_double (context, x);
      }
    else
	sqlite3_result_null (context);
}

static void
fnct_math_tan (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
/* SQL function:
/ tan(double X)
/
/ Returns the tangent of X, where X is given in radians
/ or NULL if any error is encountered
*/
    int int_value;
    double x;
    if (sqlite3_value_type (argv[0]) == SQLITE_FLOAT)
      {
	  x = tan (sqlite3_value_double (argv[0]));
	  sqlite3_result_double (context, x);
      }
    else if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
      {
	  int_value = sqlite3_value_int (argv[0]);
	  x = int_value;
	  x = tan (x);
	  sqlite3_result_double (context, x);
      }
    else
	sqlite3_result_null (context);
}

#endif /* end supporting SQL math functions */

static void
fnct_GeomFromExifGpsBlob (sqlite3_context * context, int argc,
			  sqlite3_value ** argv)
{
/* SQL function:
/ GeomFromExifGpsBlob(BLOB encoded image)
/
/ returns:
/ a POINT geometry
/ or NULL if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    gaiaGeomCollPtr geom;
    unsigned char *geoblob;
    int geosize;
    double longitude;
    double latitude;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_null (context);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    if (gaiaGetGpsCoords (p_blob, n_bytes, &longitude, &latitude))
      {
	  geom = gaiaAllocGeomColl ();
	  geom->Srid = 4326;
	  gaiaAddPointToGeomColl (geom, longitude, latitude);
	  gaiaToSpatiaLiteBlobWkb (geom, &geoblob, &geosize);
	  gaiaFreeGeomColl (geom);
	  sqlite3_result_blob (context, geoblob, geosize, free);
      }
    else
	sqlite3_result_null (context);
}

static void
blob_guess (sqlite3_context * context, int argc, sqlite3_value ** argv,
	    int request)
{
/* SQL function:
/ IsGifBlob(BLOB encoded image)
/ IsPngBlob, IsJpegBlob, IsExifBlob, IsExifGpsBlob, IsZipBlob, IsPdfBlob,IsGeometryBlob
/
/ returns:
/ 1 if the required BLOB_TYPE is TRUE
/ 0 otherwise
/ or -1 if any error is encountered
*/
    unsigned char *p_blob;
    int n_bytes;
    int blob_type;
    if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
      {
	  sqlite3_result_int (context, -1);
	  return;
      }
    p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
    n_bytes = sqlite3_value_bytes (argv[0]);
    blob_type = gaiaGuessBlobType (p_blob, n_bytes);
    if (request == GAIA_GEOMETRY_BLOB)
      {
	  if (blob_type == GAIA_GEOMETRY_BLOB)
	      sqlite3_result_int (context, 1);
	  else
	      sqlite3_result_int (context, 0);
	  return;
      }
    if (request == GAIA_ZIP_BLOB)
      {
	  if (blob_type == GAIA_ZIP_BLOB)
	      sqlite3_result_int (context, 1);
	  else
	      sqlite3_result_int (context, 0);
	  return;
      }
    if (request == GAIA_PDF_BLOB)
      {
	  if (blob_type == GAIA_PDF_BLOB)
	      sqlite3_result_int (context, 1);
	  else
	      sqlite3_result_int (context, 0);
	  return;
      }
    if (request == GAIA_GIF_BLOB)
      {
	  if (blob_type == GAIA_GIF_BLOB)
	      sqlite3_result_int (context, 1);
	  else
	      sqlite3_result_int (context, 0);
	  return;
      }
    if (request == GAIA_PNG_BLOB)
      {
	  if (blob_type == GAIA_PNG_BLOB)
	      sqlite3_result_int (context, 1);
	  else
	      sqlite3_result_int (context, 0);
	  return;
      }
    if (request == GAIA_JPEG_BLOB)
      {
	  if (blob_type == GAIA_JPEG_BLOB || blob_type == GAIA_EXIF_BLOB
	      || blob_type == GAIA_EXIF_GPS_BLOB)
	      sqlite3_result_int (context, 1);
	  else
	      sqlite3_result_int (context, 0);
	  return;
      }
    if (request == GAIA_EXIF_BLOB)
      {
	  if (blob_type == GAIA_EXIF_BLOB || blob_type == GAIA_EXIF_GPS_BLOB)
	    {
		sqlite3_result_int (context, 1);
	    }
	  else
	      sqlite3_result_int (context, 0);
	  return;
      }
    if (request == GAIA_EXIF_GPS_BLOB)
      {
	  if (blob_type == GAIA_EXIF_GPS_BLOB)
	    {
		sqlite3_result_int (context, 1);
	    }
	  else
	      sqlite3_result_int (context, 0);
	  return;
      }
    sqlite3_result_int (context, -1);
}

/*
/ the following functions simply readdress the blob_guess()
/ setting the appropriate request mode
*/

static void
fnct_IsGeometryBlob (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    blob_guess (context, argc, argv, GAIA_GEOMETRY_BLOB);
}

static void
fnct_IsZipBlob (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    blob_guess (context, argc, argv, GAIA_ZIP_BLOB);
}

static void
fnct_IsPdfBlob (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    blob_guess (context, argc, argv, GAIA_PDF_BLOB);
}

static void
fnct_IsGifBlob (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    blob_guess (context, argc, argv, GAIA_GIF_BLOB);
}

static void
fnct_IsPngBlob (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    blob_guess (context, argc, argv, GAIA_PNG_BLOB);
}

static void
fnct_IsJpegBlob (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    blob_guess (context, argc, argv, GAIA_JPEG_BLOB);
}

static void
fnct_IsExifBlob (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    blob_guess (context, argc, argv, GAIA_EXIF_BLOB);
}

static void
fnct_IsExifGpsBlob (sqlite3_context * context, int argc, sqlite3_value ** argv)
{
    blob_guess (context, argc, argv, GAIA_EXIF_GPS_BLOB);
}

static void
init_static_spatialite (sqlite3 * db, char **pzErrMsg,
			const sqlite3_api_routines * pApi)
{
    SQLITE_EXTENSION_INIT2 (pApi);
/* setting the POSIX locale for numeric */
    setlocale (LC_NUMERIC, "POSIX");
    sqlite3_create_function (db, "GeometryConstraints", 3, SQLITE_ANY, 0,
			     fnct_GeometryConstraints, 0, 0);
    sqlite3_create_function (db, "CheckSpatialMetaData", 0, SQLITE_ANY, 0,
			     fnct_CheckSpatialMetaData, 0, 0);
    sqlite3_create_function (db, "AutoFDOStart", 0, SQLITE_ANY, 0,
			     fnct_AutoFDOStart, 0, 0);
    sqlite3_create_function (db, "AutoFDOStop", 0, SQLITE_ANY, 0,
			     fnct_AutoFDOStop, 0, 0);
    sqlite3_create_function (db, "InitFDOSpatialMetaData", 0, SQLITE_ANY, 0,
			     fnct_InitFDOSpatialMetaData, 0, 0);
    sqlite3_create_function (db, "AddFDOGeometryColumn", 6, SQLITE_ANY, 0,
			     fnct_AddFDOGeometryColumn, 0, 0);
    sqlite3_create_function (db, "RecoverFDOGeometryColumn", 6, SQLITE_ANY, 0,
			     fnct_RecoverFDOGeometryColumn, 0, 0);
    sqlite3_create_function (db, "DiscardFDOGeometryColumn", 2, SQLITE_ANY, 0,
			     fnct_DiscardFDOGeometryColumn, 0, 0);
    sqlite3_create_function (db, "InitSpatialMetaData", 0, SQLITE_ANY, 0,
			     fnct_InitSpatialMetaData, 0, 0);
    sqlite3_create_function (db, "AddGeometryColumn", 5, SQLITE_ANY, 0,
			     fnct_AddGeometryColumn, 0, 0);
    sqlite3_create_function (db, "AddGeometryColumn", 6, SQLITE_ANY, 0,
			     fnct_AddGeometryColumn, 0, 0);
    sqlite3_create_function (db, "RecoverGeometryColumn", 5, SQLITE_ANY, 0,
			     fnct_RecoverGeometryColumn, 0, 0);
    sqlite3_create_function (db, "DiscardGeometryColumn", 2, SQLITE_ANY, 0,
			     fnct_DiscardGeometryColumn, 0, 0);
    sqlite3_create_function (db, "CreateSpatialIndex", 2, SQLITE_ANY, 0,
			     fnct_CreateSpatialIndex, 0, 0);
    sqlite3_create_function (db, "CreateMbrCache", 2, SQLITE_ANY, 0,
			     fnct_CreateMbrCache, 0, 0);
    sqlite3_create_function (db, "DisableSpatialIndex", 2, SQLITE_ANY, 0,
			     fnct_DisableSpatialIndex, 0, 0);
    sqlite3_create_function (db, "RebuildGeometryTriggers", 2, SQLITE_ANY, 0,
			     fnct_RebuildGeometryTriggers, 0, 0);
    sqlite3_create_function (db, "AsText", 1, SQLITE_ANY, 0, fnct_AsText, 0, 0);
    sqlite3_create_function (db, "AsSvg", 1, SQLITE_ANY, 0, fnct_AsSvg1, 0, 0);
    sqlite3_create_function (db, "AsSvg", 2, SQLITE_ANY, 0, fnct_AsSvg2, 0, 0);
    sqlite3_create_function (db, "AsSvg", 3, SQLITE_ANY, 0, fnct_AsSvg3, 0, 0);
    sqlite3_create_function (db, "AsFGF", 2, SQLITE_ANY, 0, fnct_AsFGF, 0, 0);
    sqlite3_create_function (db, "AsBinary", 1, SQLITE_ANY, 0, fnct_AsBinary, 0,
			     0);
    sqlite3_create_function (db, "GeomFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomFromText1, 0, 0);
    sqlite3_create_function (db, "GeomFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomFromText2, 0, 0);
    sqlite3_create_function (db, "GeometryFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomFromText1, 0, 0);
    sqlite3_create_function (db, "GeometryFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomFromText2, 0, 0);
    sqlite3_create_function (db, "GeomCollFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromText1, 0, 0);
    sqlite3_create_function (db, "GeomCollFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromText2, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromText1, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromText2, 0, 0);
    sqlite3_create_function (db, "PointFromText", 1, SQLITE_ANY, 0,
			     fnct_PointFromText1, 0, 0);
    sqlite3_create_function (db, "PointFromText", 2, SQLITE_ANY, 0,
			     fnct_PointFromText2, 0, 0);
    sqlite3_create_function (db, "LineFromText", 1, SQLITE_ANY, 0,
			     fnct_LineFromText1, 0, 0);
    sqlite3_create_function (db, "LineFromText", 2, SQLITE_ANY, 0,
			     fnct_LineFromText2, 0, 0);
    sqlite3_create_function (db, "LineStringFromText", 1, SQLITE_ANY, 0,
			     fnct_LineFromText1, 0, 0);
    sqlite3_create_function (db, "LineStringFromText", 2, SQLITE_ANY, 0,
			     fnct_LineFromText2, 0, 0);
    sqlite3_create_function (db, "PolyFromText", 1, SQLITE_ANY, 0,
			     fnct_PolyFromText1, 0, 0);
    sqlite3_create_function (db, "PolyFromText", 2, SQLITE_ANY, 0,
			     fnct_PolyFromText2, 0, 0);
    sqlite3_create_function (db, "PolygonFromText", 1, SQLITE_ANY, 0,
			     fnct_PolyFromText1, 0, 0);
    sqlite3_create_function (db, "PolygonFromText", 2, SQLITE_ANY, 0,
			     fnct_PolyFromText2, 0, 0);
    sqlite3_create_function (db, "MPointFromText", 1, SQLITE_ANY, 0,
			     fnct_MPointFromText1, 0, 0);
    sqlite3_create_function (db, "MPointFromText", 2, SQLITE_ANY, 0,
			     fnct_MPointFromText2, 0, 0);
    sqlite3_create_function (db, "MultiPointFromText", 1, SQLITE_ANY, 0,
			     fnct_MPointFromText1, 0, 0);
    sqlite3_create_function (db, "MultiPointFromText", 2, SQLITE_ANY, 0,
			     fnct_MPointFromText2, 0, 0);
    sqlite3_create_function (db, "MLineFromText", 1, SQLITE_ANY, 0,
			     fnct_MLineFromText1, 0, 0);
    sqlite3_create_function (db, "MLineFromText", 2, SQLITE_ANY, 0,
			     fnct_MLineFromText2, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromText", 1, SQLITE_ANY, 0,
			     fnct_MLineFromText1, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromText", 2, SQLITE_ANY, 0,
			     fnct_MLineFromText2, 0, 0);
    sqlite3_create_function (db, "MPolyFromText", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromText1, 0, 0);
    sqlite3_create_function (db, "MPolyFromText", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromText2, 0, 0);
    sqlite3_create_function (db, "MultiPolygonFromText", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromText1, 0, 0);
    sqlite3_create_function (db, "MultiPolygonFromText", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromText2, 0, 0);
    sqlite3_create_function (db, "GeomFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeomFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomFromWkb2, 0, 0);
    sqlite3_create_function (db, "GeometryFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeometryFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomFromWkb2, 0, 0);
    sqlite3_create_function (db, "GeomCollFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeomCollFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb2, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb2, 0, 0);
    sqlite3_create_function (db, "PointFromWKB", 1, SQLITE_ANY, 0,
			     fnct_PointFromWkb1, 0, 0);
    sqlite3_create_function (db, "PointFromWKB", 2, SQLITE_ANY, 0,
			     fnct_PointFromWkb2, 0, 0);
    sqlite3_create_function (db, "LineFromWKB", 1, SQLITE_ANY, 0,
			     fnct_LineFromWkb1, 0, 0);
    sqlite3_create_function (db, "LineFromWKB", 2, SQLITE_ANY, 0,
			     fnct_LineFromWkb2, 0, 0);
    sqlite3_create_function (db, "LineStringFromWKB", 1, SQLITE_ANY, 0,
			     fnct_LineFromWkb1, 0, 0);
    sqlite3_create_function (db, "LineStringFromWKB", 2, SQLITE_ANY, 0,
			     fnct_LineFromWkb2, 0, 0);
    sqlite3_create_function (db, "PolyFromWKB", 1, SQLITE_ANY, 0,
			     fnct_PolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "PolyFromWKB", 2, SQLITE_ANY, 0,
			     fnct_PolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "PolygonFromWKB", 1, SQLITE_ANY, 0,
			     fnct_PolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "PolygonFromWKB", 2, SQLITE_ANY, 0,
			     fnct_PolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "MPointFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPointFromWkb1, 0, 0);
    sqlite3_create_function (db, "MPointFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPointFromWkb2, 0, 0);
    sqlite3_create_function (db, "MultiPointFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPointFromWkb1, 0, 0);
    sqlite3_create_function (db, "MultiPointFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPointFromWkb2, 0, 0);
    sqlite3_create_function (db, "MLineFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MLineFromWkb1, 0, 0);
    sqlite3_create_function (db, "MLineFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MLineFromWkb2, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MLineFromWkb1, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MLineFromWkb2, 0, 0);
    sqlite3_create_function (db, "MPolyFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "MPolyFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "MultiPolygonFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "MultiPolygonFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "GeomFromFGF", 1, SQLITE_ANY, 0,
			     fnct_GeometryFromFGF1, 0, 0);
    sqlite3_create_function (db, "GeomFromFGF", 2, SQLITE_ANY, 0,
			     fnct_GeometryFromFGF2, 0, 0);
    sqlite3_create_function (db, "Dimension", 1, SQLITE_ANY, 0, fnct_Dimension,
			     0, 0);
    sqlite3_create_function (db, "GeometryType", 1, SQLITE_ANY, 0,
			     fnct_GeometryType, 0, 0);
    sqlite3_create_function (db, "GeometryAliasType", 1, SQLITE_ANY, 0,
			     fnct_GeometryAliasType, 0, 0);
    sqlite3_create_function (db, "SRID", 1, SQLITE_ANY, 0, fnct_SRID, 0, 0);
    sqlite3_create_function (db, "SetSRID", 2, SQLITE_ANY, 0, fnct_SetSRID, 0,
			     0);
    sqlite3_create_function (db, "IsEmpty", 1, SQLITE_ANY, 0, fnct_IsEmpty, 0,
			     0);
    sqlite3_create_function (db, "Envelope", 1, SQLITE_ANY, 0, fnct_Envelope, 0,
			     0);
    sqlite3_create_function (db, "X", 1, SQLITE_ANY, 0, fnct_X, 0, 0);
    sqlite3_create_function (db, "Y", 1, SQLITE_ANY, 0, fnct_Y, 0, 0);
    sqlite3_create_function (db, "NumPoints", 1, SQLITE_ANY, 0, fnct_NumPoints,
			     0, 0);
    sqlite3_create_function (db, "StartPoint", 1, SQLITE_ANY, 0,
			     fnct_StartPoint, 0, 0);
    sqlite3_create_function (db, "EndPoint", 1, SQLITE_ANY, 0, fnct_EndPoint, 0,
			     0);
    sqlite3_create_function (db, "PointN", 2, SQLITE_ANY, 0, fnct_PointN, 0, 0);
    sqlite3_create_function (db, "ExteriorRing", 1, SQLITE_ANY, 0,
			     fnct_ExteriorRing, 0, 0);
    sqlite3_create_function (db, "NumInteriorRing", 1, SQLITE_ANY, 0,
			     fnct_NumInteriorRings, 0, 0);
    sqlite3_create_function (db, "NumInteriorRings", 1, SQLITE_ANY, 0,
			     fnct_NumInteriorRings, 0, 0);
    sqlite3_create_function (db, "InteriorRingN", 2, SQLITE_ANY, 0,
			     fnct_InteriorRingN, 0, 0);
    sqlite3_create_function (db, "NumGeometries", 1, SQLITE_ANY, 0,
			     fnct_NumGeometries, 0, 0);
    sqlite3_create_function (db, "GeometryN", 2, SQLITE_ANY, 0, fnct_GeometryN,
			     0, 0);
    sqlite3_create_function (db, "MBRContains", 2, SQLITE_ANY, 0,
			     fnct_MbrContains, 0, 0);
    sqlite3_create_function (db, "MbrDisjoint", 2, SQLITE_ANY, 0,
			     fnct_MbrDisjoint, 0, 0);
    sqlite3_create_function (db, "MBREqual", 2, SQLITE_ANY, 0, fnct_MbrEqual, 0,
			     0);
    sqlite3_create_function (db, "MbrIntersects", 2, SQLITE_ANY, 0,
			     fnct_MbrIntersects, 0, 0);
    sqlite3_create_function (db, "MBROverlaps", 2, SQLITE_ANY, 0,
			     fnct_MbrOverlaps, 0, 0);
    sqlite3_create_function (db, "MbrTouches", 2, SQLITE_ANY, 0,
			     fnct_MbrTouches, 0, 0);
    sqlite3_create_function (db, "MbrWithin", 2, SQLITE_ANY, 0, fnct_MbrWithin,
			     0, 0);
    sqlite3_create_function (db, "ShiftCoords", 3, SQLITE_ANY, 0,
			     fnct_ShiftCoords, 0, 0);
    sqlite3_create_function (db, "ShiftCoordinates", 3, SQLITE_ANY, 0,
			     fnct_ShiftCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoords", 2, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoordinates", 2, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoords", 3, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoordinates", 3, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "RotateCoords", 2, SQLITE_ANY, 0,
			     fnct_RotateCoords, 0, 0);
    sqlite3_create_function (db, "RotateCoordinates", 2, SQLITE_ANY, 0,
			     fnct_RotateCoords, 0, 0);
    sqlite3_create_function (db, "ReflectCoords", 3, SQLITE_ANY, 0,
			     fnct_ReflectCoords, 0, 0);
    sqlite3_create_function (db, "ReflectCoordinates", 3, SQLITE_ANY, 0,
			     fnct_ReflectCoords, 0, 0);
    sqlite3_create_function (db, "SwapCoords", 1, SQLITE_ANY, 0,
			     fnct_ReflectCoords, 0, 0);
    sqlite3_create_function (db, "SwapCoordinates", 1, SQLITE_ANY, 0,
			     fnct_ReflectCoords, 0, 0);
    sqlite3_create_function (db, "BuildMbr", 4, SQLITE_ANY, 0, fnct_BuildMbr1,
			     0, 0);
    sqlite3_create_function (db, "BuildMbr", 5, SQLITE_ANY, 0, fnct_BuildMbr2,
			     0, 0);
    sqlite3_create_function (db, "BuildCircleMbr", 3, SQLITE_ANY, 0,
			     fnct_BuildCircleMbr1, 0, 0);
    sqlite3_create_function (db, "BuildCircleMbr", 4, SQLITE_ANY, 0,
			     fnct_BuildCircleMbr2, 0, 0);
    sqlite3_create_function (db, "MbrMinX", 1, SQLITE_ANY, 0, fnct_MbrMinX, 0,
			     0);
    sqlite3_create_function (db, "MbrMaxX", 1, SQLITE_ANY, 0, fnct_MbrMaxX, 0,
			     0);
    sqlite3_create_function (db, "MbrMinY", 1, SQLITE_ANY, 0, fnct_MbrMinY, 0,
			     0);
    sqlite3_create_function (db, "MbrMaxY", 1, SQLITE_ANY, 0, fnct_MbrMaxY, 0,
			     0);
    sqlite3_create_function (db, "MakePoint", 2, SQLITE_ANY, 0, fnct_MakePoint1,
			     0, 0);
    sqlite3_create_function (db, "MakePoint", 3, SQLITE_ANY, 0, fnct_MakePoint2,
			     0, 0);
    sqlite3_create_function (db, "BuildMbrFilter", 4, SQLITE_ANY, 0,
			     fnct_BuildMbrFilter, 0, 0);
    sqlite3_create_function (db, "FilterMbrWithin", 4, SQLITE_ANY, 0,
			     fnct_FilterMbrWithin, 0, 0);
    sqlite3_create_function (db, "FilterMbrContains", 4, SQLITE_ANY, 0,
			     fnct_FilterMbrContains, 0, 0);
    sqlite3_create_function (db, "FilterMbrIntersects", 4, SQLITE_ANY, 0,
			     fnct_FilterMbrIntersects, 0, 0);
    sqlite3_create_function (db, "BuildRings", 1, SQLITE_ANY, 0,
			     fnct_BuildRings, 0, 0);

/* some BLOB/JPEG/EXIF functions */
    sqlite3_create_function (db, "IsGeometryBlob", 1, SQLITE_ANY, 0,
			     fnct_IsGeometryBlob, 0, 0);
    sqlite3_create_function (db, "IsZipBlob", 1, SQLITE_ANY, 0, fnct_IsZipBlob,
			     0, 0);
    sqlite3_create_function (db, "IsPdfBlob", 1, SQLITE_ANY, 0, fnct_IsPdfBlob,
			     0, 0);
    sqlite3_create_function (db, "IsGifBlob", 1, SQLITE_ANY, 0, fnct_IsGifBlob,
			     0, 0);
    sqlite3_create_function (db, "IsPngBlob", 1, SQLITE_ANY, 0, fnct_IsPngBlob,
			     0, 0);
    sqlite3_create_function (db, "IsJpegBlob", 1, SQLITE_ANY, 0,
			     fnct_IsJpegBlob, 0, 0);
    sqlite3_create_function (db, "IsExifBlob", 1, SQLITE_ANY, 0,
			     fnct_IsExifBlob, 0, 0);
    sqlite3_create_function (db, "IsExifGpsBlob", 1, SQLITE_ANY, 0,
			     fnct_IsExifGpsBlob, 0, 0);
    sqlite3_create_function (db, "GeomFromExifGpsBlob", 1, SQLITE_ANY, 0,
			     fnct_GeomFromExifGpsBlob, 0, 0);

#if OMIT_MATHSQL == 0		/* supporting SQL math functions */

/* some extra math functions */
    sqlite3_create_function (db, "abs", 1, SQLITE_ANY, 0, fnct_math_abs, 0, 0);
    sqlite3_create_function (db, "acos", 1, SQLITE_ANY, 0, fnct_math_acos, 0,
			     0);
    sqlite3_create_function (db, "asin", 1, SQLITE_ANY, 0, fnct_math_asin, 0,
			     0);
    sqlite3_create_function (db, "atan", 1, SQLITE_ANY, 0, fnct_math_atan, 0,
			     0);
    sqlite3_create_function (db, "ceil", 1, SQLITE_ANY, 0, fnct_math_ceil, 0,
			     0);
    sqlite3_create_function (db, "ceiling", 1, SQLITE_ANY, 0, fnct_math_ceil, 0,
			     0);
    sqlite3_create_function (db, "cos", 1, SQLITE_ANY, 0, fnct_math_cos, 0, 0);
    sqlite3_create_function (db, "cot", 1, SQLITE_ANY, 0, fnct_math_cot, 0, 0);
    sqlite3_create_function (db, "degrees", 1, SQLITE_ANY, 0, fnct_math_degrees,
			     0, 0);
    sqlite3_create_function (db, "exp", 1, SQLITE_ANY, 0, fnct_math_exp, 0, 0);
    sqlite3_create_function (db, "floor", 1, SQLITE_ANY, 0, fnct_math_floor, 0,
			     0);
    sqlite3_create_function (db, "ln", 1, SQLITE_ANY, 0, fnct_math_logn, 0, 0);
    sqlite3_create_function (db, "log", 1, SQLITE_ANY, 0, fnct_math_logn, 0, 0);
    sqlite3_create_function (db, "log", 2, SQLITE_ANY, 0, fnct_math_logn2, 0,
			     0);
    sqlite3_create_function (db, "log2", 1, SQLITE_ANY, 0, fnct_math_log_2, 0,
			     0);
    sqlite3_create_function (db, "log10", 1, SQLITE_ANY, 0, fnct_math_log_10, 0,
			     0);
    sqlite3_create_function (db, "pi", 0, SQLITE_ANY, 0, fnct_math_pi, 0, 0);
    sqlite3_create_function (db, "pow", 2, SQLITE_ANY, 0, fnct_math_pow, 0, 0);
    sqlite3_create_function (db, "power", 2, SQLITE_ANY, 0, fnct_math_pow, 0,
			     0);
    sqlite3_create_function (db, "radians", 1, SQLITE_ANY, 0, fnct_math_radians,
			     0, 0);
    sqlite3_create_function (db, "round", 1, SQLITE_ANY, 0, fnct_math_round, 0,
			     0);
    sqlite3_create_function (db, "sign", 1, SQLITE_ANY, 0, fnct_math_sign, 0,
			     0);
    sqlite3_create_function (db, "sin", 1, SQLITE_ANY, 0, fnct_math_sin, 0, 0);
    sqlite3_create_function (db, "stddev_pop", 1, SQLITE_ANY, 0, 0,
			     fnct_math_stddev_step, fnct_math_stddev_pop_final);
    sqlite3_create_function (db, "stddev_samp", 1, SQLITE_ANY, 0, 0,
			     fnct_math_stddev_step,
			     fnct_math_stddev_samp_final);
    sqlite3_create_function (db, "sqrt", 1, SQLITE_ANY, 0, fnct_math_sqrt, 0,
			     0);
    sqlite3_create_function (db, "tan", 1, SQLITE_ANY, 0, fnct_math_tan, 0, 0);
    sqlite3_create_function (db, "var_pop", 1, SQLITE_ANY, 0, 0,
			     fnct_math_stddev_step, fnct_math_var_pop_final);
    sqlite3_create_function (db, "var_samp", 1, SQLITE_ANY, 0, 0,
			     fnct_math_stddev_step, fnct_math_var_samp_final);

#endif /* end supporting SQL math functions */

#if OMIT_PROJ == 0		/* including PROJ.4 */

    sqlite3_create_function (db, "Transform", 2, SQLITE_ANY, 0, fnct_Transform,
			     0, 0);

#endif /* end including PROJ.4 */

#if OMIT_GEOS == 0		/* including GEOS */

    initGEOS (geos_error, geos_error);
    sqlite3_create_function (db, "Boundary", 1, SQLITE_ANY, 0, fnct_Boundary, 0,
			     0);
    sqlite3_create_function (db, "IsClosed", 1, SQLITE_ANY, 0, fnct_IsClosed, 0,
			     0);
    sqlite3_create_function (db, "IsSimple", 1, SQLITE_ANY, 0, fnct_IsSimple, 0,
			     0);
    sqlite3_create_function (db, "IsRing", 1, SQLITE_ANY, 0, fnct_IsRing, 0, 0);
    sqlite3_create_function (db, "IsValid", 1, SQLITE_ANY, 0, fnct_IsValid, 0,
			     0);
    sqlite3_create_function (db, "GLength", 1, SQLITE_ANY, 0, fnct_Length, 0,
			     0);
    sqlite3_create_function (db, "Area", 1, SQLITE_ANY, 0, fnct_Area, 0, 0);
    sqlite3_create_function (db, "Centroid", 1, SQLITE_ANY, 0, fnct_Centroid, 0,
			     0);
    sqlite3_create_function (db, "PointOnSurface", 1, SQLITE_ANY, 0,
			     fnct_PointOnSurface, 0, 0);
    sqlite3_create_function (db, "Simplify", 2, SQLITE_ANY, 0, fnct_Simplify, 0,
			     0);
    sqlite3_create_function (db, "SimplifyPreserveTopology", 2, SQLITE_ANY, 0,
			     fnct_SimplifyPreserveTopology, 0, 0);
    sqlite3_create_function (db, "ConvexHull", 1, SQLITE_ANY, 0,
			     fnct_ConvexHull, 0, 0);
    sqlite3_create_function (db, "Buffer", 2, SQLITE_ANY, 0, fnct_Buffer, 0, 0);
    sqlite3_create_function (db, "Intersection", 2, SQLITE_ANY, 0,
			     fnct_Intersection, 0, 0);
    sqlite3_create_function (db, "GUnion", 1, SQLITE_ANY, 0, 0, fnct_Union_step,
			     fnct_Union_final);
    sqlite3_create_function (db, "GUnion", 2, SQLITE_ANY, 0, fnct_Union, 0, 0);
    sqlite3_create_function (db, "Difference", 2, SQLITE_ANY, 0,
			     fnct_Difference, 0, 0);
    sqlite3_create_function (db, "SymDifference", 2, SQLITE_ANY, 0,
			     fnct_SymDifference, 0, 0);
    sqlite3_create_function (db, "Equals", 2, SQLITE_ANY, 0, fnct_Equals, 0, 0);
    sqlite3_create_function (db, "Intersects", 2, SQLITE_ANY, 0,
			     fnct_Intersects, 0, 0);
    sqlite3_create_function (db, "Disjoint", 2, SQLITE_ANY, 0, fnct_Disjoint, 0,
			     0);
    sqlite3_create_function (db, "Overlaps", 2, SQLITE_ANY, 0, fnct_Overlaps, 0,
			     0);
    sqlite3_create_function (db, "Crosses", 2, SQLITE_ANY, 0, fnct_Crosses, 0,
			     0);
    sqlite3_create_function (db, "Touches", 2, SQLITE_ANY, 0, fnct_Touches, 0,
			     0);
    sqlite3_create_function (db, "Within", 2, SQLITE_ANY, 0, fnct_Within, 0, 0);
    sqlite3_create_function (db, "Contains", 2, SQLITE_ANY, 0, fnct_Contains, 0,
			     0);
    sqlite3_create_function (db, "Relate", 3, SQLITE_ANY, 0, fnct_Relate, 0, 0);
    sqlite3_create_function (db, "Distance", 2, SQLITE_ANY, 0, fnct_Distance, 0,
			     0);
    sqlite3_create_function (db, "BdPolyFromText", 1, SQLITE_ANY, 0,
			     fnct_BdPolyFromText1, 0, 0);
    sqlite3_create_function (db, "BdPolyFromText", 2, SQLITE_ANY, 0,
			     fnct_BdPolyFromText2, 0, 0);
    sqlite3_create_function (db, "BdMPolyFromText", 1, SQLITE_ANY, 0,
			     fnct_BdMPolyFromText1, 0, 0);
    sqlite3_create_function (db, "BdMPolyFromText", 2, SQLITE_ANY, 0,
			     fnct_BdMPolyFromText2, 0, 0);
    sqlite3_create_function (db, "BdPolyFromWKB", 1, SQLITE_ANY, 0,
			     fnct_BdPolyFromWKB1, 0, 0);
    sqlite3_create_function (db, "BdPolyFromWKB", 2, SQLITE_ANY, 0,
			     fnct_BdPolyFromWKB2, 0, 0);
    sqlite3_create_function (db, "BdMPolyFromWKB", 1, SQLITE_ANY, 0,
			     fnct_BdMPolyFromWKB1, 0, 0);
    sqlite3_create_function (db, "BdMPolyFromWKB", 2, SQLITE_ANY, 0,
			     fnct_BdMPolyFromWKB2, 0, 0);
    sqlite3_create_function (db, "Polygonize", 1, SQLITE_ANY, 0,
			     fnct_Polygonize1, 0, 0);
    sqlite3_create_function (db, "Polygonize", 2, SQLITE_ANY, 0,
			     fnct_Polygonize2, 0, 0);

#endif /* end including GEOS */

/* initializing the VirtualShape  extension */
    virtualshape_extension_init (db);
/* initializing the VirtualText extension */
    virtualtext_extension_init (db);
/* initializing the VirtualNetwork  extension */
    virtualnetwork_extension_init (db);
/* initializing the MbrCache  extension */
    mbrcache_extension_init (db);
/* initializing the VirtualFDO  extension */
    virtualfdo_extension_init (db);
/* setting a timeout handler */
    sqlite3_busy_timeout (db, 5000);
}

void
spatialite_init (int verbose)
{
/* used when SQLite initializes SpatiaLite via statically linked lib */
    sqlite3_auto_extension ( (void (*)(void)) init_static_spatialite);
    if (verbose)
      {
	  printf ("SpatiaLite version ..: %s", spatialite_version ());
	  printf ("\tSupported Extensions:\n");
	  printf ("\t- 'VirtualShape'\t[direct Shapefile access]\n");
	  printf ("\t- 'VirtualText\t\t[direct CSV/TXT access]\n");
	  printf ("\t- 'VirtualNetwork\t[Dijkstra shortest path]\n");
	  printf ("\t- 'RTree'\t\t[Spatial Index - R*Tree]\n");
	  printf ("\t- 'MbrCache'\t\t[Spatial Index - MBR cache]\n");
	  printf ("\t- 'VirtualFDO'\t\t[FDO-OGR interoperability]\n");
	  printf ("\t- 'SpatiaLite'\t\t[Spatial SQL - OGC]\n");
      }
#if OMIT_PROJ == 0		/* PROJ.4 version */
    if (verbose)
	printf ("PROJ.4 version ......: %s\n", pj_get_release ());
#endif /* end including PROJ.4 */
#if OMIT_GEOS == 0		/* GEOS version */
    if (verbose)
	printf ("GEOS version ........: %s\n", GEOSversion ());
#endif /* end GEOS version */
}

SPATIALITE_DECLARE int
sqlite3_extension_init (sqlite3 * db, char **pzErrMsg,
			const sqlite3_api_routines * pApi)
{
/* SQLite invokes this routine once when it dynamically loads the extension. */
    SQLITE_EXTENSION_INIT2 (pApi);
    setlocale (LC_NUMERIC, "POSIX");
    sqlite3_create_function (db, "GeometryConstraints", 3, SQLITE_ANY, 0,
			     fnct_GeometryConstraints, 0, 0);
    sqlite3_create_function (db, "CheckSpatialMetaData", 0, SQLITE_ANY, 0,
			     fnct_CheckSpatialMetaData, 0, 0);
    sqlite3_create_function (db, "AutoFDOStart", 0, SQLITE_ANY, 0,
			     fnct_AutoFDOStart, 0, 0);
    sqlite3_create_function (db, "AutoFDOStop", 0, SQLITE_ANY, 0,
			     fnct_AutoFDOStop, 0, 0);
    sqlite3_create_function (db, "InitFDOSpatialMetaData", 0, SQLITE_ANY, 0,
			     fnct_InitFDOSpatialMetaData, 0, 0);
    sqlite3_create_function (db, "AddFDOGeometryColumn", 6, SQLITE_ANY, 0,
			     fnct_AddFDOGeometryColumn, 0, 0);
    sqlite3_create_function (db, "RecoverFDOGeometryColumn", 6, SQLITE_ANY, 0,
			     fnct_RecoverFDOGeometryColumn, 0, 0);
    sqlite3_create_function (db, "DiscardFDOGeometryColumn", 2, SQLITE_ANY, 0,
			     fnct_DiscardFDOGeometryColumn, 0, 0);
    sqlite3_create_function (db, "InitSpatialMetaData", 0, SQLITE_ANY, 0,
			     fnct_InitSpatialMetaData, 0, 0);
    sqlite3_create_function (db, "AddGeometryColumn", 5, SQLITE_ANY, 0,
			     fnct_AddGeometryColumn, 0, 0);
    sqlite3_create_function (db, "AddGeometryColumn", 6, SQLITE_ANY, 0,
			     fnct_AddGeometryColumn, 0, 0);
    sqlite3_create_function (db, "RecoverGeometryColumn", 5, SQLITE_ANY, 0,
			     fnct_RecoverGeometryColumn, 0, 0);
    sqlite3_create_function (db, "DiscardGeometryColumn", 2, SQLITE_ANY, 0,
			     fnct_DiscardGeometryColumn, 0, 0);
    sqlite3_create_function (db, "CreateSpatialIndex", 2, SQLITE_ANY, 0,
			     fnct_CreateSpatialIndex, 0, 0);
    sqlite3_create_function (db, "CreateMbrCache", 2, SQLITE_ANY, 0,
			     fnct_CreateMbrCache, 0, 0);
    sqlite3_create_function (db, "DisableSpatialIndex", 2, SQLITE_ANY, 0,
			     fnct_DisableSpatialIndex, 0, 0);
    sqlite3_create_function (db, "RebuildGeometryTriggers", 2, SQLITE_ANY, 0,
			     fnct_RebuildGeometryTriggers, 0, 0);
    sqlite3_create_function (db, "AsText", 1, SQLITE_ANY, 0, fnct_AsText, 0, 0);
    sqlite3_create_function (db, "AsSvg", 1, SQLITE_ANY, 0, fnct_AsSvg1, 0, 0);
    sqlite3_create_function (db, "AsSvg", 2, SQLITE_ANY, 0, fnct_AsSvg2, 0, 0);
    sqlite3_create_function (db, "AsSvg", 3, SQLITE_ANY, 0, fnct_AsSvg3, 0, 0);
    sqlite3_create_function (db, "AsFGF", 2, SQLITE_ANY, 0, fnct_AsFGF, 0, 0);
    sqlite3_create_function (db, "AsBinary", 1, SQLITE_ANY, 0, fnct_AsBinary, 0,
			     0);
    sqlite3_create_function (db, "GeomFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomFromText1, 0, 0);
    sqlite3_create_function (db, "GeomFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomFromText2, 0, 0);
    sqlite3_create_function (db, "GeometryFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomFromText1, 0, 0);
    sqlite3_create_function (db, "GeometryFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomFromText2, 0, 0);
    sqlite3_create_function (db, "GeomCollFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromText1, 0, 0);
    sqlite3_create_function (db, "GeomCollFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromText2, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromText", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromText1, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromText", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromText2, 0, 0);
    sqlite3_create_function (db, "LineFromText", 1, SQLITE_ANY, 0,
			     fnct_LineFromText1, 0, 0);
    sqlite3_create_function (db, "LineFromText", 2, SQLITE_ANY, 0,
			     fnct_LineFromText2, 0, 0);
    sqlite3_create_function (db, "LineStringFromText", 1, SQLITE_ANY, 0,
			     fnct_LineFromText1, 0, 0);
    sqlite3_create_function (db, "LineStringFromText", 2, SQLITE_ANY, 0,
			     fnct_LineFromText2, 0, 0);
    sqlite3_create_function (db, "MLineFromText", 1, SQLITE_ANY, 0,
			     fnct_MLineFromText1, 0, 0);
    sqlite3_create_function (db, "MLineFromText", 2, SQLITE_ANY, 0,
			     fnct_MLineFromText2, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromText", 1, SQLITE_ANY, 0,
			     fnct_MLineFromText1, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromText", 2, SQLITE_ANY, 0,
			     fnct_MLineFromText2, 0, 0);
    sqlite3_create_function (db, "MPointFromText", 1, SQLITE_ANY, 0,
			     fnct_MPointFromText1, 0, 0);
    sqlite3_create_function (db, "MPointFromText", 2, SQLITE_ANY, 0,
			     fnct_MPointFromText2, 0, 0);
    sqlite3_create_function (db, "MultiPointFromText", 1, SQLITE_ANY, 0,
			     fnct_MPointFromText1, 0, 0);
    sqlite3_create_function (db, "MultiPointFromText", 2, SQLITE_ANY, 0,
			     fnct_MPointFromText2, 0, 0);
    sqlite3_create_function (db, "MPolyFromText", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromText1, 0, 0);
    sqlite3_create_function (db, "MPolyFromText", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromText2, 0, 0);
    sqlite3_create_function (db, "MultiPolygonFromText", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromText1, 0, 0);
    sqlite3_create_function (db, "MultiPolygonFromText", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromText2, 0, 0);
    sqlite3_create_function (db, "PointFromText", 1, SQLITE_ANY, 0,
			     fnct_PointFromText1, 0, 0);
    sqlite3_create_function (db, "PointFromText", 2, SQLITE_ANY, 0,
			     fnct_PointFromText2, 0, 0);
    sqlite3_create_function (db, "PolyFromText", 1, SQLITE_ANY, 0,
			     fnct_PolyFromText1, 0, 0);
    sqlite3_create_function (db, "PolyFromText", 2, SQLITE_ANY, 0,
			     fnct_PolyFromText2, 0, 0);
    sqlite3_create_function (db, "PolygonFromText", 1, SQLITE_ANY, 0,
			     fnct_PolyFromText1, 0, 0);
    sqlite3_create_function (db, "PolygomFromText", 2, SQLITE_ANY, 0,
			     fnct_PolyFromText2, 0, 0);
    sqlite3_create_function (db, "GeomFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeomFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomFromWkb2, 0, 0);
    sqlite3_create_function (db, "GeometryFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeometryFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomFromWkb2, 0, 0);
    sqlite3_create_function (db, "GeomCollFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeomCollFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb2, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromWKB", 1, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb1, 0, 0);
    sqlite3_create_function (db, "GeometryCollectionFromWKB", 2, SQLITE_ANY, 0,
			     fnct_GeomCollFromWkb2, 0, 0);
    sqlite3_create_function (db, "LineFromWKB", 1, SQLITE_ANY, 0,
			     fnct_LineFromWkb1, 0, 0);
    sqlite3_create_function (db, "LineFromWKB", 2, SQLITE_ANY, 0,
			     fnct_LineFromWkb2, 0, 0);
    sqlite3_create_function (db, "LineStringFromWKB", 1, SQLITE_ANY, 0,
			     fnct_LineFromWkb1, 0, 0);
    sqlite3_create_function (db, "LineStringFromWKB", 2, SQLITE_ANY, 0,
			     fnct_LineFromWkb2, 0, 0);
    sqlite3_create_function (db, "MLineFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MLineFromWkb1, 0, 0);
    sqlite3_create_function (db, "MLineFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MLineFromWkb2, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MLineFromWkb1, 0, 0);
    sqlite3_create_function (db, "MultiLineStringFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MLineFromWkb2, 0, 0);
    sqlite3_create_function (db, "MPointFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPointFromWkb1, 0, 0);
    sqlite3_create_function (db, "MPointFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPointFromWkb2, 0, 0);
    sqlite3_create_function (db, "MultiPointFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPointFromWkb1, 0, 0);
    sqlite3_create_function (db, "MultiPointFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPointFromWkb2, 0, 0);
    sqlite3_create_function (db, "MPolyFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "MPolyFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "MultiPolygonFromWKB", 1, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "MultiPolygomFromWKB", 2, SQLITE_ANY, 0,
			     fnct_MPolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "PointFromWKB", 1, SQLITE_ANY, 0,
			     fnct_PointFromWkb1, 0, 0);
    sqlite3_create_function (db, "PointFromWKB", 2, SQLITE_ANY, 0,
			     fnct_PointFromWkb2, 0, 0);
    sqlite3_create_function (db, "PolyFromWKB", 1, SQLITE_ANY, 0,
			     fnct_PolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "PolyFromWKB", 2, SQLITE_ANY, 0,
			     fnct_PolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "PolygonFromWKB", 1, SQLITE_ANY, 0,
			     fnct_PolyFromWkb1, 0, 0);
    sqlite3_create_function (db, "PolygonFromWKB", 2, SQLITE_ANY, 0,
			     fnct_PolyFromWkb2, 0, 0);
    sqlite3_create_function (db, "GeomFromFGF", 1, SQLITE_ANY, 0,
			     fnct_GeometryFromFGF1, 0, 0);
    sqlite3_create_function (db, "GeomFromFGF", 2, SQLITE_ANY, 0,
			     fnct_GeometryFromFGF2, 0, 0);
    sqlite3_create_function (db, "Dimension", 1, SQLITE_ANY, 0, fnct_Dimension,
			     0, 0);
    sqlite3_create_function (db, "GeometryType", 1, SQLITE_ANY, 0,
			     fnct_GeometryType, 0, 0);
    sqlite3_create_function (db, "GeometryAliasType", 1, SQLITE_ANY, 0,
			     fnct_GeometryAliasType, 0, 0);
    sqlite3_create_function (db, "SRID", 1, SQLITE_ANY, 0, fnct_SRID, 0, 0);
    sqlite3_create_function (db, "SetSrid", 2, SQLITE_ANY, 0, fnct_SetSRID, 0,
			     0);
    sqlite3_create_function (db, "IsEmpty", 1, SQLITE_ANY, 0, fnct_IsEmpty, 0,
			     0);
    sqlite3_create_function (db, "Envelope", 1, SQLITE_ANY, 0, fnct_Envelope, 0,
			     0);
    sqlite3_create_function (db, "X", 1, SQLITE_ANY, 0, fnct_X, 0, 0);
    sqlite3_create_function (db, "Y", 1, SQLITE_ANY, 0, fnct_Y, 0, 0);
    sqlite3_create_function (db, "NumPoints", 1, SQLITE_ANY, 0, fnct_NumPoints,
			     0, 0);
    sqlite3_create_function (db, "StartPoint", 1, SQLITE_ANY, 0,
			     fnct_StartPoint, 0, 0);
    sqlite3_create_function (db, "EndPoint", 1, SQLITE_ANY, 0, fnct_EndPoint, 0,
			     0);
    sqlite3_create_function (db, "PointN", 2, SQLITE_ANY, 0, fnct_PointN, 0, 0);
    sqlite3_create_function (db, "ExteriorRing", 1, SQLITE_ANY, 0,
			     fnct_ExteriorRing, 0, 0);
    sqlite3_create_function (db, "NumInteriorRings", 1, SQLITE_ANY, 0,
			     fnct_NumInteriorRings, 0, 0);
    sqlite3_create_function (db, "NumInteriorRing", 1, SQLITE_ANY, 0,
			     fnct_NumInteriorRings, 0, 0);
    sqlite3_create_function (db, "InteriorRingN", 2, SQLITE_ANY, 0,
			     fnct_InteriorRingN, 0, 0);
    sqlite3_create_function (db, "NumGeometries", 1, SQLITE_ANY, 0,
			     fnct_NumGeometries, 0, 0);
    sqlite3_create_function (db, "GeometryN", 2, SQLITE_ANY, 0, fnct_GeometryN,
			     0, 0);
    sqlite3_create_function (db, "MBRContains", 2, SQLITE_ANY, 0,
			     fnct_MbrContains, 0, 0);
    sqlite3_create_function (db, "MBRDisjoint", 2, SQLITE_ANY, 0,
			     fnct_MbrDisjoint, 0, 0);
    sqlite3_create_function (db, "MBREqual", 2, SQLITE_ANY, 0, fnct_MbrEqual, 0,
			     0);
    sqlite3_create_function (db, "MBRIntersects", 2, SQLITE_ANY, 0,
			     fnct_MbrIntersects, 0, 0);
    sqlite3_create_function (db, "MBROverlaps", 2, SQLITE_ANY, 0,
			     fnct_MbrOverlaps, 0, 0);
    sqlite3_create_function (db, "MBRTouches", 2, SQLITE_ANY, 0,
			     fnct_MbrTouches, 0, 0);
    sqlite3_create_function (db, "MBRWithin", 2, SQLITE_ANY, 0, fnct_MbrWithin,
			     0, 0);
    sqlite3_create_function (db, "ShiftCoords", 3, SQLITE_ANY, 0,
			     fnct_ShiftCoords, 0, 0);
    sqlite3_create_function (db, "ShiftCoordinates", 3, SQLITE_ANY, 0,
			     fnct_ShiftCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoords", 2, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoords", 3, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoordinates", 2, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "ScaleCoordinates", 3, SQLITE_ANY, 0,
			     fnct_ScaleCoords, 0, 0);
    sqlite3_create_function (db, "RotateCoords", 2, SQLITE_ANY, 0,
			     fnct_RotateCoords, 0, 0);
    sqlite3_create_function (db, "ReflectCoords", 3, SQLITE_ANY, 0,
			     fnct_ReflectCoords, 0, 0);
    sqlite3_create_function (db, "RotateCoordinates", 2, SQLITE_ANY, 0,
			     fnct_RotateCoords, 0, 0);
    sqlite3_create_function (db, "ReflectCoordinates", 3, SQLITE_ANY, 0,
			     fnct_ReflectCoords, 0, 0);
    sqlite3_create_function (db, "SwapCoordinates", 1, SQLITE_ANY, 0,
			     fnct_SwapCoords, 0, 0);
    sqlite3_create_function (db, "BuildMbr", 4, SQLITE_ANY, 0, fnct_BuildMbr1,
			     0, 0);
    sqlite3_create_function (db, "BuildMbr", 5, SQLITE_ANY, 0, fnct_BuildMbr2,
			     0, 0);
    sqlite3_create_function (db, "BuildCircleMbr", 3, SQLITE_ANY, 0,
			     fnct_BuildCircleMbr1, 0, 0);
    sqlite3_create_function (db, "BuildCircleMbr", 4, SQLITE_ANY, 0,
			     fnct_BuildCircleMbr2, 0, 0);
    sqlite3_create_function (db, "MbrMinX", 1, SQLITE_ANY, 0, fnct_MbrMinX, 0,
			     0);
    sqlite3_create_function (db, "MbrMaxX", 1, SQLITE_ANY, 0, fnct_MbrMaxX, 0,
			     0);
    sqlite3_create_function (db, "MbrMinY", 1, SQLITE_ANY, 0, fnct_MbrMinY, 0,
			     0);
    sqlite3_create_function (db, "MbrMaxY", 1, SQLITE_ANY, 0, fnct_MbrMaxY, 0,
			     0);
    sqlite3_create_function (db, "MakePoint", 2, SQLITE_ANY, 0, fnct_MakePoint1,
			     0, 0);
    sqlite3_create_function (db, "MakePoint", 3, SQLITE_ANY, 0, fnct_MakePoint2,
			     0, 0);
    sqlite3_create_function (db, "BuildMbrFilter", 4, SQLITE_ANY, 0,
			     fnct_BuildMbrFilter, 0, 0);
    sqlite3_create_function (db, "FilterMbrWithin", 4, SQLITE_ANY, 0,
			     fnct_FilterMbrWithin, 0, 0);
    sqlite3_create_function (db, "FilterMbrContains", 4, SQLITE_ANY, 0,
			     fnct_FilterMbrContains, 0, 0);
    sqlite3_create_function (db, "FilterMbrIntersects", 4, SQLITE_ANY, 0,
			     fnct_FilterMbrIntersects, 0, 0);
    sqlite3_create_function (db, "BuildRings", 1, SQLITE_ANY, 0,
			     fnct_BuildRings, 0, 0);

/* some BLOB/JPEG/EXIF functions */
    sqlite3_create_function (db, "IsGeometryBlob", 1, SQLITE_ANY, 0,
			     fnct_IsGeometryBlob, 0, 0);
    sqlite3_create_function (db, "IsZipBlob", 1, SQLITE_ANY, 0, fnct_IsZipBlob,
			     0, 0);
    sqlite3_create_function (db, "IsPdfBlob", 1, SQLITE_ANY, 0, fnct_IsPdfBlob,
			     0, 0);
    sqlite3_create_function (db, "IsGifBlob", 1, SQLITE_ANY, 0, fnct_IsGifBlob,
			     0, 0);
    sqlite3_create_function (db, "IsPngBlob", 1, SQLITE_ANY, 0, fnct_IsPngBlob,
			     0, 0);
    sqlite3_create_function (db, "IsJpegBlob", 1, SQLITE_ANY, 0,
			     fnct_IsJpegBlob, 0, 0);
    sqlite3_create_function (db, "IsExifBlob", 1, SQLITE_ANY, 0,
			     fnct_IsExifBlob, 0, 0);
    sqlite3_create_function (db, "IsExifGpsBlob", 1, SQLITE_ANY, 0,
			     fnct_IsExifGpsBlob, 0, 0);
    sqlite3_create_function (db, "GeomFromExifGpsBlob", 1, SQLITE_ANY, 0,
			     fnct_GeomFromExifGpsBlob, 0, 0);

#if OMIT_MATHSQL == 0		/* supporting SQL math functions */

/* some extra math functions */
    sqlite3_create_function (db, "abs", 1, SQLITE_ANY, 0, fnct_math_abs, 0, 0);
    sqlite3_create_function (db, "acos", 1, SQLITE_ANY, 0, fnct_math_acos, 0,
			     0);
    sqlite3_create_function (db, "asin", 1, SQLITE_ANY, 0, fnct_math_asin, 0,
			     0);
    sqlite3_create_function (db, "atan", 1, SQLITE_ANY, 0, fnct_math_atan, 0,
			     0);
    sqlite3_create_function (db, "ceil", 1, SQLITE_ANY, 0, fnct_math_ceil, 0,
			     0);
    sqlite3_create_function (db, "ceiling", 1, SQLITE_ANY, 0, fnct_math_ceil, 0,
			     0);
    sqlite3_create_function (db, "cos", 1, SQLITE_ANY, 0, fnct_math_cos, 0, 0);
    sqlite3_create_function (db, "cot", 1, SQLITE_ANY, 0, fnct_math_cot, 0, 0);
    sqlite3_create_function (db, "degrees", 1, SQLITE_ANY, 0, fnct_math_degrees,
			     0, 0);
    sqlite3_create_function (db, "exp", 1, SQLITE_ANY, 0, fnct_math_exp, 0, 0);
    sqlite3_create_function (db, "floor", 1, SQLITE_ANY, 0, fnct_math_floor, 0,
			     0);
    sqlite3_create_function (db, "ln", 1, SQLITE_ANY, 0, fnct_math_logn, 0, 0);
    sqlite3_create_function (db, "log", 1, SQLITE_ANY, 0, fnct_math_logn, 0, 0);
    sqlite3_create_function (db, "log", 2, SQLITE_ANY, 0, fnct_math_logn2, 0,
			     0);
    sqlite3_create_function (db, "log2", 1, SQLITE_ANY, 0, fnct_math_log_2, 0,
			     0);
    sqlite3_create_function (db, "log10", 1, SQLITE_ANY, 0, fnct_math_log_10, 0,
			     0);
    sqlite3_create_function (db, "pi", 0, SQLITE_ANY, 0, fnct_math_pi, 0, 0);
    sqlite3_create_function (db, "pow", 2, SQLITE_ANY, 0, fnct_math_pow, 0, 0);
    sqlite3_create_function (db, "power", 2, SQLITE_ANY, 0, fnct_math_pow, 0,
			     0);
    sqlite3_create_function (db, "radians", 1, SQLITE_ANY, 0, fnct_math_radians,
			     0, 0);
    sqlite3_create_function (db, "round", 1, SQLITE_ANY, 0, fnct_math_round, 0,
			     0);
    sqlite3_create_function (db, "sign", 1, SQLITE_ANY, 0, fnct_math_sign, 0,
			     0);
    sqlite3_create_function (db, "sin", 1, SQLITE_ANY, 0, fnct_math_sin, 0, 0);
    sqlite3_create_function (db, "stddev_pop", 1, SQLITE_ANY, 0, 0,
			     fnct_math_stddev_step, fnct_math_stddev_pop_final);
    sqlite3_create_function (db, "stddev_samp", 1, SQLITE_ANY, 0, 0,
			     fnct_math_stddev_step,
			     fnct_math_stddev_samp_final);
    sqlite3_create_function (db, "sqrt", 1, SQLITE_ANY, 0, fnct_math_sqrt, 0,
			     0);
    sqlite3_create_function (db, "tan", 1, SQLITE_ANY, 0, fnct_math_tan, 0, 0);
    sqlite3_create_function (db, "var_pop", 1, SQLITE_ANY, 0, 0,
			     fnct_math_stddev_step, fnct_math_var_pop_final);
    sqlite3_create_function (db, "var_samp", 1, SQLITE_ANY, 0, 0,
			     fnct_math_stddev_step, fnct_math_var_samp_final);

#endif /* end supporting SQL math functions */

#if OMIT_PROJ == 0		/* including PROJ.4 */

    sqlite3_create_function (db, "Transform", 2, SQLITE_ANY, 0, fnct_Transform,
			     0, 0);

#endif /* end including PROJ.4 */

#if OMIT_GEOS == 0		/* including GEOS */

    initGEOS (geos_error, geos_error);
    sqlite3_create_function (db, "Equals", 2, SQLITE_ANY, 0, fnct_Equals, 0, 0);
    sqlite3_create_function (db, "Intersects", 2, SQLITE_ANY, 0,
			     fnct_Intersects, 0, 0);
    sqlite3_create_function (db, "Disjoint", 2, SQLITE_ANY, 0, fnct_Disjoint, 0,
			     0);
    sqlite3_create_function (db, "Overlaps", 2, SQLITE_ANY, 0, fnct_Overlaps, 0,
			     0);
    sqlite3_create_function (db, "Crosses", 2, SQLITE_ANY, 0, fnct_Crosses, 0,
			     0);
    sqlite3_create_function (db, "Touches", 2, SQLITE_ANY, 0, fnct_Touches, 0,
			     0);
    sqlite3_create_function (db, "Within", 2, SQLITE_ANY, 0, fnct_Within, 0, 0);
    sqlite3_create_function (db, "Contains", 2, SQLITE_ANY, 0, fnct_Contains, 0,
			     0);
    sqlite3_create_function (db, "Relate", 3, SQLITE_ANY, 0, fnct_Relate, 0, 0);
    sqlite3_create_function (db, "Distance", 2, SQLITE_ANY, 0, fnct_Distance, 0,
			     0);
    sqlite3_create_function (db, "Intersection", 2, SQLITE_ANY, 0,
			     fnct_Intersection, 0, 0);
    sqlite3_create_function (db, "Difference", 2, SQLITE_ANY, 0,
			     fnct_Difference, 0, 0);
    sqlite3_create_function (db, "GUnion", 1, SQLITE_ANY, 0, 0, fnct_Union_step,
			     fnct_Union_final);
    sqlite3_create_function (db, "GUnion", 2, SQLITE_ANY, 0, fnct_Union, 0, 0);
    sqlite3_create_function (db, "SymDifference", 2, SQLITE_ANY, 0,
			     fnct_SymDifference, 0, 0);
    sqlite3_create_function (db, "Boundary", 1, SQLITE_ANY, 0, fnct_Boundary, 0,
			     0);
    sqlite3_create_function (db, "GLength", 1, SQLITE_ANY, 0, fnct_Length, 0,
			     0);
    sqlite3_create_function (db, "Area", 1, SQLITE_ANY, 0, fnct_Area, 0, 0);
    sqlite3_create_function (db, "Centroid", 1, SQLITE_ANY, 0, fnct_Centroid, 0,
			     0);
    sqlite3_create_function (db, "PointOnSurface", 1, SQLITE_ANY, 0,
			     fnct_PointOnSurface, 0, 0);
    sqlite3_create_function (db, "Simplify", 2, SQLITE_ANY, 0, fnct_Simplify, 0,
			     0);
    sqlite3_create_function (db, "SimplifyPreserveTopology", 2, SQLITE_ANY, 0,
			     fnct_SimplifyPreserveTopology, 0, 0);
    sqlite3_create_function (db, "ConvexHull", 1, SQLITE_ANY, 0,
			     fnct_ConvexHull, 0, 0);
    sqlite3_create_function (db, "Buffer", 2, SQLITE_ANY, 0, fnct_Buffer, 0, 0);
    sqlite3_create_function (db, "IsClosed", 1, SQLITE_ANY, 0, fnct_IsClosed, 0,
			     0);
    sqlite3_create_function (db, "IsSimple", 1, SQLITE_ANY, 0, fnct_IsSimple, 0,
			     0);
    sqlite3_create_function (db, "IsRing", 1, SQLITE_ANY, 0, fnct_IsRing, 0, 0);
    sqlite3_create_function (db, "IsValid", 1, SQLITE_ANY, 0, fnct_IsValid, 0,
			     0);
    sqlite3_create_function (db, "BdPolyFromText", 1, SQLITE_ANY, 0,
			     fnct_BdPolyFromText1, 0, 0);
    sqlite3_create_function (db, "BdPolyFromText", 2, SQLITE_ANY, 0,
			     fnct_BdPolyFromText2, 0, 0);
    sqlite3_create_function (db, "BdMPolyFromText", 1, SQLITE_ANY, 0,
			     fnct_BdMPolyFromText1, 0, 0);
    sqlite3_create_function (db, "BdMPolyFromText", 2, SQLITE_ANY, 0,
			     fnct_BdMPolyFromText2, 0, 0);
    sqlite3_create_function (db, "BdPolyFromWKB", 1, SQLITE_ANY, 0,
			     fnct_BdPolyFromWKB1, 0, 0);
    sqlite3_create_function (db, "BdPolyFromWKB", 2, SQLITE_ANY, 0,
			     fnct_BdPolyFromWKB2, 0, 0);
    sqlite3_create_function (db, "BdMPolyFromWKB", 1, SQLITE_ANY, 0,
			     fnct_BdMPolyFromWKB1, 0, 0);
    sqlite3_create_function (db, "BdMPolyFromWKB", 2, SQLITE_ANY, 0,
			     fnct_BdMPolyFromWKB2, 0, 0);
    sqlite3_create_function (db, "Polygonize", 1, SQLITE_ANY, 0,
			     fnct_Polygonize1, 0, 0);
    sqlite3_create_function (db, "Polygonize", 2, SQLITE_ANY, 0,
			     fnct_Polygonize2, 0, 0);

#endif /* end including GEOS */

/* initializing the VirtualShape  extension */
    virtualshape_extension_init (db);
/* initializing the VirtualText  extension */
    virtualtext_extension_init (db);
/* initializing the VirtualNetwork  extension */
    virtualnetwork_extension_init (db);
/* initializing the MbrCache  extension */
    mbrcache_extension_init (db);
/* initializing the VirtualFDO  extension */
    virtualfdo_extension_init (db);
/* setting a timeout handler */
    sqlite3_busy_timeout (db, 5000);

    printf ("SpatiaLite version ..: %s", spatialite_version ());
    printf ("\tSupported Extensions:\n");
    printf ("\t- 'VirtualShape'\t[direct Shapefile access]\n");
    printf ("\t- 'VirtualText'\t\t[direct CSV/TXT access]\n");
    printf ("\t- 'VirtualNetwork\t[Dijkstra shortest path]\n");
    printf ("\t- 'RTree'\t\t[Spatial Index - R*Tree]\n");
    printf ("\t- 'MbrCache'\t\t[Spatial Index - MBR cache]\n");
    printf ("\t- 'VirtualFDO'\t\t[FDO-OGR interoperability]\n");
    printf ("\t- 'SpatiaLite'\t\t[Spatial SQL - OGC]\n");
#if OMIT_PROJ == 0		/* PROJ.4 version */
    printf ("PROJ.4 %s\n", pj_get_release ());
    fflush (stdout);
#endif /* end including PROJ.4 */
#if OMIT_GEOS == 0		/* GEOS version */
    printf ("GEOS version %s\n", GEOSversion ());
    fflush (stdout);
#endif /* end GEOS version */
    return 0;
}

SPATIALITE_DECLARE sqlite3_int64
math_llabs (sqlite3_int64 value)
{
/* replacing the C99 llabs() function */
  return value<0 ? -value : value;
}

SPATIALITE_DECLARE double
math_round (double value)
{
/* replacing the C99 round() function */
    double min = floor (value);
    if (fabs (value - min) < 0.5)
	return min;
    return min + 1.0;
}
/**************** End file: spatialite.c **********/


/**************** Begin file: mbrcache.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <string.h> */
/* #include <float.h> */

/* #include <spatialite/sqlite3.h> */
/* #include <spatialite/spatialite.h> */
/* #include <spatialite/gaiageo.h> */

#if defined(_WIN32) && !defined(__MINGW32__)
#define LONG64_MAX	_I64_MAX
#define LONG64_MIN	_I64_MIN
#else
#define LONG64_MAX	9223372036854775807LL
#define LONG64_MIN	(-LONG64_MAX + 1)
#endif

static struct sqlite3_module my_mbr_module;

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
    const sqlite3_module *pModule;	/* ptr to sqlite module: USED INTERNALLY BY SQLITE */
    int nRef;			/* # references: USED INTERNALLY BY SQLITE */
    char *zErrMsg;		/* error message: USE INTERNALLY BY SQLITE */
    sqlite3 *db;		/* the sqlite db holding the virtual table */
    struct mbr_cache *cache;	/* the  MBR's cache */
    char *table_name;		/* the main table to be cached */
    char *column_name;		/* the column to be cached */
    int error;			/* some previous error disables any operation */
} MbrCache;
typedef MbrCache *MbrCachePtr;

typedef struct MbrCacheCursortStruct
{
/* extends the sqlite3_vtab_cursor struct */
    MbrCachePtr pVtab;		/* Virtual table of this cursor */
    int eof;			/* the EOF marker */
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
cache_bitmask (int x)
{
/* return the bitmask corresponding to index X */
    switch (x)
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
cache_alloc ()
{
/* allocates and initializes an empty cache struct */
    struct mbr_cache *p = malloc (sizeof (struct mbr_cache));
    p->first = NULL;
    p->last = NULL;
    p->current = NULL;
    return p;
}

static struct mbr_cache_page *
cache_page_alloc ()
{
/* allocates and initializes a cache page */
    int i;
    struct mbr_cache_block *pb;
    struct mbr_cache_page *p = malloc (sizeof (struct mbr_cache_page));
    p->bitmap = 0x00000000;
    p->next = NULL;
    p->minx = DBL_MAX;
    p->miny = DBL_MAX;
    p->maxx = -DBL_MAX;
    p->maxy = -DBL_MAX;
    for (i = 0; i < 32; i++)
      {
	  pb = p->blocks + i;
	  pb->bitmap = 0x00000000;
	  pb->minx = DBL_MAX;
	  pb->miny = DBL_MAX;
	  pb->maxx = -DBL_MAX;
	  pb->maxy = DBL_MAX;
      }
    p->max_rowid = LONG64_MIN;
    p->min_rowid = LONG64_MAX;
    return p;
}

static void
cache_destroy (struct mbr_cache *p)
{
/* memory cleanup; destroying a cache and any page into the cache */
    struct mbr_cache_page *pp;
    struct mbr_cache_page *ppn;
    if (!p)
	return;
    pp = p->first;
    while (pp)
      {
	  ppn = pp->next;
	  free (pp);
	  pp = ppn;
      }
    free (p);
}

static int
cache_get_free_block (struct mbr_cache_page *pp)
{
/* scans a cache page, returning the index of the first available block containing a free cell */
    int ib;
    for (ib = 0; ib < 32; ib++)
      {
	  if ((pp->bitmap & cache_bitmask (ib)) == 0x00000000)
	      return ib;
      }
    return -1;
}

static void
cache_fix_page_bitmap (struct mbr_cache_page *pp)
{
/* updating the cache page bitmap */
    int ib;
    for (ib = 0; ib < 32; ib++)
      {
	  if (pp->blocks[ib].bitmap == 0xffffffff)
	    {
		/* all the cells into this block are used; marking the page bitmap */
		pp->bitmap |= cache_bitmask (ib);
	    }
      }
}

static int
cache_get_free_cell (struct mbr_cache_block *pb)
{
/* scans a cache block, returning the index of the first free cell */
    int ic;
    for (ic = 0; ic < 32; ic++)
      {
	  if ((pb->bitmap & cache_bitmask (ic)) == 0x00000000)
	      return ic;
      }
    return -1;
}

static struct mbr_cache_page *
cache_get_free_page (struct mbr_cache *p)
{
/* return a pointer to the first cache page containing a free cell */
    struct mbr_cache_page *pp;
    if (!(p->first))
      {
	  /* the cache is empty; so we surely need to allocate the first page */
	  pp = cache_page_alloc ();
	  p->first = pp;
	  p->last = pp;
	  p->current = pp;
	  return pp;
      }
    if (p->current)
      {
	  /* checking if there is at least a free block into the current page */
	  if (p->current->bitmap != 0xffffffff)
	      return p->current;
      }
    pp = p->first;
    while (pp)
      {
	  /* scanning the page list in order to discover if there is an exixsting page not yet completly filled */
	  if (pp->bitmap != 0xffffffff)
	    {
		p->current = pp;
		return pp;
	    }
	  pp = pp->next;
      }
/* we have to allocate a new page */
    pp = cache_page_alloc ();
    p->last->next = pp;
    p->last = pp;
    p->current = pp;
    return pp;
}

static void
cache_insert_cell (struct mbr_cache *p, sqlite3_int64 rowid, double minx,
		   double miny, double maxx, double maxy)
{
/* inserting a new cell */
    struct mbr_cache_page *pp = cache_get_free_page (p);
    int ib = cache_get_free_block (pp);
    struct mbr_cache_block *pb = pp->blocks + ib;
    int ic = cache_get_free_cell (pb);
    struct mbr_cache_cell *pc = pb->cells + ic;
    pc->rowid = rowid;
    pc->minx = minx;
    pc->miny = miny;
    pc->maxx = maxx;
    pc->maxy = maxy;
/* marking the cache cell as used into the block bitmap */
    pb->bitmap |= cache_bitmask (ic);
/* updating the cache block MBR */
    if (pb->minx > minx)
	pb->minx = minx;
    if (pb->maxx < maxx)
	pb->maxx = maxx;
    if (pb->miny > miny)
	pb->miny = miny;
    if (pb->maxy < maxy)
	pb->maxy = maxy;
/* updading the cache page MBR */
    if (pp->minx > minx)
	pp->minx = minx;
    if (pp->maxx < maxx)
	pp->maxx = maxx;
    if (pp->miny > miny)
	pp->miny = miny;
    if (pp->maxy < maxy)
	pp->maxy = maxy;
/* fixing the cache page bitmap */
    cache_fix_page_bitmap (pp);
/* updating min-max rowid into the cache page */
    if (pp->min_rowid > rowid)
	pp->min_rowid = rowid;
    if (pp->max_rowid < rowid)
	pp->max_rowid = rowid;
}

static struct mbr_cache *
cache_load (sqlite3 * handle, const char *table, const char *column)
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
    sprintf (sql,
	     "SELECT ROWID, MbrMinX(\"%s\"), MbrMinY(\"%s\"), MbrMaxX(\"%s\"), MbrMaxY(\"%s\") FROM \"%s\"",
	     column, column, column, column, table);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
/* some error occurred */
	  fprintf (stderr, "cache SQL error: %s\n", sqlite3_errmsg (handle));
	  return NULL;
      }
    p_cache = cache_alloc ();
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	    {
		v1 = 0;
		v2 = 0;
		v3 = 0;
		v4 = 0;
		v5 = 0;
		if (sqlite3_column_type (stmt, 0) == SQLITE_INTEGER)
		    v1 = 1;
		if (sqlite3_column_type (stmt, 1) == SQLITE_FLOAT)
		    v2 = 1;
		if (sqlite3_column_type (stmt, 1) == SQLITE_FLOAT)
		    v3 = 1;
		if (sqlite3_column_type (stmt, 1) == SQLITE_FLOAT)
		    v4 = 1;
		if (sqlite3_column_type (stmt, 1) == SQLITE_FLOAT)
		    v5 = 1;
		if (v1 && v2 && v3 && v4 && v5)
		  {
		      /* ok, this entity is a valid one; inserting them into the MBR's cache */
		      rowid = sqlite3_column_int (stmt, 0);
		      minx = sqlite3_column_double (stmt, 1);
		      miny = sqlite3_column_double (stmt, 2);
		      maxx = sqlite3_column_double (stmt, 3);
		      maxy = sqlite3_column_double (stmt, 4);
		      cache_insert_cell (p_cache, rowid, minx, miny, maxx,
					 maxy);
		  }
	    }
	  else
	    {
/* some unexpected error occurred */
		printf ("sqlite3_step() error: %s\n", sqlite3_errmsg (handle));
		sqlite3_finalize (stmt);
		cache_destroy (p_cache);
		return NULL;
	    }
      }
/* we have now to finalize the query [memory cleanup] */
    sqlite3_finalize (stmt);
    return p_cache;
}

static int
cache_find_next_cell (struct mbr_cache_page **page, int *i_block, int *i_cell,
		      struct mbr_cache_cell **cell)
{
/* finding next cached cell */
    struct mbr_cache_page *pp = *page;
    struct mbr_cache_block *pb;
    struct mbr_cache_cell *pc;
    int ib;
    int ic;
    int sib = *i_block;
    int sic = *i_cell;
    while (pp)
      {
	  for (ib = sib; ib < 32; ib++)
	    {
		pb = pp->blocks + ib;
		for (ic = sic; ic < 32; ic++)
		  {
		      if ((pb->bitmap & cache_bitmask (ic)) == 0x00000000)
			  continue;
		      pc = pb->cells + ic;
		      if (pc == *cell)
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
cache_find_next_mbr (struct mbr_cache_page **page, int *i_block, int *i_cell,
		     struct mbr_cache_cell **cell, double minx, double miny,
		     double maxx, double maxy, int mode)
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
    while (pp)
      {
	  ok_mbr = 0;
	  if (pp->maxx >= minx && pp->minx <= maxx && pp->maxy >= miny
	      && pp->miny <= maxy)
	      ok_mbr = 1;
	  if (ok_mbr)
	    {
		for (ib = sib; ib < 32; ib++)
		  {
		      pb = pp->blocks + ib;
		      ok_mbr = 0;
		      if (pb->maxx >= minx && pb->minx <= maxx
			  && pb->maxy >= miny && pb->miny <= maxy)
			  ok_mbr = 1;
		      if (ok_mbr)
			{
			    for (ic = sic; ic < 32; ic++)
			      {
				  if ((pb->bitmap & cache_bitmask (ic)) ==
				      0x00000000)
				      continue;
				  pc = pb->cells + ic;
				  ok_mbr = 0;
				  if (mode == GAIA_FILTER_MBR_INTERSECTS)
				    {
					/* MBR INTERSECTS */
					if (pc->maxx >= minx && pc->minx <= maxx
					    && pc->maxy >= miny
					    && pc->miny <= maxy)
					    ok_mbr = 1;
				    }
				  else if (mode == GAIA_FILTER_MBR_CONTAINS)
				    {
					/* MBR CONTAINS */
					if (minx >= pc->minx && maxx <= pc->maxx
					    && miny >= pc->miny
					    && maxy <= pc->maxy)
					    ok_mbr = 1;
				    }
				  else
				    {
					/* MBR WITHIN */
					if (pc->minx >= minx && pc->maxx <= maxx
					    && pc->miny >= miny
					    && pc->maxy <= maxy)
					    ok_mbr = 1;
				    }
				  if (ok_mbr)
				    {
					if (pc == *cell)
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
cache_find_by_rowid (struct mbr_cache_page *pp, sqlite3_int64 rowid)
{
/* trying to find a row by rowid from the Mbr cache */
    struct mbr_cache_block *pb;
    struct mbr_cache_cell *pc;
    int ib;
    int ic;
    while (pp)
      {
	  if (rowid >= pp->min_rowid && rowid <= pp->max_rowid)
	    {
		for (ib = 0; ib < 32; ib++)
		  {
		      pb = pp->blocks + ib;
		      for (ic = 0; ic < 32; ic++)
			{
			    if ((pb->bitmap & cache_bitmask (ic)) == 0x00000000)
				continue;
			    pc = pb->cells + ic;
			    if (pc->rowid == rowid)
				return pc;
			}
		  }
	    }
	  pp = pp->next;
      }
    return 0;
}

static void
cache_update_page (struct mbr_cache_page *pp, int i_block)
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
    pb->maxx = -DBL_MAX;
    pb->maxy = -DBL_MAX;
    for (ic = 0; ic < 32; ic++)
      {
	  if ((pb->bitmap & cache_bitmask (ic)) == 0x00000000)
	      continue;
	  pc = pb->cells + ic;
	  if (pb->minx > pc->minx)
	      pb->minx = pc->minx;
	  if (pb->miny > pc->miny)
	      pb->miny = pc->miny;
	  if (pb->maxx < pc->maxx)
	      pb->maxx = pc->maxx;
	  if (pb->maxy < pc->maxy)
	      pb->maxy = pc->maxy;
      }
/* updating the cache page MBR */
    pp->minx = DBL_MAX;
    pp->miny = DBL_MAX;
    pp->maxx = -DBL_MAX;
    pp->maxy = -DBL_MAX;
    pp->min_rowid = LONG64_MAX;
    pp->max_rowid = LONG64_MIN;
    for (ib = 0; ib < 32; ib++)
      {
	  pb = pp->blocks + ib;
	  for (ic = 0; ic < 32; ic++)
	    {
		if ((pb->bitmap & cache_bitmask (ic)) == 0x00000000)
		    continue;
		pc = pb->cells + ic;
		if (pp->minx > pc->minx)
		    pp->minx = pc->minx;
		if (pp->miny > pc->miny)
		    pp->miny = pc->miny;
		if (pp->maxx < pc->maxx)
		    pp->maxx = pc->maxx;
		if (pp->maxy < pc->maxy)
		    pp->maxy = pc->maxy;
		if (pp->min_rowid > pc->rowid)
		    pp->min_rowid = pc->rowid;
		if (pp->max_rowid < pc->rowid)
		    pp->max_rowid = pc->rowid;
	    }
      }
}

static int
cache_delete_cell (struct mbr_cache_page *pp, sqlite3_int64 rowid)
{
/* trying to delete a row identified by rowid from the Mbr cache */
    struct mbr_cache_block *pb;
    struct mbr_cache_cell *pc;
    int ib;
    int ic;
    while (pp)
      {
	  if (rowid >= pp->min_rowid && rowid <= pp->max_rowid)
	    {
		for (ib = 0; ib < 32; ib++)
		  {
		      pb = pp->blocks + ib;
		      for (ic = 0; ic < 32; ic++)
			{
			    if ((pb->bitmap & cache_bitmask (ic)) == 0x00000000)
				continue;
			    pc = pb->cells + ic;
			    if (pc->rowid == rowid)
			      {
				  /* marking the cell as free */
				  pb->bitmap &= ~(cache_bitmask (ic));
				  /* marking the block as not full */
				  pp->bitmap &= ~(cache_bitmask (ib));
				  /* updating the cache block and cache page MBR */
				  cache_update_page (pp, ib);
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
cache_update_cell (struct mbr_cache_page *pp, sqlite3_int64 rowid, double minx,
		   double miny, double maxx, double maxy)
{
/* trying to update a row identified by rowid from the Mbr cache */
    struct mbr_cache_block *pb;
    struct mbr_cache_cell *pc;
    int ib;
    int ic;
    while (pp)
      {
	  if (rowid >= pp->min_rowid && rowid <= pp->max_rowid)
	    {
		for (ib = 0; ib < 32; ib++)
		  {
		      pb = pp->blocks + ib;
		      for (ic = 0; ic < 32; ic++)
			{
			    if ((pb->bitmap & cache_bitmask (ic)) == 0x00000000)
				continue;
			    pc = pb->cells + ic;
			    if (pc->rowid == rowid)
			      {
				  /* updating the cell MBR */
				  pc->minx = minx;
				  pc->miny = miny;
				  pc->maxx = maxx;
				  pc->maxy = maxy;
				  /* updating the cache block and cache page MBR */
				  cache_update_page (pp, ib);
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
mbrc_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	     sqlite3_vtab ** ppVTab, char **pzErr)
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
    p_vt = (MbrCachePtr) sqlite3_malloc (sizeof (MbrCache));
    if (!p_vt)
	return SQLITE_NOMEM;
    *ppVTab = (sqlite3_vtab *) p_vt;
    p_vt->pModule = &my_mbr_module;
    p_vt->nRef = 0;
    p_vt->zErrMsg = NULL;
    p_vt->db = db;
    p_vt->table_name = NULL;
    p_vt->column_name = NULL;
    p_vt->cache = NULL;
/* checking for table_name and geo_column_name */
    if (argc == 5)
      {
	  vtable = argv[2];
	  table = argv[3];
	  column = argv[4];
	  len = strlen (table);
	  p_vt->table_name = sqlite3_malloc (len + 1);
	  strcpy (p_vt->table_name, table);
	  len = strlen (column);
	  p_vt->column_name = sqlite3_malloc (len + 1);
	  strcpy (p_vt->column_name, column);
      }
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[MbrCache module] CREATE VIRTUAL: illegal arg list {table_name, geo_column_name}");
	  return SQLITE_ERROR;
      }
/* retrieving the base table columns */
    err = 0;
    ok_tbl = 0;
    ok_col = 0;
    sprintf (sql, "PRAGMA table_info(\"%s\")", table);
    ret = sqlite3_get_table (db, sql, &results, &n_rows, &n_columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  err = 1;
	  goto illegal;
      }
    if (n_rows > 1)
      {
	  ok_tbl = 1;
	  for (i = 1; i <= n_rows; i++)
	    {
		col_name = results[(i * n_columns) + 1];
		if (strcasecmp (col_name, column) == 0)
		    ok_col = 1;
	    }
	  sqlite3_free_table (results);
	  if (!ok_col)
	      err = 1;
      }
    else
	err = 1;
  illegal:
    if (err)
      {
	  /* something is going the wrong way; creating a stupid default table */
	  sprintf (sql, "CREATE TABLE \"%s\" (\"rowid\" INTEGER, \"mbr\" BLOB)",
		   vtable);
	  if (sqlite3_declare_vtab (db, sql) != SQLITE_OK)
	    {
		*pzErr =
		    sqlite3_mprintf
		    ("[MbrCache module] cannot build the VirtualTable\n");
		return SQLITE_ERROR;
	    }
	  p_vt->error = 1;
	  *ppVTab = (sqlite3_vtab *) p_vt;
	  return SQLITE_OK;
      }
    p_vt->error = 0;
    sprintf (sql, "CREATE TABLE \"%s\" (", vtable);
    strcat (sql, "\"rowid\" INTEGER, \"mbr\" BLOB)");
    if (sqlite3_declare_vtab (db, sql) != SQLITE_OK)
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[MbrCache module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
	       sql);
	  return SQLITE_ERROR;
      }
    *ppVTab = (sqlite3_vtab *) p_vt;
    return SQLITE_OK;
}

static int
mbrc_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	      sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table - simply aliases mbrc_create() */
    return mbrc_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
mbrc_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIdxInfo)
{
/* best index selection */
    int i;
    int err = 1;
    int errors = 0;
    int mbr = 0;
    int rowid = 0;
    for (i = 0; i < pIdxInfo->nConstraint; i++)
      {
	  /* verifying the constraints */
	  struct sqlite3_index_constraint *p = &(pIdxInfo->aConstraint[i]);
	  if (p->usable)
	    {
		if (p->iColumn == 0 && p->op == SQLITE_INDEX_CONSTRAINT_EQ)
		    rowid++;
		else if (p->iColumn == 1 && p->op == SQLITE_INDEX_CONSTRAINT_EQ)
		    mbr++;
		else
		    errors++;
	    }
      }
    if (mbr == 1 && rowid == 0 && errors == 0)
      {
	  /* this one is a valid spatially-filtered query */
	  pIdxInfo->idxNum = 2;
	  for (i = 0; i < pIdxInfo->nConstraint; i++)
	    {
		pIdxInfo->aConstraintUsage[i].argvIndex = 1;
		pIdxInfo->aConstraintUsage[i].omit = 1;
	    }
	  err = 0;
      }
    if (mbr == 0 && rowid == 1 && errors == 0)
      {
	  /* this one is a valid rowid-filtered query */
	  pIdxInfo->idxNum = 1;
	  pIdxInfo->estimatedCost = 1.0;
	  for (i = 0; i < pIdxInfo->nConstraint; i++)
	    {
		if (pIdxInfo->aConstraint[i].usable)
		  {
		      pIdxInfo->aConstraintUsage[i].argvIndex = 1;
		      pIdxInfo->aConstraintUsage[i].omit = 1;
		  }
	    }
	  err = 0;
      }
    if (mbr == 0 && rowid == 0 && errors == 0)
      {
	  /* this one is a valid unfiltered query */
	  pIdxInfo->idxNum = 0;
	  err = 0;
      }
    if (err)
      {
	  /* illegal query */
	  pIdxInfo->idxNum = -1;
      }
    return SQLITE_OK;
}

static int
mbrc_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    MbrCachePtr p_vt = (MbrCachePtr) pVTab;
    if (p_vt->cache)
	cache_destroy (p_vt->cache);
    if (p_vt->table_name)
	sqlite3_free (p_vt->table_name);
    if (p_vt->column_name)
	sqlite3_free (p_vt->column_name);
    sqlite3_free (p_vt);
    return SQLITE_OK;
}

static int
mbrc_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases mbrc_disconnect() */
    return mbrc_disconnect (pVTab);
}

static void
mbrc_read_row_unfiltered (MbrCacheCursorPtr cursor)
{
/* trying to read the next row from the Mbr cache - unfiltered mode */
    struct mbr_cache_page *page = cursor->current_page;
    struct mbr_cache_cell *cell = cursor->current_cell;
    int i_block = cursor->current_block_index;
    int i_cell = cursor->current_cell_index;
    if (cache_find_next_cell (&page, &i_block, &i_cell, &cell))
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
mbrc_read_row_filtered (MbrCacheCursorPtr cursor)
{
/* trying to read the next row from the Mbr cache - spatially filter mode */
    struct mbr_cache_page *page = cursor->current_page;
    struct mbr_cache_cell *cell = cursor->current_cell;
    int i_block = cursor->current_block_index;
    int i_cell = cursor->current_cell_index;
    if (cache_find_next_mbr
	(&page, &i_block, &i_cell, &cell, cursor->minx, cursor->miny,
	 cursor->maxx, cursor->maxy, cursor->mbr_mode))
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
mbrc_read_row_by_rowid (MbrCacheCursorPtr cursor, sqlite3_int64 rowid)
{
/* trying to find a row by rowid from the Mbr cache */
    struct mbr_cache_cell *cell =
	cache_find_by_rowid (cursor->pVtab->cache->first, rowid);
    if (cell)
	cursor->current_cell = cell;
    else
      {
	  cursor->current_cell = NULL;
	  cursor->eof = 1;
      }
}

static int
mbrc_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    MbrCachePtr p_vt = (MbrCachePtr) pVTab;
    MbrCacheCursorPtr cursor =
	(MbrCacheCursorPtr) sqlite3_malloc (sizeof (MbrCacheCursor));
    if (cursor == NULL)
	return SQLITE_ERROR;
    cursor->pVtab = p_vt;
    if (p_vt->error)
      {
	  cursor->eof = 1;
	  *ppCursor = (sqlite3_vtab_cursor *) cursor;
	  return SQLITE_OK;
      }
    if (!(p_vt->cache))
	p_vt->cache =
	    cache_load (p_vt->db, p_vt->table_name, p_vt->column_name);
    cursor->current_page = cursor->pVtab->cache->first;
    cursor->current_block_index = 0;
    cursor->current_cell_index = 0;
    cursor->current_cell = NULL;
    cursor->eof = 0;
    *ppCursor = (sqlite3_vtab_cursor *) cursor;
    return SQLITE_OK;
}

static int
mbrc_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    sqlite3_free (pCursor);
    return SQLITE_OK;
}

static int
mbrc_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	     int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    MbrCacheCursorPtr cursor = (MbrCacheCursorPtr) pCursor;
    if (cursor->pVtab->error)
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
    if (idxNum == 0)
      {
	  /* unfiltered mode */
	  mbrc_read_row_unfiltered (cursor);
	  return SQLITE_OK;
      }
    if (idxNum == 1)
      {
	  /* filtering by ROWID */
	  sqlite3_int64 rowid = sqlite3_value_int64 (argv[0]);
	  mbrc_read_row_by_rowid (cursor, rowid);
	  return SQLITE_OK;
      }
    if (idxNum == 2)
      {
	  /* filtering by MBR spatial relation */
	  unsigned char *p_blob;
	  int n_bytes;
	  double minx;
	  double miny;
	  double maxx;
	  double maxy;
	  int mode;
	  if (sqlite3_value_type (argv[0]) != SQLITE_BLOB)
	      cursor->eof = 1;
	  else
	    {
		p_blob = (unsigned char *) sqlite3_value_blob (argv[0]);
		n_bytes = sqlite3_value_bytes (argv[0]);
		if (gaiaParseFilterMbr
		    (p_blob, n_bytes, &minx, &miny, &maxx, &maxy, &mode))
		  {
		      if (mode == GAIA_FILTER_MBR_WITHIN
			  || mode == GAIA_FILTER_MBR_CONTAINS
			  || mode == GAIA_FILTER_MBR_INTERSECTS)
			{
			    cursor->minx = minx;
			    cursor->miny = miny;
			    cursor->maxx = maxx;
			    cursor->maxy = maxy;
			    cursor->mbr_mode = mode;
			    mbrc_read_row_filtered (cursor);
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
mbrc_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching a next row from cursor */
    MbrCacheCursorPtr cursor = (MbrCacheCursorPtr) pCursor;
    if (cursor->pVtab->error)
      {
	  cursor->eof = 1;
	  return SQLITE_OK;
      }
    if (cursor->strategy == 0)
	mbrc_read_row_unfiltered (cursor);
    else if (cursor->strategy == 2)
	mbrc_read_row_filtered (cursor);
    else
	cursor->eof = 1;
    return SQLITE_OK;
}

static int
mbrc_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    MbrCacheCursorPtr cursor = (MbrCacheCursorPtr) pCursor;
    return cursor->eof;
}

static int
mbrc_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	     int column)
{
/* fetching value for the Nth column */
    MbrCacheCursorPtr cursor = (MbrCacheCursorPtr) pCursor;
    if (!(cursor->current_cell))
	sqlite3_result_null (pContext);
    else
      {
	  if (column == 0)
	    {
		/* the PRIMARY KEY column */
		sqlite3_result_int64 (pContext, cursor->current_cell->rowid);
	    }
	  if (column == 1)
	    {
		/* the MBR column */
		char envelope[1024];
		sprintf (envelope,
			 "POLYGON((%1.2lf %1.2lf, %1.2lf %1.2lf, %1.2lf %1.2lf, %1.2lf %1.2lf, %1.2lf %1.2lf))",
			 cursor->current_cell->minx, cursor->current_cell->miny,
			 cursor->current_cell->maxx, cursor->current_cell->miny,
			 cursor->current_cell->maxx, cursor->current_cell->maxy,
			 cursor->current_cell->minx, cursor->current_cell->maxy,
			 cursor->current_cell->minx,
			 cursor->current_cell->miny);
		sqlite3_result_text (pContext, envelope, strlen (envelope),
				     SQLITE_TRANSIENT);
	    }
      }
    return SQLITE_OK;
}

static int
mbrc_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    MbrCacheCursorPtr cursor = (MbrCacheCursorPtr) pCursor;
    *pRowid = cursor->current_cell->rowid;
    return SQLITE_OK;
}

static int
mbrc_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	     sqlite_int64 * pRowid)
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
    MbrCachePtr p_vtab = (MbrCachePtr) pVTab;
    if (p_vtab->error)
	return SQLITE_OK;
    if (!(p_vtab->cache))
	p_vtab->cache =
	    cache_load (p_vtab->db, p_vtab->table_name, p_vtab->column_name);
    if (argc == 1)
      {
	  /* performing a DELETE */
	  if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
	    {
		rowid = sqlite3_value_int64 (argv[0]);
		cache_delete_cell (p_vtab->cache->first, rowid);
	    }
	  else
	      illegal = 1;
      }
    else
      {
	  if (sqlite3_value_type (argv[0]) == SQLITE_NULL)
	    {
		/* performing an INSERT */
		if (argc == 4)
		  {
		      if (sqlite3_value_type (argv[2]) == SQLITE_INTEGER
			  && sqlite3_value_type (argv[3]) == SQLITE_BLOB)
			{
			    rowid = sqlite3_value_int64 (argv[2]);
			    p_blob =
				(unsigned char *) sqlite3_value_blob (argv[3]);
			    n_bytes = sqlite3_value_bytes (argv[3]);
			    if (gaiaParseFilterMbr
				(p_blob, n_bytes, &minx, &miny, &maxx, &maxy,
				 &mode))
			      {
				  if (mode == GAIA_FILTER_MBR_DECLARE)
				    {
					if (!cache_find_by_rowid
					    (p_vtab->cache->first, rowid))
					    cache_insert_cell (p_vtab->cache,
							       rowid, minx,
							       miny, maxx,
							       maxy);
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
		if (argc == 4)
		  {
		      if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER
			  && sqlite3_value_type (argv[3]) == SQLITE_BLOB)
			{
			    rowid = sqlite3_value_int64 (argv[0]);
			    p_blob =
				(unsigned char *) sqlite3_value_blob (argv[3]);
			    n_bytes = sqlite3_value_bytes (argv[3]);
			    if (gaiaParseFilterMbr
				(p_blob, n_bytes, &minx, &miny, &maxx, &maxy,
				 &mode))
			      {
				  if (mode == GAIA_FILTER_MBR_DECLARE)
				      cache_update_cell (p_vtab->cache->first,
							 rowid, minx, miny,
							 maxx, maxy);
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
    if (illegal)
	return SQLITE_MISMATCH;
    return SQLITE_OK;
}

static int
mbrc_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
mbrc_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
mbrc_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
mbrc_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

int
sqlite3MbrCacheInit (sqlite3 * db)
{
    int rc = SQLITE_OK;
    my_mbr_module.iVersion = 1;
    my_mbr_module.xCreate = &mbrc_create;
    my_mbr_module.xConnect = &mbrc_connect;
    my_mbr_module.xBestIndex = &mbrc_best_index;
    my_mbr_module.xDisconnect = &mbrc_disconnect;
    my_mbr_module.xDestroy = &mbrc_destroy;
    my_mbr_module.xOpen = &mbrc_open;
    my_mbr_module.xClose = &mbrc_close;
    my_mbr_module.xFilter = &mbrc_filter;
    my_mbr_module.xNext = &mbrc_next;
    my_mbr_module.xEof = &mbrc_eof;
    my_mbr_module.xColumn = &mbrc_column;
    my_mbr_module.xRowid = &mbrc_rowid;
    my_mbr_module.xUpdate = &mbrc_update;
    my_mbr_module.xBegin = &mbrc_begin;
    my_mbr_module.xSync = &mbrc_sync;
    my_mbr_module.xCommit = &mbrc_commit;
    my_mbr_module.xRollback = &mbrc_rollback;
    my_mbr_module.xFindFunction = NULL;
    sqlite3_create_module_v2 (db, "MbrCache", &my_mbr_module, NULL, 0);
    return rc;
}

int
mbrcache_extension_init (sqlite3 * db)
{
    return sqlite3MbrCacheInit (db);
}
/**************** End file: mbrcache.c **********/


/**************** Begin file: virtualshape.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <string.h> */
/* #include <spatialite/sqlite3.h> */
/* #include <spatialite/spatialite.h> */
/* #include <spatialite/gaiaaux.h> */
/* #include <spatialite/gaiageo.h> */

static struct sqlite3_module my_shape_module;

typedef struct VirtualShapeStruct
{
/* extends the sqlite3_vtab struct */
    const sqlite3_module *pModule;	/* ptr to sqlite module: USED INTERNALLY BY SQLITE */
    int nRef;			/* # references: USED INTERNALLY BY SQLITE */
    char *zErrMsg;		/* error message: USE INTERNALLY BY SQLITE */
    sqlite3 *db;		/* the sqlite db holding the virtual table */
    gaiaShapefilePtr Shp;	/* the Shapefile struct */
    int Srid;			/* the Shapefile SRID */
} VirtualShape;
typedef VirtualShape *VirtualShapePtr;

typedef struct VirtualShapeCursorStruct
{
/* extends the sqlite3_vtab_cursor struct */
    VirtualShapePtr pVtab;	/* Virtual table of this cursor */
    long current_row;		/* the current row ID */
    int blobSize;
    unsigned char *blobGeometry;
    int eof;			/* the EOF marker */
} VirtualShapeCursor;
typedef VirtualShapeCursor *VirtualShapeCursorPtr;

static int
vshp_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	     sqlite3_vtab ** ppVTab, char **pzErr)
{
/* creates the virtual table connected to some shapefile */
    char buf[4096];
    char field[128];
    VirtualShapePtr p_vt;
    char path[2048];
    char encoding[128];
    const char *pEncoding = NULL;
    int len;
    const char *pPath = NULL;
    int srid;
    gaiaDbfFieldPtr pFld;
    int cnt;
    int col_cnt;
    int seed;
    int dup;
    int idup;
    char dummyName[4096];
    char **col_name = NULL;
/* checking for shapefile PATH */
    if (argc == 6)
      {
	  pPath = argv[3];
	  len = strlen (pPath);
	  if ((*(pPath + 0) == '\'' || *(pPath + 0) == '"')
	      && (*(pPath + len - 1) == '\'' || *(pPath + len - 1) == '"'))
	    {
		/* the path is enclosed between quotes - we need to dequote it */
		strcpy (path, pPath + 1);
		len = strlen (path);
		*(path + len - 1) = '\0';
	    }
	  else
	      strcpy (path, pPath);
	  pEncoding = argv[4];
	  len = strlen (pEncoding);
	  if ((*(pEncoding + 0) == '\'' || *(pEncoding + 0) == '"')
	      && (*(pEncoding + len - 1) == '\''
		  || *(pEncoding + len - 1) == '"'))
	    {
		/* the charset-name is enclosed between quotes - we need to dequote it */
		strcpy (encoding, pEncoding + 1);
		len = strlen (encoding);
		*(encoding + len - 1) = '\0';
	    }
	  else
	      strcpy (encoding, pEncoding);
	  srid = atoi (argv[5]);
	  if (srid <= 0)
	      srid = -1;
      }
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualShape module] CREATE VIRTUAL: illegal arg list {shp_path, encoding, srid}");
	  return SQLITE_ERROR;
      }
    p_vt = (VirtualShapePtr) sqlite3_malloc (sizeof (VirtualShape));
    if (!p_vt)
	return SQLITE_NOMEM;
    p_vt->pModule = &my_shape_module;
    p_vt->nRef = 0;
    p_vt->zErrMsg = NULL;
    p_vt->db = db;
    p_vt->Shp = gaiaAllocShapefile ();
    p_vt->Srid = srid;
/* trying to open files etc in order to ensure we actually have a genuine shapefile */
    gaiaOpenShpRead (p_vt->Shp, path, encoding, "UTF-8");
    if (!(p_vt->Shp->Valid))
      {
	  /* something is going the wrong way; creating a stupid default table */
	  sprintf (buf, "CREATE TABLE %s (PKUID INTEGER, Geometry BLOB)",
		   argv[1]);
	  if (sqlite3_declare_vtab (db, buf) != SQLITE_OK)
	    {
		*pzErr =
		    sqlite3_mprintf
		    ("[VirtualShape module] cannot build a table from Shapefile\n");
		return SQLITE_ERROR;
	    }
	  *ppVTab = (sqlite3_vtab *) p_vt;
	  return SQLITE_OK;
      }
    if (p_vt->Shp->
	Shape == 3
	|| p_vt->
	Shp->
	Shape == 13
	|| p_vt->
	Shp->
	Shape == 23
	|| p_vt->
	Shp->Shape == 5 || p_vt->Shp->Shape == 15 || p_vt->Shp->Shape == 25)
      {
	  /* fixing anyway the Geometry type for LINESTRING/MULTILINESTRING or POLYGON/MULTIPOLYGON */
	  gaiaShpAnalyze (p_vt->Shp);
      }
/* preparing the COLUMNs for this VIRTUAL TABLE */
    strcpy (buf, "CREATE TABLE ");
    strcat (buf, argv[2]);
    strcat (buf, " (PKUID INTEGER, Geometry BLOB");
/* checking for duplicate / illegal column names and antialising them */
    col_cnt = 0;
    pFld = p_vt->Shp->Dbf->First;
    while (pFld)
      {
	  /* counting DBF fields */
	  col_cnt++;
	  pFld = pFld->Next;
      }
    col_name = malloc (sizeof (char *) * col_cnt);
    cnt = 0;
    seed = 0;
    pFld = p_vt->Shp->Dbf->First;
    while (pFld)
      {
	  if (gaiaIllegalSqlName (pFld->Name)
	      || gaiaIsReservedSqlName (pFld->Name)
	      || gaiaIsReservedSqliteName (pFld->Name))
	      sprintf (dummyName, "\"%s\"", pFld->Name);
	  else
	      strcpy (dummyName, pFld->Name);
	  dup = 0;
	  for (idup = 0; idup < cnt; idup++)
	    {
		if (strcasecmp (dummyName, *(col_name + idup)) == 0)
		    dup = 1;
	    }
	  if (strcasecmp (dummyName, "PKUID") == 0)
	      dup = 1;
	  if (strcasecmp (dummyName, "Geometry") == 0)
	      dup = 1;
	  if (dup)
	      sprintf (dummyName, "COL_%d", seed++);
	  if (pFld->Type == 'N')
	    {
		if (pFld->Decimals > 0 || pFld->Length > 18)
		    sprintf (field, "%s DOUBLE", dummyName);
		else
		    sprintf (field, "%s INTEGER", dummyName);
	    }
	  else
	      sprintf (field, "%s VARCHAR(%d)", dummyName, pFld->Length);
	  strcat (buf, ", ");
	  strcat (buf, field);
	  len = strlen (dummyName);
	  *(col_name + cnt) = malloc (len + 1);
	  strcpy (*(col_name + cnt), dummyName);
	  cnt++;
	  pFld = pFld->Next;
      }
    strcat (buf, ")");
    if (col_name)
      {
	  /* releasing memory allocation for column names */
	  for (cnt = 0; cnt < col_cnt; cnt++)
	      free (*(col_name + cnt));
	  free (col_name);
      }
    if (sqlite3_declare_vtab (db, buf) != SQLITE_OK)
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualShape module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
	       buf);
	  return SQLITE_ERROR;
      }
    *ppVTab = (sqlite3_vtab *) p_vt;
    return SQLITE_OK;
}

static int
vshp_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	      sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table to some shapefile - simply aliases vshp_create() */
    return vshp_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
vshp_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIndex)
{
/* best index selection */
    return SQLITE_OK;
}

static int
vshp_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    VirtualShapePtr p_vt = (VirtualShapePtr) pVTab;
    if (p_vt->Shp)
	gaiaFreeShapefile (p_vt->Shp);
    sqlite3_free (p_vt);
    return SQLITE_OK;
}

static int
vshp_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases vshp_disconnect() */
    return vshp_disconnect (pVTab);
}

static void
vshp_read_row (VirtualShapeCursorPtr cursor)
{
/* trying to read a "row" from shapefile */
    int ret;
    gaiaGeomCollPtr geom;
    if (!(cursor->pVtab->Shp->Valid))
      {
	  cursor->eof = 1;
	  return;
      }
    if (cursor->blobGeometry)
      {
	  free (cursor->blobGeometry);
	  cursor->blobGeometry = NULL;
      }
    ret =
	gaiaReadShpEntity (cursor->pVtab->Shp, cursor->current_row,
			   cursor->pVtab->Srid);
    if (!ret)
      {
	  if (!(cursor->pVtab->Shp->LastError))	/* normal SHP EOF */
	    {
		cursor->eof = 1;
		return;
	    }
	  /* an error occurred */
	  fprintf (stderr, "%s\n", cursor->pVtab->Shp->LastError);
	  cursor->eof = 1;
	  return;
      }
    cursor->current_row++;
    geom = cursor->pVtab->Shp->Dbf->Geometry;
    if (geom)
      {
	  /* preparing the BLOB representing Geometry */
	  gaiaToSpatiaLiteBlobWkb (geom, &(cursor->blobGeometry),
				   &(cursor->blobSize));
      }
}

static int
vshp_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    VirtualShapeCursorPtr cursor =
	(VirtualShapeCursorPtr) sqlite3_malloc (sizeof (VirtualShapeCursor));
    if (cursor == NULL)
	return SQLITE_ERROR;
    cursor->pVtab = (VirtualShapePtr) pVTab;
    cursor->current_row = 0;
    cursor->blobGeometry = NULL;
    cursor->blobSize = 0;
    cursor->eof = 0;
    *ppCursor = (sqlite3_vtab_cursor *) cursor;
    vshp_read_row (cursor);
    return SQLITE_OK;
}

static int
vshp_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    if (cursor->blobGeometry)
	free (cursor->blobGeometry);
    sqlite3_free (pCursor);
    return SQLITE_OK;
}

static int
vshp_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	     int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    return SQLITE_OK;
}

static int
vshp_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching a next row from cursor */
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    vshp_read_row (cursor);
    return SQLITE_OK;
}

static int
vshp_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    return cursor->eof;
}

static int
vshp_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	     int column)
{
/* fetching value for the Nth column */
    int nCol = 2;
    gaiaGeomCollPtr geom;
    gaiaDbfFieldPtr pFld;
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    if (column == 0)
      {
	  /* the PRIMARY KEY column */
	  sqlite3_result_int (pContext, cursor->current_row);
	  return SQLITE_OK;
      }
    if (column == 1)
      {
	  /* the GEOMETRY column */
	  geom = cursor->pVtab->Shp->Dbf->Geometry;
	  if (geom)
	      sqlite3_result_blob (pContext, cursor->blobGeometry,
				   cursor->blobSize, SQLITE_STATIC);
	  else
	      sqlite3_result_null (pContext);
	  return SQLITE_OK;
      }
    pFld = cursor->pVtab->Shp->Dbf->First;
    while (pFld)
      {
	  /* column values */
	  if (nCol == column)
	    {
		if (!(pFld->Value))
		    sqlite3_result_null (pContext);
		else
		  {
		      switch (pFld->Value->Type)
			{
			case GAIA_INT_VALUE:
			    sqlite3_result_int64 (pContext,
						  pFld->Value->IntValue);
			    break;
			case GAIA_DOUBLE_VALUE:
			    sqlite3_result_double (pContext,
						   pFld->Value->DblValue);
			    break;
			case GAIA_TEXT_VALUE:
			    sqlite3_result_text (pContext,
						 pFld->Value->TxtValue,
						 strlen (pFld->Value->TxtValue),
						 SQLITE_STATIC);
			    break;
			default:
			    sqlite3_result_null (pContext);
			    break;
			}
		  }
		break;
	    }
	  nCol++;
	  pFld = pFld->Next;
      }
    return SQLITE_OK;
}

static int
vshp_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    VirtualShapeCursorPtr cursor = (VirtualShapeCursorPtr) pCursor;
    *pRowid = cursor->current_row;
    return SQLITE_OK;
}

static int
vshp_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	     sqlite_int64 * pRowid)
{
/* generic update [INSERT / UPDATE / DELETE */
    return SQLITE_READONLY;
}

static int
vshp_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
vshp_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
vshp_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
vshp_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

int
sqlite3VirtualShapeInit (sqlite3 * db)
{
    int rc = SQLITE_OK;
    my_shape_module.iVersion = 1;
    my_shape_module.xCreate = &vshp_create;
    my_shape_module.xConnect = &vshp_connect;
    my_shape_module.xBestIndex = &vshp_best_index;
    my_shape_module.xDisconnect = &vshp_disconnect;
    my_shape_module.xDestroy = &vshp_destroy;
    my_shape_module.xOpen = &vshp_open;
    my_shape_module.xClose = &vshp_close;
    my_shape_module.xFilter = &vshp_filter;
    my_shape_module.xNext = &vshp_next;
    my_shape_module.xEof = &vshp_eof;
    my_shape_module.xColumn = &vshp_column;
    my_shape_module.xRowid = &vshp_rowid;
    my_shape_module.xUpdate = &vshp_update;
    my_shape_module.xBegin = &vshp_begin;
    my_shape_module.xSync = &vshp_sync;
    my_shape_module.xCommit = &vshp_commit;
    my_shape_module.xRollback = &vshp_rollback;
    my_shape_module.xFindFunction = NULL;
    sqlite3_create_module_v2 (db, "VirtualShape", &my_shape_module, NULL, 0);
    return rc;
}

int
virtualshape_extension_init (sqlite3 * db)
{
    return sqlite3VirtualShapeInit (db);
}
/**************** End file: virtualshape.c **********/


/**************** Begin file: virtualnetwork.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <string.h> */
/* #include <float.h> */
/* #include <spatialite/sqlite3.h> */
/* #include <spatialite/spatialite.h> */
/* #include <spatialite/gaiaaux.h> */
/* #include <spatialite/gaiageo.h> */

static struct sqlite3_module my_net_module;

/******************************************************************************
/
/ VirtualNetwork structs
/
******************************************************************************/

typedef struct NetworkArcStruct
{
/* an ARC */
    const struct NetworkNodeStruct *NodeFrom;
    const struct NetworkNodeStruct *NodeTo;
    int ArcRowid;
    double Cost;
} NetworkArc;
typedef NetworkArc *NetworkArcPtr;

typedef struct NetworkNodeStruct
{
/* a NODE */
    int InternalIndex;
    int Id;
    char *Code;
    int NumArcs;
    NetworkArcPtr Arcs;
} NetworkNode;
typedef NetworkNode *NetworkNodePtr;

typedef struct NetworkStruct
{
/* the main NETWORK structure */
    int EndianArch;
    int MaxCodeLength;
    int CurrentIndex;
    int NodeCode;
    int NumNodes;
    char *TableName;
    char *FromColumn;
    char *ToColumn;
    char *GeometryColumn;
    NetworkNodePtr Nodes;
} Network;
typedef Network *NetworkPtr;

typedef struct ArcSolutionStruct
{
/* Geometry corresponding to an Arc used by Dijkstra shortest path solution */
    int ArcRowid;
    char *FromCode;
    char *ToCode;
    int FromId;
    int ToId;
    int Points;
    double *Coords;
    int Srid;
    struct ArcSolutionStruct *Next;

} ArcSolution;
typedef ArcSolution *ArcSolutionPtr;

typedef struct RowSolutionStruct
{
/* a row into the Dijkstra shortest path solution */
    NetworkArcPtr Arc;
    struct RowSolutionStruct *Next;

} RowSolution;
typedef RowSolution *RowSolutionPtr;

typedef struct SolutionStruct
{
/* the Dijkstra shortest path solution */
    ArcSolutionPtr FirstArc;
    ArcSolutionPtr LastArc;
    NetworkNodePtr From;
    NetworkNodePtr To;
    RowSolutionPtr First;
    RowSolutionPtr Last;
    RowSolutionPtr CurrentRow;
    int CurrentRowId;
    double TotalCost;
    gaiaGeomCollPtr Geometry;
} Solution;
typedef Solution *SolutionPtr;

/******************************************************************************
/
/ Dijkstra structs
/
******************************************************************************/

typedef struct DijkstraNode
{
    int Id;
    struct DijkstraNode **To;
    NetworkArcPtr *Link;
    int DimTo;
    struct DijkstraNode *PreviousNode;
    NetworkArcPtr Arc;
    double Distance;
    int Value;
} DijkstraNode;
typedef DijkstraNode *DijkstraNodePtr;

typedef struct DijkstraNodes
{
    DijkstraNodePtr Nodes;
    int Dim;
    int DimLink;
} DijkstraNodes;
typedef DijkstraNodes *DijkstraNodesPtr;

typedef struct DjikstraHeapStruct
{
    DijkstraNodePtr *Values;
    int Head;
    int Tail;
} DijkstraHeap;
typedef DijkstraHeap *DijkstraHeapPtr;

/******************************************************************************
/
/ VirtualTable structs
/
******************************************************************************/

typedef struct VirtualNetworkStruct
{
/* extends the sqlite3_vtab struct */
    const sqlite3_module *pModule;	/* ptr to sqlite module: USED INTERNALLY BY SQLITE */
    int nRef;			/* # references: USED INTERNALLY BY SQLITE */
    char *zErrMsg;		/* error message: USE INTERNALLY BY SQLITE */
    sqlite3 *db;		/* the sqlite db holding the virtual table */
    NetworkPtr graph;		/* the NETWORK structure */
} VirtualNetwork;
typedef VirtualNetwork *VirtualNetworkPtr;

typedef struct VirtualNetworkCursortStruct
{
/* extends the sqlite3_vtab_cursor struct */
    VirtualNetworkPtr pVtab;	/* Virtual table of this cursor */
    SolutionPtr solution;	/* the current solution */
    int eof;			/* the EOF marker */
} VirtualNetworkCursor;
typedef VirtualNetworkCursor *VirtualNetworkCursorPtr;

/*
/
/  implementation of the Dijkstra Shortest Path algorithm
/
////////////////////////////////////////////////////////////
/
/ Author: Luigi Costalli luigi.costalli@gmail.com
/ version 1.0. 2008 October 21
 /
 */

static DijkstraNodesPtr
dijkstra_init (NetworkPtr graph)
{
/* allocating and initializing the Dijkstra struct */
    int i;
    int j;
    DijkstraNodesPtr nd;
    NetworkNodePtr nn;
/* allocating the main Nodes struct */
    nd = malloc (sizeof (DijkstraNodes));
/* allocating and initializing  Nodes array */
    nd->Nodes = malloc (sizeof (DijkstraNode) * graph->NumNodes);
    nd->Dim = graph->NumNodes;
    nd->DimLink = 0;
    for (i = 0; i < graph->NumNodes; i++)
      {
	  /* initializing the Nodes array */
	  nn = graph->Nodes + i;
	  nd->Nodes[i].Id = nn->InternalIndex;
	  nd->Nodes[i].DimTo = nn->NumArcs;
	  nd->Nodes[i].To = malloc (sizeof (DijkstraNodePtr) * nn->NumArcs);
	  nd->Nodes[i].Link = malloc (sizeof (NetworkArcPtr) * nn->NumArcs);
	  for (j = 0; j < nn->NumArcs; j++)
	    {
		/*  setting the outcoming Arcs for the current Node */
		nd->DimLink++;
		nd->Nodes[i].To[j] =
		    nd->Nodes + nn->Arcs[j].NodeTo->InternalIndex;
		nd->Nodes[i].Link[j] = nn->Arcs + j;
	    }
      }
    return (nd);
}

static void
dijkstra_free (DijkstraNodes * e)
{
/* memory cleanup; freeing the Dijkstra struct */
    int i;
    for (i = 0; i < e->Dim; i++)
      {
	  if (e->Nodes[i].DimTo != 0)
	    {
		free (e->Nodes[i].Link);
		free (e->Nodes[i].To);
	    }
      }
    free (e->Nodes);
    free (e);
}

static DijkstraHeapPtr
dijkstra_heap_init (int dim)
{
/* allocating the Nodes ordered list */
    DijkstraHeapPtr h;
    h = malloc (sizeof (DijkstraHeap));
    h->Values = malloc (sizeof (DijkstraNodePtr) * dim);
    h->Head = 0;
    h->Tail = 0;
    return (h);
}

static void
dijkstra_heap_free (DijkstraHeapPtr h)
{
/* freeing the Nodes ordered list */
    free (h->Values);
    free (h);
}

static int
dijkstra_compare (const void *a, const void *b)
{
/* comparison function for QSORT */
    return (int) (((DijkstraNodePtr) a)->Distance -
		  ((DijkstraNodePtr) b)->Distance);
}

static void
dijkstra_push (DijkstraHeapPtr h, DijkstraNodePtr n)
{
/* inserting a Node into the ordered list */
    h->Values[h->Tail] = n;
    h->Tail++;
}

static DijkstraNodePtr
dijkstra_pop (DijkstraHeapPtr h)
{
/* fetching the minimum value from the ordered list */
    DijkstraNodePtr n;
    qsort (h->
	   Values +
	   h->Head,
	   h->Tail - h->Head, sizeof (DijkstraNodePtr), dijkstra_compare);
    n = h->Values[h->Head];
    h->Head++;
    return (n);
}

static NetworkArcPtr *
dijkstra_shortest_path (DijkstraNodesPtr e, NetworkNodePtr pfrom,
			NetworkNodePtr pto, int *ll)
{
/* identifying the Shortest Path */
    int from;
    int to;
    int i;
    int k;
    DijkstraNodePtr n;
    int cnt;
    NetworkArcPtr *result;
    DijkstraHeapPtr h;
/* setting From/To */
    from = pfrom->InternalIndex;
    to = pto->InternalIndex;
/* initializing the heap */
    h = dijkstra_heap_init (e->DimLink);
/* initializing the graph */
    for (i = 0; i < e->Dim; i++)
      {
	  e->Nodes[i].PreviousNode = NULL;
	  e->Nodes[i].Arc = NULL;
	  e->Nodes[i].Value = 0;
	  e->Nodes[i].Distance = DBL_MAX;
      }
/* pushes the From node into the Nodes list */
    e->Nodes[from].Distance = 0.0;
    dijkstra_push (h, e->Nodes + from);
    while (h->Tail != h->Head)
      {
	  /* Dijsktra loop */
	  n = dijkstra_pop (h);
	  if (n->Id == to)
	    {
		/* destination reached */
		break;
	    }
	  n->Value = 1;
	  for (i = 0; i < n->DimTo; i++)
	    {
		if (n->To[i]->Value == 0)
		  {
		      if (n->To[i]->Distance > n->Distance + n->Link[i]->Cost)
			{
			    n->To[i]->Distance = n->Distance + n->Link[i]->Cost;
			    n->To[i]->PreviousNode = n;
			    n->To[i]->Arc = n->Link[i];
			    dijkstra_push (h, n->To[i]);
			}
		  }
	    }
      }
    dijkstra_heap_free (h);
    cnt = 0;
    n = e->Nodes + to;
    while (n->PreviousNode != NULL)
      {
	  /* counting how many Arcs are into the Shortest Path solution */
	  cnt++;
	  n = n->PreviousNode;
      }
/* allocating the solution */
    result = malloc (sizeof (NetworkArcPtr) * cnt);
    k = cnt - 1;
    n = e->Nodes + to;
    while (n->PreviousNode != NULL)
      {
	  /* inserting an Arc  into the solution */
	  result[k] = n->Arc;
	  n = n->PreviousNode;
	  k--;
      }
    *ll = cnt;
    return (result);
};

/* END of Luigi Costalli Dijkstra Shortest Path implementation */

static int
cmp_nodes_code (const void *p1, const void *p2)
{
/* compares two nodes  by CODE [for BSEARCH] */
    NetworkNodePtr pN1 = (NetworkNodePtr) p1;
    NetworkNodePtr pN2 = (NetworkNodePtr) p2;
    return strcmp (pN1->Code, pN2->Code);
}

static int
cmp_nodes_id (const void *p1, const void *p2)
{
/* compares two nodes  by ID [for BSEARCH] */
    NetworkNodePtr pN1 = (NetworkNodePtr) p1;
    NetworkNodePtr pN2 = (NetworkNodePtr) p2;
    return pN1->Id - pN2->Id;
}

static NetworkNodePtr
find_node_by_code (NetworkPtr graph, const char *code)
{
/* searching a Node (by Code) into the sorted list */
    NetworkNodePtr ret;
    NetworkNode pN;
    pN.Code = (char *) code;
    ret =
	bsearch (&pN, graph->Nodes, graph->NumNodes, sizeof (NetworkNode),
		 cmp_nodes_code);
    return ret;
}

static NetworkNodePtr
find_node_by_id (NetworkPtr graph, const int id)
{
/* searching a Node (by Id) into the sorted list */
    NetworkNodePtr ret;
    NetworkNode pN;
    pN.Id = id;
    ret =
	bsearch (&pN, graph->Nodes, graph->NumNodes, sizeof (NetworkNode),
		 cmp_nodes_id);
    return ret;
}

static void
delete_solution (SolutionPtr solution)
{
/* deleting the current solution */
    ArcSolutionPtr pA;
    ArcSolutionPtr pAn;
    RowSolutionPtr pR;
    RowSolutionPtr pRn;
    if (!solution)
	return;
    pA = solution->FirstArc;
    while (pA)
      {
	  pAn = pA->Next;
	  if (pA->FromCode)
	      free (pA->FromCode);
	  if (pA->ToCode)
	      free (pA->ToCode);
	  if (pA->Coords)
	      free (pA->Coords);
	  free (pA);
	  pA = pAn;
      }
    pR = solution->First;
    while (pR)
      {
	  pRn = pR->Next;
	  free (pR);
	  pR = pRn;
      }
    if (solution->Geometry)
	gaiaFreeGeomColl (solution->Geometry);
    free (solution);
};

static void
reset_solution (SolutionPtr solution)
{
/* resetting the current solution */
    ArcSolutionPtr pA;
    ArcSolutionPtr pAn;
    RowSolutionPtr pR;
    RowSolutionPtr pRn;
    if (!solution)
	return;
    pA = solution->FirstArc;
    while (pA)
      {
	  pAn = pA->Next;
	  if (pA->FromCode)
	      free (pA->FromCode);
	  if (pA->ToCode)
	      free (pA->ToCode);
	  if (pA->Coords)
	      free (pA->Coords);
	  free (pA);
	  pA = pAn;
      }
    pR = solution->First;
    while (pR)
      {
	  pRn = pR->Next;
	  free (pR);
	  pR = pRn;
      }
    if (solution->Geometry)
	gaiaFreeGeomColl (solution->Geometry);
    solution->FirstArc = NULL;
    solution->LastArc = NULL;
    solution->From = NULL;
    solution->To = NULL;
    solution->First = NULL;
    solution->Last = NULL;
    solution->CurrentRow = NULL;
    solution->CurrentRowId = 0;
    solution->TotalCost = 0.0;
    solution->Geometry = NULL;
};

static SolutionPtr
alloc_solution ()
{
/* allocates and initializes the current solution */
    SolutionPtr p = malloc (sizeof (Solution));
    p->FirstArc = NULL;
    p->LastArc = NULL;
    p->From = NULL;
    p->To = NULL;
    p->First = NULL;
    p->Last = NULL;
    p->CurrentRow = NULL;
    p->CurrentRowId = 0;
    p->TotalCost = 0.0;
    p->Geometry = NULL;
    return p;
};

static void
add_arc_to_solution (SolutionPtr solution, NetworkArcPtr arc)
{
/* inserts an Arc into the Dijkstra Shortest Path solution */
    RowSolutionPtr p = malloc (sizeof (RowSolution));
    p->Arc = arc;
    p->Next = NULL;
    solution->TotalCost += arc->Cost;
    if (!(solution->First))
	solution->First = p;
    if (solution->Last)
	solution->Last->Next = p;
    solution->Last = p;
}

static void
add_arc_geometry_to_solution (SolutionPtr solution, int arc_id,
			      const char *from_code, const char *to_code,
			      int from_id, int to_id, int points,
			      double *coords, int srid)
{
/* inserts an Arc Geometry into the Dijkstra Shortest Path solution */
    int len;
    ArcSolutionPtr p = malloc (sizeof (ArcSolution));
    p->ArcRowid = arc_id;
    p->FromCode = NULL;
    len = strlen (from_code);
    if (len > 0)
      {
	  p->FromCode = malloc (len + 1);
	  strcpy (p->FromCode, from_code);
      }
    p->ToCode = NULL;
    len = strlen (to_code);
    if (len > 0)
      {
	  p->ToCode = malloc (len + 1);
	  strcpy (p->ToCode, to_code);
      }
    p->FromId = from_id;
    p->ToId = to_id;
    p->Points = points;
    p->Coords = coords;
    p->Srid = srid;
    p->Next = NULL;
    if (!(solution->FirstArc))
	solution->FirstArc = p;
    if (solution->LastArc)
	solution->LastArc->Next = p;
    solution->LastArc = p;
}

static void
dijkstra_solve (sqlite3 * handle, NetworkPtr graph, SolutionPtr solution)
{
/* computing a Dijkstra Shortest Path solution */
    int cnt;
    int i;
    char sql[4096];
    char dummy[64];
    int err;
    int error = 0;
    int ret;
    int arc_id;
    const unsigned char *blob;
    int size;
    int from_id;
    int to_id;
    char from_code[128];
    char to_code[128];
    sqlite3_stmt *stmt;
    NetworkArcPtr *shortest_path;
    DijkstraNodesPtr dijkstra = dijkstra_init (graph);
    shortest_path =
	dijkstra_shortest_path (dijkstra, solution->From, solution->To, &cnt);
    dijkstra_free (dijkstra);
    if (cnt > 0)
      {
	  /* building the solution */
	  for (i = 0; i < cnt; i++)
	      add_arc_to_solution (solution, shortest_path[i]);
      }
/* preparing the Geometry representing this solution */
    sprintf (sql,
	     "SELECT ROWID, \"%s\", \"%s\", \"%s\" FROM \"%s\" WHERE ROWID IN (",
	     graph->
	     FromColumn,
	     graph->ToColumn, graph->GeometryColumn, graph->TableName);
    for (i = 0; i < cnt; i++)
      {
	  if (i == 0)
	      sprintf (dummy, "%d", shortest_path[i]->ArcRowid);
	  else
	      sprintf (dummy, ",%d", shortest_path[i]->ArcRowid);
	  strcat (sql, dummy);
      }
    if (shortest_path)
	free (shortest_path);
    strcat (sql, ")");
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  error = 1;
	  goto abort;
      }
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	    {
		arc_id = -1;
		from_id = -1;
		to_id = -1;
		*from_code = '\0';
		*to_code = '\0';
		blob = NULL;
		size = 0;
		err = 0;
		if (sqlite3_column_type (stmt, 0) == SQLITE_INTEGER)
		    arc_id = sqlite3_column_int (stmt, 0);
		else
		    err = 1;
		if (graph->NodeCode)
		  {
		      /* nodes are identified by TEXT codes */
		      if (sqlite3_column_type (stmt, 1) == SQLITE_TEXT)
			  strcpy (from_code,
				  (char *) sqlite3_column_text (stmt, 1));
		      else
			  err = 1;
		      if (sqlite3_column_type (stmt, 2) == SQLITE_TEXT)
			  strcpy (to_code,
				  (char *) sqlite3_column_text (stmt, 2));
		      else
			  err = 1;
		  }
		else
		  {
		      /* nodes are identified by INTEGER ids */
		      if (sqlite3_column_type (stmt, 1) == SQLITE_INTEGER)
			  from_id = sqlite3_column_int (stmt, 1);
		      else
			  err = 1;
		      if (sqlite3_column_type (stmt, 2) == SQLITE_INTEGER)
			  to_id = sqlite3_column_int (stmt, 2);
		      else
			  err = 1;
		  }
		if (sqlite3_column_type (stmt, 3) == SQLITE_BLOB)
		  {
		      blob =
			  (const unsigned char *) sqlite3_column_blob (stmt, 3);
		      size = sqlite3_column_bytes (stmt, 3);
		  }
		else
		    err = 1;
		if (err)
		    error = 1;
		else
		  {
		      /* saving the Arc geometry into the temporary struct */
		      gaiaGeomCollPtr geom =
			  gaiaFromSpatiaLiteBlobWkb (blob, size);
		      if (geom)
			{
			    /* OK, we have fetched a valid Geometry */
			    if (geom->FirstPoint == NULL
				&& geom->FirstPolygon == NULL
				&& geom->FirstLinestring != NULL
				&& geom->FirstLinestring ==
				geom->LastLinestring)
			      {
				  /* Geometry is LINESTRING as expected */
				  int iv;
				  int points = geom->FirstLinestring->Points;
				  double *coords =
				      malloc (sizeof (double) * (points * 2));
				  for (iv = 0; iv < points; iv++)
				    {
					double x;
					double y;
					gaiaGetPoint (geom->FirstLinestring->
						      Coords, iv, &x, &y);
					*(coords + ((iv * 2) + 0)) = x;
					*(coords + ((iv * 2) + 1)) = y;
				    }
				  add_arc_geometry_to_solution (solution,
								arc_id,
								from_code,
								to_code,
								from_id, to_id,
								points, coords,
								geom->Srid);
			      }
			    else
				error = 1;
			    gaiaFreeGeomColl (geom);
			}
		      else
			  error = 1;
		  }
	    }
      }
    sqlite3_finalize (stmt);
  abort:
    if (!error)
      {
	  /* building the Geometry representing the Dijsktra Shortest Path Solution */
	  gaiaLinestringPtr ln;
	  int tot_pts = 0;
	  RowSolutionPtr pR;
	  ArcSolutionPtr pA;
	  int srid = -1;
	  if (solution->FirstArc)
	      srid = (solution->FirstArc)->Srid;
	  pR = solution->First;
	  while (pR)
	    {
		pA = solution->FirstArc;
		while (pA)
		  {
		      /* computing how many vertices do we need to build the LINESTRING */
		      if (pR->Arc->ArcRowid == pA->ArcRowid)
			{
			    if (pR == solution->First)
				tot_pts += pA->Points;
			    else
				tot_pts += (pA->Points - 1);
			    if (pA->Srid != srid)
				srid = -1;
			}
		      pA = pA->Next;
		  }
		pR = pR->Next;
	    }
	  /* creating the Shortest Path Geometry - LINESTRING */
	  ln = gaiaAllocLinestring (tot_pts);
	  solution->Geometry = gaiaAllocGeomColl ();
	  solution->Geometry->Srid = srid;
	  gaiaInsertLinestringInGeomColl (solution->Geometry, ln);
	  tot_pts = 0;
	  pR = solution->First;
	  while (pR)
	    {
		/* building the LINESTRING */
		int skip;
		if (pR == solution->First)
		    skip = 0;	/* for first arc we must copy any vertex */
		else
		    skip = 1;	/* for subsequent arcs we must skip first vertex [already inserted from previous arc] */
		pA = solution->FirstArc;
		while (pA)
		  {
		      if (pR->Arc->ArcRowid == pA->ArcRowid)
			{
			    /* copying vertices from correspoinding Arc Geometry */
			    int ini;
			    int iv;
			    int rev;
			    double x;
			    double y;
			    if (graph->NodeCode)
			      {
				  /* nodes are identified by TEXT codes */
				  if (strcmp
				      (pR->Arc->NodeFrom->Code,
				       pA->ToCode) == 0)
				      rev = 1;
				  else
				      rev = 0;
			      }
			    else
			      {
				  /* nodes are identified by INTEGER ids */
				  if (pR->Arc->NodeFrom->Id == pA->ToId)
				      rev = 1;
				  else
				      rev = 0;
			      }
			    if (rev)
			      {
				  /* copying Arc vertices in reverse order */
				  if (skip)
				      ini = pA->Points - 2;
				  else
				      ini = pA->Points - 1;
				  for (iv = ini; iv >= 0; iv--)
				    {
					x = *(pA->Coords + ((iv * 2) + 0));
					y = *(pA->Coords + ((iv * 2) + 1));
					gaiaSetPoint (ln->Coords, tot_pts, x,
						      y);
					tot_pts++;
				    }
			      }
			    else
			      {
				  /* copying Arc vertices in normal order */
				  if (skip)
				      ini = 1;
				  else
				      ini = 0;
				  for (iv = ini; iv < pA->Points; iv++)
				    {
					x = *(pA->Coords + ((iv * 2) + 0));
					y = *(pA->Coords + ((iv * 2) + 1));
					gaiaSetPoint (ln->Coords, tot_pts, x,
						      y);
					tot_pts++;
				    }
			      }
			    break;
			}
		      pA = pA->Next;
		  }
		pR = pR->Next;
	    }
      }
}

static void
network_free (NetworkPtr p)
{
/* memory cleanup; freeing any allocation for the network struct */
    NetworkNodePtr pN;
    int i;
    if (!p)
	return;
    for (i = 0; i < p->NumNodes; i++)
      {
	  pN = p->Nodes + i;
	  if (pN->Code)
	      free (pN->Code);
	  if (pN->Arcs)
	      free (pN->Arcs);
      }
    if (p->TableName)
	free (p->TableName);
    if (p->FromColumn)
	free (p->FromColumn);
    if (p->ToColumn)
	free (p->ToColumn);
    if (p->GeometryColumn)
	free (p->GeometryColumn);
    free (p);
}

static NetworkPtr
network_init (const unsigned char *blob, int size)
{
/* parsing the HEADER block */
    NetworkPtr graph;
    int nodes;
    int node_code;
    int max_code_length;
    int endian_arch = gaiaEndianArch ();
    const char *table;
    const char *from;
    const char *to;
    const char *geom;
    int len;
    const unsigned char *ptr;
    if (size < 9)
	return NULL;
    if (*(blob + 0) != GAIA_NET_START)	/* signature */
	return NULL;
    if (*(blob + 1) != GAIA_NET_HEADER)	/* signature */
	return NULL;
    nodes = gaiaImport32 (blob + 2, 1, endian_arch);	/* # nodes */
    if (nodes <= 0)
	return NULL;
    if (*(blob + 6) == GAIA_NET_CODE)	/* Nodes identified by a TEXT code */
	node_code = 1;
    else if (*(blob + 6) == GAIA_NET_ID)	/* Nodes indentified by an INTEGER id */
	node_code = 0;
    else
	return NULL;
    max_code_length = *(blob + 7);	/* Max TEXT Code length */
    if (*(blob + 8) != GAIA_NET_TABLE)	/* signature for TABLE NAME */
	return NULL;
    ptr = blob + 9;
    len = gaiaImport16 (ptr, 1, endian_arch);	/* TABLE NAME is varlen */
    ptr += 2;
    table = (char *) ptr;
    ptr += len;
    if (*ptr != GAIA_NET_FROM)	/* signature for FromNode COLUMN */
	return NULL;
    ptr++;
    len = gaiaImport16 (ptr, 1, endian_arch);	/* FromNode COLUMN is varlen */
    ptr += 2;
    from = (char *) ptr;
    ptr += len;
    if (*ptr != GAIA_NET_TO)	/* signature for Toode COLUMN */
	return NULL;
    ptr++;
    len = gaiaImport16 (ptr, 1, endian_arch);	/* ToNode COLUMN is varlen */
    ptr += 2;
    to = (char *) ptr;
    ptr += len;
    if (*ptr != GAIA_NET_GEOM)	/* signature for Geometry COLUMN */
	return NULL;
    ptr++;
    len = gaiaImport16 (ptr, 1, endian_arch);	/* Geometry COLUMN is varlen */
    ptr += 2;
    geom = (char *) ptr;
    ptr += len;
    if (*ptr != GAIA_NET_END)	/* signature */
	return NULL;
    graph = malloc (sizeof (Network));
    graph->EndianArch = endian_arch;
    graph->CurrentIndex = 0;
    graph->NodeCode = node_code;
    graph->MaxCodeLength = max_code_length;
    graph->NumNodes = nodes;
    graph->Nodes = malloc (sizeof (NetworkNode) * nodes);
    len = strlen (table);
    graph->TableName = malloc (len + 1);
    strcpy (graph->TableName, table);
    len = strlen (from);
    graph->FromColumn = malloc (len + 1);
    strcpy (graph->FromColumn, from);
    len = strlen (to);
    graph->ToColumn = malloc (len + 1);
    strcpy (graph->ToColumn, to);
    len = strlen (geom);
    graph->GeometryColumn = malloc (len + 1);
    strcpy (graph->GeometryColumn, geom);
    return graph;
}

static int
network_block (NetworkPtr graph, const unsigned char *blob, int size)
{
/* parsing a NETWORK Block */
    const unsigned char *in = blob;
    int nodes;
    int i;
    int ia;
    int index;
    char code[256];
    int nodeId = -1;
    int arcs;
    NetworkNodePtr pN;
    NetworkArcPtr pA;
    int len;
    int arcId;
    int nodeToIdx;
    double cost;
    if (size < 3)
	goto error;
    if (*in++ != GAIA_NET_BLOCK)	/* signature */
	goto error;
    nodes = gaiaImport16 (in, 1, graph->EndianArch);	/* # Nodes */
    in += 2;
    for (i = 0; i < nodes; i++)
      {
	  /* parsing each node */
	  if ((size - (in - blob)) < 5)
	      goto error;
	  if (*in++ != GAIA_NET_NODE)	/* signature */
	      goto error;
	  index = gaiaImport32 (in, 1, graph->EndianArch);	/* node internal index */
	  in += 4;
	  if (index < 0 || index >= graph->NumNodes)
	      goto error;
	  if (graph->NodeCode)
	    {
		/* Nodes are identified by a TEXT Code */
		if ((size - (in - blob)) < graph->MaxCodeLength)
		    goto error;
		memcpy (code, in, graph->MaxCodeLength);
		in += graph->MaxCodeLength;
	    }
	  else
	    {
		/* Nodes are identified by an INTEGER Id */
		if ((size - (in - blob)) < 4)
		    goto error;
		nodeId = gaiaImport32 (in, 1, graph->EndianArch);	/* the Node ID */
		in += 4;
	    }
	  if ((size - (in - blob)) < 2)
	      goto error;
	  arcs = gaiaImport16 (in, 1, graph->EndianArch);	/* # Arcs */
	  in += 2;
	  if (arcs < 0)
	      goto error;
	  /* initializing the Node */
	  pN = graph->Nodes + index;
	  pN->InternalIndex = index;
	  if (graph->NodeCode)
	    {
		/* Nodes are identified by a TEXT Code */
		pN->Id = -1;
		len = strlen (code);
		pN->Code = malloc (len + 1);
		strcpy (pN->Code, code);
	    }
	  else
	    {
		/* Nodes are identified by an INTEGER Id */
		pN->Id = nodeId;
		pN->Code = NULL;
	    }
	  pN->NumArcs = arcs;
	  if (arcs)
	    {
		/* parsing the Arcs */
		pN->Arcs = malloc (sizeof (NetworkArc) * arcs);
		for (ia = 0; ia < arcs; ia++)
		  {
		      /* parsing each Arc */
		      if ((size - (in - blob)) < 18)
			  goto error;
		      if (*in++ != GAIA_NET_ARC)	/* signature */
			  goto error;
		      arcId = gaiaImport32 (in, 1, graph->EndianArch);	/* # Arc ROWID */
		      in += 4;
		      nodeToIdx = gaiaImport32 (in, 1, graph->EndianArch);	/* # NodeTo internal index */
		      in += 4;
		      cost = gaiaImport64 (in, 1, graph->EndianArch);	/* # Cost */
		      in += 8;
		      if (*in++ != GAIA_NET_END)	/* signature */
			  goto error;
		      pA = pN->Arcs + ia;
		      /* initializing the Arc */
		      if (nodeToIdx < 0 || nodeToIdx >= graph->NumNodes)
			  goto error;
		      pA->NodeFrom = pN;
		      pA->NodeTo = graph->Nodes + nodeToIdx;
		      pA->ArcRowid = arcId;
		      pA->Cost = cost;
		  }
	    }
	  else
	      pN->Arcs = NULL;
	  if ((size - (in - blob)) < 1)
	      goto error;
	  if (*in++ != GAIA_NET_END)	/* signature */
	      goto error;
      }
    return 1;
  error:
    return 0;
}

static NetworkPtr
load_network (sqlite3 * handle, const char *table)
{
/* loads the NETWORK struct */
    NetworkPtr graph = NULL;
    sqlite3_stmt *stmt;
    char sql[1024];
    int ret;
    int header = 1;
    const unsigned char *blob;
    int size;
    sprintf (sql, "SELECT \"NetworkData\" FROM \"%s\" ORDER BY \"Id\"", table);
    ret = sqlite3_prepare_v2 (handle, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
	goto abort;
    while (1)
      {
	  ret = sqlite3_step (stmt);
	  if (ret == SQLITE_DONE)
	      break;
	  if (ret == SQLITE_ROW)
	    {
		if (sqlite3_column_type (stmt, 0) == SQLITE_BLOB)
		  {
		      blob =
			  (const unsigned char *) sqlite3_column_blob (stmt, 0);
		      size = sqlite3_column_bytes (stmt, 0);
		      if (header)
			{
			    /* parsing the HEADER block */
			    graph = network_init (blob, size);
			    header = 0;
			}
		      else
			{
			    /* parsing ordinary Blocks */
			    if (!graph)
			      {
				  sqlite3_finalize (stmt);
				  goto abort;
			      }
			    if (!network_block (graph, blob, size))
			      {
				  sqlite3_finalize (stmt);
				  goto abort;
			      }
			}
		  }
		else
		  {
		      sqlite3_finalize (stmt);
		      goto abort;
		  }
	    }
	  else
	    {
		sqlite3_finalize (stmt);
		goto abort;
	    }
      }
    sqlite3_finalize (stmt);
    return graph;
  abort:
    network_free (graph);
    return NULL;
}

static int
vnet_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	     sqlite3_vtab ** ppVTab, char **pzErr)
{
/* creates the virtual table connected to some shapefile */
    VirtualNetworkPtr p_vt;
    char buf[1024];
    int err;
    int ret;
    int i;
    int n_rows;
    int n_columns;
    const char *vtable;
    const char *table;
    const char *col_name;
    char **results;
    char *err_msg = NULL;
    char sql[4096];
    int ok_tbl;
    int ok_id;
    int ok_data;
    NetworkPtr graph = NULL;
/* checking for table_name and geo_column_name */
    if (argc == 4)
      {
	  vtable = argv[2];
	  table = argv[3];
      }
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualNetwork module] CREATE VIRTUAL: illegal arg list {NETWORK-DATAtable}\n");
	  return SQLITE_ERROR;
      }
/* retrieving the base table columns */
    err = 0;
    ok_tbl = 0;
    ok_id = 0;
    ok_data = 0;
    sprintf (sql, "PRAGMA table_info(\"%s\")", table);
    ret = sqlite3_get_table (db, sql, &results, &n_rows, &n_columns, &err_msg);
    if (ret != SQLITE_OK)
      {
	  err = 1;
	  goto illegal;
      }
    if (n_rows > 1)
      {
	  ok_tbl = 1;
	  for (i = 1; i <= n_rows; i++)
	    {
		col_name = results[(i * n_columns) + 1];
		if (strcasecmp (col_name, "id") == 0)
		    ok_id = 1;
		if (strcasecmp (col_name, "networkdata") == 0)
		    ok_data = 1;
	    }
	  sqlite3_free_table (results);
	  if (!ok_id)
	      err = 1;
	  if (!ok_data)
	      err = 1;
      }
    else
	err = 1;
  illegal:
    if (err)
      {
	  /* something is going the wrong way */
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualNetwork module] cannot build a valid NETWORK\n");
	  return SQLITE_ERROR;
      }
    p_vt = (VirtualNetworkPtr) sqlite3_malloc (sizeof (VirtualNetwork));
    if (!p_vt)
	return SQLITE_NOMEM;
    graph = load_network (db, table);
    if (!graph)
      {
	  /* something is going the wrong way */
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualNetwork module] cannot build a valid NETWORK\n");
	  return SQLITE_ERROR;
      }
    p_vt->db = db;
    p_vt->graph = graph;
    p_vt->pModule = &my_net_module;
    p_vt->nRef = 0;
    p_vt->zErrMsg = NULL;
/* preparing the COLUMNs for this VIRTUAL TABLE */
    strcpy (buf, "CREATE TABLE \"");
    strcat (buf, vtable);
    strcat (buf, "\" (\"ArcRowid\" INTEGER, ");
    if (p_vt->graph->NodeCode)
	strcat (buf, "\"NodeFrom\" TEXT, \"NodeTo\" TEXT,");
    else
	strcat (buf, "\"NodeFrom\" INTEGER, \"NodeTo\" INTEGER,");
    strcat (buf, " \"Cost\" DOUBLE, \"Geometry\" BLOB)");
    if (sqlite3_declare_vtab (db, buf) != SQLITE_OK)
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualNetwork module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
	       buf);
	  return SQLITE_ERROR;
      }
    *ppVTab = (sqlite3_vtab *) p_vt;
    return SQLITE_OK;
}

static int
vnet_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	      sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table to some shapefile - simply aliases vshp_create() */
    return vnet_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
vnet_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIdxInfo)
{
/* best index selection */
    int i;
    int errors = 0;
    int err = 1;
    int from = 0;
    int to = 0;
    int i_from = -1;
    int i_to = -1;
    for (i = 0; i < pIdxInfo->nConstraint; i++)
      {
	  /* verifying the constraints */
	  struct sqlite3_index_constraint *p = &(pIdxInfo->aConstraint[i]);
	  if (p->usable)
	    {
		if (p->iColumn == 1 && p->op == SQLITE_INDEX_CONSTRAINT_EQ)
		  {
		      from++;
		      i_from = i;
		  }
		else if (p->iColumn == 2 && p->op == SQLITE_INDEX_CONSTRAINT_EQ)
		  {
		      to++;
		      i_to = i;
		  }
		else
		    errors++;
	    }
      }
    if (from == 1 && to == 1 && errors == 0)
      {
	  /* this one is a valid Dijskra Shortest Path query */
	  if (i_from < i_to)
	      pIdxInfo->idxNum = 1;	/* first arg is FROM */
	  else
	      pIdxInfo->idxNum = 2;	/* first arg is TO */
	  pIdxInfo->estimatedCost = 1.0;
	  for (i = 0; i < pIdxInfo->nConstraint; i++)
	    {
		if (pIdxInfo->aConstraint[i].usable)
		  {
		      pIdxInfo->aConstraintUsage[i].argvIndex = i + 1;
		      pIdxInfo->aConstraintUsage[i].omit = 1;
		  }
	    }
	  err = 0;
      }
    if (err)
      {
	  /* illegal query */
	  pIdxInfo->idxNum = 0;
      }
    return SQLITE_OK;
}

static int
vnet_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    VirtualNetworkPtr p_vt = (VirtualNetworkPtr) pVTab;
    if (p_vt->graph)
	network_free (p_vt->graph);
    sqlite3_free (p_vt);
    return SQLITE_OK;
}

static int
vnet_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases vshp_disconnect() */
    return vnet_disconnect (pVTab);
}

static void
vnet_read_row (VirtualNetworkCursorPtr cursor)
{
/* trying to read a "row" from Dijkstra solution */
    if (cursor->solution->CurrentRow == NULL)
	cursor->eof = 1;
    else
	cursor->eof = 0;
    return;
}

static int
vnet_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    VirtualNetworkCursorPtr cursor =
	(VirtualNetworkCursorPtr)
	sqlite3_malloc (sizeof (VirtualNetworkCursor));
    if (cursor == NULL)
	return SQLITE_ERROR;
    cursor->pVtab = (VirtualNetworkPtr) pVTab;
    cursor->solution = alloc_solution ();
    cursor->eof = 0;
    *ppCursor = (sqlite3_vtab_cursor *) cursor;
    return SQLITE_OK;
}

static int
vnet_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    VirtualNetworkCursorPtr cursor = (VirtualNetworkCursorPtr) pCursor;
    delete_solution (cursor->solution);
    sqlite3_free (pCursor);
    return SQLITE_OK;
}

static int
vnet_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	     int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    int node_code = 0;
    VirtualNetworkCursorPtr cursor = (VirtualNetworkCursorPtr) pCursor;
    VirtualNetworkPtr net = (VirtualNetworkPtr) cursor->pVtab;
    node_code = net->graph->NodeCode;
    reset_solution (cursor->solution);
    cursor->eof = 1;
    if (idxNum == 1 && argc == 2)
      {
	  /* retrieving the Dijkstra From/To params */
	  if (node_code)
	    {
		/* Nodes are identified by TEXT Codes */
		if (sqlite3_value_type (argv[0]) == SQLITE_TEXT)
		    cursor->solution->From =
			find_node_by_code (net->graph,
					   (char *)
					   sqlite3_value_text (argv[0]));
		if (sqlite3_value_type (argv[1]) == SQLITE_TEXT)
		    cursor->solution->To =
			find_node_by_code (net->graph,
					   (char *)
					   sqlite3_value_text (argv[1]));
	    }
	  else
	    {
		/* Nodes are identified by INT Ids */
		if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
		    cursor->solution->From =
			find_node_by_id (net->graph,
					 sqlite3_value_int (argv[0]));
		if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
		    cursor->solution->To =
			find_node_by_id (net->graph,
					 sqlite3_value_int (argv[1]));
	    }
      }
    if (idxNum == 2 && argc == 2)
      {
	  /* retrieving the Dijkstra To/From params */
	  if (node_code)
	    {
		/* Nodes are identified by TEXT Codes */
		if (sqlite3_value_type (argv[0]) == SQLITE_TEXT)
		    cursor->solution->To =
			find_node_by_code (net->graph,
					   (char *)
					   sqlite3_value_text (argv[0]));
		if (sqlite3_value_type (argv[1]) == SQLITE_TEXT)
		    cursor->solution->From =
			find_node_by_code (net->graph,
					   (char *)
					   sqlite3_value_text (argv[1]));
	    }
	  else
	    {
		/* Nodes are identified by INT Ids */
		if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
		    cursor->solution->To =
			find_node_by_id (net->graph,
					 sqlite3_value_int (argv[0]));
		if (sqlite3_value_type (argv[1]) == SQLITE_INTEGER)
		    cursor->solution->From =
			find_node_by_id (net->graph,
					 sqlite3_value_int (argv[1]));
	    }
      }
    if (cursor->solution->From && cursor->solution->To)
      {
	  cursor->eof = 0;
	  dijkstra_solve (net->db, net->graph, cursor->solution);
	  return SQLITE_OK;
      }
    return SQLITE_OK;
}

static int
vnet_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching a next row from cursor */
    VirtualNetworkCursorPtr cursor = (VirtualNetworkCursorPtr) pCursor;
    if (cursor->solution->CurrentRowId == 0)
	cursor->solution->CurrentRow = cursor->solution->First;
    else
	cursor->solution->CurrentRow = cursor->solution->CurrentRow->Next;
    if (!(cursor->solution->CurrentRow))
      {
	  cursor->eof = 1;
	  return SQLITE_OK;
      }
    (cursor->solution->CurrentRowId)++;
    vnet_read_row (cursor);
    return SQLITE_OK;
}

static int
vnet_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    VirtualNetworkCursorPtr cursor = (VirtualNetworkCursorPtr) pCursor;
    return cursor->eof;
}

static int
vnet_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	     int column)
{
/* fetching value for the Nth column */
    RowSolutionPtr row;
    int node_code = 0;
    VirtualNetworkCursorPtr cursor = (VirtualNetworkCursorPtr) pCursor;
    VirtualNetworkPtr net = (VirtualNetworkPtr) cursor->pVtab;
    node_code = net->graph->NodeCode;
    if (cursor->solution->CurrentRow == 0)
      {
	  /* special case: this one is the solution summary */
	  if (column == 0)
	    {
		/* the ArcRowId column */
		sqlite3_result_null (pContext);
	    }
	  if (column == 1)
	    {
		/* the NodeFrom column */
		if (node_code)
		    sqlite3_result_text (pContext, cursor->solution->From->Code,
					 strlen (cursor->solution->From->Code),
					 SQLITE_STATIC);
		else
		    sqlite3_result_int (pContext, cursor->solution->From->Id);
	    }
	  if (column == 2)
	    {
		/* the NodeTo column */
		if (node_code)
		    sqlite3_result_text (pContext, cursor->solution->To->Code,
					 strlen (cursor->solution->To->Code),
					 SQLITE_STATIC);
		else
		    sqlite3_result_int (pContext, cursor->solution->To->Id);
	    }
	  if (column == 3)
	    {
		/* the Cost column */
		sqlite3_result_double (pContext, cursor->solution->TotalCost);
	    }
	  if (column == 4)
	    {
		/* the Geometry column */
		if (!(cursor->solution->Geometry))
		    sqlite3_result_null (pContext);
		else
		  {
		      /* builds the BLOB geometry to be returned */
		      int len;
		      unsigned char *p_result = NULL;
		      gaiaToSpatiaLiteBlobWkb (cursor->solution->Geometry,
					       &p_result, &len);
		      sqlite3_result_blob (pContext, p_result, len, free);
		  }
	    }
      }
    else
      {
	  /* ordinary case: this one is an Arc used by the solution */
	  row = cursor->solution->CurrentRow;
	  if (column == 0)
	    {
		/* the ArcRowId column */
		sqlite3_result_int (pContext, row->Arc->ArcRowid);
	    }
	  if (column == 1)
	    {
		/* the NodeFrom column */
		if (node_code)
		    sqlite3_result_text (pContext, row->Arc->NodeFrom->Code,
					 strlen (row->Arc->NodeFrom->Code),
					 SQLITE_STATIC);
		else
		    sqlite3_result_int (pContext, row->Arc->NodeFrom->Id);
	    }
	  if (column == 2)
	    {
		/* the NodeTo column */
		if (node_code)
		    sqlite3_result_text (pContext, row->Arc->NodeTo->Code,
					 strlen (row->Arc->NodeTo->Code),
					 SQLITE_STATIC);
		else
		    sqlite3_result_int (pContext, row->Arc->NodeTo->Id);
	    }
	  if (column == 3)
	    {
		/* the Cost column */
		sqlite3_result_double (pContext, row->Arc->Cost);
	    }
	  if (column == 4)
	    {
		/* the Geometry column */
		sqlite3_result_null (pContext);
	    }
      }
    return SQLITE_OK;
}

static int
vnet_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    VirtualNetworkCursorPtr cursor = (VirtualNetworkCursorPtr) pCursor;
    *pRowid = cursor->solution->CurrentRowId;
    return SQLITE_OK;
}

static int
vnet_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	     sqlite_int64 * pRowid)
{
/* generic update [INSERT / UPDATE / DELETE */
    return SQLITE_READONLY;
}

static int
vnet_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
vnet_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
vnet_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
vnet_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

int
sqlite3VirtualNetworkInit (sqlite3 * db)
{
    int rc = SQLITE_OK;
    my_net_module.iVersion = 1;
    my_net_module.xCreate = &vnet_create;
    my_net_module.xConnect = &vnet_connect;
    my_net_module.xBestIndex = &vnet_best_index;
    my_net_module.xDisconnect = &vnet_disconnect;
    my_net_module.xDestroy = &vnet_destroy;
    my_net_module.xOpen = &vnet_open;
    my_net_module.xClose = &vnet_close;
    my_net_module.xFilter = &vnet_filter;
    my_net_module.xNext = &vnet_next;
    my_net_module.xEof = &vnet_eof;
    my_net_module.xColumn = &vnet_column;
    my_net_module.xRowid = &vnet_rowid;
    my_net_module.xUpdate = &vnet_update;
    my_net_module.xBegin = &vnet_begin;
    my_net_module.xSync = &vnet_sync;
    my_net_module.xCommit = &vnet_commit;
    my_net_module.xRollback = &vnet_rollback;
    my_net_module.xFindFunction = NULL;
    sqlite3_create_module_v2 (db, "VirtualNetwork", &my_net_module, NULL, 0);
    return rc;
}

int
virtualnetwork_extension_init (sqlite3 * db)
{
    return sqlite3VirtualNetworkInit (db);
}
/**************** End file: virtualnetwork.c **********/


/**************** Begin file: virtualfdo.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <string.h> */
/* #include <limits.h> */
/* #include <spatialite/sqlite3.h> */
/* #include <spatialite/spatialite.h> */
/* #include <spatialite/gaiaaux.h> */
/* #include <spatialite/gaiageo.h> */

/* constants definining FDO-OGR Geometry formats */
#define FDO_OGR_NONE	0
#define FDO_OGR_WKT		1
#define FDO_OGR_WKB		2
#define FDO_OGR_FGF		3

#if defined(_WIN32) && !defined(__MINGW32__)
#define LONG64_MAX	_I64_MAX
#define LONG64_MIN	_I64_MIN
#else
#define LONG64_MAX	9223372036854775807LL
#define LONG64_MIN	(-LONG64_MAX + 1)
#endif

struct sqlite3_module my_fdo_module;

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
    const sqlite3_module *pModule;	/* ptr to sqlite module: USED INTERNALLY BY SQLITE */
    int nRef;			/* # references: USED INTERNALLY BY SQLITE */
    char *zErrMsg;		/* error message: USE INTERNALLY BY SQLITE */
    sqlite3 *db;		/* the sqlite db holding the virtual table */
    char *table;		/* the real-table name */
    int nColumns;		/* the # columns into the table */
    char **Column;		/* the name for each column */
    char **Type;		/* the type for each column */
    int *NotNull;		/* NotNull clause for each column */
    SqliteValuePtr *Value;	/* the current-row value for each column */
    int nGeometries;		/* # Geometry columns into the table */
    char **GeoColumn;		/* the name for each Geometry column */
    int *Srid;			/* the SRID for each Geometry column */
    int *GeoType;		/* the Type for each Geometry column */
    int *Format;		/* the Format for each Geometry column */
    int *CoordDimensions;	/* # Dimensions for each Geometry column */
} VirtualFDO;
typedef VirtualFDO *VirtualFDOPtr;

typedef struct VirtualFDOCursorStruct
{
/* extends the sqlite3_vtab_cursor struct */
    VirtualFDOPtr pVtab;	/* Virtual table of this cursor */
    sqlite3_int64 current_row;	/* the current row ID */
    int eof;			/* the EOF marker */
} VirtualFDOCursor;
typedef VirtualFDOCursor *VirtualFDOCursorPtr;

static SqliteValuePtr
value_alloc ()
{
/* allocates and initialites a Value multitype */
    SqliteValuePtr p = malloc (sizeof (SqliteValue));
    p->Type = SQLITE_NULL;
    p->Text = NULL;
    p->Blob = NULL;
    return p;
}

static void
value_free (SqliteValuePtr p)
{
/* freeing a Value multitype */
    if (!p)
	return;
    if (p->Text)
	free (p->Text);
    if (p->Blob)
	free (p->Blob);
    free (p);
}

static void
value_set_null (SqliteValuePtr p)
{
/* setting a NULL value to the multitype */
    if (!p)
	return;
    p->Type = SQLITE_NULL;
    if (p->Text)
	free (p->Text);
    if (p->Blob)
	free (p->Blob);
    p->Text = NULL;
    p->Blob = NULL;
}

static void
value_set_int (SqliteValuePtr p, sqlite3_int64 value)
{
/* setting an INT value to the multitype */
    if (!p)
	return;
    p->Type = SQLITE_INTEGER;
    if (p->Text)
	free (p->Text);
    if (p->Blob)
	free (p->Blob);
    p->Text = NULL;
    p->Blob = NULL;
    p->IntValue = value;
}

static void
value_set_double (SqliteValuePtr p, double value)
{
/* setting a DOUBLE value to the multitype */
    if (!p)
	return;
    p->Type = SQLITE_FLOAT;
    if (p->Text)
	free (p->Text);
    if (p->Blob)
	free (p->Blob);
    p->Text = NULL;
    p->Blob = NULL;
    p->DoubleValue = value;
}

static void
value_set_text (SqliteValuePtr p, const char *value, int size)
{
/* setting a TEXT value to the multitype */
    if (!p)
	return;
    p->Type = SQLITE_TEXT;
    if (p->Text)
	free (p->Text);
    if (p->Blob)
	free (p->Blob);
    p->Blob = NULL;
    p->Text = malloc (size);
    memcpy (p->Text, value, size);
    p->Size = size;
}

static void
value_set_blob (SqliteValuePtr p, const unsigned char *value, int size)
{
/* setting a BLOB value to the multitype */
    if (!p)
	return;
    p->Type = SQLITE_BLOB;
    if (p->Text)
	free (p->Text);
    if (p->Blob)
	free (p->Blob);
    p->Text = NULL;
    p->Blob = malloc (size);
    memcpy (p->Blob, value, size);
    p->Size = size;
}

static void
free_table (VirtualFDOPtr p_vt)
{
/* memory cleanup; freeing the virtual table struct */
    int i;
    if (!p_vt)
	return;
    if (p_vt->Column)
      {
	  for (i = 0; i < p_vt->nColumns; i++)
	    {
		if (*(p_vt->Column + i))
		    sqlite3_free (*(p_vt->Column + i));
	    }
	  sqlite3_free (p_vt->Column);
      }
    if (p_vt->Type)
      {
	  for (i = 0; i < p_vt->nColumns; i++)
	    {
		if (*(p_vt->Type + i))
		    sqlite3_free (*(p_vt->Type + i));
	    }
	  sqlite3_free (p_vt->Type);
      }
    if (p_vt->NotNull)
	sqlite3_free (p_vt->NotNull);
    if (p_vt->Value)
      {
	  for (i = 0; i < p_vt->nColumns; i++)
	    {
		if (*(p_vt->Value + i))
		    value_free (*(p_vt->Value + i));
	    }
	  sqlite3_free (p_vt->Value);
      }
    if (p_vt->GeoColumn)
      {
	  for (i = 0; i < p_vt->nGeometries; i++)
	    {
		if (*(p_vt->GeoColumn + i))
		    sqlite3_free (*(p_vt->GeoColumn + i));
	    }
	  sqlite3_free (p_vt->GeoColumn);
      }
    if (p_vt->Srid)
	sqlite3_free (p_vt->Srid);
    if (p_vt->GeoType)
	sqlite3_free (p_vt->GeoType);
    if (p_vt->Format)
	sqlite3_free (p_vt->Format);
    if (p_vt->CoordDimensions)
	sqlite3_free (p_vt->CoordDimensions);
    sqlite3_free (p_vt);
}

static int
vfdo_insert_row (VirtualFDOPtr p_vt, sqlite3_int64 * rowid, int argc,
		 sqlite3_value ** argv)
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
    char prefix[16];
    const char *text;
    const unsigned char *blob;
    char *text_wkt;
    unsigned char *blob_wkb;
    int size;
    char sql[4096];
    char buf[256];
    gaiaGeomCollPtr geom;
    sprintf (sql, "INSERT INTO \"%s\" ", p_vt->table);
    for (ic = 0; ic < p_vt->nColumns; ic++)
      {
	  if (ic == 0)
	      strcpy(prefix, "(");
	  else
	      strcpy(prefix, ", ");
	  sprintf (buf, "%s\"%s\"", prefix, *(p_vt->Column + ic));
	  strcat (sql, buf);
      }
    strcat (sql, ") VALUES ");
    for (ic = 0; ic < p_vt->nColumns; ic++)
      {
	  if (ic == 0)
	      strcpy(prefix, "(");
	  else
	      strcpy(prefix, ", ");
	  sprintf (buf, "%s?", prefix);
	  strcat (sql, buf);
      }
    strcat (sql, ")");
    ret = sqlite3_prepare_v2 (p_vt->db, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
	return SQLITE_ERROR;
    for (i = 2; i < argc; i++)
      {
	  geom_done = 0;
	  for (ig = 0; ig < p_vt->nGeometries; ig++)
	    {
		if (strcasecmp
		    (*(p_vt->Column + i - 2), *(p_vt->GeoColumn + ig)) == 0)
		  {
		      /* this one is a Geometry column */
		      if (sqlite3_value_type (argv[i]) == SQLITE_BLOB)
			{
			    blob = sqlite3_value_blob (argv[i]);
			    size = sqlite3_value_bytes (argv[i]);
			    geom = gaiaFromSpatiaLiteBlobWkb (blob, size);
			    if (geom)
			      {
				  if (geom->Srid != *(p_vt->Srid + ig))
				    {
					/* SRID constraint violation */
					geom_constraint_err = 1;
					goto error;
				    }
				  /* checking for TYPE constraint violation */
				  if (gaiaGeometryType (geom) !=
				      *(p_vt->GeoType + ig))
				    {
					/* Geometry TYPE constraint violation */
					geom_constraint_err = 1;
					goto error;
				    }
				  switch (*(p_vt->Format + ig))
				    {
				    case FDO_OGR_WKT:
					gaiaOutWkt (geom, &text_wkt);
					if (text_wkt)
					    sqlite3_bind_text (stmt, i - 1,
							       text_wkt,
							       strlen
							       (text_wkt),
							       free);
					else
					  {
					      err_geom = 1;
					      goto error;
					  }
					break;
				    case FDO_OGR_WKB:
					gaiaToWkb (geom, &blob_wkb, &size);
					if (blob_wkb)
					    sqlite3_bind_blob (stmt, i - 1,
							       blob_wkb, size,
							       free);
					else
					  {
					      err_geom = 1;
					      goto error;
					  }
					break;
				    case FDO_OGR_FGF:
					gaiaToFgf (geom, &blob_wkb, &size,
						   *(p_vt->CoordDimensions +
						     ig));
					if (blob_wkb)
					    sqlite3_bind_blob (stmt, i - 1,
							       blob_wkb, size,
							       free);
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
		      else if (sqlite3_value_type (argv[i]) == SQLITE_NULL)
			  sqlite3_bind_null (stmt, i - 1);
		      else
			{
			    err_geom = 1;
			    goto error;
			}
		      geom_done = 1;
		  }
	    }
	  if (geom_done)
	      continue;
	  switch (sqlite3_value_type (argv[i]))
	    {
	    case SQLITE_INTEGER:
		sqlite3_bind_int64 (stmt, i - 1, sqlite3_value_int64 (argv[i]));
		break;
	    case SQLITE_FLOAT:
		sqlite3_bind_double (stmt, i - 1,
				     sqlite3_value_double (argv[i]));
		break;
	    case SQLITE_TEXT:
		text = (char *) sqlite3_value_text (argv[i]);
		size = sqlite3_value_bytes (argv[i]);
		sqlite3_bind_text (stmt, i - 1, text, size, SQLITE_STATIC);
		break;
	    case SQLITE_BLOB:
		blob = sqlite3_value_blob (argv[i]);
		size = sqlite3_value_bytes (argv[i]);
		sqlite3_bind_blob (stmt, i - 1, blob, size, SQLITE_STATIC);
		break;
	    case SQLITE_NULL:
	    default:
		sqlite3_bind_null (stmt, i - 1);
		break;
	    };
      }
  error:
    if (err_geom || geom_constraint_err)
      {
	  sqlite3_finalize (stmt);
	  return SQLITE_CONSTRAINT;
      }
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  sqlite3_finalize (stmt);
	  return ret;
      }
    sqlite3_finalize (stmt);
    *rowid = sqlite3_last_insert_rowid (p_vt->db);
    return SQLITE_OK;
}

static int
vfdo_update_row (VirtualFDOPtr p_vt, sqlite3_int64 rowid, int argc,
		 sqlite3_value ** argv)
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
    char prefix[16];
    const char *text;
    const unsigned char *blob;
    char *text_wkt;
    unsigned char *blob_wkb;
    int size;
    char sql[4096];
    char buf[256];
    gaiaGeomCollPtr geom;
    sprintf (sql, "UPDATE \"%s\" SET", p_vt->table);
    for (ic = 0; ic < p_vt->nColumns; ic++)
      {
	  if (ic == 0)
	      strcpy(prefix, " ");
	  else
	      strcpy(prefix, ", ");
	  sprintf (buf, "%s\"%s\" = ?", prefix, *(p_vt->Column + ic));
	  strcat (sql, buf);
      }
#if defined(_WIN32) || defined(__MINGW32__)
/* CAVEAT: M$ rutime doesn't supports %lld for 64 bits */
    sprintf (buf, " WHERE ROWID = %I64d", rowid);
#else
    sprintf (buf, " WHERE ROWID = %lld", rowid);
#endif
    strcat (sql, buf);
    ret = sqlite3_prepare_v2 (p_vt->db, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
	return SQLITE_ERROR;
    for (i = 2; i < argc; i++)
      {
	  geom_done = 0;
	  for (ig = 0; ig < p_vt->nGeometries; ig++)
	    {
		if (strcasecmp
		    (*(p_vt->Column + i - 2), *(p_vt->GeoColumn + ig)) == 0)
		  {
		      /* this one is a Geometry column */
		      if (sqlite3_value_type (argv[i]) == SQLITE_BLOB)
			{
			    blob = sqlite3_value_blob (argv[i]);
			    size = sqlite3_value_bytes (argv[i]);
			    geom = gaiaFromSpatiaLiteBlobWkb (blob, size);
			    if (geom)
			      {
				  if (geom->Srid != *(p_vt->Srid + ig))
				    {
					/* SRID constraint violation */
					geom_constraint_err = 1;
					goto error;
				    }
				  /* checking for TYPE constraint violation */
				  if (gaiaGeometryType (geom) !=
				      *(p_vt->GeoType + ig))
				    {
					/* Geometry TYPE constraint violation */
					geom_constraint_err = 1;
					goto error;
				    }
				  switch (*(p_vt->Format + ig))
				    {
				    case FDO_OGR_WKT:
					gaiaOutWkt (geom, &text_wkt);
					if (text_wkt)
					    sqlite3_bind_text (stmt, i - 1,
							       text_wkt,
							       strlen
							       (text_wkt),
							       free);
					else
					  {
					      err_geom = 1;
					      goto error;
					  }
					break;
				    case FDO_OGR_WKB:
					gaiaToWkb (geom, &blob_wkb, &size);
					if (blob_wkb)
					    sqlite3_bind_blob (stmt, i - 1,
							       blob_wkb, size,
							       free);
					else
					  {
					      err_geom = 1;
					      goto error;
					  }
					break;
				    case FDO_OGR_FGF:
					gaiaToFgf (geom, &blob_wkb, &size,
						   *(p_vt->CoordDimensions +
						     ig));
					if (blob_wkb)
					    sqlite3_bind_blob (stmt, i - 1,
							       blob_wkb, size,
							       free);
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
		      else if (sqlite3_value_type (argv[i]) == SQLITE_NULL)
			  sqlite3_bind_null (stmt, i - 1);
		      else
			{
			    err_geom = 1;
			    goto error;
			}
		      geom_done = 1;
		  }
	    }
	  if (geom_done)
	      continue;
	  switch (sqlite3_value_type (argv[i]))
	    {
	    case SQLITE_INTEGER:
		sqlite3_bind_int64 (stmt, i - 1, sqlite3_value_int64 (argv[i]));
		break;
	    case SQLITE_FLOAT:
		sqlite3_bind_double (stmt, i - 1,
				     sqlite3_value_double (argv[i]));
		break;
	    case SQLITE_TEXT:
		text = (char *) sqlite3_value_text (argv[i]);
		size = sqlite3_value_bytes (argv[i]);
		sqlite3_bind_text (stmt, i - 1, text, size, SQLITE_STATIC);
		break;
	    case SQLITE_BLOB:
		blob = sqlite3_value_blob (argv[i]);
		size = sqlite3_value_bytes (argv[i]);
		sqlite3_bind_blob (stmt, i - 1, blob, size, SQLITE_STATIC);
		break;
	    case SQLITE_NULL:
	    default:
		sqlite3_bind_null (stmt, i - 1);
		break;
	    };
      }
  error:
    if (err_geom || geom_constraint_err)
      {
	  sqlite3_finalize (stmt);
	  return SQLITE_CONSTRAINT;
      }
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_DONE || ret == SQLITE_ROW)
	;
    else
      {
	  sqlite3_finalize (stmt);
	  return ret;
      }
    sqlite3_finalize (stmt);
    return SQLITE_OK;
}

static int
vfdo_delete_row (VirtualFDOPtr p_vt, sqlite3_int64 rowid)
{
/* trying to delete a row from FDO-OGR real-table */
    char sql[1024];
    int ret;
#if defined(_WIN32) || defined(__MINGW32__)
/* CAVEAT: M$ runtime doesn't supports %lld for 64 bits */
    sprintf (sql, "DELETE FROM \"%s\" WHERE ROWID = %I64d", p_vt->table, rowid);
#else
    sprintf (sql, "DELETE FROM \"%s\" WHERE ROWID = %lld", p_vt->table, rowid);
#endif
    ret = sqlite3_exec (p_vt->db, sql, NULL, NULL, NULL);
    return ret;
}

static void
vfdo_read_row (VirtualFDOCursorPtr cursor)
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
    strcpy (sql, "SELECT ROWID");
    for (ic = 0; ic < cursor->pVtab->nColumns; ic++)
      {
	  sprintf (buf, ",\"%s\"", *(cursor->pVtab->Column + ic));
	  strcat (sql, buf);
      }
    sprintf (buf,
#if defined(_WIN32) || defined (__MINGW32__)
/* CAVEAT: M$ runtime doesn't supports %lld for 64 bits */
	     " FROM \"%s\" WHERE ROWID >= %I64d",
#else
	     " FROM \"%s\" WHERE ROWID >= %lld",
#endif
	     cursor->pVtab->table, cursor->current_row);
    strcat (sql, buf);
    ret =
	sqlite3_prepare_v2 (cursor->pVtab->db, sql, strlen (sql), &stmt, NULL);
    if (ret != SQLITE_OK)
      {
	  /* an error occurred */
	  cursor->eof = 1;
	  return;
      }
    ret = sqlite3_step (stmt);
    if (ret == SQLITE_ROW)
      {
	  pk = sqlite3_column_int64 (stmt, 0);
	  for (ic = 0; ic < cursor->pVtab->nColumns; ic++)
	    {
		/* fetching column values */
		geom_done = 0;
		for (ig = 0; ig < cursor->pVtab->nGeometries; ig++)
		  {
		      if (strcasecmp
			  (*(cursor->pVtab->Column + ic),
			   *(cursor->pVtab->GeoColumn + ig)) == 0)
			{
			    /* this one is a Geometry column */
			    switch (*(cursor->pVtab->Format + ig))
			      {
			      case FDO_OGR_WKT:
				  if (sqlite3_column_type (stmt, ic + 1) ==
				      SQLITE_TEXT)
				    {
					/* trying to parse a WKT Geometry */
					wkt =
					    sqlite3_column_text (stmt, ic + 1);
					geom = gaiaParseWkt (wkt, -1);
					if (!geom)
					    value_set_null (*
							    (cursor->pVtab->
							     Value + ic));
					else
					  {
					      geom->Srid =
						  *(cursor->pVtab->Srid + ig);
					      gaiaToSpatiaLiteBlobWkb (geom,
								       &xblob,
								       &size);
					      if (xblob)
						  value_set_blob (*
								  (cursor->
								   pVtab->
								   Value + ic),
								  xblob, size);
					      else
						  value_set_null (*
								  (cursor->
								   pVtab->
								   Value + ic));
					      gaiaFreeGeomColl (geom);
					  }
				    }
				  else
				      value_set_null (*
						      (cursor->pVtab->Value +
						       ic));
				  break;
			      case FDO_OGR_WKB:
				  if (sqlite3_column_type (stmt, ic + 1) ==
				      SQLITE_BLOB)
				    {
					/* trying to parse a WKB Geometry */
					blob =
					    sqlite3_column_blob (stmt, ic + 1);
					size =
					    sqlite3_column_bytes (stmt, ic + 1);
					geom = gaiaFromWkb (blob, size);
					if (!geom)
					    value_set_null (*
							    (cursor->pVtab->
							     Value + ic));
					else
					  {
					      geom->Srid =
						  *(cursor->pVtab->Srid + ig);
					      gaiaToSpatiaLiteBlobWkb (geom,
								       &xblob,
								       &size);
					      if (xblob)
						  value_set_blob (*
								  (cursor->
								   pVtab->
								   Value + ic),
								  xblob, size);
					      else
						  value_set_null (*
								  (cursor->
								   pVtab->
								   Value + ic));
					      gaiaFreeGeomColl (geom);
					  }
				    }
				  else
				      value_set_null (*
						      (cursor->pVtab->Value +
						       ic));
				  break;
			      case FDO_OGR_FGF:
				  if (sqlite3_column_type (stmt, ic + 1) ==
				      SQLITE_BLOB)
				    {
					/* trying to parse an FGF Geometry */
					blob =
					    sqlite3_column_blob (stmt, ic + 1);
					size =
					    sqlite3_column_bytes (stmt, ic + 1);
					geom = gaiaFromFgf (blob, size);
					if (!geom)
					    value_set_null (*
							    (cursor->pVtab->
							     Value + ic));
					else
					  {
					      geom->Srid =
						  *(cursor->pVtab->Srid + ig);
					      gaiaToSpatiaLiteBlobWkb (geom,
								       &xblob,
								       &size);
					      if (xblob)
						  value_set_blob (*
								  (cursor->
								   pVtab->
								   Value + ic),
								  xblob, size);
					      else
						  value_set_null (*
								  (cursor->
								   pVtab->
								   Value + ic));
					      gaiaFreeGeomColl (geom);
					  }
				    }
				  else
				      value_set_null (*
						      (cursor->pVtab->Value +
						       ic));
				  break;
			      default:
				  value_set_null (*(cursor->pVtab->Value + ic));
				  break;
			      };
			    geom_done = 1;
			}
		  }
		if (geom_done)
		    continue;
		switch (sqlite3_column_type (stmt, ic + 1))
		  {
		  case SQLITE_INTEGER:
		      value_set_int (*(cursor->pVtab->Value + ic),
				     sqlite3_column_int64 (stmt, ic + 1));
		      break;
		  case SQLITE_FLOAT:
		      value_set_double (*(cursor->pVtab->Value + ic),
					sqlite3_column_double (stmt, ic + 1));
		      break;
		  case SQLITE_TEXT:
		      text = (char *) sqlite3_column_text (stmt, ic + 1);
		      size = sqlite3_column_bytes (stmt, ic + 1);
		      value_set_text (*(cursor->pVtab->Value + ic), text, size);
		      break;
		  case SQLITE_BLOB:
		      blob = sqlite3_column_blob (stmt, ic + 1);
		      size = sqlite3_column_bytes (stmt, ic + 1);
		      value_set_blob (*(cursor->pVtab->Value + ic), blob, size);
		      break;
		  case SQLITE_NULL:
		  default:
		      value_set_null (*(cursor->pVtab->Value + ic));
		      break;
		  };
	    }
      }
    else
      {
	  /* an error occurred */
	  sqlite3_finalize (stmt);
	  cursor->eof = 1;
	  return;
      }
    sqlite3_finalize (stmt);
    cursor->eof = 0;
    cursor->current_row = pk;
}

static int
vfdo_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	     sqlite3_vtab ** ppVTab, char **pzErr)
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
    char prefix[16];
    VirtualFDOPtr p_vt = NULL;
/* checking for table_name */
    if (argc == 4)
      {
	  vtable = argv[2];
	  table = argv[3];
      }
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualFDO module] CREATE VIRTUAL: illegal arg list {table_name}\n");
	  return SQLITE_ERROR;
      }
/* retrieving the base table columns */
    sprintf (sql, "PRAGMA table_info(\"%s\")", table);
    ret = sqlite3_get_table (db, sql, &results, &n_rows, &n_columns, NULL);
    if (ret != SQLITE_OK)
	goto illegal;
    if (n_rows >= 1)
      {
	  p_vt = (VirtualFDOPtr) sqlite3_malloc (sizeof (VirtualFDO));
	  if (!p_vt)
	      return SQLITE_NOMEM;
	  p_vt->db = db;
	  p_vt->nRef = 0;
	  p_vt->zErrMsg = NULL;
	  len = strlen (table);
	  p_vt->table = sqlite3_malloc (len + 1);
	  strcpy (p_vt->table, table);
	  p_vt->nColumns = n_rows;
	  p_vt->Column = sqlite3_malloc (sizeof (char *) * n_rows);
	  p_vt->Type = sqlite3_malloc (sizeof (char *) * n_rows);
	  p_vt->NotNull = sqlite3_malloc (sizeof (int) * n_rows);
	  p_vt->Value = sqlite3_malloc (sizeof (SqliteValuePtr) * n_rows);
	  for (i = 0; i < n_rows; i++)
	    {
		*(p_vt->Column + i) = NULL;
		*(p_vt->Type + i) = NULL;
		*(p_vt->NotNull + i) = -1;
		*(p_vt->Value + i) = value_alloc ();
	    }
	  p_vt->nGeometries = 0;
	  p_vt->GeoColumn = NULL;
	  p_vt->Srid = NULL;
	  p_vt->GeoType = NULL;
	  p_vt->Format = NULL;
	  p_vt->CoordDimensions = NULL;
	  for (i = 1; i <= n_rows; i++)
	    {
		col_name = results[(i * n_columns) + 1];
		col_type = results[(i * n_columns) + 2];
		if (atoi (results[(i * n_columns) + 3]) == 0)
		    not_null = 0;
		else
		    not_null = 1;
		len = strlen (col_name);
		*(p_vt->Column + (i - 1)) = sqlite3_malloc (len + 1);
		strcpy (*(p_vt->Column + (i - 1)), col_name);
		len = strlen (col_type);
		*(p_vt->Type + (i - 1)) = sqlite3_malloc (len + 1);
		strcpy (*(p_vt->Type + (i - 1)), col_type);
		*(p_vt->NotNull + (i - 1)) = not_null;
	    }
	  sqlite3_free_table (results);
      }
    else
	goto illegal;
/* retrieving the base table columns */
    strcpy (sql,
	    "SELECT f_geometry_column, geometry_type, srid, geometry_format, coord_dimension\n");
    strcat (sql, "FROM geometry_columns WHERE f_table_name LIKE '");
    strcat (sql, table);
    strcat (sql, "'");
    ret = sqlite3_get_table (db, sql, &results, &n_rows, &n_columns, NULL);
    if (ret != SQLITE_OK)
	goto illegal;
    if (n_rows >= 1)
      {
	  p_vt->nGeometries = n_rows;
	  p_vt->GeoColumn = sqlite3_malloc (sizeof (char *) * n_rows);
	  p_vt->Srid = sqlite3_malloc (sizeof (char *) * n_rows);
	  p_vt->GeoType = sqlite3_malloc (sizeof (int) * n_rows);
	  p_vt->Format = sqlite3_malloc (sizeof (int) * n_rows);
	  p_vt->CoordDimensions = sqlite3_malloc (sizeof (int) * n_rows);
	  for (i = 0; i < n_rows; i++)
	    {
		*(p_vt->GeoColumn + i) = NULL;
		*(p_vt->Srid + i) = -1;
		*(p_vt->GeoType + i) = -1;
		*(p_vt->Format + i) = FDO_OGR_NONE;
		*(p_vt->CoordDimensions + i) = GAIA_XY;
	    }
	  for (i = 1; i <= n_rows; i++)
	    {
		col_name = results[(i * n_columns) + 0];
		type = atoi (results[(i * n_columns) + 1]);
		srid = atoi (results[(i * n_columns) + 2]);
		format = results[(i * n_columns) + 3];
		coord_dimension = atoi (results[(i * n_columns) + 4]);
		len = strlen (col_name);
		*(p_vt->GeoColumn + (i - 1)) = sqlite3_malloc (len + 1);
		strcpy (*(p_vt->GeoColumn + (i - 1)), col_name);
		*(p_vt->GeoType + (i - 1)) = type;
		*(p_vt->Srid + (i - 1)) = srid;
		if (strcasecmp (format, "WKT") == 0)
		    *(p_vt->Format + (i - 1)) = FDO_OGR_WKT;
		if (strcasecmp (format, "WKB") == 0)
		    *(p_vt->Format + (i - 1)) = FDO_OGR_WKB;
		if (strcasecmp (format, "FGF") == 0)
		    *(p_vt->Format + (i - 1)) = FDO_OGR_FGF;
		if (coord_dimension == 3)
		    *(p_vt->CoordDimensions + (i - 1)) = GAIA_XY_Z;
		else if (coord_dimension == 4)
		    *(p_vt->CoordDimensions + (i - 1)) = GAIA_XY_Z_M;
		else
		    *(p_vt->CoordDimensions + (i - 1)) = GAIA_XY;
	    }
	  sqlite3_free_table (results);
      }
    else
	goto illegal;
/* preparing the COLUMNs for this VIRTUAL TABLE */
    strcpy (sql, "CREATE TABLE \"");
    strcat (sql, vtable);
    strcat (sql, "\" ");
    for (i = 0; i < p_vt->nColumns; i++)
      {
	  if (i == 0)
	      strcpy(prefix, "(");
	  else
	      strcpy(prefix, ", ");
	  sprintf (buf, "%s\"%s\" %s", prefix, *(p_vt->Column + i),
		   *(p_vt->Type + i));
	  if (*(p_vt->NotNull + i))
	      strcat (buf, " NOT NULL");
	  strcat (sql, buf);
      }
    strcat (sql, ")");
    if (sqlite3_declare_vtab (db, sql) != SQLITE_OK)
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualFDO module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
	       sql);
	  return SQLITE_ERROR;
      }
    *ppVTab = (sqlite3_vtab *) p_vt;
    return SQLITE_OK;
  illegal:
/* something is going the wrong way */
    if (p_vt)
	free_table (p_vt);
    *pzErr =
	sqlite3_mprintf
	("[VirtualFDO module] '%s' isn't a valid FDO-OGR Geometry table\n",
	 table);
    return SQLITE_ERROR;
}

static int
vfdo_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	      sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table to some shapefile - simply aliases vfdo_create() */
    return vfdo_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
vfdo_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIndex)
{
/* best index selection */
    return SQLITE_OK;
}

static int
vfdo_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    VirtualFDOPtr p_vt = (VirtualFDOPtr) pVTab;
    free_table (p_vt);
    return SQLITE_OK;
}

static int
vfdo_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases vfdo_disconnect() */
    return vfdo_disconnect (pVTab);
}

static int
vfdo_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    VirtualFDOCursorPtr cursor =
	(VirtualFDOCursorPtr) sqlite3_malloc (sizeof (VirtualFDOCursor));
    if (cursor == NULL)
	return SQLITE_ERROR;
    cursor->pVtab = (VirtualFDOPtr) pVTab;
    cursor->current_row = LONG64_MIN;
    cursor->eof = 0;
    *ppCursor = (sqlite3_vtab_cursor *) cursor;
    vfdo_read_row (cursor);
    return SQLITE_OK;
}

static int
vfdo_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    sqlite3_free (pCursor);
    return SQLITE_OK;
}

static int
vfdo_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	     int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    return SQLITE_OK;
}

static int
vfdo_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching next row from cursor */
    VirtualFDOCursorPtr cursor = (VirtualFDOCursorPtr) pCursor;
    (cursor->current_row)++;
    vfdo_read_row (cursor);
    return SQLITE_OK;
}

static int
vfdo_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    VirtualFDOCursorPtr cursor = (VirtualFDOCursorPtr) pCursor;
    return cursor->eof;
}

static int
vfdo_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	     int column)
{
/* fetching value for the Nth column */
    VirtualFDOCursorPtr cursor = (VirtualFDOCursorPtr) pCursor;
    SqliteValuePtr value;
    if (column >= 0 && column < cursor->pVtab->nColumns)
      {
	  value = *(cursor->pVtab->Value + column);
	  switch (value->Type)
	    {
	    case SQLITE_INTEGER:
		sqlite3_result_int64 (pContext, value->IntValue);
		break;
	    case SQLITE_FLOAT:
		sqlite3_result_double (pContext, value->DoubleValue);
		break;
	    case SQLITE_TEXT:
		sqlite3_result_text (pContext, value->Text, value->Size,
				     SQLITE_STATIC);
		break;
	    case SQLITE_BLOB:
		sqlite3_result_blob (pContext, value->Blob, value->Size,
				     SQLITE_STATIC);
		break;
	    default:
		sqlite3_result_null (pContext);
		break;
	    };
      }
    else
	sqlite3_result_null (pContext);
    return SQLITE_OK;
}

static int
vfdo_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    VirtualFDOCursorPtr cursor = (VirtualFDOCursorPtr) pCursor;
    *pRowid = cursor->current_row;
    return SQLITE_OK;
}

static int
vfdo_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	     sqlite_int64 * pRowid)
{
/* generic update [INSERT / UPDATE / DELETE */
    sqlite3_int64 rowid;
    int ret;
    VirtualFDOPtr p_vt = (VirtualFDOPtr) pVTab;
    if (argc == 1)
      {
	  /* performing a DELETE */
	  if (sqlite3_value_type (argv[0]) == SQLITE_INTEGER)
	    {
		rowid = sqlite3_value_int64 (argv[0]);
		ret = vfdo_delete_row (p_vt, rowid);
	    }
	  else
	      ret = SQLITE_MISMATCH;
      }
    else
      {
	  if (sqlite3_value_type (argv[0]) == SQLITE_NULL)
	    {
		/* performing an INSERT */
		ret = vfdo_insert_row (p_vt, &rowid, argc, argv);
		if (ret == SQLITE_OK)
		    *pRowid = rowid;
	    }
	  else
	    {
		/* performing an UPDATE */
		rowid = sqlite3_value_int64 (argv[0]);
		ret = vfdo_update_row (p_vt, rowid, argc, argv);
	    }
      }
    return ret;
}

static int
vfdo_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
vfdo_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
vfdo_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
vfdo_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

int
sqlite3VirtualFDOInit (sqlite3 * db)
{
    int rc = SQLITE_OK;
    my_fdo_module.iVersion = 1;
    my_fdo_module.xCreate = &vfdo_create;
    my_fdo_module.xConnect = &vfdo_connect;
    my_fdo_module.xBestIndex = &vfdo_best_index;
    my_fdo_module.xDisconnect = &vfdo_disconnect;
    my_fdo_module.xDestroy = &vfdo_destroy;
    my_fdo_module.xOpen = &vfdo_open;
    my_fdo_module.xClose = &vfdo_close;
    my_fdo_module.xFilter = &vfdo_filter;
    my_fdo_module.xNext = &vfdo_next;
    my_fdo_module.xEof = &vfdo_eof;
    my_fdo_module.xColumn = &vfdo_column;
    my_fdo_module.xRowid = &vfdo_rowid;
    my_fdo_module.xUpdate = &vfdo_update;
    my_fdo_module.xBegin = &vfdo_begin;
    my_fdo_module.xSync = &vfdo_sync;
    my_fdo_module.xCommit = &vfdo_commit;
    my_fdo_module.xRollback = &vfdo_rollback;
    my_fdo_module.xFindFunction = NULL;
    sqlite3_create_module_v2 (db, "VirtualFDO", &my_fdo_module, NULL, 0);
    return rc;
}

int
virtualfdo_extension_init (sqlite3 * db)
{
    return sqlite3VirtualFDOInit (db);
}
/**************** End file: virtualfdo.c **********/


/**************** Begin file: virtualtext.c **********/

/* #include <stdlib.h> */
/* #include <stdio.h> */
/* #include <string.h> */
/* #include <spatialite/sqlite3.h> */
/* #include <spatialite/spatialite.h> */
/* #include <spatialite/gaiaaux.h> */

#define VRTTXT_TEXT		1
#define VRTTXT_INTEGER	2
#define VRTTXT_DOUBLE	3

struct sqlite3_module my_text_module;

struct row_buffer
{
/* a complete row */
    int n_cells;		/* how many cells are stored into this line */
    char **cells;		/* the cells array */
    struct row_buffer *next;	/* pointer for linked list */
};

struct text_buffer
{
    int max_n_cells;		/* the maximun cell index */
    char **titles;		/* the column titles array */
    char *types;		/* the column types array */
    int n_rows;			/* the number of rows */
    struct row_buffer **rows;	/* the rows array */
    struct row_buffer *first;	/* pointers to build a linked list of rows */
    struct row_buffer *last;
};

typedef struct VirtualTextStruct
{
/* extends the sqlite3_vtab struct */
    const sqlite3_module *pModule;	/* ptr to sqlite module: USED INTERNALLY BY SQLITE */
    int nRef;			/* # references: USED INTERNALLY BY SQLITE */
    char *zErrMsg;		/* error message: USED INTERNALLY BY SQLITE */
    sqlite3 *db;		/* the sqlite db holding the virtual table */
    struct text_buffer *buffer;	/* the in-memory buffer storing text */
} VirtualText;
typedef VirtualText *VirtualTextPtr;

typedef struct VirtualTextCursortStruct
{
/* extends the sqlite3_vtab_cursor struct */
    VirtualTextPtr pVtab;	/* Virtual table of this cursor */
    long current_row;		/* the current row ID */
    int eof;			/* the EOF marker */
} VirtualTextCursor;
typedef VirtualTextCursor *VirtualTextCursorPtr;

static void
text_insert_row (struct text_buffer *text, char **fields, int max_cell)
{
/* inserting a row into the text buffer struct */
    int i;
    struct row_buffer *row = malloc (sizeof (struct row_buffer));
    row->n_cells = max_cell + 1;
    if (max_cell < 0)
	row->cells = NULL;
    else
      {
	  row->cells = malloc (sizeof (char *) * (max_cell + 1));
	  for (i = 0; i < row->n_cells; i++)
	    {
		/* setting cell values */
		*(row->cells + i) = *(fields + i);
	    }
      }
    row->next = NULL;
/* inserting the row into the linked list */
    if (!(text->first))
	text->first = row;
    if (text->last)
	text->last->next = row;
    text->last = row;
}

static struct text_buffer *
text_buffer_alloc ()
{
/* allocating and initializing the text buffer struct */
    struct text_buffer *text = malloc (sizeof (struct text_buffer));
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
text_buffer_free (struct text_buffer *text)
{
/* memory cleanup - freeing the text buffer */
    int i;
    struct row_buffer *row;
    if (!text)
	return;
    row = text->first;
    while (row)
      {
	  for (i = 0; i < row->n_cells; i++)
	    {
		if (*(row->cells + i))
		    free (*(row->cells + i));
	    }
	  row = row->next;
      }
    if (text->types)
	free (text->types);
    free (text);
}

static int
text_is_integer (char *value)
{
/* checking if this value can be an INTEGER */
    int invalids = 0;
    int digits = 0;
    int signs = 0;
    char last = '\0';
    char *p = value;
    while (*p != '\0')
      {
	  last = *p;
	  if (*p >= '0' && *p <= '9')
	      digits++;
	  else if (*p == '+' || *p == '-')
	      signs++;
	  else
	      signs++;
	  p++;
      }
    if (invalids)
	return 0;
    if (signs > 1)
	return 0;
    if (signs)
      {
	  if (*value == '+' || *value == '-' || last == '+' || last == '-')
	      ;
	  else
	      return 0;
      }
    return 1;
}

static int
text_is_double (char *value, char decimal_separator)
{
/* checking if this value can be a DOUBLE */
    int invalids = 0;
    int digits = 0;
    int signs = 0;
    int points = 0;
    char last = '\0';
    char *p = value;
    while (*p != '\0')
      {
	  last = *p;
	  if (*p >= '0' && *p <= '9')
	      digits++;
	  else if (*p == '+' || *p == '-')
	      points++;
	  else
	    {
		if (decimal_separator == ',')
		  {
		      if (*p == ',')
			  points++;
		      else
			  invalids++;
		  }
		else
		  {
		      if (*p == '.')
			  points++;
		      else
			  invalids++;
		  }
	    }
	  p++;
      }
    if (invalids)
	return 0;
    if (points > 1)
	return 0;
    if (signs > 1)
	return 0;
    if (signs)
      {
	  if (*value == '+' || *value == '-' || last == '+' || last == '-')
	      ;
	  else
	      return 0;
      }
    return 1;
}

static void
text_clean_integer (char *value)
{
/* cleaning an integer value */
    char last;
    char buffer[35536];
    int len = strlen (value);
    last = value[len - 1];
    if (last == '-' || last == '+')
      {
	  /* trailing sign; transforming into a leading sign */
	  *buffer = last;
	  strcpy (buffer + 1, value);
	  buffer[len - 1] = '\0';
	  strcpy (value, buffer);
      }
}

static void
text_clean_double (char *value)
{
/* cleaning an integer value */
    char *p;
    char last;
    char buffer[35536];
    int len = strlen (value);
    last = value[len - 1];
    if (last == '-' || last == '+')
      {
	  /* trailing sign; transforming into a leading sign */
	  *buffer = last;
	  strcpy (buffer + 1, value);
	  buffer[len - 1] = '\0';
	  strcpy (value, buffer);
      }
    p = value;
    while (*p != '\0')
      {
	  /* transforming COMMAs into POINTs */
	  if (*p == ',')
	      *p = '.';
	  p++;
      }
}

static int
text_clean_text (char **value, void *toUtf8)
{
/* cleaning a TEXT value and converting to UTF-8 */
    char *text = *value;
    char *utf8text;
    int err;
    int i;
    int oldlen = strlen (text);
    int newlen;
    for (i = oldlen - 1; i > 0; i++)
      {
	  /* cleaning up trailing spaces */
	  if (text[i] == ' ')
	      text[i] = '\0';
	  else
	      break;
      }
    utf8text = gaiaConvertToUTF8 (toUtf8, text, oldlen, &err);
    if (err)
	return 1;
    newlen = strlen (utf8text);
    if (newlen <= oldlen)
	strcpy (*value, utf8text);
    else
      {
	  free (*value);
	  *value = malloc (newlen + 1);
	  strcpy (*value, utf8text);
      }
    return 0;
}

static struct text_buffer *
text_parse (char *path, char *encoding, char first_line_titles,
	    char field_separator, char text_separator, char decimal_separator)
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
    for (fld = 0; fld < 4096; fld++)
      {
	  /* preparing an empty row */
	  fields[fld] = NULL;
      }
/* trying to open the text file */
    in = fopen (path, "rb");
    if (!in)
	return NULL;
    text = text_buffer_alloc ();
    fld = 0;
    while ((c = getc (in)) != EOF)
      {
	  /* parsing the file, one char at each time */
	  if (c == '\r' && !is_string)
	    {
		last = (char) c;
		continue;
	    }
	  if (c == field_separator && !is_string)
	    {
		/* insering a field into the fields tmp array */
		last = (char) c;
		*p = '\0';
		len = strlen (buffer);
		if (len)
		  {
		      fields[fld] = malloc (len + 1);
		      strcpy (fields[fld], buffer);
		  }
		fld++;
		p = buffer;
		*p = '\0';
		continue;
	    }
	  if (c == text_separator)
	    {
		/* found a text separator */
		if (is_string)
		  {
		      is_string = 0;
		      last = (char) c;
		  }
		else
		  {
		      if (last == text_separator)
			  *p++ = text_separator;
		      is_string = 1;
		  }
		continue;
	    }
	  last = (char) c;
	  if (c == '\n' && !is_string)
	    {
		/* inserting the row into the text buffer */
		*p = '\0';
		len = strlen (buffer);
		if (len)
		  {
		      fields[fld] = malloc (len + 1);
		      strcpy (fields[fld], buffer);
		  }
		fld++;
		p = buffer;
		*p = '\0';
		max_cell = -1;
		for (fld = 0; fld < 4096; fld++)
		  {
		      if (fields[fld])
			  max_cell = fld;
		  }
		text_insert_row (text, fields, max_cell);
		for (fld = 0; fld < 4096; fld++)
		  {
		      /* resetting an empty row */
		      fields[fld] = NULL;
		  }
		fld = 0;
		continue;
	    }
	  *p++ = (char) c;
      }
    fclose (in);
/* checking if the text file really seems to contain a table */
    nrows = 0;
    ncols = 0;
    errs = 0;
    row = text->first;
    while (row)
      {
	  if (first_line_titles && row == text->first)
	    {
		/* skipping first line */
		row = row->next;
		continue;
	    }
	  nrows++;
	  if (row->n_cells > ncols)
	      ncols = row->n_cells;
	  row = row->next;
      }
    if (nrows == 0 && ncols == 0)
      {
	  text_buffer_free (text);
	  return NULL;
      }
    text->n_rows = nrows;
/* going to check the column types */
    text->max_n_cells = ncols;
    text->types = malloc (sizeof (char) * text->max_n_cells);
    first_valid_row = malloc (sizeof (char) * text->max_n_cells);
    for (fld = 0; fld < text->max_n_cells; fld++)
      {
	  /* initally assuming any cell contains TEXT */
	  *(text->types + fld) = VRTTXT_TEXT;
	  *(first_valid_row + fld) = 1;
      }
    row = text->first;
    while (row)
      {
	  if (first_line_titles && row == text->first)
	    {
		/* skipping first line */
		row = row->next;
		continue;
	    }
	  for (fld = 0; fld < row->n_cells; fld++)
	    {
		if (*(row->cells + fld))
		  {
		      if (text_is_integer (*(row->cells + fld)))
			{
			    if (*(first_valid_row + fld))
			      {
				  *(text->types + fld) = VRTTXT_INTEGER;
				  *(first_valid_row + fld) = 0;
			      }
			}
		      else if (text_is_double
			       (*(row->cells + fld), decimal_separator))
			{
			    if (*(first_valid_row + fld))
			      {
				  *(text->types + fld) = VRTTXT_DOUBLE;
				  *(first_valid_row + fld) = 0;
			      }
			    else
			      {
				  /* promoting an INTEGER column to be of the DOUBLE type */
				  if (*(text->types + fld) == VRTTXT_INTEGER)
				      *(text->types + fld) = VRTTXT_DOUBLE;
			      }
			}
		      else
			{
			    /* this column is anyway of the TEXT type */
			    *(text->types + fld) = VRTTXT_TEXT;
			    if (*(first_valid_row + fld))
				*(first_valid_row + fld) = 0;
			}
		  }
	    }
	  row = row->next;
      }
    free (first_valid_row);
/* preparing the column names */
    text->titles = malloc (sizeof (char *) * text->max_n_cells);
    if (first_line_titles)
      {
	  for (fld = 0; fld < text->max_n_cells; fld++)
	    {
		if (fld >= text->first->n_cells)
		  {
		      /* this column name is NULL; setting a default name */
		      sprintf (title, "COL%03d", fld + 1);
		      len = strlen (title);
		      *(text->titles + fld) = malloc (len + 1);
		      strcpy (*(text->titles + fld), title);
		  }
		else
		  {
		      if (*(text->first->cells + fld))
			{
			    len = strlen (*(text->first->cells + fld));
			    *(text->titles + fld) = malloc (len + 1);
			    strcpy (*(text->titles + fld),
				    *(text->first->cells + fld));
			    name = *(text->titles + fld);
			    for (i = 0; i < len; i++)
			      {
				  /* masking any space in the column name */
				  if (*(name + i) == ' ')
				      *(name + i) = '_';
			      }
			}
		      else
			{
			    /* this column name is NULL; setting a default name */
			    sprintf (title, "COL%03d", fld + 1);
			    len = strlen (title);
			    *(text->titles + fld) = malloc (len + 1);
			    strcpy (*(text->titles + fld), title);
			}
		  }
	    }
      }
    else
      {
	  for (fld = 0; fld < text->max_n_cells; fld++)
	    {
		sprintf (title, "COL%03d", fld + 1);
		len = strlen (title);
		*(text->titles + fld) = malloc (len + 1);
		strcpy (*(text->titles + fld), title);
	    }
      }
/* cleaning cell values when needed */
    toUtf8 = gaiaCreateUTF8Converter (encoding);
    if (!toUtf8)
      {
	  text_buffer_free (text);
	  return NULL;
      }
    encoding_errors = 0;
    row = text->first;
    while (row)
      {
	  if (first_line_titles && row == text->first)
	    {
		/* skipping first line */
		row = row->next;
		continue;
	    }
	  for (fld = 0; fld < row->n_cells; fld++)
	    {
		if (*(row->cells + fld))
		  {
		      if (*(text->types + fld) == VRTTXT_INTEGER)
			  text_clean_integer (*(row->cells + fld));
		      else if (*(text->types + fld) == VRTTXT_DOUBLE)
			  text_clean_double (*(row->cells + fld));
		      else
			  encoding_errors +=
			      text_clean_text (row->cells + fld, toUtf8);
		  }
	    }
	  row = row->next;
      }
    gaiaFreeUTF8Converter (toUtf8);
    if (encoding_errors)
      {
	  text_buffer_free (text);
	  return NULL;
      }
/* ok, we can now go to prepare the rows array */
    text->rows = malloc (sizeof (struct row_buffer *) * text->n_rows);
    ir = 0;
    row = text->first;
    while (row)
      {
	  if (first_line_titles && row == text->first)
	    {
		/* skipping first line */
		row = row->next;
		continue;
	    }
	  *(text->rows + ir++) = row;
	  row = row->next;
      }
    return text;
}

static int
vtxt_create (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	     sqlite3_vtab ** ppVTab, char **pzErr)
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
    if (argc >= 5 && argc <= 9)
      {
	  vtable = argv[1];
	  pPath = argv[3];
	  len = strlen (pPath);
	  if ((*(pPath + 0) == '\'' || *(pPath + 0) == '"')
	      && (*(pPath + len - 1) == '\'' || *(pPath + len - 1) == '"'))
	    {
		/* the path is enclosed between quotes - we need to dequote it */
		strcpy (path, pPath + 1);
		len = strlen (path);
		*(path + len - 1) = '\0';
	    }
	  else
	      strcpy (path, pPath);
	  pEncoding = argv[4];
	  len = strlen (pEncoding);
	  if ((*(pEncoding + 0) == '\'' || *(pEncoding + 0) == '"')
	      && (*(pEncoding + len - 1) == '\''
		  || *(pEncoding + len - 1) == '"'))
	    {
		/* the charset-name is enclosed between quotes - we need to dequote it */
		strcpy (encoding, pEncoding + 1);
		len = strlen (encoding);
		*(encoding + len - 1) = '\0';
	    }
	  else
	      strcpy (encoding, pEncoding);
	  if (argc >= 6)
	    {
		if (*(argv[5]) == '0' || *(argv[5]) == 'n' || *(argv[5]) == 'N')
		    first_line_titles = 0;
	    }
	  if (argc >= 7)
	    {
		if (strcasecmp (argv[6], "COMMA") == 0)
		    decimal_separator = ',';
	    }
	  if (argc >= 8)
	    {
		if (strcasecmp (argv[7], "SINGLEQUOTE") == 0)
		    text_separator = '\'';
	    }
	  if (argc == 9)
	    {
		if (strlen (argv[8]) == 3)
		  {
		      if (strcasecmp (argv[8], "TAB") == 0)
			  field_separator = '\t';
		      if (*(argv[8] + 0) == '\'' && *(argv[8] + 2) == '\'')
			  field_separator = *(argv[8] + 1);
		  }
	    }
      }
    else
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualText module] CREATE VIRTUAL: illegal arg list\n"
	       "\t\t{ text_path, encoding [, first_row_as_titles [, [decimal_separator [, text_separator, [field_separator] ] ] ] }\n");
	  return SQLITE_ERROR;
      }
    p_vt = (VirtualTextPtr) sqlite3_malloc (sizeof (VirtualText));
    if (!p_vt)
	return SQLITE_NOMEM;
    p_vt->pModule = &my_text_module;
    p_vt->nRef = 0;
    p_vt->zErrMsg = NULL;
    p_vt->db = db;
    text =
	text_parse (path, encoding, first_line_titles, field_separator,
		    text_separator, decimal_separator);
    if (!text)
      {
	  /* something is going the wrong way; creating a stupid default table */
	  sprintf (sql, "CREATE TABLE %s (ROWNO INTEGER)", vtable);
	  if (sqlite3_declare_vtab (db, sql) != SQLITE_OK)
	    {
		*pzErr =
		    sqlite3_mprintf
		    ("[VirtualText module] cannot build a table from TEXT file\n");
		return SQLITE_ERROR;
	    }
	  p_vt->buffer = NULL;
	  *ppVTab = (sqlite3_vtab *) p_vt;
	  return SQLITE_OK;
      }
    p_vt->buffer = text;
/* preparing the COLUMNs for this VIRTUAL TABLE */
    sprintf (sql, "CREATE TABLE %s (ROWNO INTEGER", vtable);
    col_name = malloc (sizeof (char *) * text->max_n_cells);
    seed = 0;
    for (i = 0; i < text->max_n_cells; i++)
      {
	  strcat (sql, ", ");
	  if (gaiaIllegalSqlName (*(text->titles + i))
	      || gaiaIsReservedSqlName (*(text->titles + i))
	      || gaiaIsReservedSqliteName (*(text->titles + i)))
	      sprintf (dummyName, "\"%s\"", *(text->titles + i));
	  else
	      strcpy (dummyName, *(text->titles + i));
	  dup = 0;
	  for (idup = 0; idup < i; idup++)
	    {
		if (strcasecmp (dummyName, *(col_name + idup)) == 0)
		    dup = 1;
	    }
	  if (strcasecmp (dummyName, "PKUID") == 0)
	      dup = 1;
	  if (strcasecmp (dummyName, "Geometry") == 0)
	      dup = 1;
	  if (dup)
	      sprintf (dummyName, "COL_%d", seed++);
	  len = strlen (dummyName);
	  *(col_name + i) = malloc (len + 1);
	  strcpy (*(col_name + i), dummyName);
	  strcat (sql, dummyName);
	  if (*(text->types + i) == VRTTXT_INTEGER)
	      strcat (sql, " INTEGER");
	  else if (*(text->types + i) == VRTTXT_DOUBLE)
	      strcat (sql, " DOUBLE");
	  else
	      strcat (sql, " TEXT");
      }
    strcat (sql, ")");
    if (col_name)
      {
	  /* releasing memory allocation for column names */
	  for (i = 0; i < text->max_n_cells; i++)
	      free (*(col_name + i));
	  free (col_name);
      }
    if (sqlite3_declare_vtab (db, sql) != SQLITE_OK)
      {
	  *pzErr =
	      sqlite3_mprintf
	      ("[VirtualText module] CREATE VIRTUAL: invalid SQL statement \"%s\"",
	       sql);
	  return SQLITE_ERROR;
      }
    *ppVTab = (sqlite3_vtab *) p_vt;
    return SQLITE_OK;
}

static int
vtxt_connect (sqlite3 * db, void *pAux, int argc, const char *const *argv,
	      sqlite3_vtab ** ppVTab, char **pzErr)
{
/* connects the virtual table to some shapefile - simply aliases vshp_create() */
    return vtxt_create (db, pAux, argc, argv, ppVTab, pzErr);
}

static int
vtxt_best_index (sqlite3_vtab * pVTab, sqlite3_index_info * pIndex)
{
/* best index selection */
    return SQLITE_OK;
}

static int
vtxt_disconnect (sqlite3_vtab * pVTab)
{
/* disconnects the virtual table */
    VirtualTextPtr p_vt = (VirtualTextPtr) pVTab;
    if (p_vt->buffer)
	text_buffer_free (p_vt->buffer);
    sqlite3_free (p_vt);
    return SQLITE_OK;
}

static int
vtxt_destroy (sqlite3_vtab * pVTab)
{
/* destroys the virtual table - simply aliases vtxt_disconnect() */
    return vtxt_disconnect (pVTab);
}

static int
vtxt_open (sqlite3_vtab * pVTab, sqlite3_vtab_cursor ** ppCursor)
{
/* opening a new cursor */
    VirtualTextCursorPtr cursor =
	(VirtualTextCursorPtr) sqlite3_malloc (sizeof (VirtualTextCursor));
    if (cursor == NULL)
	return SQLITE_NOMEM;
    cursor->pVtab = (VirtualTextPtr) pVTab;
    cursor->current_row = 0;
    cursor->eof = 0;
    *ppCursor = (sqlite3_vtab_cursor *) cursor;
    if (!(cursor->pVtab->buffer))
	cursor->eof = 1;
    return SQLITE_OK;
}

static int
vtxt_close (sqlite3_vtab_cursor * pCursor)
{
/* closing the cursor */
    VirtualTextCursorPtr cursor = (VirtualTextCursorPtr) pCursor;
    sqlite3_free (cursor);
    return SQLITE_OK;
}

static int
vtxt_filter (sqlite3_vtab_cursor * pCursor, int idxNum, const char *idxStr,
	     int argc, sqlite3_value ** argv)
{
/* setting up a cursor filter */
    return SQLITE_OK;
}

static int
vtxt_next (sqlite3_vtab_cursor * pCursor)
{
/* fetching next row from cursor */
    VirtualTextCursorPtr cursor = (VirtualTextCursorPtr) pCursor;
    if (!(cursor->pVtab->buffer))
      {
	  cursor->eof = 1;
	  return SQLITE_OK;
      }
    cursor->current_row++;
    if (cursor->current_row >= cursor->pVtab->buffer->n_rows)
	cursor->eof = 1;
    return SQLITE_OK;
}

static int
vtxt_eof (sqlite3_vtab_cursor * pCursor)
{
/* cursor EOF */
    VirtualTextCursorPtr cursor = (VirtualTextCursorPtr) pCursor;
    return cursor->eof;
}

static int
vtxt_column (sqlite3_vtab_cursor * pCursor, sqlite3_context * pContext,
	     int column)
{
/* fetching value for the Nth column */
    struct row_buffer *row;
    int nCol = 1;
    int i;
    VirtualTextCursorPtr cursor = (VirtualTextCursorPtr) pCursor;
    struct text_buffer *text = cursor->pVtab->buffer;
    if (column == 0)
      {
	  /* the ROWNO column */
	  sqlite3_result_int (pContext, cursor->current_row + 1);
	  return SQLITE_OK;
      }
    row = *(text->rows + cursor->current_row);
    for (i = 0; i < text->max_n_cells; i++)
      {
	  if (nCol == column)
	    {
		if (i >= row->n_cells)
		    sqlite3_result_null (pContext);
		else
		  {
		      if (*(row->cells + i))
			{
			    if (*(text->types + i) == VRTTXT_INTEGER)
				sqlite3_result_int (pContext,
						    atoi (*(row->cells + i)));
			    else if (*(text->types + i) == VRTTXT_DOUBLE)
				sqlite3_result_double (pContext,
						       atof (*
							     (row->cells + i)));
			    else
				sqlite3_result_text (pContext,
						     *(row->cells + i),
						     strlen (*(row->cells + i)),
						     SQLITE_STATIC);
			}
		      else
			  sqlite3_result_null (pContext);
		  }
	    }
	  nCol++;
      }
    return SQLITE_OK;
}

static int
vtxt_rowid (sqlite3_vtab_cursor * pCursor, sqlite_int64 * pRowid)
{
/* fetching the ROWID */
    VirtualTextCursorPtr cursor = (VirtualTextCursorPtr) pCursor;
    *pRowid = cursor->current_row;
    return SQLITE_OK;
}

static int
vtxt_update (sqlite3_vtab * pVTab, int argc, sqlite3_value ** argv,
	     sqlite_int64 * pRowid)
{
/* generic update [INSERT / UPDATE / DELETE */
    return SQLITE_READONLY;
}

static int
vtxt_begin (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
vtxt_sync (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
vtxt_commit (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

static int
vtxt_rollback (sqlite3_vtab * pVTab)
{
/* BEGIN TRANSACTION */
    return SQLITE_OK;
}

int
sqlite3VirtualTextInit (sqlite3 * db)
{
    int rc = SQLITE_OK;
    my_text_module.iVersion = 1;
    my_text_module.xCreate = &vtxt_create;
    my_text_module.xConnect = &vtxt_connect;
    my_text_module.xBestIndex = &vtxt_best_index;
    my_text_module.xDisconnect = &vtxt_disconnect;
    my_text_module.xDestroy = &vtxt_destroy;
    my_text_module.xOpen = &vtxt_open;
    my_text_module.xClose = &vtxt_close;
    my_text_module.xFilter = &vtxt_filter;
    my_text_module.xNext = &vtxt_next;
    my_text_module.xEof = &vtxt_eof;
    my_text_module.xColumn = &vtxt_column;
    my_text_module.xRowid = &vtxt_rowid;
    my_text_module.xUpdate = &vtxt_update;
    my_text_module.xBegin = &vtxt_begin;
    my_text_module.xSync = &vtxt_sync;
    my_text_module.xCommit = &vtxt_commit;
    my_text_module.xRollback = &vtxt_rollback;
    my_text_module.xFindFunction = NULL;
    sqlite3_create_module_v2 (db, "VirtualText", &my_text_module, NULL, 0);
    return rc;
}

int
virtualtext_extension_init (sqlite3 * db)
{
    return sqlite3VirtualTextInit (db);
}
/**************** End file: virtualtext.c **********/


/**************** Begin file: version.c **********/

/* #include <spatialite/sqlite3.h> */
/* #include <spatialite.h> */

const char spatialiteversion[] = "2.3.0";

SPATIALITE_DECLARE const char *
spatialite_version (void)
{
    return spatialiteversion;
}
/**************** End file: version.c **********/

