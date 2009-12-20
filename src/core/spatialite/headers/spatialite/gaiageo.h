/* 
 gaiageo.h -- Gaia common support for geometries
  
 version 2.4, 2009 September 17

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

/* stdio.h included for FILE objects. */
#include <stdio.h>

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
#define GAIA_UNKNOWN			0
#define GAIA_POINT			1
#define GAIA_LINESTRING			2
#define GAIA_POLYGON			3
#define GAIA_MULTIPOINT			4
#define GAIA_MULTILINESTRING		5
#define GAIA_MULTIPOLYGON		6
#define GAIA_GEOMETRYCOLLECTION		7
#define GAIA_POINTZ			1001
#define GAIA_LINESTRINGZ		1002
#define GAIA_POLYGONZ			1003
#define GAIA_MULTIPOINTZ		1004
#define GAIA_MULTILINESTRINGZ		1005
#define GAIA_MULTIPOLYGONZ		1006
#define GAIA_GEOMETRYCOLLECTIONZ	1007
#define GAIA_POINTM			2001
#define GAIA_LINESTRINGM		2002
#define GAIA_POLYGONM			2003
#define GAIA_MULTIPOINTM		2004
#define GAIA_MULTILINESTRINGM		2005
#define GAIA_MULTIPOLYGONM		2006
#define GAIA_GEOMETRYCOLLECTIONM	2007
#define GAIA_POINTZM			3001
#define GAIA_LINESTRINGZM		3002
#define GAIA_POLYGONZM			3003
#define GAIA_MULTIPOINTZM		3004
#define GAIA_MULTILINESTRINGZM		3005
#define GAIA_MULTIPOLYGONZM		3006
#define GAIA_GEOMETRYCOLLECTIONZM	3007

/* constants that defines Compressed GEOMETRY CLASSes */
#define GAIA_COMPRESSED_LINESTRING		1000002
#define GAIA_COMPRESSED_POLYGON			1000003
#define GAIA_COMPRESSED_LINESTRINGZ		1001002
#define GAIA_COMPRESSED_POLYGONZ		1001003
#define GAIA_COMPRESSED_LINESTRINGM		1002002
#define GAIA_COMPRESSED_POLYGONM		1002003
#define GAIA_COMPRESSED_LINESTRINGZM		1003002
#define GAIA_COMPRESSED_POLYGONZM		1003003

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
#define GAIA_FILTER_MBR_WITHIN		74
#define GAIA_FILTER_MBR_CONTAINS	77
#define GAIA_FILTER_MBR_INTERSECTS	79
#define GAIA_FILTER_MBR_DECLARE		89

/* constants defining SVG default values */
#define GAIA_SVG_DEFAULT_RELATIVE 	0
#define GAIA_SVG_DEFAULT_PRECISION	6
#define GAIA_SVG_DEFAULT_MAX_PRECISION 15

/* constants used for VirtualNetwork */
#define GAIA_NET_START		0x67
#define GAIA_NET64_START	0x68
#define GAIA_NET_END		0x87
#define GAIA_NET_HEADER		0xc0
#define GAIA_NET_CODE		0xa6
#define GAIA_NET_ID		0xb5
#define GAIA_NET_NODE		0xde
#define GAIA_NET_ARC		0x54
#define GAIA_NET_TABLE		0xa0
#define GAIA_NET_FROM		0xa1
#define GAIA_NET_TO		0xa2
#define GAIA_NET_GEOM		0xa3
#define GAIA_NET_NAME		0xa4
#define GAIA_NET_BLOCK		0xed

/* constants used for Coordinate Dimensions */
#define GAIA_XY		0x00
#define GAIA_XY_Z	0x01
#define GAIA_XY_M	0x02
#define GAIA_XY_Z_M	0x03

/* constants used for length unit conversion */
#define GAIA_KM		0
#define GAIA_M		1
#define GAIA_DM		2
#define GAIA_CM		3
#define GAIA_MM		4
#define GAIA_KMI	5
#define GAIA_IN		6
#define GAIA_FT		7
#define GAIA_YD		8
#define GAIA_MI		9
#define GAIA_FATH	10
#define GAIA_CH		11
#define GAIA_LINK	12
#define GAIA_US_IN	13
#define GAIA_US_FT	14
#define GAIA_US_YD	15
#define GAIA_US_CH	16
#define GAIA_US_MI	17
#define GAIA_IND_YD	18
#define GAIA_IND_FT	19
#define GAIA_IND_CH	20
#define GAIA_MIN_UNIT	GAIA_KM
#define GAIA_MAX_UNIT	GAIA_IND_CH

