/*
  Copyright 2008 Brad Hards <bradh@frogmouth.net>
  Copyright 2009 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#include "EmfRecords.h"

#include "EmfEnums.h"
#include "EmfObjects.h"
#include "Bitmap.h"

#include <QDebug>

namespace QEmf
{


/*****************************************************************************/


BitBltRecord::BitBltRecord( QDataStream &stream, quint32 recordSize )
	: m_bitmap(0)
{
	//qDebug() << "stream position at the start: " << stream.device()->pos();
	//qDebug() << "record size: " << recordSize;

	stream >> m_bounds;

	stream >> m_xDest;          // x, y of upper left corner of the destination.
	stream >> m_yDest;
	stream >> m_cxDest;         // width, height of the rectangle in logical coords.
	stream >> m_cyDest;
	//qDebug() << "Destination" << m_xDest << m_yDest << m_cxDest << m_cyDest;

	stream >> m_BitBltRasterOperation;
	//qDebug() << "bitblt raster operation:" << hex << m_BitBltRasterOperation << dec;

	stream >> m_xSrc;           // x, y of the source
	stream >> m_ySrc;
	//qDebug() << "Source" << m_xSrc << m_ySrc;

	//qDebug() << "position before the matrix: " << stream.device()->pos();
	stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
	float M11, M12, M21, M22, Dx, Dy;
	stream >> M11;              // Transformation matrix
	stream >> M12;
	stream >> M21;
	stream >> M22;
	stream >> Dx;
	stream >> Dy;
	m_XFormSrc = QTransform( M11, M12, M21, M22, Dx, Dy );
	//qDebug() << "Matrix" << m_XFormSrc;
	//qDebug() << "position after the matrix: " << stream.device()->pos();

	stream >> m_red >> m_green >> m_blue >> m_reserved;
	//qDebug() << "Background color" << m_red << m_green << m_blue << m_reserved;
	//qDebug() << "position after background color: " << stream.device()->pos();

	stream >> m_UsageSrc;
	//qDebug() << "Color table interpretation" << m_UsageSrc;

	stream >> m_offBmiSrc;      // Offset to start of bitmap header from start of record
	stream >> m_cbBmiSrc;       // Size of source bitmap header
	stream >> m_offBitsSrc;     // Offset to source bitmap from start of record
	stream >> m_cbBitsSrc;      // Size of source bitmap
#if 0
	qDebug() << "header offset:" << m_offBmiSrc;
	qDebug() << "header size:  " << m_cbBmiSrc;
	qDebug() << "bitmap offset:" << m_offBitsSrc;
	qDebug() << "bitmap size:  " << m_cbBitsSrc;
#endif

	//qDebug() << "stream position before the image: " << stream.device()->pos();
	if (m_cbBmiSrc > 0) {
		m_bitmap = new Bitmap( stream, recordSize, 8 + 23 * 4, // header + 23 ints
							   m_offBmiSrc, m_cbBmiSrc,
							   m_offBitsSrc, m_cbBitsSrc );
	}

	//qDebug() << "stream position at the end: " << stream.device()->pos();
}

BitBltRecord::~BitBltRecord()
{
	delete m_bitmap;
}

bool BitBltRecord::hasImage() const
{
	return m_bitmap && m_bitmap->hasImage();
	//return ( ( m_cbBmiSrc != 0 ) && ( m_cbBitsSrc != 0 ) );
}

QImage BitBltRecord::image()
{
	return m_bitmap->image();
#if 0
	if ( ! hasImage() ) {
		return 0;
	}

	if ( m_image != 0 ) {
		return m_image;
	}

	QImage::Format format = QImage::Format_Invalid;
	if ( m_BmiSrc->bitCount() == BI_BITCOUNT_4 ) {
		if ( m_BmiSrc->compression() == 0x00 ) {
			format = QImage::Format_RGB555;
		} else {
			qDebug() << "Unexpected compression format for BI_BITCOUNT_4:"
					 << m_BmiSrc->compression();
			Q_ASSERT( 0 );
		}
	} else if ( m_BmiSrc->bitCount() == BI_BITCOUNT_5 ) {
		format = QImage::Format_RGB888;
	} else {
		qDebug() << "Unexpected format:" << m_BmiSrc->bitCount();
		Q_ASSERT( 0 );
	}
	m_image = new QImage( (const uchar*)m_imageData.constData(),
						  m_BmiSrc->width(), m_BmiSrc->height(), format );

	return m_image;
#endif
}

