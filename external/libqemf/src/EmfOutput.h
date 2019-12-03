/*
  Copyright 2008 Brad Hards  <bradh@frogmouth.net>
  Copyright 2009 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#ifndef EMFOUTPUT_H
#define EMFOUTPUT_H

#include <QList>
#include <QPainter>
#include <QRect> // also provides QSize, QPoint
#include <QString>
#include <QVariant>

#include "EmfEnums.h"
#include "EmfHeader.h"
#include "EmfRecords.h"

/**
   \file

   Primary definitions for EMF output strategies
*/

/**
   Namespace for Enhanced Metafile (EMF) classes
*/
namespace QEmf
{

/**
	Abstract output strategy for EMF Parser
*/
class AbstractOutput
{
public:
	AbstractOutput() {};
	virtual ~AbstractOutput() {};

	bool load(const QString&);
	bool load(const QByteArray &);

	/**
	   Initialisation routine

	   \param header the EMF Header record
	*/
	virtual void init( const Header *header ) = 0;

	/**
	   Cleanup routine

	   This function is called when the painting is done.  Any
	   initializations that are done in init() can be undone here if
	   necessary.

	   \param header the EMF Header record
	*/
	virtual void cleanup( const Header *header ) = 0;

	/**
	   Close-out routine
	*/
	virtual void eof() = 0;

	/**
	   Handler for the EMR_SETPIXELV record type

	   This fills a specified pixel with a particular color

	   \param point the point to fill
	   \param red the red component of the color
	   \param green the green component of the color
	   \param blue the blue component of the color
	   \param reserved reserved component of the color
	*/
	virtual void setPixelV( QPoint &point, quint8 red, quint8 green, quint8 blue, quint8 reserved ) = 0;

	/**
	   Handler for the EMR_CREATEPEN record type

	   This creates a pen object (at position ihPen) using the specified parameters.

	   \param ihPen the internal handle for the pen to be created
	   \param penStyle the pen configuration (style, type) - see the PenStyle enumeration
	   \param x the width of the pen
	   \param y reserved value - ignore this
	   \param red the red component of the pen color
	   \param green the green component of the pen color
	   \param blue the blue component of the pen color
	   \param reserved reserved value - ignore this
	*/
	virtual void createPen( quint32 ihPen, quint32 penStyle, quint32 x, quint32 y,
				quint8 red, quint8 green, quint8 blue, quint32 brushStyle, quint8 reserved ) = 0;

	/**
	   Handler for the EMR_CREATEBRUSHINDIRECT record type
	*/
	virtual void createBrushIndirect( quint32 ihBrush, quint32 BrushStyle, quint8 red,
					  quint8 green, quint8 blue, quint8 reserved,
					  quint32 BrushHatch ) = 0;

	virtual void createMonoBrush( quint32 ihBrush, Bitmap *bitmap ) = 0;
	virtual void createDibPatternBrushPT( quint32 ihBrush, Bitmap *bitmap ) = 0;

	/**
	   Handler for the EMR_SETMAPMODE record type.

	   From [MS-EMF]:\n
	   <em>The EMR_SETMAPMODE record specifies the mapping mode of the
	   playback device context. The mapping mode specifies the unit of
	   measure used to transform page-space units into device-space
	   units, and also specifies the orientation of the device's x and
	   y axes.</em>

	   The valid mapping modes are:
	   - 0x01 (MM_TEXT): Each logical unit is mapped to one device pixel.
	   - 0x02 (MM_LOMETRIC): Each logical unit is mapped to 0.1 millimeter
	   - 0x03 (MM_HIMETRIC): Each logical unit is mapped to 0.01 millimeter.
	   - 0x04 (MM_LOENGLISH): Each logical unit is mapped to 0.01 inch.
	   - 0x05 (MM_HIENGLISH): Each logical unit is mapped to 0.001 inch.
	   - 0x06 (MM_TWIPS): Each logical unit is mapped to one twentieth of a printer's point (1/1440 inch).
	   - 0x07 (MM_ISOTROPIC): Logical units are mapped to arbitrary units with equally-scaled axes; that is, one unit along the x-axis is equal to one unit along the y-axis.
	   - 0x08 (MM_ANISOTROPIC): Logical units are mapped to arbitrary units with arbitrarily scaled axes.

	   Note that increasing x is to the right, and increasing y is up,
	   except for MM_TEXT, where it is down.

	   You can expect to get several calls to this function
	   (e.g. MM_ANISOTROPIC, followed by MM_HIMETRIC). If you maintain
	   state based on this call, you probably need to maintain the
	   dimensions / direction separate from the isotropic /
	   anisotropic state.

	   \param mapMode the mapping mode value
	*/
	virtual void setMapMode( const quint32 mapMode ) = 0;