/* constants used for SHAPES */
#define GAIA_SHP_NULL		0
#define GAIA_SHP_POINT		1
#define GAIA_SHP_POLYLINE	3
#define GAIA_SHP_POLYGON	5
#define GAIA_SHP_MULTIPOINT	8
#define GAIA_SHP_POINTZ		11
#define GAIA_SHP_POLYLINEZ	13
#define GAIA_SHP_POLYGONZ	15
#define GAIA_SHP_MULTIPOINTZ	18
#define GAIA_SHP_POINTM		21
#define GAIA_SHP_POLYLINEM	23
#define GAIA_SHP_POLYGONM	25
#define GAIA_SHP_MULTIPOINTM	28

/* macros */
#define gaiaGetPoint(xy,v,x,y)	\
				{*x = xy[(v) * 2]; \
				 *y = xy[(v) * 2 + 1];}

#define gaiaSetPoint(xy,v,x,y)	\
				{xy[(v) * 2] = x; \
				 xy[(v) * 2 + 1] = y;}

#define gaiaGetPointXYZ(xyz,v,x,y,z)	\
				{*x = xyz[(v) * 3]; \
				 *y = xyz[(v) * 3 + 1]; \
				 *z = xyz[(v) * 3 + 2];}

#define gaiaSetPointXYZ(xyz,v,x,y,z)	\
				{xyz[(v) * 3] = x; \
				 xyz[(v) * 3 + 1] = y; \
				 xyz[(v) * 3 + 2] = z;}

#define gaiaGetPointXYM(xym,v,x,y,m)	\
				{*x = xym[(v) * 3]; \
				 *y = xym[(v) * 3 + 1]; \
				 *m = xym[(v) * 3 + 2];}

#define gaiaSetPointXYM(xym,v,x,y,m)	\
				{xym[(v) * 3] = x; \
				 xym[(v) * 3 + 1] = y; \
				 xym[(v) * 3 + 2] = m;}

#define gaiaGetPointXYZM(xyzm,v,x,y,z,m)	\
				{*x = xyzm[(v) * 4]; \
				 *y = xyzm[(v) * 4 + 1]; \
				 *z = xyzm[(v) * 4 + 2]; \
				 *m = xyzm[(v) * 4 + 3];}

#define gaiaSetPointXYZM(xyzm,v,x,y,z,m)	\
				{xyzm[(v) * 4] = x; \
				 xyzm[(v) * 4 + 1] = y; \
				 xyzm[(v) * 4 + 2] = z; \
				 xyzm[(v) * 4 + 3] = m;}

    typedef struct gaiaPointStruct
    {
/* an OpenGis POINT */
	double X;		/* X,Y coordinates */
	double Y;
	double Z;		/* Z coordinate */
	double M;		/* M measure */
	int DimensionModel;	/* (x,y), (x,y,z), (x,y,m) or (x,y,z,m) */
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
	int DimensionModel;	/* (x,y), (x,y,z), (x,y,m) or (x,y,z,m) */
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
	int DimensionModel;	/* (x,y), (x,y,z), (x,y,m) or (x,y,z,m) */
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
	int DimensionModel;	/* (x,y), (x,y,z), (x,y,m) or (x,y,z,m) */
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
	int DimensionModel;	/* (x,y), (x,y,z), (x,y,m) or (x,y,z,m) */
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
	unsigned char Length;	/* field total length [in bytes] */
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
	int EffectiveType;	/* the effective Geometry-type, as determined by gaiaShpAnalyze() */
	int EffectiveDims;	/* the effective Dimensions [XY, XYZ, XYM, XYZM], as determined by gaiaShpAnalyze() */
    } gaiaShapefile;
    typedef gaiaShapefile *gaiaShapefilePtr;