/*****************************************************************************/
StretchDiBitsRecord::StretchDiBitsRecord( QDataStream &stream, quint32 recordSize )
	: m_bitmap(0)
{
	//qDebug() << "stream position at the start: " << stream.device()->pos();
	//qDebug() << "recordSize =" << recordSize;

	stream >> m_Bounds;
	stream >> m_xDest;
	stream >> m_yDest;
	stream >> m_xSrc;
	stream >> m_ySrc;
	stream >> m_cxSrc;
	stream >> m_cySrc;

	stream >> m_offBmiSrc;
	stream >> m_cbBmiSrc;
	stream >> m_offBitsSrc;
	stream >> m_cbBitsSrc;

	stream >> m_UsageSrc;       // How to interpret color table values.
	stream >> m_BitBltRasterOperation;
	stream >> m_cxDest;
	stream >> m_cyDest;
#if 0
	qDebug() << "bounds:" << m_Bounds;
	qDebug() << "destination:" << QPoint(m_xDest, m_yDest) << QSize(m_cxDest, m_cyDest);
	qDebug() << "source:" << QPoint(m_xSrc, m_ySrc) << QSize(m_cxSrc, m_cySrc);
	qDebug() << "header offset:" << m_offBmiSrc;
	qDebug() << "header size:  " << m_cbBmiSrc;
	qDebug() << "bitmap offset:" << m_offBitsSrc;
	qDebug() << "bitmap size:  " << m_cbBitsSrc;

	qDebug() << "m_BitBltRasterOperation =" << hex << m_BitBltRasterOperation << dec;
#endif

	//qDebug() << "stream position before the image: " << stream.device()->pos();
	if (m_cbBmiSrc > 0) {
		m_bitmap = new Bitmap( stream, recordSize, 8 + 18 * 4, // header + 18 ints
							   m_offBmiSrc, m_cbBmiSrc,
							   m_offBitsSrc, m_cbBitsSrc );
	}
	//qDebug() << "stream position at the end: " << stream.device()->pos();
#if 0
	// Read away those bytes that preceed the header.  These are undefined
	// according to the spec.  80 is the size of the record above.
	qint32 dummy;
	int    padding = 0;
	while (m_offBmiSrc - padding > 80) {
		stream >> dummy;
		padding += 4;
	}
	m_BmiSrc = new BitmapHeader( stream, m_cbBmiSrc );

	// 40 is the size of the header record.
	while (m_offBitsSrc - padding > 80 + 40) {
		stream >> dummy;
		padding += 4;
	}
	m_imageData.resize( m_cbBitsSrc );
	stream.readRawData( m_imageData.data(), m_cbBitsSrc );
#endif
}

StretchDiBitsRecord::~StretchDiBitsRecord()
{
	if (m_bitmap)
		delete m_bitmap;
}

QRect StretchDiBitsRecord::bounds() const
{
	return m_Bounds;
}

bool StretchDiBitsRecord::hasImage() const
{
	if (!m_bitmap)
		return false;
	return m_bitmap->hasImage();
}

quint16 StretchDiBitsRecord::bitCount() const
{
	if (!m_bitmap)
		return -1;
	return m_bitmap->header()->bitCount();
}

QBitmap StretchDiBitsRecord::mask()
{
	return m_bitmap ? m_bitmap->mask() : QBitmap();
}

QVector<QRgb> StretchDiBitsRecord::maskColorTable()
{
	return m_bitmap ? m_bitmap->header()->colorTable() : QVector<QRgb>();
}