	/**
	   Handler for the EMR_SETMETARGN record type
	*/
	virtual void setMetaRgn() = 0;

	/**
	   Handler for the EMR_SETWINDOWORGEX record type

	   \param origin the origin of the window in logical coordinates
	*/
	virtual void setWindowOrgEx( const QPoint &origin ) = 0;

	/**
	   Handler for the EMR_SETWINDOWEXTEX record type

	   \param size the size of the window in logical coordinates
	*/
	virtual void setWindowExtEx( const QSize &size ) = 0;

	/**
	   Handler for the EMR_SETVIEWPORTORGEX record type

	   \param origin the origin of the viewport in logical coordinates
	*/
	virtual void setViewportOrgEx( const QPoint &origin ) = 0;

	/**
	   Handler for the EMR_SETVIEWPORTEXTEX record type

	   \param size the size of the viewport in logical coordinates
	*/
	virtual void setViewportExtEx( const QSize &size ) = 0;

	/**
	   Handler for the EMR_SETBKMODE record type

	   \param backgroundMode the background fill mode
	*/
	virtual void setBkMode( const quint32 backgroundMode ) = 0;

	/**
	   Handler for the EMR_SETPOLYFILLMODE record type

	   \param polyFillMode the fill mode
	*/
	virtual void setPolyFillMode( const quint32 polyFillMode ) = 0;

	/**
	   Handler for the EMR_SETLAYOUT record type

	   \param layoutMode the layout mode
	*/
	virtual void setLayout( const quint32 layoutMode ) = 0;

	/**
	   Handler for the EMR_MODIFYWORLDTRANSFORM record type

	   There are a range of modes:
	   - 0x01 (MWT_IDENTIFY): Reset current world transform to identity matrix
	   - 0x02 (MWT_LEFTMULTIPLY): Left multiply this matrix with current matrix.
	   - 0x03 (MWT_RIGHTMULTIPLY): Right multiply current matrix with this matrix.
	   - 0x04 (MWT_SET): Set the world transform.

	   \param mode the mode to use.
	   \param M11
	   \param M12
	   \param M21
	   \param M22
	   \param Dx
	   \param Dy
	*/
	virtual void modifyWorldTransform(quint32 mode, float M11, float M12,
					  float M21, float M22, float Dx, float Dy ) = 0;

	/**
	   Handler for the EMR_SETWORLDTRANSFORM record type

	   \param M11
	   \param M12
	   \param M21
	   \param M22
	   \param Dx
	   \param Dy
	*/
	virtual void setWorldTransform( float M11, float M12, float M21,
					float M22, float Dx, float Dy ) = 0;

	/**
	   Select a previously created (or stock) object

	   \param ihObject the reference number for the object to select
	*/
	virtual void selectObject( const quint32 ihObject ) = 0;

	/**
	   Delete a previously created (or stock) object

	   \param ihObject the reference number for the object to delete
	*/
	virtual void deleteObject( const quint32 ihObject ) = 0;

	/**
	   Handler for the EMR_ARC record type

	   \param box the bounding box
	   \param start the coordinates of the point that defines the first radial end point
	   \param end the coordinates of the point that defines the second radial end point
	*/
	virtual void arc( const QRect &box, const QPoint &start, const QPoint &end ) = 0;

	/**
	   Handler for the EMR_CHORD record type

	   \param box the bounding box
	   \param start the coordinates of the point that defines the first radial end point
	   \param end the coordinates of the point that defines the second radial end point
	*/
	virtual void chord( const QRect &box, const QPoint &start, const QPoint &end ) = 0;

	/**
	   Handler for the EMR_PIE record type

	   \param box the bounding box
	   \param start the coordinates of the point that defines the first radial end point
	   \param end the coordinates of the point that defines the second radial end point
	*/
	virtual void pie( const QRect &box, const QPoint &start, const QPoint &end ) = 0;

	/**
	  Handler for the EMR_ELLIPSE record type

	  \param box the bounding box for the ellipse
	*/
	virtual void ellipse( const QRect &box ) = 0;

