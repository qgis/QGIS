/***************************************************************************
                         qgspointcloudrequest.cpp
                         -----------------------
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

#include "qgis.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudattribute.h"

QgsPointCloudRequest::QgsPointCloudRequest() = default;

bool QgsPointCloudRequest::operator==( const QgsPointCloudRequest &other ) const
{
  return mFilterRect == other.filterRect() &&
         mAttributes.toFields() == other.attributes().toFields();
}

QgsPointCloudAttributeCollection QgsPointCloudRequest::attributes() const
{
  return mAttributes;
}

void QgsPointCloudRequest::setAttributes( const QgsPointCloudAttributeCollection &attributes )
{
  mAttributes = attributes;
}

uint qHash( const QgsPointCloudRequest &request )
{
  return qHash( request.filterRect() ) ^ qHash( QVariant( request.attributes().toFields() ) );
}