QImage StretchDiBitsRecord::image()
{
	return m_bitmap ? m_bitmap->image() : QImage();
#if 0
	if ( m_image != 0 ) {
		return m_image;
	}

	QImage::Format format = QImage::Format_Invalid;

	// Start by determining which QImage format we are going to use.
	if (m_BmiSrc->bitCount() == BI_BITCOUNT_1) {
		format = QImage::Format_Mono;
	} else if ( m_BmiSrc->bitCount() == BI_BITCOUNT_4 ) {
		if ( m_BmiSrc->compression() == BI_RGB ) {
			format = QImage::Format_RGB555;
		} else {
			qDebug() << "Unexpected compression format for BI_BITCOUNT_4:"
						  << m_BmiSrc->compression();
			Q_ASSERT( 0 );
		}
	} else if ( m_BmiSrc->bitCount() == BI_BITCOUNT_5 ) {
		format = QImage::Format_RGB888;
	} else {
		qDebug() << "Unexpected format:" << m_BmiSrc->bitCount();
		//Q_ASSERT(0);
	}

	// According to MS-WMF 2.2.2.3, the sign of the height decides if
	// this is a compressed bitmap or not.
	if (m_BmiSrc->height() > 0) {
		// This bitmap is a top-down bitmap without compression.
		m_image = new QImage( (const uchar*)m_imageData.constData(),
							  m_BmiSrc->width(), m_BmiSrc->height(), format );

		// The WMF images are in the BGR color order.
		if (format == QImage::Format_RGB888)
			*m_image = m_image->rgbSwapped();

		// We have to mirror this bitmap in the X axis since WMF images are stored bottom-up.
		*m_image = m_image->mirrored(false, true);
	} else {
		// This bitmap is a bottom-up bitmap which uses compression.
		switch (m_BmiSrc->compression()) {
		case BI_RGB:
			m_image = new QImage( (const uchar*)m_imageData.constData(),
								  m_BmiSrc->width(), -m_BmiSrc->height(), format );
			// The WMF images are in the BGR color order.
			*m_image = m_image->rgbSwapped();
			break;

		// These compressions are not yet supported, so return an empty image.
		case BI_RLE8:
		case BI_RLE4:
		case BI_BITFIELDS:
		case BI_JPEG:
		case BI_PNG:
		case BI_CMYK:
		case BI_CMYKRLE8:
		case BI_CMYKRLE4:
		default:
			m_image = new QImage(m_BmiSrc->width(), m_BmiSrc->height(), format);
			break;
		}
	}

	return m_image;
#endif
}

/*****************************************************************************/
AlphaBlendRecord::AlphaBlendRecord(QDataStream &stream, quint32 recordSize)
	: m_bitmap(0)
{
	qint32 startPos = stream.device()->pos();

	stream >> m_Bounds;
	stream >> m_xDest;
	stream >> m_yDest;
	stream >> m_cxDest;
	stream >> m_cyDest;

	// Read BLENDFUNCTION structure
	stream >> m_BlendOp;
	stream >> m_BlendFlags;
	stream >> m_SrcConstantAlpha;
	stream >> m_AlphaFormat;

	stream >> m_xSrc;
	stream >> m_ySrc;

	// Read XFORM structure
	stream.setFloatingPointPrecision(QDataStream::SinglePrecision);
	float M11, M12, M21, M22, Dx, Dy;
	stream >> M11;              // Transformation matrix
	stream >> M12;
	stream >> M21;
	stream >> M22;
	stream >> Dx;
	stream >> Dy;
	m_XFormSrc = QTransform(M11, M12, M21, M22, Dx, Dy);

	stream >> m_BkColorSrc;
	stream >> m_UsageSrc;       // How to interpret color table values.
	stream >> m_offBmiSrc;
	stream >> m_cbBmiSrc;
	stream >> m_offBitsSrc;
	stream >> m_cbBitsSrc;
	stream >> m_cxSrc;
	stream >> m_cySrc;

#ifdef DEBUG_EMFPARSER
	qDebug() << "stream position at the start: " << startPos;
	qDebug() << "recordSize =" << recordSize;
	qDebug() << "bounds:" << m_Bounds;
	qDebug() << "destination:" << QPoint(m_xDest, m_yDest) << QSize(m_cxDest, m_cyDest);
	qDebug() << "source:" << QPoint(m_xSrc, m_ySrc) << QSize(m_cxSrc, m_cySrc);
	qDebug() << "SrcConstantAlpha: " << m_SrcConstantAlpha;
	qDebug() << "UsageSrc:" << m_UsageSrc;
	qDebug() << "XformSrc" << m_XFormSrc;
	qDebug() << "header offset (offBmiSrc): " << m_offBmiSrc;
	qDebug() << "header size (cbBmiSrc): " << m_cbBmiSrc;
	qDebug() << "bitmap offset (offBitsSrc): " << m_offBitsSrc;
	qDebug() << "bitmap size (cbBitsSrc): " << m_cbBitsSrc;
	qDebug() << "stream position before the image: " << stream.device()->pos();
#endif

	if (m_cbBmiSrc > 0){
		qint32 usedBytes = stream.device()->pos() - startPos + 8;//we must add the header size (8)
		m_bitmap = new Bitmap(stream, recordSize, usedBytes, m_offBmiSrc, m_cbBmiSrc, m_offBitsSrc, m_cbBitsSrc);
	}

#ifdef DEBUG_EMFPARSER
	qDebug() << "stream position at the end: " << stream.device()->pos();
#endif
}

