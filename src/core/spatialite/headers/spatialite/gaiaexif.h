/* 
 gaiaexif.h -- Gaia common EXIF Metadata reading functions
  
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
#define GAIA_TIFF_BLOB		9
#define GAIA_WAVELET_BLOB	10

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