/* function prototipes */

    GAIAGEO_DECLARE int gaiaEndianArch (void);
    GAIAGEO_DECLARE short gaiaImport16 (const unsigned char *p,
					int little_endian,
					int little_endian_arch);
    GAIAGEO_DECLARE int gaiaImport32 (const unsigned char *p, int little_endian,
				      int little_endian_arch);
    GAIAGEO_DECLARE float gaiaImportF32 (const unsigned char *p,
					 int little_endian,
					 int little_endian_arch);
    GAIAGEO_DECLARE double gaiaImport64 (const unsigned char *p,
					 int little_endian,
					 int little_endian_arch);
    GAIAGEO_DECLARE sqlite3_int64 gaiaImportI64 (const unsigned char *p,
						 int little_endian,
						 int little_endian_arch);
    GAIAGEO_DECLARE void gaiaExport16 (unsigned char *p, short value,
				       int little_endian,
				       int little_endian_arch);
    GAIAGEO_DECLARE void gaiaExport32 (unsigned char *p, int value,
				       int little_endian,
				       int little_endian_arch);
    GAIAGEO_DECLARE void gaiaExportF32 (unsigned char *p, float value,
					int little_endian,
					int little_endian_arch);
    GAIAGEO_DECLARE void gaiaExport64 (unsigned char *p, double value,
				       int little_endian,
				       int little_endian_arch);
    GAIAGEO_DECLARE void gaiaExportI64 (unsigned char *p, sqlite3_int64 value,
					int little_endian,
					int little_endian_arch);
    GAIAGEO_DECLARE gaiaPointPtr gaiaAllocPoint (double x, double y);
    GAIAGEO_DECLARE gaiaPointPtr gaiaAllocPointXYZ (double x, double y,
						    double z);
    GAIAGEO_DECLARE gaiaPointPtr gaiaAllocPointXYM (double x, double y,
						    double m);
    GAIAGEO_DECLARE gaiaPointPtr gaiaAllocPointXYZM (double x, double y,
						     double z, double m);
    GAIAGEO_DECLARE void gaiaFreePoint (gaiaPointPtr ptr);
    GAIAGEO_DECLARE gaiaLinestringPtr gaiaAllocLinestring (int vert);
    GAIAGEO_DECLARE gaiaLinestringPtr gaiaAllocLinestringXYZ (int vert);
    GAIAGEO_DECLARE gaiaLinestringPtr gaiaAllocLinestringXYM (int vert);
    GAIAGEO_DECLARE gaiaLinestringPtr gaiaAllocLinestringXYZM (int vert);
    GAIAGEO_DECLARE void gaiaFreeLinestring (gaiaLinestringPtr ptr);
    GAIAGEO_DECLARE void gaiaCopyLinestringCoords (gaiaLinestringPtr dst,
						   gaiaLinestringPtr src);
    GAIAGEO_DECLARE gaiaRingPtr gaiaAllocRing (int vert);
    GAIAGEO_DECLARE gaiaRingPtr gaiaAllocRingXYZ (int vert);
    GAIAGEO_DECLARE gaiaRingPtr gaiaAllocRingXYM (int vert);
    GAIAGEO_DECLARE gaiaRingPtr gaiaAllocRingXYZM (int vert);
    GAIAGEO_DECLARE void gaiaFreeRing (gaiaRingPtr ptr);
    GAIAGEO_DECLARE void gaiaCopyRingCoords (gaiaRingPtr dst, gaiaRingPtr src);
    GAIAGEO_DECLARE gaiaPolygonPtr gaiaAllocPolygon (int vert, int excl);
    GAIAGEO_DECLARE gaiaPolygonPtr gaiaAllocPolygonXYZ (int vert, int excl);
    GAIAGEO_DECLARE gaiaPolygonPtr gaiaAllocPolygonXYM (int vert, int excl);
    GAIAGEO_DECLARE gaiaPolygonPtr gaiaAllocPolygonXYZM (int vert, int excl);
    GAIAGEO_DECLARE gaiaPolygonPtr gaiaCreatePolygon (gaiaRingPtr ring);
    GAIAGEO_DECLARE void gaiaFreePolygon (gaiaPolygonPtr p);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaAllocGeomColl (void);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaAllocGeomCollXYZ (void);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaAllocGeomCollXYM (void);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaAllocGeomCollXYZM (void);
    GAIAGEO_DECLARE void gaiaFreeGeomColl (gaiaGeomCollPtr p);
    GAIAGEO_DECLARE void gaiaAddPointToGeomColl (gaiaGeomCollPtr p, double x,
						 double y);
    GAIAGEO_DECLARE void gaiaAddPointToGeomCollXYZ (gaiaGeomCollPtr p, double x,
						    double y, double z);
    GAIAGEO_DECLARE void gaiaAddPointToGeomCollXYM (gaiaGeomCollPtr p, double x,
						    double y, double m);
    GAIAGEO_DECLARE void gaiaAddPointToGeomCollXYZM (gaiaGeomCollPtr p,
						     double x, double y,
						     double z, double m);
    GAIAGEO_DECLARE void gaiaMbrLinestring (gaiaLinestringPtr line);
    GAIAGEO_DECLARE void gaiaMbrRing (gaiaRingPtr rng);
    GAIAGEO_DECLARE void gaiaMbrPolygon (gaiaPolygonPtr polyg);
    GAIAGEO_DECLARE void gaiaMbrGeometry (gaiaGeomCollPtr geom);
    GAIAGEO_DECLARE void gaiaZRangeLinestring (gaiaLinestringPtr line,
					       double *min, double *max);
    GAIAGEO_DECLARE void gaiaZRangeRing (gaiaRingPtr rng, double *min,
					 double *max);
    GAIAGEO_DECLARE void gaiaZRangePolygon (gaiaPolygonPtr polyg, double *min,
					    double *max);
    GAIAGEO_DECLARE void gaiaZRangeGeometry (gaiaGeomCollPtr geom, double *min,
					     double *max);
    GAIAGEO_DECLARE void gaiaMRangeLinestring (gaiaLinestringPtr line,
					       double *min, double *max);
    GAIAGEO_DECLARE void gaiaMRangeRing (gaiaRingPtr rng, double *min,
					 double *max);
    GAIAGEO_DECLARE void gaiaMRangePolygon (gaiaPolygonPtr polyg, double *min,
					    double *max);
    GAIAGEO_DECLARE void gaiaMRangeGeometry (gaiaGeomCollPtr geom, double *min,
					     double *max);
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
    GAIAGEO_DECLARE double gaiaMeasureLength (int dims, double *coords,
					      int vert);
    GAIAGEO_DECLARE double gaiaMeasureArea (gaiaRingPtr ring);
    GAIAGEO_DECLARE void gaiaRingCentroid (gaiaRingPtr ring, double *rx,
					   double *ry);
    GAIAGEO_DECLARE void gaiaClockwise (gaiaRingPtr p);
    GAIAGEO_DECLARE int gaiaIsPointOnRingSurface (gaiaRingPtr ring, double pt_x,
						  double pt_y);
    GAIAGEO_DECLARE double gaiaMinDistance (double x0, double y0,
					    int dims, double *coords,
					    int n_vert);
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
    GAIAGEO_DECLARE void gaiaToCompressedBlobWkb (gaiaGeomCollPtr geom,
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
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaCastGeomCollToXY (gaiaGeomCollPtr geom);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaCastGeomCollToXYZ (gaiaGeomCollPtr
							   geom);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaCastGeomCollToXYM (gaiaGeomCollPtr
							   geom);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaCastGeomCollToXYZM (gaiaGeomCollPtr
							    geom);
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
    GAIAGEO_DECLARE void gaiaFree (void *ptr);
    GAIAGEO_DECLARE int gaiaEllipseParams (const char *name, double *a,
					   double *b, double *rf);
    GAIAGEO_DECLARE double gaiaGreatCircleDistance (double a, double b,
						    double lat1, double lon1,
						    double lat2, double lon2);
    GAIAGEO_DECLARE double gaiaGeodesicDistance (double a, double b, double rf,
						 double lat1, double lon1,
						 double lat2, double lon2);
    GAIAGEO_DECLARE double gaiaGreatCircleTotalLength (double a, double b,
						       int dims, double *coords,
						       int vert);
    GAIAGEO_DECLARE double gaiaGeodesicTotalLength (double a, double b,
						    double rf, int dims,
						    double *coords, int vert);
    GAIAGEO_DECLARE int gaiaConvertLength (double value, int unit_from,
					   int unit_to, double *cvt);

#ifndef OMIT_PROJ		/* including PROJ.4 */

    GAIAGEO_DECLARE double gaiaRadsToDegs (double rads);
    GAIAGEO_DECLARE double gaiaDegsToRads (double degs);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaTransform (gaiaGeomCollPtr org,
						   char *proj_from,
						   char *proj_to);

#endif				/* end including PROJ.4 */

#ifndef OMIT_GEOS		/* including GEOS */

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
    GAIAGEO_DECLARE void *gaiaToGeos (const gaiaGeomCollPtr gaia);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaFromGeos_XY (const void *geos);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaFromGeos_XYZ (const void *geos);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaFromGeos_XYM (const void *geos);
    GAIAGEO_DECLARE gaiaGeomCollPtr gaiaFromGeos_XYZM (const void *geos);

#endif				/* end including GEOS */

#ifdef __cplusplus
}
#endif

#endif				/* _GAIAGEO_H */