bool AlphaBlendRecord::hasImage() const
{
	if (!m_bitmap)
		return false;
	return m_bitmap->hasImage();
}

QImage AlphaBlendRecord::image()
{
	return m_bitmap ? m_bitmap->image((m_SrcConstantAlpha == 255) ? QImage::Format_ARGB32_Premultiplied : QImage::Format_ARGB32) : QImage();
}

AlphaBlendRecord::~AlphaBlendRecord()
{
	if (m_bitmap)
		delete m_bitmap;
}

/*****************************************************************************/
ExtCreateFontIndirectWRecord::ExtCreateFontIndirectWRecord( QDataStream &stream, quint32 size )
{
	stream >> m_ihFonts;
	size -= 12;

	// TODO: Check size, we might need to do a LogFontExDv parse
	stream >> m_height;
	stream >> m_width;
	size -= 8;

	stream >> m_escapement;
	size -= 4;

	stream >> m_orientation;
	size -= 4;

	stream >> m_weight;
	size -= 4;

	stream >> m_italic;
	stream >> m_underline;
	stream >> m_strikeout;
	stream >> m_charSet;
	size -= 4;

	stream >> m_outPrecision;
	stream >> m_clipPrecision;
	stream >> m_quality;
	stream >> m_pitchAndFamily;
	size -= 4;

	QChar myChar[64];
	for ( int i = 0; i < 32; ++i ) {
	stream >> myChar[i];
	}
	size -= 64;

	for ( int i = 0; i < 32; ++i ) {
	if ( ! myChar[i].isNull() ) {
		m_facename.append( myChar[i] );
	}
	}

#if 0
	for ( int i = 0; i < 64; ++i ) {
	stream >> myChar[i];
	}
	size -= 128;

	for ( int i = 0; i < 64; ++i ) {
	if ( ! myChar[i].isNull() ) {
		m_fullName.append( myChar[i] );
	}
	}
	qDebug() << "fullName:" << m_fullName;

	for ( int i = 0; i < 32; ++i ) {
	stream >> myChar[i];
	}
	size -= 64;
	for ( int i = 0; i < 32; ++i ) {
	if ( ! myChar[i].isNull() ) {
		m_style.append( myChar[i] );
	}
	}
	qDebug() << "style:" << m_style;

	for ( int i = 0; i < 32; ++i ) {
	stream >> myChar[i];
	}
	size -= 64;
	for ( int i = 0; i < 32; ++i ) {
	if ( ! myChar[i].isNull() ) {
		m_script.append( myChar[i] );
	}
	}
	qDebug() << "script:" << m_script;
#endif
	soakBytes( stream, size ); // rest of the record.
}

ExtCreateFontIndirectWRecord::~ExtCreateFontIndirectWRecord()
{
}

void ExtCreateFontIndirectWRecord::soakBytes( QDataStream &stream, int numBytes )
{
	quint8 scratch;
	for ( int i = 0; i < numBytes; ++i ) {
		stream >> scratch;
	}
}

}
