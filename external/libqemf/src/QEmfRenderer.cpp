/*
  Copyright 2008        Brad Hards  <bradh@frogmouth.net>
  Copyright 2009 - 2010 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#include "QEmfRenderer.h"
#include "EmfObjects.h"

#ifdef Q_OS_WIN
#define _USE_MATH_DEFINES
#include <cmath>
#endif

#include <math.h>
#include <QDebug>
#include <QPoint>

#define DEBUG_EMFPAINT 0
#define DEBUG_PAINTER_TRANSFORM 0

namespace QEmf
{


static QPainter::CompositionMode  rasteropToQtComposition(long rop);

// ================================================================
//                         Class QEmfRenderer


QEmfRenderer::QEmfRenderer()
	: m_header( 0 )
	, m_path( 0 )
	, m_currentlyBuildingPath(false)
	, m_fillRule(Qt::OddEvenFill)
	, m_mapMode(MM_TEXT)
	, m_textAlignMode(TA_NOUPDATECP) // == TA_TOP == TA_LEFT
	, m_currentCoords()
	, m_current_bitmap_mask(QBitmap())
	,m_current_mask_color(QColor(Qt::color1))
{
	m_painter         = 0;
	m_painterSaves    = 0;
	m_outputSize      = QSize();
	m_keepAspectRatio = true;
}

QEmfRenderer::QEmfRenderer(QPainter &painter, QSize &size, bool keepAspectRatio)
	: m_header( 0 )
	, m_path( 0 )
	, m_currentlyBuildingPath(false)
	, m_windowExtIsSet(false)
	, m_viewportExtIsSet(false)
	, m_windowViewportIsSet(false)
	, m_fillRule(Qt::OddEvenFill)
	, m_mapMode(MM_TEXT)
	, m_textAlignMode(TA_NOUPDATECP) // == TA_TOP == TA_LEFT
	, m_currentCoords()
	, m_current_bitmap_mask(QBitmap())
	,m_current_mask_color(QColor(Qt::color1))
{
	m_painter         = &painter;
	m_painterSaves    = 0;
	m_outputSize      = size;
	m_keepAspectRatio = keepAspectRatio;
}

QEmfRenderer::~QEmfRenderer()
{
	delete m_header;
	delete m_path;
}

void QEmfRenderer::paintBounds(const Header *header)
{
	// The rectangle is in device coordinates.
	QRectF  rect(header->bounds());
	m_painter->save();

	// Draw a simple cross in a rectangle to show the bounds.
	m_painter->setPen(QPen(QColor(172, 196, 206)));
	m_painter->drawRect(rect);
	m_painter->drawLine(rect.topLeft(), rect.bottomRight());
	m_painter->drawLine(rect.bottomLeft(), rect.topRight());

	m_painter->restore();
}

void QEmfRenderer::init( const Header *header )
{
	// Save the header since we need the frame and bounds inside the drawing.
	m_header = new Header(*header);

	QSize headerBoundsSize = header->bounds().size();

#if DEBUG_EMFPAINT
	qDebug() << "----------------------------------------------------------------------";
	qDebug() << "Shape size               =" << m_outputSize.width() << m_outputSize.height() << " pt";
	qDebug() << "----------------------------------------------------------------------";
	qDebug() << "Boundary box (dev units) =" << header->bounds().x() << header->bounds().y()
				  << header->bounds().width() << header->bounds().height();
	qDebug() << "Frame (phys size)        =" << header->frame().x() << header->frame().y()
				  << header->frame().width() << header->frame().height() << " *0.01 mm";

	qDebug() << "Device =" << header->device().width() << header->device().height();
	qDebug() << "Millimeters =" << header->millimeters().width()
				  << header->millimeters().height();
#endif

#if DEBUG_PAINTER_TRANSFORM
	printPainterTransform("In init, before save:");
#endif

	// This is restored in cleanup().
	m_painter->save();

	// Calculate how much the painter should be resized to fill the
	// outputSize with output.
	qreal scaleX = qreal( m_outputSize.width() ) / headerBoundsSize.width();
	qreal scaleY = qreal( m_outputSize.height() ) / headerBoundsSize.height();
	if ( m_keepAspectRatio ) {
		// Use the smaller value so that we don't get an overflow in
		// any direction.
		if ( scaleX > scaleY )
			scaleX = scaleY;
		else
			scaleY = scaleX;
	}
#if DEBUG_EMFPAINT
	qDebug() << "scale = " << scaleX << ", " << scaleY;
#endif

	// Transform the EMF object so that it fits in the shape.  The
	// topleft of the EMF will be the top left of the shape.
	m_painter->scale( scaleX, scaleY );
	m_painter->translate(-header->bounds().left(), -header->bounds().top());
#if DEBUG_PAINTER_TRANSFORM
	printPainterTransform("after fitting into shape");
#endif

	// Save the scale so that we can use it when setting lineWidth.
	m_outputScale = (scaleX + scaleY) / 2;
	m_textRotation = .0;

	// Calculate translation if we should center the EMF in the
	// area and keep the aspect ratio.
#if 0 // Should apparently be upper left.  See bug 265868
	if ( m_keepAspectRatio ) {
		m_painter->translate((m_outputSize.width() / scaleX - headerBoundsSize.width()) / 2,
							 (m_outputSize.height() / scaleY - headerBoundsSize.height()) / 2);
#if DEBUG_PAINTER_TRANSFORM
		printPainterTransform("after translation for keeping center in the shape");
#endif
	}
#endif

	m_outputTransform = m_painter->transform();
	m_worldTransform = QTransform();
	m_pathTransform = QTransform();

	// For calculations of window / viewport during the painting
	m_windowOrg = QPoint(0, 0);
	m_viewportOrg = QPoint(0, 0);
	m_windowExtIsSet = false;
	m_viewportExtIsSet = false;
	m_windowViewportIsSet = false;

#if DEBUG_EMFPAINT
	paintBounds(header);
#endif
}

void QEmfRenderer::cleanup( const Header *header )
{
	Q_UNUSED( header );

#if DEBUG_EMFPAINT
	if (m_painterSaves > 0)
		qDebug() << "WARNING: UNRESTORED DC's:" << m_painterSaves;
#endif

	// Restore all the save()s that were done during the processing.
	for (int i = 0; i < m_painterSaves; ++i)
		m_painter->restore();
	m_painterSaves = 0;

	// Restore the painter to what it was before init() was called.
	m_painter->restore();
}


void QEmfRenderer::eof()
{
}

void QEmfRenderer::setPixelV( QPoint &point, quint8 red, quint8 green, quint8 blue, quint8 reserved )
{
	Q_UNUSED( reserved );

#if DEBUG_EMFPAINT
	qDebug() << point << red << green << blue;
#endif

	m_painter->save();

	QPen pen;
	pen.setColor( QColor( red, green, blue ) );
	m_painter->setPen( pen );
	m_painter->drawPoint( point );

	m_painter->restore();
}


void QEmfRenderer::beginPath()
{
#if DEBUG_EMFPAINT
	qDebug();
#endif

	if (m_path)
		delete m_path;
	m_path = new QPainterPath;
	m_currentlyBuildingPath = true;
}

void QEmfRenderer::closeFigure()
{
#if DEBUG_EMFPAINT
	qDebug();
#endif

	m_path->closeSubpath();
}

void QEmfRenderer::endPath()
{
#if DEBUG_EMFPAINT
	qDebug();
#endif

	m_path->setFillRule( m_fillRule );
	m_currentlyBuildingPath = false;
}

void QEmfRenderer::saveDC()
{
#if DEBUG_EMFPAINT
	qDebug();
#endif

	// A little trick here: Save the worldTransform in the painter.
	// If we didn't do this, we would have to create a separate stack
	// for these.
	//
	// FIXME: We should collect all the parts of the DC that are not
	//        stored in the painter and save them separately.
	QTransform  savedTransform = m_painter->worldTransform();
	m_painter->setWorldTransform(m_worldTransform);

	m_painter->save();
	++m_painterSaves;

	m_painter->setWorldTransform(savedTransform);
}

void QEmfRenderer::restoreDC( const qint32 savedDC )
{
#if DEBUG_EMFPAINT
	qDebug() << savedDC;
#endif

	// Note that savedDC is always negative
	for (int i = 0; i < -savedDC; ++i) {
		if (m_painterSaves > 0) {
			m_painter->restore();
			--m_painterSaves;
		}
		else {
			qDebug() << "restoreDC(): try to restore painter without save" << savedDC - i;
			break;
		}
	}

	// We used a trick in saveDC() and stored the worldTransform in
	// the painter.  Now restore the full transformation.
	m_worldTransform = m_painter->worldTransform();
	QTransform newMatrix = m_worldTransform * m_outputTransform;
	m_painter->setWorldTransform( newMatrix );
}

void QEmfRenderer::setMetaRgn()
{
	qDebug() << "EMR_SETMETARGN not yet implemented";
}


// ----------------------------------------------------------------
//                 World Transform, Window and Viewport


// General note about coordinate spaces and transforms:
//
// There are several coordinate spaces in use when drawing an EMF file:
//  1. The object space, in which the objects' coordinates are expressed inside the EMF.
//     In general there are several of these.
//  2. The page space, which is where they end up being painted in the EMF picture.
//     The union of these form the bounding box of the EMF.
//  3. (possibly) the output space, where the EMF picture itself is placed
//     and/or scaled, rotated, etc
//
// The transform between spaces 1. and 2. is called the World Transform.
// The world transform can be changed either through calls to change
// the window or viewport or through calls to setWorldTransform() or
// modifyWorldTransform().
//
// The transform between spaces 2. and 3. is the transform that the QPainter
// already contains when it is given to us.  We need to save this and reapply
// it after the world transform has changed. We call this transform the Output
// Transform in lack of a better word. (Some sources call it the Device Transform.)
//

// An unanswered question:
//
// The file mp07_embedded_ppt.pptx in the test files contains the
// following sequence of records:
// - SetWorldTransform
// - ModifyWorldTransform
// - SetViewportOrg  <-- doesn't change anything
// - SetWindowOrg    <-- doesn't change anything
// - ExtTextOutw
//
// I was previously under the impression that whenever a
// Set{Window,Viewport}{Org,Ext} record was encountered, the world
// transform was supposed to be recalculated. But in this file, it
// destroys the world transform. The question is which of the
// following alternatives is true:
//
// 1. The world transform should only be recalculated if the
//    Set{Window,Viewport}{Org,Ext} record actually changes anything.
//
// 2. The transformations set by {Set,Modify}WorldTransform records
//    should always be reapplied after a change in window or viewport.
//
// 3. Something else
//
// I have for now implemented option 1. See the FIXME's in
// SetWindowOrg et al.
//


// Set Window and Viewport
void QEmfRenderer::recalculateWorldTransform()
{
	m_worldTransform = QTransform();

	// If neither the window nor viewport extension is set, then there
	// is no way to perform the calculation.  Just give up.
	if (!m_windowExtIsSet && !m_viewportExtIsSet)
		return;

	// Negative window extensions mean flip the picture.  Handle this here.
	bool  flip = false;
	qreal midpointX = 0.0;
	qreal midpointY = 0.0;
	qreal scaleX = 1.0;
	qreal scaleY = 1.0;
	if (m_windowExt.width() < 0) {
		midpointX = m_windowOrg.x() + m_windowExt.width() / qreal(2.0);
		scaleX = -1.0;
		flip = true;
	}
	if (m_windowExt.height() < 0) {
		midpointY = m_windowOrg.y() + m_windowExt.height() / qreal(2.0);
		scaleY = -1.0;
		flip = true;
	}
	if (flip) {
		//qDebug() << "Flipping" << midpointX << midpointY << scaleX << scaleY;
		m_worldTransform.translate(midpointX, midpointY);
		m_worldTransform.scale(scaleX, scaleY);
		m_worldTransform.translate(-midpointX, -midpointY);
		//qDebug() << "After flipping for window" << mWorldTransform;
	}

	// Update the world transform if both window and viewport are set...
	// FIXME: Check windowExt == 0 in any direction
	if (m_windowExtIsSet && m_viewportExtIsSet) {
		// Both window and viewport are set.
		qreal windowViewportScaleX = qreal(m_viewportExt.width()) / qreal(m_windowExt.width());
		qreal windowViewportScaleY = qreal(m_viewportExt.height()) / qreal(m_windowExt.height());

		m_worldTransform.translate(-m_windowOrg.x(), -m_windowOrg.y());
		m_worldTransform.scale(windowViewportScaleX, windowViewportScaleY);
		m_worldTransform.translate(m_viewportOrg.x(), m_viewportOrg.y());
	}

	// ...and apply it to the painter
	m_painter->setWorldTransform(m_worldTransform);
	m_windowViewportIsSet = true;

	// Apply the output transform.
	QTransform newMatrix = m_worldTransform * m_outputTransform;
	m_painter->setWorldTransform( newMatrix );
}


void QEmfRenderer::setWindowOrgEx( const QPoint &origin )
{
#if DEBUG_EMFPAINT
	qDebug() << origin;
#endif

	// FIXME: See unanswered question at the start of this section.
	if (m_windowOrg == origin) {
		//qDebug() << "same origin as before";
		return;
	}

	m_windowOrg = origin;

	recalculateWorldTransform();
}

void QEmfRenderer::setWindowExtEx( const QSize &size )
{
#if DEBUG_EMFPAINT
	qDebug() << size;
#endif

	// FIXME: See unanswered question at the start of this section.
	if (m_windowExt == size) {
		//qDebug() << "same extension as before";
		return;
	}

	m_windowExt = size;
	m_windowExtIsSet = true;

	recalculateWorldTransform();
}

void QEmfRenderer::setViewportOrgEx( const QPoint &origin )
{
#if DEBUG_EMFPAINT
	qDebug() << origin;
#endif

	// FIXME: See unanswered question at the start of this section.
	if (m_viewportOrg == origin) {
		//qDebug() << "same origin as before";
		return;
	}

	m_viewportOrg = origin;

	recalculateWorldTransform();
}

void QEmfRenderer::setViewportExtEx( const QSize &size )
{
#if DEBUG_EMFPAINT
	qDebug() << size;
#endif

	// FIXME: See unanswered question at the start of this section.
	if (m_viewportExt == size) {
		//qDebug() << "same extension as before";
		return;
	}

	m_viewportExt = size;
	m_viewportExtIsSet = true;

	recalculateWorldTransform();
}



void QEmfRenderer::modifyWorldTransform( quint32 mode, float M11, float M12,
												  float M21, float M22, float Dx, float Dy )
{
#if DEBUG_EMFPAINT
	if (mode == MWT_IDENTITY)
		qDebug() << "Identity matrix";
	else
		qDebug() << mode << M11 << M12 << M21 << M22 << Dx << Dy;
#endif

	QTransform matrix( M11, M12, M21, M22, Dx, Dy);

	if ( mode == MWT_IDENTITY ) {
		m_worldTransform = QTransform();
	} else if ( mode == MWT_LEFTMULTIPLY ) {
		m_worldTransform = matrix * m_worldTransform;
	} else if ( mode == MWT_RIGHTMULTIPLY ) {
		m_worldTransform = m_worldTransform * matrix;
	} else if ( mode == MWT_SET ) {
		m_worldTransform = matrix;
	} else {
	qWarning() << "Unimplemented transform mode" << mode;
	}

	m_pathTransform = m_currentlyBuildingPath ? m_worldTransform : QTransform();

	// Apply the output transform.
	QTransform newMatrix = m_worldTransform * m_outputTransform;
	m_painter->setWorldTransform( newMatrix );
}

void QEmfRenderer::setWorldTransform( float M11, float M12, float M21,
											   float M22, float Dx, float Dy )
{
#if DEBUG_EMFPAINT
	qDebug() << M11 << M12 << M21 << M22 << Dx << Dy;
#endif

	QTransform matrix( M11, M12, M21, M22, Dx, Dy);

	m_worldTransform = matrix;

	// Apply the output transform.
	QTransform newMatrix = m_worldTransform * m_outputTransform;
	m_painter->setWorldTransform( newMatrix );
}


// ----------------------------------------------------------------


void QEmfRenderer::createPen(quint32 ihPen, quint32 penStyle, quint32 x, quint32 y, quint8 red, quint8 green, quint8 blue,
							quint32 brushStyle, quint8 reserved)
{
	Q_UNUSED(y);
	Q_UNUSED(brushStyle);
	Q_UNUSED(reserved);

#if DEBUG_EMFPAINT
	qDebug() << ihPen << hex << penStyle << dec << x << y << red << green << blue << reserved;
#endif

	QPen pen;
	pen.setColor(QColor(red, green, blue));

	switch (penStyle & PS_STYLE_MASK){
	case PS_SOLID:
		pen.setStyle(Qt::SolidLine);
		break;
	case PS_DASH:
		pen.setStyle(Qt::DashLine);
		break;
	case PS_DOT:
		pen.setStyle(Qt::DotLine);
		break;
	case PS_DASHDOT:
		pen.setStyle(Qt::DashDotLine);
		break;
	case PS_DASHDOTDOT:
		pen.setStyle(Qt::DashDotDotLine);
		break;
	case PS_NULL:
		pen.setStyle(Qt::NoPen);
		break;
	case PS_INSIDEFRAME:
		// FIXME: We don't properly support this
		pen.setStyle(Qt::SolidLine);
		break;
	case PS_USERSTYLE:
		qDebug() << "UserStyle pen not yet supported, using SolidLine";
		pen.setStyle(Qt::SolidLine);
		break;
	case PS_ALTERNATE:
		qDebug() << "Alternate pen not yet supported, using DashLine";
		pen.setStyle(Qt::DashLine);
		break;
	default:
		qDebug() << "unexpected pen type, using SolidLine" << (penStyle & PS_STYLE_MASK);
		pen.setStyle(Qt::SolidLine);
	}

	switch (penStyle & PS_ENDCAP_MASK){
	case PS_ENDCAP_ROUND:
		pen.setCapStyle(Qt::RoundCap);
		break;
	case PS_ENDCAP_SQUARE:
		pen.setCapStyle(Qt::SquareCap);
		break;
	case PS_ENDCAP_FLAT:
		pen.setCapStyle(Qt::FlatCap);
		break;
	default:
		qDebug() << "unexpected cap style, using SquareCap" << (penStyle & PS_ENDCAP_MASK);
		pen.setCapStyle(Qt::SquareCap);
	}

	switch (penStyle & PS_JOIN_MASK){
	case PS_JOIN_ROUND:
		pen.setJoinStyle(Qt::RoundJoin);
		break;
	case PS_JOIN_BEVEL:
		pen.setJoinStyle(Qt::BevelJoin);
		break;
	case PS_JOIN_MITER:
		pen.setJoinStyle(Qt::MiterJoin);
		break;
	default:
		qDebug() << "unexpected join style: " << (penStyle & PS_JOIN_MASK);
	}

	if (x > INT_MAX){
		qDebug() << "Warning: ignored pen width larger than maximum allowed value, x:" << x;
		pen.setWidthF((m_outputScale < 1.0) ? m_outputScale : 1.0);
	} else
		pen.setWidthF((m_outputScale < 1.0) ? x*m_outputScale : x);

	bool cosmetic = false;
	switch (penStyle & PS_TYPE_MASK){
	case PS_COSMETIC:
		cosmetic = true;
	break;
	case PS_GEOMETRIC:
	break;
	default:
		qDebug() << "unexpected pen type: " << (penStyle & PS_TYPE_MASK);
	}
	pen.setCosmetic(cosmetic);

	m_objectTable.insert(ihPen, pen);
}

void QEmfRenderer::createBrushIndirect( quint32 ihBrush, quint32 brushStyle,
										 quint8 red, quint8 green, quint8 blue, quint8 reserved, quint32 brushHatch)
{
	Q_UNUSED( reserved );
	Q_UNUSED( brushHatch );

#if DEBUG_EMFPAINT
	qDebug() << ihBrush << hex << brushStyle << dec << red << green << blue << reserved << brushHatch;
#endif

	QBrush brush;

	switch ( brushStyle ) {
	case BS_SOLID:
		brush.setStyle( Qt::SolidPattern );
	break;
	case BS_NULL:
		brush.setStyle( Qt::NoBrush );
	break;
	case BS_HATCHED:
			switch (brushHatch){
			case HS_BDIAGONAL:
				brush.setStyle(Qt::BDiagPattern);
			break;
			case HS_CROSS:
				brush.setStyle(Qt::CrossPattern);
			break;
			case HS_DIAGCROSS:
				brush.setStyle(Qt::DiagCrossPattern);
			break;
			case HS_FDIAGONAL:
				brush.setStyle(Qt::FDiagPattern);
			break;
			case HS_HORIZONTAL:
				brush.setStyle(Qt::HorPattern);
			break;
			case HS_VERTICAL:
				brush.setStyle(Qt::VerPattern);
			break;
			}
	break;
	case BS_PATTERN:
		Q_ASSERT( 0 );
	break;
	case BS_INDEXED:
		Q_ASSERT( 0 );
	break;
	case BS_DIBPATTERN:
		Q_ASSERT( 0 );
	break;
	case BS_DIBPATTERNPT:
		Q_ASSERT( 0 );
	break;
	case BS_PATTERN8X8:
		Q_ASSERT( 0 );
	break;
	case BS_DIBPATTERN8X8:
		Q_ASSERT( 0 );
	break;
	case BS_MONOPATTERN:
		Q_ASSERT( 0 );
	break;
	default:
		Q_ASSERT( 0 );
	}

	brush.setColor(QColor(red, green, blue));

	// TODO: Handle the BrushHatch enum.

	m_objectTable.insert( ihBrush, brush );
}

void QEmfRenderer::createMonoBrush( quint32 ihBrush, Bitmap *bitmap )
{
	m_objectTable.insert(ihBrush, QBrush(bitmap->image()));
}

void QEmfRenderer::createDibPatternBrushPT(quint32 ihBrush, Bitmap *bitmap)
{
	m_objectTable.insert(ihBrush, QBrush(bitmap->image()));
}

void QEmfRenderer::extCreateFontIndirectW( const ExtCreateFontIndirectWRecord &extCreateFontIndirectW )
{
	QFont font(extCreateFontIndirectW.fontFace());
	font.setWeight( convertFontWeight( extCreateFontIndirectW.weight() ) );

	if ( extCreateFontIndirectW.height() < 0 ) {
		font.setPixelSize( -1 * extCreateFontIndirectW.height() );
	} else if ( extCreateFontIndirectW.height() > 0 ) {
		font.setPixelSize( extCreateFontIndirectW.height() );
	} // zero is "use a default size" which is effectively no-op here.

	// .snp files don't always provide 0x01 for italics
	if ( extCreateFontIndirectW.italic() != 0x00 ) {
	font.setItalic( true );
	}

	if ( extCreateFontIndirectW.underline() != 0x00 ) {
	font.setUnderline( true );
	}

	m_textRotation = extCreateFontIndirectW.orientation();

	m_objectTable.insert( extCreateFontIndirectW.ihFonts(), font );
}

void QEmfRenderer::selectStockObject( const quint32 ihObject )
{
#if DEBUG_EMFPAINT
	qDebug() << ihObject;
#endif

	switch ( ihObject ) {
	case WHITE_BRUSH:
	m_painter->setBrush( QBrush(QColor(Qt::white)));
	break;
	case LTGRAY_BRUSH:
	m_painter->setBrush( QBrush(QColor(Qt::lightGray)));
	break;
	case GRAY_BRUSH:
	m_painter->setBrush( QBrush(QColor(Qt::gray)));
	break;
	case DKGRAY_BRUSH:
	m_painter->setBrush( QBrush(QColor(Qt::darkGray)));
	break;
	case BLACK_BRUSH:
	m_painter->setBrush( QBrush(QColor(Qt::black)));
	break;
	case NULL_BRUSH:
	m_painter->setBrush( QBrush() );
	break;
	case WHITE_PEN:
	m_painter->setPen( QPen(QColor(Qt::white)));
	break;
	case BLACK_PEN:
	m_painter->setPen( QPen(QColor(Qt::black)));
	break;
	case NULL_PEN:
	m_painter->setPen( QPen( Qt::NoPen ) );
	break;
	case OEM_FIXED_FONT:
	case ANSI_FIXED_FONT:
	case SYSTEM_FIXED_FONT:
		{
			QFont  font(QString("Fixed"));
			m_painter->setFont(font);
			break;
		}
	case ANSI_VAR_FONT:
	case DEFAULT_GUI_FONT:      // Not sure if this is true, but it should work well
		{
			QFont  font(QString("Helvetica")); // Could also be "System"
			m_painter->setFont(font);
			break;
		}
	break;
	case SYSTEM_FONT:
	// TODO: handle this
	break;
	case DEVICE_DEFAULT_FONT:
	// TODO: handle this
	break;
	case DEFAULT_PALETTE:
	break;
	case DC_BRUSH:
		// FIXME
	break;
	case DC_PEN:
		// FIXME
	break;
	default:
	qWarning() << "Unexpected stock object:" << ( ihObject & 0x8000000 );
	}
}

void QEmfRenderer::selectObject( const quint32 ihObject )
{
#if DEBUG_EMFPAINT
	qDebug() << hex << ihObject << dec;
#endif

	if ( ihObject & 0x80000000 ) {
	selectStockObject( ihObject );
	} else {
		if (m_objectTable.isEmpty())
			return;

	QVariant obj = m_objectTable.value( ihObject );

	switch ( obj.type() ) {
	case QVariant::Pen :
		m_painter->setPen( obj.value<QPen>() );
	break;
	case QVariant::Brush :
		m_painter->setBrush( obj.value<QBrush>() );
	break;
	case QVariant::Font :
		m_painter->setFont( obj.value<QFont>() );
	break;
	default:
		qDebug() << "Unexpected type:" << obj.typeName() << ihObject;
		break;
	}
	}
}

void QEmfRenderer::deleteObject( const quint32 ihObject )
{
	m_objectTable.take( ihObject );
}

void QEmfRenderer::arc( const QRect &box, const QPoint &start, const QPoint &end )
{
#if DEBUG_EMFPAINT
	qDebug() << box << start << end;
#endif

	QPoint centrePoint = box.center();

	qreal startAngle = angleFromArc( centrePoint, start );
	qreal endAngle   = angleFromArc( centrePoint, end );
	qreal spanAngle  = angularSpan( startAngle, endAngle );

	m_painter->drawArc( box, startAngle*16, spanAngle*16 );
}

void QEmfRenderer::chord( const QRect &box, const QPoint &start, const QPoint &end )
{
#if DEBUG_EMFPAINT
	qDebug() << box << start << end;
#endif

	QPoint centrePoint = box.center();

	qreal startAngle = angleFromArc( centrePoint, start );
	qreal endAngle   = angleFromArc( centrePoint, end );
	qreal spanAngle  = angularSpan( startAngle, endAngle );

	m_painter->drawChord( box, startAngle*16, spanAngle*16 );
}

void QEmfRenderer::pie( const QRect &box, const QPoint &start, const QPoint &end )
{
#if DEBUG_EMFPAINT
	qDebug() << box << start << end;
#endif

	QPoint centrePoint = box.center();

	qreal startAngle = angleFromArc( centrePoint, start );
	qreal endAngle   = angleFromArc( centrePoint, end );
	qreal spanAngle  = angularSpan( startAngle, endAngle );

	m_painter->drawPie( box, startAngle*16, spanAngle*16 );
}

void QEmfRenderer::ellipse( const QRect &box )
{
#if DEBUG_EMFPAINT
	qDebug() << box;
#endif

	m_painter->drawEllipse( box );
}

void QEmfRenderer::rectangle( const QRect &box )
{
#if DEBUG_EMFPAINT
	qDebug() << box;
#endif

	m_painter->drawRect( box );
}

void QEmfRenderer::setMapMode( const quint32 mapMode )
{
#if DEBUG_EMFPAINT
	qDebug() << "Set map mode:" << mapMode;
#endif

	m_mapMode = (MapMode)mapMode;
}

void QEmfRenderer::setBkMode( const quint32 backgroundMode )
{
#if DEBUG_EMFPAINT
	qDebug() << backgroundMode;
#endif

	if ( backgroundMode == TRANSPARENT ) {
		m_painter->setBackgroundMode( Qt::TransparentMode );
	} else if ( backgroundMode == OPAQUE ) {
		m_painter->setBackgroundMode( Qt::OpaqueMode );
	} else {
		qDebug() << "EMR_SETBKMODE: Unexpected value -" << backgroundMode;
		Q_ASSERT( 0 );
	}
}

void QEmfRenderer::setPolyFillMode( const quint32 polyFillMode )
{
#if DEBUG_EMFPAINT
	qDebug() << polyFillMode;
#endif

	if ( polyFillMode == ALTERNATE ) {
	m_fillRule = Qt::OddEvenFill;
	} else if ( polyFillMode == WINDING ) {
	m_fillRule = Qt::WindingFill;
	} else {
	qDebug() << "EMR_SETPOLYFILLMODE: Unexpected value -" << polyFillMode;
	Q_ASSERT( 0 );
	}
}

void QEmfRenderer::setLayout( const quint32 layoutMode )
{
#if DEBUG_EMFPAINT
	qDebug() << layoutMode;
#endif

	if ( layoutMode == LAYOUT_LTR ) {
		m_painter->setLayoutDirection( Qt::LeftToRight );
	} else if ( layoutMode == LAYOUT_RTL ) {
		m_painter->setLayoutDirection( Qt::RightToLeft );
	} else {
		qDebug() << "EMR_SETLAYOUT: Unexpected value -" << layoutMode;
		Q_ASSERT( 0 );
	}
}

void QEmfRenderer::setTextAlign( const quint32 textAlignMode )
{
#if DEBUG_EMFPAINT
	qDebug() << textAlignMode;
#endif

	m_textAlignMode = textAlignMode;
}

void QEmfRenderer::setTextColor( const quint8 red, const quint8 green, const quint8 blue,
										  const quint8 reserved )
{
	Q_UNUSED( reserved );

#if DEBUG_EMFPAINT
	qDebug() << red << green << blue << reserved;
#endif

	m_textPen.setColor( QColor( red, green, blue ) );
}

void QEmfRenderer::setBkColor( const quint8 red, const quint8 green, const quint8 blue,
										const quint8 reserved )
{
	Q_UNUSED( reserved );

#if DEBUG_EMFPAINT
	qDebug() << red << green << blue << reserved;
#endif

	m_painter->setBackground( QBrush( QColor( red, green, blue ) ) );
}


#define DEBUG_TEXTOUT 0

void QEmfRenderer::extTextOut( const QRect &bounds, const EmrTextObject &textObject )
{
	const QPoint  &referencePoint = textObject.referencePoint();
	const QString &text = textObject.textString();

#if DEBUG_EMFPAINT
	qDebug() << "Ref point: " << referencePoint
				  << "options: " << hex << textObject.options() << dec
				  << "rectangle: " << bounds
				  << "text: " << textObject.textString();
#endif

	int x = referencePoint.x();
	int y = referencePoint.y();

	// The TA_UPDATECP flag tells us to use the current position
	if (m_textAlignMode & TA_UPDATECP){
		// (left, top) position = current logical position
		x = m_currentCoords.x();
		y = m_currentCoords.y();
	#if DEBUG_EMFPAINT
		qDebug() << "TA_UPDATECP: use current logical position" << x << y;
	#endif
	} else if (bounds.isNull()){// (left, top) position = current logical position
		if (referencePoint == QPoint(0, -1))
			x = m_currentCoords.x();
		if (y < 0)
			y = m_currentCoords.y();
	#if DEBUG_EMFPAINT
		qDebug() << "Invalid bounds -> use current logical position" << x << y;
	#endif
	}

	QFontMetrics fm = m_painter->fontMetrics();
	int textWidth  = fm.width(text);
	int textHeight = fm.height();

	// Make (x, y) be the coordinates of the upper left corner of the
	// rectangle surrounding the text.
	//
	// FIXME: Handle RTL text.

	// Horizontal align.  Default is TA_LEFT.
	if ((m_textAlignMode & TA_HORZMASK) == TA_CENTER)
		x -= (textWidth / 2);
	else if ((m_textAlignMode & TA_HORZMASK) == TA_RIGHT)
		x -= textWidth;

	// Vertical align.  Default is TA_TOP
	if ((m_textAlignMode & TA_VERTMASK) == TA_BASELINE)
		y -= (textHeight - fm.descent());
	else if ((m_textAlignMode & TA_VERTMASK) == TA_BOTTOM) {
		y -= textHeight;
	}

#if DEBUG_EMFPAINT
	qDebug() << "textWidth = " << textWidth << "height = " << textHeight;

	qDebug() << "font = " << m_painter->font()
				  << "pointSize = " << m_painter->font().pointSize()
				  << "ascent = " << fm.ascent() << "descent = " << fm.descent()
				  << "height = " << fm.height()
				  << "leading = " << fm.leading();
	qDebug() << "actual point = " << x << y;
#endif

	// Debug code that paints a rectangle around the output area.
#if DEBUG_TEXTOUT
	m_painter->save();
	m_painter->setPen(Qt::black);
	m_painter->drawRect(QRect(x, y, textWidth, textHeight));
	if (bounds.isValid()){
		m_painter->setPen(Qt::red);
		m_painter->drawRect(bounds);
	}
	m_painter->restore();
#endif

	// Actual painting starts here.
	m_painter->save();

	// Find out how much we have to scale the text to make it fit into
	// the output rectangle.  Normally this wouldn't be necessary, but
	// when fonts are switched, the replacement fonts are sometimes
	// wider than the original fonts.
	//QRect worldRect(m_worldTransform.mapRect(QRect(x, y, textWidth, textHeight)));
	//qDebug() << "rects:" << QRect(x, y, textWidth, textHeight) << bounds;
	qreal scaleX = qreal(1.0);
	qreal scaleY = qreal(1.0);
	if (bounds.isValid()){
		/*if ((bounds.width() < worldRect.width()))
			scaleX = qreal(bounds.width()) / qreal(worldRect.width());
		if (bounds.height() < worldRect.height())
			scaleY = qreal(bounds.height()) / qreal(worldRect.height());*/
		if ((bounds.width() < textWidth))
			scaleX = qreal(bounds.width()) / qreal(textWidth);
		if (bounds.height() < textHeight)
			scaleY = qreal(bounds.height()) / qreal(textHeight);
	}
