/***************************************************************************
                         qgseptpointcloudblockrequest.h
                         --------------------
    begin                : April 2022
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

#ifndef QGSEPTPOINTCLOUDBLOCKREQUEST_H
#define QGSEPTPOINTCLOUDBLOCKREQUEST_H

#include <QObject>

#include "qgspointcloudblockrequest.h"

#define SIP_NO_FILE

class QgsPointCloudAttributeCollection;
class QgsPointCloudBlock;

/**
 * \ingroup core
 * \brief Base class for handling loading QgsPointCloudBlock asynchronously from a remote EPT dataset
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsEptPointCloudBlockRequest : public QgsPointCloudBlockRequest
{
    Q_OBJECT
  public:

    /**
     * QgsPointCloudBlockRequest constructor
     * Requests the block data of size \a blockSize at offset blockOffset
     * Note: It is the responsablitiy of the caller to delete the block if it was loaded correctly
     */
    QgsEptPointCloudBlockRequest( const IndexedPointCloudNode &node, const QString &Uri, const QString &dataType,
                                  const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes,
                                  const QgsVector3D &scale, const QgsVector3D &offset, const QgsPointCloudExpression &filterExpression, const QgsRectangle &filterRect );

    ~QgsEptPointCloudBlockRequest() = default;
  private:
    QString mDataType;
  private slots:
    void blockFinishedLoading();
};

#endif // QGSEPTPOINTCLOUDBLOCKREQUEST_H
