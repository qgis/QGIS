/***************************************************************************
                         qgspointcloudblockrequest.cpp
                         --------------------
    begin                : March 2021
    copyright            : (C) 2021 by Belgacem Nedjima
    email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudblockrequest.h"
#include "moc_qgspointcloudblockrequest.cpp"

//
// QgsPointCloudBlockRequest
//

///@cond PRIVATE

QgsPointCloudBlockRequest::QgsPointCloudBlockRequest( const QgsPointCloudNodeId &node, const QString &uri,
    const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes,
    const QgsVector3D &scale, const QgsVector3D &offset, const QgsPointCloudExpression &filterExpression, const QgsRectangle &filterRect )
  : mNode( node )
  , mUri( uri )
  , mAttributes( attributes )
  , mRequestedAttributes( requestedAttributes )
  , mScale( scale )
  , mOffset( offset )
  , mFilterExpression( filterExpression )
  , mFilterRect( filterRect )
{
}

QgsPointCloudBlockRequest::~QgsPointCloudBlockRequest() = default;

std::unique_ptr<QgsPointCloudBlock> QgsPointCloudBlockRequest::takeBlock()
{
  return std::move( mBlock );
}

QString QgsPointCloudBlockRequest::errorStr()
{
  return mErrorStr;
}

///@endcond