#if DEBUG_EMFPAINT
	qDebug() << "scale:" << scaleX << scaleY;
#endif

	if ((m_textRotation == .0) && (scaleX < qreal(1.0) || scaleY < qreal(1.0))){
		m_painter->translate(-x, -y);
		m_painter->scale(scaleX, scaleY);
		m_painter->translate(x / scaleX, y / scaleY);
	}

	// Use the special pen defined by mTextPen for text.
	QPen savePen = m_painter->pen();
	m_painter->setPen(m_textPen);
	if (m_textRotation != .0){
		m_painter->setWorldTransform(m_outputTransform);
		m_painter->translate(referencePoint.x(), referencePoint.y());
		m_painter->rotate(m_textRotation);
		if (scaleX < qreal(1.0))
			m_painter->scale((1.0 - scaleX), 1.0);
		m_painter->drawText(0, 0, text);
	} else
		m_painter->drawText(int(x / scaleX), int(y / scaleY), textWidth, textHeight, Qt::AlignLeft | Qt::AlignTop, text);
	m_painter->setPen(savePen);

	m_painter->restore();
}

void QEmfRenderer::moveToEx( const qint32 x, const qint32 y )
{
#if DEBUG_EMFPAINT
	qDebug() << x << y;
#endif

	if ( m_currentlyBuildingPath ){
		if (!m_pathTransform.isIdentity())
			m_path->moveTo(m_pathTransform.map(QPoint(x, y)));
		else
			m_path->moveTo( QPoint( x, y ) );
	} else
		m_currentCoords = QPoint( x, y );
}

