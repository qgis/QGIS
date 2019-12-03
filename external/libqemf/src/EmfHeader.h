/*
  Copyright 2008 Brad Hards <bradh@frogmouth.net>
  Copyright 2009 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#ifndef EMFHEADER_H
#define EMFHEADER_H

#include <QDataStream>
#include <QRect> // also provides QSize
#include <QString>

/**
   \file

   Primary definitions for EMF Header record
*/

/**
   Namespace for Enhanced Metafile (EMF) classes
*/
namespace QEmf
{

/**
	Simple representation of an EMF File header

	See MS-EMF Section 2.3.4.2 for details
*/
class Header
{
public:
	/**
	   Constructor for header

	   \param stream the stream to read the header structure from
	*/
	explicit Header(QDataStream &stream);
	~Header();

	/**
	   Check whether this is a valid EMF Header
	 */
	bool isValid() const;

	/**
	   The number of records in the metafile
	 */
	quint32 recordCount() const;

	/**
	   The bounding box of the file content, in device units
	*/
	QRect bounds() const;

	/**
	   The frame of the file content, in 0.01 mm units
	*/
	QRect frame() const;

	QSize device() const;
	QSize millimeters() const;

private:
	// Temporary hack to read some bytes.
	void soakBytes( QDataStream &stream, int numBytes );

	quint32 mType;
	quint32 mSize;
	QRect mBounds;
	QRect mFrame;
	quint32 mSignature;
	quint32 mVersion;
	quint32 mBytes;
	quint32 mRecords;
	quint16 mHandles;
	quint16 mReserved;
	quint32 m_nDescription;
	quint32 m_offDescription;
	quint32 m_nPalEntries;
	QSize mDevice;      // this might need to be converted to something better
	QSize mMillimeters; // this might need to be converted to something better
};

}

#endif
