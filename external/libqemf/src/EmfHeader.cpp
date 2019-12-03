/*
  Copyright 2008 Brad Hards <bradh@frogmouth.net>
  Copyright 2015 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#include "EmfHeader.h"

#include <QDebug>

namespace QEmf
{

/*****************************************************************************/
const quint32 ENHMETA_SIGNATURE = 0x464D4520;

Header::Header( QDataStream &stream )
{
	stream >> mType;
	stream >> mSize;
	stream >> mBounds;
	stream >> mFrame;
	stream >> mSignature;
	stream >> mVersion;
	stream >> mBytes;
	stream >> mRecords;
	stream >> mHandles;
	stream >> mReserved;
	stream >> m_nDescription;
	stream >> m_offDescription;
	stream >> m_nPalEntries;
	stream >> mDevice;
	stream >> mMillimeters;
	if ( ( ENHMETA_SIGNATURE == mSignature ) && ( m_nDescription != 0 ) ){
		// we have optional EmfDescription, but don't know how to read that yet.
	}

	// FIXME: We could need to read EmfMetafileHeaderExtension1 and
	//        ..2 here but we have no example of that.
	soakBytes( stream, mSize - 88 );
}

Header::~Header()
{
}

bool Header::isValid() const
{
	return ( ( 0x00000001 == mType ) && ( ENHMETA_SIGNATURE == mSignature ) );
}

QRect Header::bounds() const
{
	return mBounds;
}

QRect Header::frame() const
{
	return mFrame;
}

QSize Header::device() const
{
	return mDevice;
}

QSize Header::millimeters() const
{
	return mMillimeters;
}

quint32 Header::recordCount() const
{
	return mRecords;
}

void Header::soakBytes( QDataStream &stream, int numBytes )
{
	quint8 scratch;
	for ( int i = 0; i < numBytes; ++i ) {
		stream >> scratch;
	}
}

}