void QEmfRenderer::lineTo( const QPoint &finishPoint )
{
#if DEBUG_EMFPAINT
	qDebug() << finishPoint;
#endif

	if ( m_currentlyBuildingPath )
		m_path->lineTo(m_pathTransform.map(finishPoint));
	else {
		m_painter->drawLine( m_currentCoords, finishPoint );
		m_currentCoords = finishPoint;
	}
}

void QEmfRenderer::arcTo( const QRect &box, const QPoint &start, const QPoint &end )
{
#if DEBUG_EMFPAINT
	qDebug() << box << start << end;
#endif

	QPoint centrePoint = box.center();

	qreal startAngle = angleFromArc( centrePoint, start );
	qreal endAngle   = angleFromArc( centrePoint, end );
	qreal spanAngle  = angularSpan( startAngle, endAngle );

	m_path->arcTo( box, startAngle, spanAngle );
}

void QEmfRenderer::polygon16( const QRect &bounds, const QList<QPoint> points )
{
	Q_UNUSED( bounds );

#if DEBUG_EMFPAINT
	qDebug() << bounds << points;
#endif

	QVector<QPoint> pointVector = points.toVector();

	if (m_currentlyBuildingPath && !m_pathTransform.isIdentity())
		m_path->addPolygon(m_pathTransform.map(pointVector));

	m_painter->drawPolygon(pointVector.constData(), pointVector.size(), m_fillRule);
}

