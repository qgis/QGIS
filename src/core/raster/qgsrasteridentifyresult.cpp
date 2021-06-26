/***************************************************************************
      qgsrasteridentifyresult.cpp
     --------------------------------------
    Date                 : Apr 8, 2013
    Copyright            : (C) 2013 by Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

//#include <QTime>

#include "qgis.h"
#include "qgslogger.h"
#include "qgsrasteridentifyresult.h"
#include "qgsrasterdataprovider.h"

QgsRasterIdentifyResult::QgsRasterIdentifyResult( QgsRaster::IdentifyFormat format, const QMap<int, QVariant> &results )
  : mValid( true )
  , mFormat( format )
  , mResults( results )
{
}

QgsRasterIdentifyResult::QgsRasterIdentifyResult( const QgsError &error )
  : mError( error )
{
}

