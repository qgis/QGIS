/*
  Copyright 2008      Brad Hards  <bradh@frogmouth.net>
  Copyright 2009-2010 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#include "EmfLogger.h"
#include "EmfObjects.h"

#include <QFile>
#include <QDebug>

namespace QEmf
{


EmfLogger::EmfLogger(const QString& fileName):
m_debug(0),
m_file(0)
{
	if (!fileName.isEmpty()){
		m_file = new QFile(fileName);
		if (m_file->open(QFile::WriteOnly))
			m_debug = new QDebug(m_file);
	}
}

EmfLogger::~EmfLogger()
{
	if (m_file){
		m_file->close();
		delete m_file;
		delete m_debug;
	}
}

void EmfLogger::init( const Header *header )
{
	*m_debug << "Initialising EmfLogger";
	*m_debug << "image size:" << header->bounds().size() << "\n";
}

void EmfLogger::cleanup( const Header *header )
{
	*m_debug << "Cleanup EmfLogger";
	*m_debug << "image size:" << header->bounds().size() << "\n";
}

void EmfLogger::eof()
{
	*m_debug << "EMR_EOF" << "\n";
}

void EmfLogger::setPixelV( QPoint &point, quint8 red, quint8 green, quint8 blue, quint8 reserved )
{
	Q_UNUSED( reserved );
	*m_debug << "EMR_SETPIXELV:" << point << QColor( red, green, blue ) << "\n";
}

void EmfLogger::beginPath()
{
	*m_debug << "EMR_BEGINPATH" << "\n";
}

void EmfLogger::closeFigure()
{
	*m_debug << "EMR_CLOSEFIGURE" << "\n";
}

void EmfLogger::endPath()
{
	*m_debug << "EMR_ENDPATH" << "\n";
}

void EmfLogger::saveDC()
{
	*m_debug << "EMR_SAVEDC" << "\n";
}

void EmfLogger::restoreDC( qint32 savedDC )
{
	*m_debug << "EMR_RESTOREDC" << savedDC << "\n";
}

void EmfLogger::setMetaRgn()
{
	*m_debug << "EMR_SETMETARGN" << "\n";
}

void EmfLogger::setWindowOrgEx( const QPoint &origin )
{
	*m_debug << "EMR_SETWINDOWORGEX" << origin << "\n";
}

void EmfLogger::setWindowExtEx( const QSize &size )
{
	*m_debug << "EMR_SETWINDOWEXTEX" << size << "\n";
}

void EmfLogger::setViewportOrgEx( const QPoint &origin )
{
	*m_debug << "EMR_SETVIEWPORTORGEX" << origin << "\n";
}

void EmfLogger::setViewportExtEx( const QSize &size )
{
	*m_debug << "EMR_SETVIEWPORTEXTEX" << size << "\n";
}

void EmfLogger::deleteObject( const quint32 ihObject )
{
	*m_debug << "EMR_DELETEOBJECT:" << ihObject << "\n";
}

void EmfLogger::arc( const QRect &box, const QPoint &start, const QPoint &end )
{
	*m_debug << "EMR_ARC" << box << start << end << "\n";
}

void EmfLogger::chord( const QRect &box, const QPoint &start, const QPoint &end )
{
	*m_debug << "EMR_CHORD" << box << start << end << "\n";
}

void EmfLogger::pie( const QRect &box, const QPoint &start, const QPoint &end )
{
	*m_debug << "EMR_PIE" << box << start << end << "\n";
}

void EmfLogger::ellipse( const QRect &box )
{
	*m_debug << "EMR_ELLIPSE:" << box << "\n";
}

void EmfLogger::rectangle( const QRect &box )
{
	*m_debug << "EMR_RECTANGLE:" << box << "\n";
}

void EmfLogger::modifyWorldTransform( quint32 mode, float M11, float M12,
					float M21, float M22, float Dx, float Dy )
{
	*m_debug << "EMR_MODIFYWORLDTRANSFORM:" << mode << QTransform ( M11, M12, M21, M22, Dx, Dy ) << "\n";
}

void EmfLogger::setWorldTransform( float M11, float M12, float M21,
					 float M22, float Dx, float Dy )
{
	*m_debug << "EMR_SETWORLDTRANSFORM:" << QTransform ( M11, M12, M21, M22, Dx, Dy ) << "\n";
}

void EmfLogger::setMapMode( quint32 mapMode )
{
	QString modeAsText;
	switch ( mapMode ) {
	case MM_TEXT:
	modeAsText = QString( "map mode - text" );
	break;
	case MM_LOMETRIC:
	modeAsText = QString( "map mode - lometric" );
	break;
	case MM_HIMETRIC:
	modeAsText = QString( "map mode - himetric" );
	break;
	case MM_LOENGLISH:
	modeAsText = QString( "map mode - loenglish" );
	break;
	case MM_HIENGLISH:
	modeAsText = QString( "map mode - hienglish" );
	break;
	case MM_TWIPS:
	modeAsText = QString( "map mode - twips" );
	break;
	case MM_ISOTROPIC:
	modeAsText = QString( "map mode - isotropic" );
	break;
	case MM_ANISOTROPIC:
	modeAsText = QString( "map mode - anisotropic" );
	break;
	default:
	modeAsText = QString( "unexpected map mode: %1").arg( mapMode );
	}
	*m_debug << "EMR_SETMAPMODE:" << modeAsText << "\n";

}

void EmfLogger::setBkMode( const quint32 backgroundMode )
{
	if ( backgroundMode == TRANSPARENT ) {
		*m_debug << "EMR_SETBKMODE: Transparent" << "\n";
	} else if ( backgroundMode == OPAQUE ) {
		*m_debug << "EMR_SETBKMODE: Opaque" << "\n";
	} else {
		*m_debug << "EMR_SETBKMODE: Unexpected value -" << backgroundMode << "\n";
		Q_ASSERT( 0 );
	}
}

void EmfLogger::setPolyFillMode( const quint32 polyFillMode )
{
	if ( polyFillMode == ALTERNATE ) {
	*m_debug << "EMR_SETPOLYFILLMODE: OddEvenFill" << "\n";
	} else if ( polyFillMode == WINDING ) {
	*m_debug << "EMR_SETPOLYFILLMODE: WindingFill" << "\n";
	} else {
	*m_debug << "EMR_SETPOLYFILLMODE: Unexpected value -" << polyFillMode << "\n";
	Q_ASSERT( 0 );
	}
}

void EmfLogger::setLayout( const quint32 layoutMode )
{
	*m_debug << "EMR_SETLAYOUT:" << layoutMode << "\n";
}

void EmfLogger::extCreateFontIndirectW( const ExtCreateFontIndirectWRecord &extCreateFontIndirectW )
{
	*m_debug << "EMR_CREATEFONTINDIRECTW:" << extCreateFontIndirectW.fontFace() << "\n";
}

void EmfLogger::setTextAlign( const quint32 textAlignMode )
{
	*m_debug << "EMR_SETTEXTALIGN:" << textAlignMode << "\n";
}

void EmfLogger::setTextColor( const quint8 red, const quint8 green, const quint8 blue,
				const quint8 reserved )
{
	Q_UNUSED( reserved );
	*m_debug << "EMR_SETTEXTCOLOR" << QColor( red, green, blue ) << "\n";
}

void EmfLogger::setBkColor( const quint8 red, const quint8 green, const quint8 blue,
							  const quint8 reserved )
{
	Q_UNUSED( reserved );
	*m_debug << "EMR_SETBKCOLOR" << QColor( red, green, blue ) << "\n";
}

void EmfLogger::createPen(quint32 ihPen, quint32 penStyle, quint32 x, quint32 y,
				   quint8 red, quint8 green, quint8 blue, quint32 brushStyle, quint8 reserved )
{
	Q_UNUSED( y );
	Q_UNUSED( reserved );

	*m_debug << "EMR_CREATEPEN" << "ihPen:" << ihPen << ", penStyle:" << penStyle
				  << "width:" << x << "color:" << QColor( red, green, blue ) << ", brushStyle:" << brushStyle<< "\n";
}

void EmfLogger::createBrushIndirect( quint32 ihBrush, quint32 BrushStyle, quint8 red,
					   quint8 green, quint8 blue, quint8 reserved,
					   quint32 BrushHatch )
{
	Q_UNUSED( reserved );

	*m_debug << "EMR_CREATEBRUSHINDIRECT:" << ihBrush << "style:" << BrushStyle
			 << "Colour:" << QColor( red, green, blue ) << ", Hatch:" << BrushHatch << "\n";
}

void EmfLogger::createMonoBrush( quint32 ihBrush, Bitmap *bitmap )
{
	*m_debug << "EMR_CREATEMONOBRUSH:" << ihBrush << "bitmap:" << bitmap << "\n";
}

void EmfLogger::createDibPatternBrushPT( quint32 ihBrush, Bitmap *bitmap )
{
	*m_debug << "EMR_CREATEDIBPATTERNBRUSHPT:" << ihBrush << "bitmap:" << bitmap << "\n";
}

void EmfLogger::selectObject( const quint32 ihObject )
{
	*m_debug << "EMR_SELECTOBJECT" << ihObject << "\n";
}

void EmfLogger::extTextOut( const QRect &bounds, const EmrTextObject &textObject )
{
	*m_debug << "EMR_EXTTEXTOUTW:" << bounds
				  << textObject.referencePoint()
				  << textObject.textString() << "\n";
}

void EmfLogger::moveToEx( const qint32 x, const qint32 y )
{
	*m_debug << "EMR_MOVETOEX" << QPoint( x, y ) << "\n";
}

void EmfLogger::lineTo( const QPoint &finishPoint )
{
	*m_debug << "EMR_LINETO" << finishPoint << "\n";
}

void EmfLogger::arcTo( const QRect &box, const QPoint &start, const QPoint &end )
{
	*m_debug << "EMR_ARCTO" << box << start << end << "\n";
}

void EmfLogger::polygon16( const QRect &bounds, const QList<QPoint> points )
{
	*m_debug << "EMR_POLYGON16" << bounds << points << "\n";
}

void EmfLogger::polyLine( const QRect &bounds, const QList<QPoint> points )
{
	*m_debug << "EMR_POLYLINE" << bounds << points << "\n";
}

void EmfLogger::polyLine16( const QRect &bounds, const QList<QPoint> points )
{
	*m_debug << "EMR_POLYLINE16" << bounds << points << "\n";
}

void EmfLogger::polyPolyLine16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
	*m_debug << "EMR_POLYPOLYLINE16" << bounds << points << "\n";
}

void EmfLogger::polyPolygon16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
	*m_debug << "EMR_POLYPOLYGON16" << bounds << points << "\n";
}

void EmfLogger::polyLineTo16( const QRect &bounds, const QList<QPoint> points )
{
	*m_debug << "EMR_POLYLINETO16" << bounds << points << "\n";
}

void EmfLogger::polyBezier16( const QRect &bounds, const QList<QPoint> points )
{
	*m_debug << "EMR_POLYBEZIER16" << bounds << points << "\n";
}

void EmfLogger::polyBezierTo16( const QRect &bounds, const QList<QPoint> points )
{
	*m_debug << "EMR_POLYBEZIERTO16" << bounds << points << "\n";
}

void EmfLogger::fillPath( const QRect &bounds )
{
	*m_debug << "EMR_FILLPATH" << bounds << "\n";
}

void EmfLogger::strokeAndFillPath( const QRect &bounds )
{
	*m_debug << "EMR_STROKEANDFILLPATH" << bounds << "\n";
}

void EmfLogger::strokePath( const QRect &bounds )
{
	*m_debug << "EMR_STROKEPATH" << bounds << "\n";
}

void EmfLogger::setMitterLimit( const quint32 limit )
{
	*m_debug  << "EMR_SETMITERLIMIT" << limit  << "\n";
}

void EmfLogger::setClipPath( quint32 regionMode )
{
	*m_debug << "EMR_SETCLIPPATH:" << regionMode << "\n";
}

void EmfLogger::bitBlt( BitBltRecord &bitBltRecord )
{
	*m_debug << "EMR_BITBLT:" << bitBltRecord.destinationRectangle() << "\n";
}

void EmfLogger::setStretchBltMode( const quint32 stretchMode )
{
	switch ( stretchMode ) {
	case 0x01:
		*m_debug << "EMR_STRETCHBLTMODE: STRETCH_ANDSCANS" << "\n";
		break;
	case 0x02:
		*m_debug << "EMR_STRETCHBLTMODE: STRETCH_ORSCANS" << "\n";
		break;
	case 0x03:
		*m_debug << "EMR_STRETCHBLTMODE: STRETCH_DELETESCANS" << "\n";
		break;
	case 0x04:
		*m_debug << "EMR_STRETCHBLTMODE: STRETCH_HALFTONE" << "\n";
		break;
	default:
		*m_debug << "EMR_STRETCHBLTMODE - unknown stretch mode:" << stretchMode << "\n";
	}
}

void EmfLogger::stretchDiBits( StretchDiBitsRecord &stretchDiBitsRecord )
{
	*m_debug << "EMR_STRETCHDIBITS:" << stretchDiBitsRecord.sourceRectangle()
				  << "," << stretchDiBitsRecord.destinationRectangle() << "\n";
}

void EmfLogger::alphaBlend(AlphaBlendRecord& record)
{
	*m_debug << "EMR_ALPHABLEND:" << record.sourceRectangle()
				  << "," << record.destinationRectangle() << "\n";
}

} // xnamespace...
