/***************************************************************************
      qgsgdalproviderbase.h  -  Common base class for GDAL and WCS provider
                             -------------------
    begin                : November, 2010
    copyright            : (C) 2010 by Radim Blazek
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

#ifndef QGSGDALPROVIDERBASE_H
#define QGSGDALPROVIDERBASE_H

#include "qgsrasterdataprovider.h"
#include "qgscolorrampshader.h"

#include <QList>

#define CPL_SUPRESS_CPLUSPLUS
#include <gdal.h>

#if defined(GDAL_VERSION_NUM) && GDAL_VERSION_NUM >= 1800
#define TO8F(x) (x).toUtf8().constData()
#define FROM8(x) QString::fromUtf8(x)
#else
#define TO8F(x) QFile::encodeName( x ).constData()
#define FROM8(x) QString::fromLocal8Bit(x)
#endif

/**
  \brief Base clasee for GDAL and WCS providers.
*/
class QgsGdalProviderBase
{
  public:
    QgsGdalProviderBase( );

    ~QgsGdalProviderBase();

    /** \brief ensures that GDAL drivers are registered, but only once */
    static void registerGdalDrivers();
  protected:

    QgsRasterInterface::DataType dataTypeFromGdal( int theGdalDataType ) const;

    int colorInterpretationFromGdal( int gdalColorInterpretation ) const;

    QList<QgsColorRampShader::ColorRampItem> colorTable( GDALDatasetH gdalDataset, int bandNo )const;
};

#endif

