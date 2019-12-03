/*
 * Copyright 2010 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#include "Bitmap.h"

#include <QDataStream>
#include <QDebug>
#include <QPixmap>

#include "EmfEnums.h"


namespace QEmf
{

static void soakBytes( QDataStream &stream, int numBytes )
{
	quint8 scratch;
	for ( int i = 0; i < numBytes; ++i ) {
		stream >> scratch;
	}
}


Bitmap::Bitmap( QDataStream &stream,
				quint32 recordSize,  // total size of the EMF record
				quint32 usedBytes,   // used bytes of the EMF record before the bitmap part
				quint32 offBmiSrc, // offset to start of bitmapheader
				quint32 cbBmiSrc,  // size of bitmap header
				quint32 offBitsSrc,// offset to source bitmap
				quint32 cbBitsSrc) // size of source bitmap
	: m_hasImage(false)
	, m_header(0)
	, m_mask(QBitmap())
	, m_imageIsValid(false)
{
	int size = (recordSize - usedBytes);
	qint64 startPos = stream.device()->pos();

	// If necessary read away garbage before the bitmap header.
	if (offBmiSrc > usedBytes){
		//qDebug() << "soaking " << offBmiSrc - usedBytes << "bytes before the header";
		soakBytes(stream, offBmiSrc - usedBytes);
		usedBytes = offBmiSrc;
	}

	qint64 posBeforeHeader = stream.device()->pos();
	// Create the header
	m_header = new BitmapHeader(stream, cbBmiSrc);
	usedBytes += cbBmiSrc;

	qint64 bitCount = m_header->bitCount();
	if ((bitCount == BI_BITCOUNT_1) || (bitCount == BI_BITCOUNT_2) || (bitCount == BI_BITCOUNT_3)){
		stream.device()->seek(posBeforeHeader);

		typedef struct _BMPFILEHEADER {
				quint16 bmType;
				quint32 bmSize;
				quint16 bmReserved1;
				quint16 bmReserved2;
				quint32 bmOffBits;
		} BMPFILEHEADER;

		int sizeBmp = size + 14;

		QByteArray pattern;           // BMP header and DIB data
		pattern.resize(sizeBmp);
		pattern.fill(0);
		stream.readRawData(pattern.data() + 14, size);

		// add BMP header
		BMPFILEHEADER* bmpHeader;
		bmpHeader = (BMPFILEHEADER*)(pattern.data());
		bmpHeader->bmType = 0x4D42;
		bmpHeader->bmSize = sizeBmp;

		if (bitCount == BI_BITCOUNT_1)
			m_mask.loadFromData(pattern, "BMP");

		if (m_image.loadFromData(pattern, "BMP")){
			m_hasImage = true;
			m_imageIsValid = true;
		}

		stream.device()->seek(startPos + size);
		return;
	}

	// If necessary read away garbage between the bitmap header and the picture.
	if (offBitsSrc > usedBytes) {
		//qDebug() << "soaking " << offBmiSrc - usedBytes << "bytes between the header and the image";
		soakBytes(stream, offBitsSrc - usedBytes);
		usedBytes = offBitsSrc;
	}

	// Read the image data
	if (cbBitsSrc > 0){
		//qDebug() << "reading bitmap (" << cbBitsSrc << "bytes)";
		m_imageData.resize(cbBitsSrc);
		stream.readRawData(m_imageData.data(), cbBitsSrc);
		m_hasImage = true;

		usedBytes += cbBitsSrc;
	}

	// If necessary, read away garbage after the image.
	if (recordSize > usedBytes) {
		//qDebug() << "soaking " << recordSize - usedBytes << "bytes after the image";
		soakBytes(stream, recordSize - usedBytes);
		usedBytes = recordSize;
	}
}

Bitmap::~Bitmap()
{
	delete m_header;
	//delete m_image;
}

QImage Bitmap::image(QImage::Format format)
{
	if (!m_hasImage)
		return QImage();

	return QImage((const uchar*)m_imageData.constData(), m_header->width(), abs(m_header->height()), format);
}

#if 1
QImage Bitmap::image()
{
	if (!m_hasImage)
		return QImage();

	if (m_imageIsValid)
		return m_image;

	QImage::Format format = QImage::Format_Invalid;

	//qDebug() << "Image bitCount: " << m_header->bitCount() << "compression: " << m_header->compression();

	// Start by determining which QImage format we are going to use.
	if (m_header->bitCount() == BI_BITCOUNT_1){
		format = QImage::Format_Mono;
		return QImage();
	} else if (m_header->bitCount() == BI_BITCOUNT_4){
		if ( m_header->compression() == BI_RGB ) {
			format = QImage::Format_RGB555;
		} else {
			qDebug() << "Unexpected compression format for BI_BITCOUNT_4:" << m_header->compression();
			//Q_ASSERT( 0 );
			return QImage();
		}
	} else if ( m_header->bitCount() == BI_BITCOUNT_5 ) {
		format = QImage::Format_RGB888;
	} else if ( m_header->bitCount() == BI_BITCOUNT_6 ) {
		if (m_header->compression() == BI_RGB) {
			format = QImage::Format_RGB32;
			//format = QImage::Format_RGB888;
		} else if (m_header->compression() == BI_BITFIELDS) {
			// FIXME: The number of bits is correct, but we need to
			//        handle the actual bits per colors specifically.
			//
			// The spec says (MS_WMF section 2.1.1.3):
			//   If the Compression field of the BitmapInfoHeader
			//   Object is set to BI_BITFIELDS, the Colors field
			//   contains three DWORD color masks that specify the
			//   red, green, and blue components, respectively, of
			//   each pixel. Each DWORD in the bitmap array represents
			//   a single pixel.
			format = QImage::Format_RGB32;
		}
		else
			return QImage();
	} else {
		qDebug() << "Unexpected format:" << m_header->bitCount();
		//Q_ASSERT(0);
		return QImage();
	}

	// According to MS-WMF 2.2.2.3, the sign of the height decides if
	// this is a compressed bitmap or not.
	if (m_header->height() > 0) {
		// This bitmap is a top-down bitmap without compression.
		m_image = QImage((const uchar*)m_imageData.constData(), m_header->width(), m_header->height(), format);

		// This is a workaround around a strange bug.  Without this
		// code it shows nothing.  Note that if we use Format_ARGB32
		// to begin with, nothing is shown anyway.
		//
		// FIXME: Perhaps it could be tested again with a later
		//        version of Qt (I have 4.6.3) /iw
		if (m_header->bitCount() == BI_BITCOUNT_6 && m_header->compression() == BI_RGB)
			m_image = m_image.convertToFormat(QImage::Format_ARGB32);

		// The WMF images are in the BGR color order.
		if (format == QImage::Format_RGB888)
			m_image = m_image.rgbSwapped();

		// We have to mirror this bitmap in the X axis since WMF images are stored bottom-up.
		m_image = m_image.mirrored(false, true);
	} else {
		// This bitmap is a bottom-up bitmap which uses compression.
		switch (m_header->compression()) {
		case BI_RGB:
			m_image = QImage((const uchar*)m_imageData.constData(), m_header->width(), -m_header->height(), format);
			// The WMF images are in the BGR color order.
			//m_image = m_image.rgbSwapped();
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
			m_image = QImage(m_header->width(), m_header->height(), format);
			break;
		}
	}

	m_imageIsValid = true;
	return m_image;
}
#else
QImage *Bitmap::image()
{
	if (!m_hasImage) {
		return 0;
	}

	if (m_image) {
		return m_image;
	}

	QImage::Format format = QImage::Format_Invalid;
	if ( m_header->bitCount() == BI_BITCOUNT_4 ) {
		if ( m_header->compression() == 0x00 ) {
			format = QImage::Format_RGB555;
		} else {
			//qDebug() << "Unexpected compression format for BI_BITCOUNT_4:"
			//              << m_header->compression();
			//Q_ASSERT( 0 );
			return 0;
		}
	} else if ( m_header->bitCount() == BI_BITCOUNT_5 ) {
		format = QImage::Format_RGB888;
	} else {
		qDebug() << "Unexpected format:" << m_header->bitCount();
		//Q_ASSERT( 0 );
		return 0;
	}
	m_image = new QImage( (const uchar*)m_imageData.constData(),
						  m_header->width(), m_header->height(), format );

	return m_image;
}
#endif


} // namespace QEmf