void QEmfRenderer::polyLine( const QRect &bounds, const QList<QPoint> points )
{
	Q_UNUSED( bounds );

#if DEBUG_EMFPAINT
	qDebug() << bounds << points;
#endif

	QVector<QPoint> pointVector = points.toVector();
	m_painter->drawPolyline( pointVector.constData(), pointVector.size() );
}

void QEmfRenderer::polyLine16( const QRect &bounds, const QList<QPoint> points )
{
#if DEBUG_EMFPAINT
	qDebug() << bounds << points;
#endif

	polyLine( bounds, points );
}

void QEmfRenderer::polyPolygon16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
	Q_UNUSED( bounds );

#if DEBUG_EMFPAINT
	qDebug() << bounds << points;
#endif

	for ( int i = 0; i < points.size(); ++i ) {
		m_painter->drawPolygon( points[i].constData(), points[i].size(), m_fillRule );
	}
}

void QEmfRenderer::polyPolyLine16( const QRect &bounds, const QList< QVector< QPoint > > &points )
{
	Q_UNUSED( bounds );

#if DEBUG_EMFPAINT
	qDebug() << bounds << points;
#endif

	for ( int i = 0; i < points.size(); ++i ) {
		m_painter->drawPolyline( points[i].constData(), points[i].size() );
	}
}

