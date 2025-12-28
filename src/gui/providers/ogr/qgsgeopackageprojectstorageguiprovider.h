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


#include "qgis_sip.h"
#include "qgsprojectstorageguiprovider.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE
#define SIP_NO_FILE

class QgsGeoPackageProjectStorageGuiProvider : public QgsProjectStorageGuiProvider
{
  public:
    QString type() override { return u"geopackage"_s; }
    QString visibleName() override;
    QString showLoadGui() override;
    QString showSaveGui() override;
};

///@endcond
#endif // QGSGEOPACKAGEPROJECTSTORAGEGUIPROVIDER_H
