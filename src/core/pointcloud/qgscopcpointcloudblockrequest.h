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

#include "qgspointcloudblockrequest.h"
#include "qgslazinfo.h"

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
                                   const QgsVector3D &scale, const QgsVector3D &offset, const QgsPointCloudExpression &filterExpression, const QgsRectangle &filterRect,
                                   uint64_t blockOffset, int32_t blockSize, int pointCount, const QgsLazInfo &lazInfo );

    ~QgsCopcPointCloudBlockRequest() = default;
  private:
    uint64_t mBlockOffset;
    int32_t mBlockSize;
    int mPointCount;
    QgsLazInfo mLazInfo;
  private slots:
    void blockFinishedLoading();
};

#endif // QGSCOPCPOINTCLOUDBLOCKREQUEST_H