	/**
	  Handler for the EMR_RECTANGLE record type

	  \param box the bounding box for the rectangle
	*/
	virtual void rectangle( const QRect &box ) = 0;

	/**
	   Handler for the EMR_SETTEXTALIGN record type

	  The textAlignMode is a bit mask, see [MS-WMF] Section 2.1.2.3 for
	  values if the text has a horizontal baseline, [MS-WMF] Section
	  2.1.2.4 if the text has a vertical baseline.

	   \param textAlignMode the text alignment mode
	*/
	virtual void setTextAlign( const quint32 textAlignMode ) = 0;

	/**
	  Handler for the EMR_SETTEXTCOLOR record type

	  \param red the red component of the text color
	  \param green the blue component of the text color
	  \param blue the blue component of the text color
	  \param reserved an unused value - ignore this
	*/
	virtual void setTextColor( const quint8 red, const quint8 green, const quint8 blue,
				   const quint8 reserved ) = 0;

	/**
	  Handler for the EMR_SETBKCOLOR record type

	  \param red the red component of the background color
	  \param green the blue component of the background color
	  \param blue the blue component of the background color
	  \param reserved an unused value - ignore this
	*/
	virtual void setBkColor( const quint8 red, const quint8 green, const quint8 blue,
							 const quint8 reserved ) = 0;

	/**
	   Handler for the EMR_EXTCREATEFONTINDIRECTW record type

	   \param extCreateFontIndirectWRecord the contents of the
	   EMR_EXTCREATEFONTINDIRECTW record
	*/
	virtual void extCreateFontIndirectW( const ExtCreateFontIndirectWRecord &extCreateFontIndirectW ) = 0;

	/**
	   Handler for text rendering, as described in the
	   EMR_EXTTEXTOUTW and EMR_EXTTEXTOUTA record types.

	   \param bounds the bounds used for e.g. clipping
	   \param texObject The object describing the text.
	*/
	virtual void extTextOut( const QRect &bounds, const EmrTextObject &textObject ) = 0;

	/**
	   Handler for the EMR_BEGINPATH record type
	*/
	virtual void beginPath() = 0;

	/**
	   Handler for the EMR_CLOSEFIGURE record type
	*/
	virtual void closeFigure() = 0;

	/**
	   Handler for the EMR_ENDPATH record type
	*/
	virtual void endPath() = 0;

	/**
	   Handler for the EMR_MOVETOEX record type

	   \param x the X coordinate of the point to move to
	   \param y the Y coordiante of the point to move to
	*/
	virtual void moveToEx( const qint32 x, const qint32 y ) = 0;

	/**
	   Handler for the EMR_SAVEDC record type
	*/
	virtual void saveDC() = 0;

	/**
	   Handler for the EMR_RESTOREDC record type

	   \param savedDC the device context to restore to (always negative)
	*/
	virtual void restoreDC( const qint32 savedDC ) = 0;

	/**
	   Handler for the EMR_LINETO record type

	   \param finishPoint the point to draw to
	*/
	virtual void lineTo( const QPoint &finishPoint ) = 0;

	/**
	   Handler for the EMR_ARCTO record type

	   \param box the bounding box
	   \param start the coordinates of the point that defines the first radial end point
	   \param end the coordinates of the point that defines the second radial end point
	*/
	virtual void arcTo( const QRect &box, const QPoint &start, const QPoint &end ) = 0;

	/**
	   Handler for the EMR_POLYGON16 record type.

	   This record type specifies how to output a multi-segment filled
	   polygon.

	   \param bounds the bounding rectangle for the line segment
	   \param points the sequence of points that describe the polygon
	*/
	virtual void polygon16( const QRect &bounds, const QList<QPoint> points ) = 0;

	/**
	   Handler for the EMR_POLYLINE record type.

	   This record type specifies how to output a multi-segment line
	   (unfilled polyline).

	   \param bounds the bounding rectangle for the line segments
	   \param points the sequence of points that describe the line

	   \note the line is not meant to be closed (i.e. do not connect
	   the last point to the first point) or filled.
	*/
	virtual void polyLine( const QRect &bounds, const QList<QPoint> points ) = 0;

	/**
	   Handler for the EMR_POLYLINE16 record type.

	   This record type specifies how to output a multi-segment line
	   (unfilled polyline).

	   \param bounds the bounding rectangle for the line segment
	   \param points the sequence of points that describe the line

	   \note the line is not meant to be closed (i.e. do not connect
	   the last point to the first point) or filled.
	*/
	virtual void polyLine16( const QRect &bounds, const QList<QPoint> points ) = 0;