void QEmfRenderer::polyLineTo16( const QRect &bounds, const QList<QPoint> points )
{
	Q_UNUSED( bounds );

#if DEBUG_EMFPAINT
	qDebug() << bounds << points;
#endif

	for ( int i = 0; i < points.count(); ++i ) {
	m_path->lineTo( points[i] );
	}
}

void QEmfRenderer::polyBezier16( const QRect &bounds, const QList<QPoint> points )
{
#if DEBUG_EMFPAINT
	qDebug() << bounds << points;
#endif

	Q_UNUSED( bounds );

	if (m_currentlyBuildingPath){
		QTransform m = m_pathTransform;
		if (!m.isIdentity()){
			m_path->moveTo(m.map(points[0]));
			for (int i = 1; i < points.count(); i += 3){
				m_path->cubicTo(m.map(points[i]), m.map(points[i + 1]), m.map(points[i + 2]));
			}
		} else {
			m_path->moveTo(points[0]);
			for (int i = 1; i < points.count(); i += 3){
				m_path->cubicTo(points[i], points[i + 1], points[i + 2]);
			}
		}
	} else {
		QPainterPath path;
		path.moveTo( points[0] );
		for ( int i = 1; i < points.count(); i+=3 ) {
			path.cubicTo( points[i], points[i+1], points[i+2] );
		}
		m_painter->drawPath( path );
	}
}

