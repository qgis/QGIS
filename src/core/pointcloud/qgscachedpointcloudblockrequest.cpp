/***************************************************************************
                         qgscachedpointcloudblockrequest.cpp
                         -----------------------------------
    begin                : January 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : stefanos.natsis at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscachedpointcloudblockrequest.h"
#include "moc_qgscachedpointcloudblockrequest.cpp"

///@cond PRIVATE

QgsCachedPointCloudBlockRequest::QgsCachedPointCloudBlockRequest( QgsPointCloudBlock *block, const QgsPointCloudNodeId &node, const QString &uri,
    const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes,
    const QgsVector3D &scale, const QgsVector3D &offset, const QgsPointCloudExpression &filterExpression, const QgsRectangle &filterRect )
  : QgsPointCloudBlockRequest( node, uri, attributes, requestedAttributes, scale, offset, filterExpression, filterRect )
{
  mBlock.reset( block );
  QMetaObject::invokeMethod( this, &QgsCachedPointCloudBlockRequest::finished, Qt::QueuedConnection );
}

///@endcond