	/**
	   Handler for the EMR_POLYPOLYLINE16 record type.

	   This record type specifies how to output a set of multi-segment line
	   (unfilled polylines). Each vector in the list is a separate polyline

	   \param bounds the bounding rectangle for the line segments
	   \param points the sequence of points that describe the line

	   \note the lines are not meant to be closed (i.e. do not connect
	   the last point to the first point) or filled.
	*/
	virtual void polyPolyLine16( const QRect &bounds, const QList< QVector< QPoint > > &points ) = 0;

	/**
	   Handler for the EMR_POLYPOLYGON16 record type.

	   This record type specifies how to output a set of multi-segment polygons.
	   Each vector in the list is a separate filled polygon.

	   \param bounds the bounding rectangle for the polygons
	   \param points the sequence of points that describe the polygons
	*/
	virtual void polyPolygon16( const QRect &bounds, const QList< QVector< QPoint > > &points ) = 0;

	/**
	   Handler for the EMR_POLYLINETO16 record type.

	   This record type specifies how to output a multi-segment set of
	   lines (unfilled).

	   \param bounds the bounding rectangle for the bezier curves
	   \param points the sequence of points that describe the curves

	   \note the line is not meant to be closed (i.e. do not connect
	   the last point to the first point) or filled.
	*/
	virtual void polyLineTo16( const QRect &bounds, const QList<QPoint> points ) = 0;

	/**
	   Handler for the EMR_POLYBEZIERO16 record type.

	   This record type specifies how to output a multi-segment set of
	   bezier curves (unfilled).

	   \param bounds the bounding rectangle for the bezier curves
	   \param points the sequence of points that describe the curves

	   \note the line is not meant to be closed (i.e. do not connect
	   the last point to the first point) or filled.
	*/
	virtual void polyBezier16( const QRect &bounds, const QList<QPoint> points ) = 0;

	/**
	   Handler for the EMR_POLYLINETO16 record type.

	   This record type specifies how to output a multi-segment set of
	   bezier curves (unfilled), starting at the current point.

	   \param bounds the bounding rectangle for the bezier curves
	   \param points the sequence of points that describe the curves

	   \note the line is not meant to be closed (i.e. do not connect
	   the last point to the first point) or filled.
	*/
	virtual void polyBezierTo16( const QRect &bounds, const QList<QPoint> points ) = 0;

	/**
	   Handler for the EMR_FILLPATH record type.

	   \param bounds the bounding rectangle for the region to be filled.
	*/
	virtual void fillPath( const QRect &bounds ) = 0;

	/**
	   Handler for the EMR_STROKEANDFILLPATH record type.

	   \param bounds the bounding rectangle for the region to be stroked / filled
	*/
	virtual void strokeAndFillPath( const QRect &bounds ) = 0;

	/**
	   Handler for the EMR_STROKEPATH record type.

	   \param bounds the bounding rectangle for the region to be stroked
	*/
	virtual void strokePath( const QRect &bounds ) = 0;

	/**
	   Handler for the EMR_SETCLIPPATH record type.

	   See [MS-EMF] Section 2.1.29 for valid ways to set the path.

	   \param regionMode how to set the clipping path.
	*/
	virtual void setClipPath( const quint32 regionMode ) = 0;

	/**
	   Handler for the EMR_BITBLT record type

	   \param bitBltRecord contents of the record type
	*/
	virtual void bitBlt( BitBltRecord &bitBltRecord ) = 0;

	/**
	   Handler for the EMR_STRETCHBLTMODE record type

	   \param stretchMode the stretch mode
	*/
	virtual void setStretchBltMode( const quint32 stretchMode ) = 0;

	/**
	   Handler for the EMR_STRETCHDIBITS record type

	   \param stretchDiBitsRecord contents of the record type
	*/
	virtual void stretchDiBits( StretchDiBitsRecord &stretchDiBitsRecord ) = 0;

	/**
	   Handler for the EMR_ALPHABLEND record type

	   \param alphaBlendRecord contents of the record type
	*/
	virtual void alphaBlend(AlphaBlendRecord &alphaBlendRecord ) = 0;

	/**
	   Handler for the EMR_SETMITERLIMIT record type

	   \param limit mitter limit
	*/
	virtual void setMitterLimit(const quint32 limit) = 0;
};


}

#endif
