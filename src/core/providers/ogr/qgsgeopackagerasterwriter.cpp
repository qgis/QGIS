/***************************************************************************
  qgsgeopackagerasterwriter.cpp - QgsGeoPackageRasterWriter

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

///@cond PRIVATE

#include "gdal.h"
#include "gdal_utils.h"
#include "qgsogrutils.h"

#include "qgsgeopackagerasterwriter.h"
#include "qgscplerrorhandler_p.h"

QgsGeoPackageRasterWriter::QgsGeoPackageRasterWriter( const QgsMimeDataUtils::Uri &sourceUri, const QString &outputUrl ):
  mSourceUri( sourceUri ),
  mOutputUrl( outputUrl )
{

}

QgsGeoPackageRasterWriter::WriterError QgsGeoPackageRasterWriter::writeRaster( QgsFeedback *feedback, QString *errorMessage )
{
  const char *args[] = { "-of", "gpkg", "-co", QStringLiteral( "RASTER_TABLE=%1" ).arg( mSourceUri.name ).toUtf8().constData(), "-co", "APPEND_SUBDATASET=YES", nullptr };
  // This sends OGR/GDAL errors to the message log
  const QgsCPLErrorHandler handler;
  GDALTranslateOptions *psOptions = GDALTranslateOptionsNew( ( char ** )args, nullptr );

  GDALTranslateOptionsSetProgress( psOptions, [ ]( double dfComplete, const char *pszMessage,  void *pProgressData ) -> int
  {
    Q_UNUSED( pszMessage )
    QgsFeedback *feedback = static_cast< QgsFeedback * >( pProgressData );
    feedback->setProgress( dfComplete * 100 );
    return ! feedback->isCanceled();
  }, feedback );

  const gdal::dataset_unique_ptr hSrcDS( GDALOpen( mSourceUri.uri.toUtf8().constData(), GA_ReadOnly ) );
  if ( ! hSrcDS )
  {
    *errorMessage = QObject::tr( "Failed to open source layer %1! See the OGR panel in the message logs for details.\n\n" ).arg( mSourceUri.name );
    mHasError = true;
  }
  else
  {
    CPLErrorReset();
    const gdal::dataset_unique_ptr hOutDS( GDALTranslate( mOutputUrl.toUtf8().constData(), hSrcDS.get(), psOptions, nullptr ) );
    if ( ! hOutDS )
    {
      *errorMessage = QObject::tr( "Failed to import layer %1! See the OGR panel in the message logs for details.\n\n" ).arg( mSourceUri.name );
      mHasError = true;
    }
  }
  GDALTranslateOptionsFree( psOptions );
  return ( feedback && feedback->isCanceled() ) ? ErrUserCanceled : ( mHasError ? WriteError : NoError );
}

///@endcond
