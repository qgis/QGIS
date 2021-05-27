/***************************************************************************
                         qgseptdecoder.h
                         --------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEPTDECODER_H
#define QGSEPTDECODER_H


#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgspointcloudblock.h"
#include "qgspointcloudattribute.h"

///@cond PRIVATE
#define SIP_NO_FILE

#include <QString>

namespace QgsEptDecoder
{
  QgsPointCloudBlock *decompressBinary( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset );
  QgsPointCloudBlock *decompressBinary( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset );
  QgsPointCloudBlock *decompressZStandard( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset );
  QgsPointCloudBlock *decompressZStandard( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset );
  QgsPointCloudBlock *decompressLaz( const QString &filename, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset );
  QgsPointCloudBlock *decompressLaz( const QByteArray &data, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, const QgsVector3D &scale, const QgsVector3D &offset );
};

///@endcond
#endif // QGSEPTDECODER_H
