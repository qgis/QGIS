/***************************************************************************
                         qgscopcpointcloudblockrequest.h
                         --------------------
    begin                : March 2022
    copyright            : (C) 2022 by Belgacem Nedjima
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

#ifndef QGSCOPCPOINTCLOUDBLOCKREQUEST_H
#define QGSCOPCPOINTCLOUDBLOCKREQUEST_H

#include <QObject>

#include "qgspointcloudattribute.h"
#include "qgstiledownloadmanager.h"
#include "qgspointcloudindex.h"
#include "qgspointcloudexpression.h"
#include "qgspointcloudrequest.h"
#include "qgspointcloudblockrequest.h"

#define SIP_NO_FILE

class QgsPointCloudAttributeCollection;
class QgsPointCloudBlock;

/**
 * \ingroup core
 * \brief Base class for handling loading QgsPointCloudBlock asynchronously from a remote COPC dataset
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsCopcPointCloudBlockRequest : public QgsPointCloudBlockRequest
{
    Q_OBJECT
  public:

    /**
     * QgsPointCloudBlockRequest constructor
     * Requests the block data of size \a blockSize at offset blockOffset
     * Note: It is the responsablitiy of the caller to delete the block if it was loaded correctly
     */
    QgsCopcPointCloudBlockRequest( const IndexedPointCloudNode &node, const QString &Uri,
                                   const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes,
                                   const QgsVector3D &scale, const QgsVector3D &offset, const QgsPointCloudExpression &filterExpression,
                                   uint64_t blockOffset, int32_t blockSize, int pointCount, QByteArray lazHeader, QByteArray extraBytesData );

    void startRequest() override;
  private:
    uint64_t mBlockOffset;
    int32_t mBlockSize;
    int mPointCount;
    QByteArray mLazHeader;
    QByteArray mExtrabytesData;
  protected:
    void blockFinishedLoading() override;
};

#endif // QGSCOPCPOINTCLOUDBLOCKREQUEST_H