void QEmfRenderer::polyBezierTo16( const QRect &bounds, const QList<QPoint> points )
{
#if DEBUG_EMFPAINT
	qDebug() << bounds << points;
#endif

	Q_UNUSED( bounds );

	if (m_currentlyBuildingPath && !m_pathTransform.isIdentity()){
		QList<QPoint> mappedPoints;
        for (QPoint p : points)
			mappedPoints.append(m_pathTransform.map(p));
		for (int i = 0; i < points.count(); i += 3)
			m_path->cubicTo(mappedPoints[i], mappedPoints[i + 1], mappedPoints[i + 2]);
	} else {
		for ( int i = 0; i < points.count(); i+=3 ){
			m_path->cubicTo( points[i], points[i+1], points[i+2] );
		}
	}
}

void QEmfRenderer::fillPath( const QRect &bounds )
{
#if DEBUG_EMFPAINT
	qDebug() << bounds;
#endif

	Q_UNUSED(bounds);

	QBrush br = m_painter->brush();

	if (m_worldTransform.isScaling()){
		QTransform m = br.transform();
		m.scale(1.0/m_worldTransform.m11(), 1.0/m_worldTransform.m22());
		br.setTransform(m);
	}

	m_painter->fillPath(*m_path, br);
}

void QEmfRenderer::strokeAndFillPath( const QRect &bounds )
{
#if DEBUG_EMFPAINT
	qDebug() << bounds;
#endif

	Q_UNUSED(bounds);

	m_painter->drawPath(*m_path);
}

