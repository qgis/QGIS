/*
  Copyright 2008 Brad Hards <bradh@frogmouth.net>
  Copyright 2010 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#ifndef EMFBITMAP_H
#define EMFBITMAP_H


#include <Qt>                   // For qint, etc.
#include <QByteArray>
#include <QImage>
#include <QBitmap>

#include "BitmapHeader.h"

class QDataStream;

/**
 * @file
 *
 * definitions for Device Independent Bitmaps, as used in both WMF and EMF files.
*/

/// Namespace for Enhanced Metafile (EMF) classes

namespace QEmf
{

/**
 * @class Bitmap
 *
 * Representation of a bitmap from an EMF file
 */
class Bitmap
{
public:
	/**
	 * Constructor
	 *
	 * The Bitmap structure is built from the specified stream.
	 *
	 * \param stream the data stream to read from.
	 * \param recordSize the size of the EMF record that this bitmap is part of.
	 * \param usedBytes  number of already used bytes of the EMF record before the bitmap part
	 * \param offBmiSrc  offset to start of bitmapheader
	 * \param cbBmiSrc   size of bitmap header
	 * \param offBitsSrc offset to source bitmap
	 * \param cbBitsSrc  size of source bitmap
	 */
	Bitmap( QDataStream &stream,
			quint32 recordSize,  // total size of the EMF record
			quint32 usedBytes,   // used bytes of the EMF record before the bitmap part
			quint32 offBmiSrc,   // offset to start of bitmapheader
			quint32 cbBmiSrc,    // size of bitmap header
			quint32 offBitsSrc,  // offset to source bitmap
			quint32 cbBitsSrc);  // size of source bitmap
	~Bitmap();

	/**
	   The bitmap header
	*/
	BitmapHeader *header() const {return m_header;}

	/**
	   Return true if there is an image in this record.
	 */
	bool hasImage() const {return m_hasImage;}

	/**
	   The image.

	   QImage shares its memory already.
	*/
	QImage image();

	/**
	   The image.
	   QImage shares its memory already.
	 *
	 * \param format the format of the image requested by the caller
	*/
	QImage image(QImage::Format format);

	QBitmap mask(){return m_mask;}

private:
	// No copying for now, because we will get into trouble with the pointers.
	// The remedy is to write a real operator=() and Bitmap(Bitmap&).
	explicit Bitmap(Bitmap&);
	Bitmap &operator=(Bitmap&);

private:
	bool          m_hasImage;
	BitmapHeader *m_header;

	QBitmap       m_mask;
	QByteArray    m_imageData;
	QImage        m_image;
	bool          m_imageIsValid;
};



} // namespace QEmf

#endif
