/***************************************************************************
  qgsgeopackagerasterwriter.h - QgsGeoPackageRasterWriter

 ---------------------
 begin                : 23.8.2017
 copyright            : (C) 2017 by Alessandro Pasotti
 email                : apasotti at boundlessgeo dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSGEOPACKAGERASTERWRITER_H
#define QGSGEOPACKAGERASTERWRITER_H

///@cond PRIVATE

#define SIP_NO_FILE

#include "qgsmimedatautils.h"
#include "qgsfeedback.h"

class QgsGeoPackageRasterWriter
{
  public:

    //! Error codes
    enum WriterError
    {
      NoError = 0, //!< No errors were encountered
      WriteError, //! Generic GDAL Translate error
      ErrUserCanceled, //!< User canceled the export
    };

    QgsGeoPackageRasterWriter( const QgsMimeDataUtils::Uri &sourceUri, const QString &destinationPath );
    WriterError writeRaster( QgsFeedback *feedback, QString *errorMessage );
    const QString outputUrl() const { return mOutputUrl; }

  private:
    QgsMimeDataUtils::Uri mSourceUri;
    QString mOutputUrl;
    bool mHasError = false;
};


///@endcond

#endif // QGSGEOPACKAGERASTERWRITER_H