void QEmfRenderer::strokePath( const QRect &bounds )
{
#if DEBUG_EMFPAINT
	qDebug() << bounds;
#endif

	Q_UNUSED( bounds );
	m_painter->strokePath(*m_path, m_painter->pen());
}

void QEmfRenderer::setMitterLimit(const quint32 limit)
{
#if DEBUG_EMFPAINT
	qDebug() << limit;
#endif

	QPen pen = m_painter->pen();
	pen.setMiterLimit(limit);
	m_painter->setPen(pen);
}

void QEmfRenderer::setClipPath( const quint32 regionMode )
{
#if DEBUG_EMFPAINT
	qDebug() << hex << regionMode << dec;
#endif

	switch (regionMode){
	case RGN_AND:
		m_painter->setClipPath(*m_path, Qt::IntersectClip);
		break;
	case RGN_OR:
	#if QT_VERSION < 0x050000
		m_painter->setClipPath(*m_path, Qt::UniteClip);
	#else
		qWarning() <<  "Unexpected / unsupported clip region mode:" << regionMode;
	#endif
		break;
	case RGN_COPY:
		m_painter->setClipPath(*m_path, Qt::ReplaceClip);
		break;
	default:
		qWarning() <<  "Unexpected / unsupported clip region mode:" << regionMode;
		Q_ASSERT( 0 );
	}
}

void QEmfRenderer::bitBlt( BitBltRecord &bitBltRecord )
{
#if DEBUG_EMFPAINT
	qDebug() << bitBltRecord.xDest() << bitBltRecord.yDest()
				  << bitBltRecord.cxDest() << bitBltRecord.cyDest()
				  << hex << bitBltRecord.rasterOperation() << dec
				  << bitBltRecord.bkColorSrc();
#endif

	QRect target(bitBltRecord.xDest(), bitBltRecord.yDest(), bitBltRecord.cxDest(), bitBltRecord.cyDest());
	// 0x00f00021 is the PatCopy raster operation which just fills a rectangle with a brush.
	// This seems to be the most common one.
	//
	// FIXME: Implement the rest of the raster operations.

	quint32 rasterOp = bitBltRecord.rasterOperation();
	if (rasterOp == 0x00f00021){
		// Would have been nice if we didn't have to pull out the
		// brush to use it with fillRect()...
		QBrush brush = m_painter->brush();
		m_painter->fillRect(target, brush);
	} else if (rasterOp == 0x00AA0029){
		QBrush brush = m_painter->brush();
		//if (brush.style() != Qt::NoBrush)
			//printf("rasterOperation = 0x%X style: %d\n", bitBltRecord.rasterOperation(), brush.style());
		//m_painter->fillRect(target, brush);
	} else if (bitBltRecord.hasImage())
		m_painter->drawImage(target, bitBltRecord.image());
}

void QEmfRenderer::setStretchBltMode( const quint32 stretchMode )
{
	Q_UNUSED(stretchMode);

#if DEBUG_EMFPAINT
	qDebug() << hex << stretchMode << dec;

	switch ( stretchMode ) {
	case 0x01:
		qDebug() << "EMR_STRETCHBLTMODE: STRETCH_ANDSCANS";
		break;
	case 0x02:
		qDebug() << "EMR_STRETCHBLTMODE: STRETCH_ORSCANS";
		break;
	case 0x03:
		qDebug() << "EMR_STRETCHBLTMODE: STRETCH_DELETESCANS";
		break;
	case 0x04:
		qDebug() << "EMR_STRETCHBLTMODE: STRETCH_HALFTONE";
		break;
	default:
		qDebug() << "EMR_STRETCHBLTMODE - unknown stretch mode:" << stretchMode;
	}
#endif
}

