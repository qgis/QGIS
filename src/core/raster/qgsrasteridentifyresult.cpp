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

QgsRasterIdentifyResult::QgsRasterIdentifyResult( Qgis::RasterIdentifyFormat format, const QMap<int, QVariant> &results )
  : mValid( true )
  , mFormat( format )
  , mResults( results )
{
}

QgsRasterIdentifyResult::QgsRasterIdentifyResult( const QgsError &error )
  : mError( error )
{
}

