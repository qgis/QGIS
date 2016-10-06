/***************************************************************************
      qgsrasteridentifyresult.cpp
     --------------------------------------
    Date                 : Apr 8, 2013
    Copyright            : (C) 2013 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

//#include <QTime>

#include "qgis.h"
#include "qgslogger.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrasterdataprovider.h"

QgsRasterIdentifyResult::QgsRasterIdentifyResult()
    : mValid( false )
    , mFormat( QgsRaster::IdentifyFormatUndefined )
{
}

QgsRasterIdentifyResult::QgsRasterIdentifyResult( QgsRaster::IdentifyFormat theFormat, const QMap<int, QVariant>& theResults )
    : mValid( true )
    , mFormat( theFormat )
    , mResults( theResults )
{
}

QgsRasterIdentifyResult::QgsRasterIdentifyResult( const QgsError& theError )
    : mValid( false )
    , mFormat( QgsRaster::IdentifyFormatUndefined )
    , mError( theError )
{
}

QgsRasterIdentifyResult::~QgsRasterIdentifyResult()
{
}
