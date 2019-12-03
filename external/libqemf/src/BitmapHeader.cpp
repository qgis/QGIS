/*
 * Copyright 2010 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#include "BitmapHeader.h"

#include <QDataStream>
#include <QDebug>

namespace QEmf
{

static void soakBytes( QDataStream &stream, int numBytes )
{
	quint8 scratch;
	for ( int i = 0; i < numBytes; ++i ) {
		stream >> scratch;
	}
}


BitmapHeader::BitmapHeader( QDataStream &stream, int size )
{
	m_headerType = BitmapInfoHeader;  // The default

	int  read = 40;             // Keep track of how many bytes we have read;

	//qDebug() << "BitmapHeader pos:" << stream.device()->pos();

	// Read the data that is present in a BitmapInfoHeader (size 40)
	stream >> m_headerSize;
	stream >> m_width;
	stream >> m_height;
	stream >> m_planes;         // 16 bits
	stream >> m_bitCount;       // 16 bits
	stream >> m_compression;
	stream >> m_imageSize;

	stream >> m_xPelsPerMeter;
	stream >> m_yPelsPerMeter;
	stream >> m_colorUsed;
	stream >> m_colorImportant;

	if (m_bitCount == 1){
		typedef struct tagRGBQUAD {
				quint8 rgbBlue;
				quint8 rgbGreen;
				quint8 rgbRed;
				quint8 rgbReserved;
		} RGBQUAD;

		unsigned int colors = abs(int(size - m_headerSize))/sizeof(RGBQUAD);
		//printf("BitmapHeader size: %d m_headerSize: %d bmiColors: %d\n", size, m_headerSize, colors);
		for (unsigned int i = 0; i < colors; i++){
			quint8 rgbBlue, rgbGreen, rgbRed, rgbReserved;
			stream >> rgbBlue;
			stream >> rgbGreen;
			stream >> rgbRed;
			stream >> rgbReserved;
			m_colorTable.append(qRgb(rgbRed, rgbGreen, rgbBlue));
			//printf("i: %d r: %d g: %d b:%d a: %d\n", i, rgbRed, rgbGreen, rgbBlue, rgbReserved);
		}
	}

#ifdef DEBUG_EMFPARSER
	qDebug() << "HeaderSize:" << m_headerSize;
	qDebug() << "Width:" << m_width;
	qDebug() << "Height:" << m_height;
	qDebug() << "planes:" << m_planes;
	qDebug() << "BitCount:" << m_bitCount;
	qDebug() << "Compression:" << m_compression;
	qDebug() << "ImageSize:" << m_imageSize;
	qDebug() << "Colors used:" << m_colorUsed;
	qDebug() << "Colors Important:" << m_colorImportant;
#endif
	// BitmapV4Header (size 40+68 = 108)
	if (size >= 108) {
		m_headerType = BitmapV4Header;
		read = 108;

		stream >> m_redMask;
		stream >> m_greenMask;
		stream >> m_blueMask;
		stream >> m_alphaMask;
		stream >> m_colorSpaceType;

		// FIXME sometime: Implement the real CIEXYZTriple
		for (int i = 0; i < 9; ++i)
			stream >> m_endpoints[i];

		stream >> m_gammaRed;
		stream >> m_gammaGreen;
		stream >> m_gammaBlue;
	}

	// BitmapV5Header (size 108+16 = 124)
	if (size >= 124) {
		m_headerType = BitmapV5Header;
		read = 124;

		stream >> m_intent;
		stream >> m_profileData;
		stream >> m_profileSize;
		stream >> m_reserved;
	}

#ifdef DEBUG_EMFPARSER
	qDebug() << "header type:" << m_headerType;
	qDebug() << "header size:" << size;
	qDebug() << "read bytes: " << read;
#endif
	// Read away the overshot from the size parameter;
	if (size > read)
		soakBytes(stream, size - read);
}

BitmapHeader::~BitmapHeader()
{
}

} // namespace QEmf
