/***************************************************************************
    qgsgeopackageprojectstorageguiprovider.h
    ---------------------
    begin                : June 2019
    copyright            : (C) 2019 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOPACKAGEPROJECTSTORAGEGUIPROVIDER_H
#define QGSGEOPACKAGEPROJECTSTORAGEGUIPROVIDER_H


#include "qgsprojectstorageguiprovider.h"
#include "qgis_sip.h"

///@cond PRIVATE
#define SIP_NO_FILE

class QgsGeoPackageProjectStorageGuiProvider : public QgsProjectStorageGuiProvider
{
  public:
    QString type() override { return QStringLiteral( "geopackage" ); }
    QString visibleName() override;
    QString showLoadGui() override;
    QString showSaveGui() override;
};

///@endcond
#endif // QGSGEOPACKAGEPROJECTSTORAGEGUIPROVIDER_H
