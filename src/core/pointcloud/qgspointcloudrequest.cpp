/***************************************************************************
                         qgspointcloudrequest.cpp
                         -----------------------
    begin                : October 2020
    copyright            : (C) 2020 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudattribute.h"

QgsPointCloudRequest::QgsPointCloudRequest() = default;

QgsPointCloudAttributeCollection QgsPointCloudRequest::attributes() const
{
  return mAttributes;
}

void QgsPointCloudRequest::setAttributes( const QgsPointCloudAttributeCollection &attributes )
{
  mAttributes = attributes;
}