void QEmfRenderer::stretchDiBits(StretchDiBitsRecord &record)
{
	if (record.bitCount() == BI_BITCOUNT_1){
		m_current_bitmap_mask = record.mask();

		QVector<QRgb> maskColorTable = record.maskColorTable();
		if (!maskColorTable.isEmpty())
			m_current_mask_color = QColor(maskColorTable.first());

		return;
	}

#if DEBUG_EMFPAINT
	qDebug() << "Bounds:    " << record.bounds();
	qDebug() << "Dest rect: "
				  << record.xDest() << record.yDest() << record.cxDest() << record.cyDest();
	qDebug() << "Src rect:  "
				  << record.xSrc() << record.ySrc() << record.cxSrc() << record.cySrc();
	qDebug() << "Raster op: " << hex << record.rasterOperation() << dec;
				  //<< record.bkColorSrc();
	qDebug() << "usageSrc: " << record.usageSrc();
#endif

	QPoint targetPosition( record.xDest(), record.yDest() );
	QSize  targetSize( record.cxDest(), record.cyDest() );

	QPoint sourcePosition( record.xSrc(), record.ySrc() );
	QSize  sourceSize( record.cxSrc(), record.cySrc() );

	// special cases, from [MS-EMF] Section 2.3.1.7:
	// "This record specifies a mirror-image copy of the source bitmap to the
	// destination if the signs of the height or width fields differ. That is,
	// if cxSrc and cxDest have different signs, this record specifies a mirror
	// image of the source bitmap along the x-axis. If cySrc and cyDest have
	// different signs, this record specifies a mirror image of the source
	//  bitmap along the y-axis."
	QRect target( targetPosition, targetSize );
	QRect source( sourcePosition, sourceSize );

	QImage image = record.image();

#if DEBUG_EMFPAINT
	qDebug() << "image size" << image.size();
	qDebug() << "Before transformation:";
	qDebug() << "    target" << target;
	qDebug() << "    source" << source;
#endif
	if ( source.width() < 0 && target.width() > 0 ) {
		sourceSize.rwidth() *= -1;
		sourcePosition.rx() -= sourceSize.width();
		source = QRect( sourcePosition, sourceSize );
	}
	if ( source.width() > 0 && target.width() < 0 ) {
		targetSize.rwidth() *= -1;
		targetPosition.rx() -= targetSize.width();
		target = QRect( targetPosition, targetSize );
	}
	if ( source.height() < 0 && target.height() > 0 ) {
		sourceSize.rheight() *= -1;
		sourcePosition.ry() -= sourceSize.height();
		source = QRect( sourcePosition, sourceSize );
	}
	if ( source.height() > 0 && target.height() < 0 ) {
		targetSize.rheight() *= -1;
		targetPosition.ry() -= targetSize.height();
		target = QRect( targetPosition, targetSize );
	}

#if DEBUG_EMFPAINT
	qDebug() << "After transformation:";
	qDebug() << "    target" << target;
	qDebug() << "    source" << source;
	qDebug() << "Image" << image.size();
#endif

/*
#ifndef Q_OS_LINUX //otherwise on Linux images are all black: need to investigate composition modes
	QPainter::RenderHints      oldRenderHints = m_painter->renderHints();
	QPainter::CompositionMode  oldCompMode    = m_painter->compositionMode();
	m_painter->setRenderHints(0);// Antialiasing makes composition modes invalid
	m_painter->setCompositionMode(rasteropToQtComposition(record.rasterOperation()));
#endif
*/
	if (!m_current_bitmap_mask.isNull()){
		QPixmap pix = QPixmap::fromImage(image);
		pix.setMask(m_current_bitmap_mask.createMaskFromColor(m_current_mask_color));
		m_painter->drawPixmap(target, pix, source);
		m_current_bitmap_mask.clear();
	} else if (record.hasImage())
		m_painter->drawImage(target, image, source);

/*
#ifndef Q_OS_LINUX
	m_painter->setCompositionMode(oldCompMode);
	m_painter->setRenderHints(oldRenderHints);
#endif
*/
}

void QEmfRenderer::alphaBlend(AlphaBlendRecord& record)
{
	QImage image = record.image();
	QRect target = record.destinationRectangle();
	QRect source = record.sourceRectangle();

#if DEBUG_EMFPAINT
	qDebug() << "image size" << image.size();
	qDebug() << "    target" << target;
	qDebug() << "    source" << source;
#endif

	if (record.hasImage())
		m_painter->drawImage(target, image, source);
}

// ----------------------------------------------------------------
//                         Private functions


void QEmfRenderer::printPainterTransform(const char *leadText)
{
	QTransform  transform;

	recalculateWorldTransform();

	qDebug() << leadText << "world transform " << m_worldTransform
				  << "incl output transform: " << m_painter->transform();
}


qreal QEmfRenderer::angleFromArc( const QPoint &centrePoint, const QPoint &radialPoint )
{
	double dX = radialPoint.x() - centrePoint.x();
	double dY = centrePoint.y() - radialPoint.y();
	// Qt angles are in degrees. atan2 returns radians
	return ( atan2( dY, dX ) * 180 / M_PI );
}

qreal QEmfRenderer::angularSpan( const qreal startAngle, const qreal endAngle )
{
	qreal spanAngle = endAngle - startAngle;

	if ( spanAngle <= 0 ) {
		spanAngle += 360;
	}

	return spanAngle;
}

int QEmfRenderer::convertFontWeight( quint32 emfWeight )
{
	// FIXME: See how it's done in the wmf library and check if this is suitable here.

	if ( emfWeight == 0 ) {
		return QFont::Normal;
	} else if ( emfWeight <= 200 ) {
		return QFont::Light;
	} else if ( emfWeight <= 450 ) {
		return QFont::Normal;
	} else if ( emfWeight <= 650 ) {
		return QFont::DemiBold;
	} else if ( emfWeight <= 850 ) {
		return QFont::Bold;
	} else {
		return QFont::Black;
	}
}

static QPainter::CompositionMode  rasteropToQtComposition(long rop)
{
	// Code copied from filters/libkowmf/qwmf.cc
	// FIXME: Should be cleaned up

	/* TODO: Ternary raster operations
	0x00C000CA  dest = (source AND pattern)
	0x00F00021  dest = pattern
	0x00FB0A09  dest = DPSnoo
	0x005A0049  dest = pattern XOR dest   */
	static const struct OpTab {
		long winRasterOp;
		QPainter::CompositionMode qtRasterOp;
	}

	opTab[] = {
		// ### untested (conversion from Qt::RasterOp)
		{ 0x00CC0020, QPainter::CompositionMode_Source }, // CopyROP
		{ 0x00EE0086, QPainter::RasterOp_SourceOrDestination }, // OrROP
		{ 0x008800C6, QPainter::RasterOp_SourceAndDestination }, // AndROP
		{ 0x00660046, QPainter::RasterOp_SourceXorDestination }, // XorROP
		// ----------------------------------------------------------------
		// FIXME: Checked above this, below is still todo
		// ----------------------------------------------------------------
		{ 0x00440328, QPainter::CompositionMode_DestinationOut }, // AndNotROP
		{ 0x00330008, QPainter::CompositionMode_DestinationOut }, // NotCopyROP
		{ 0x001100A6, QPainter::CompositionMode_SourceOut }, // NandROP
		{ 0x00C000CA, QPainter::CompositionMode_Source }, // CopyROP
		{ 0x00BB0226, QPainter::CompositionMode_Destination }, // NotOrROP
		{ 0x00F00021, QPainter::CompositionMode_Source }, // CopyROP
		{ 0x00FB0A09, QPainter::CompositionMode_Source }, // CopyROP
		{ 0x005A0049, QPainter::CompositionMode_Source }, // CopyROP
		{ 0x00550009, QPainter::CompositionMode_DestinationOut }, // NotROP
		{ 0x00000042, QPainter::CompositionMode_Clear }, // ClearROP
		{ 0x00FF0062, QPainter::CompositionMode_Source } // SetROP
	};

	int i;
	for (i = 0 ; i < 15 ; i++)
		if (opTab[i].winRasterOp == rop)
			break;

	if (i < 15)
		return opTab[i].qtRasterOp;
	else
		return QPainter::CompositionMode_Source;
}

} // xnamespace...
