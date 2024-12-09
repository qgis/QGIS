/***************************************************************************
                         qgspointcloudblockrequest.h
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

#ifndef QGSPOINTCLOUDBLOCKREQUEST_H
#define QGSPOINTCLOUDBLOCKREQUEST_H

#include <QObject>

#include "qgstiledownloadmanager.h"
#include "qgspointcloudindex.h"

#define SIP_NO_FILE

/**
 * \ingroup core
 * \brief Base class for handling loading QgsPointCloudBlock asynchronously
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsPointCloudBlockRequest : public QObject
{
    Q_OBJECT
  public:

    /**
     * QgsPointCloudBlockRequest constructor
     * Note: It is the responsablitiy of the caller to delete the block if it was loaded correctly
     */
    QgsPointCloudBlockRequest( const QgsPointCloudNodeId &node, const QString &Uri,
                               const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes,
                               const QgsVector3D &scale, const QgsVector3D &offset, const QgsPointCloudExpression &filterExpression, const QgsRectangle &filterRect );


    virtual ~QgsPointCloudBlockRequest() = 0;

    /**
     * Returns the requested block. if the returned block is nullptr, that means the data request failed.
     */
    std::unique_ptr<QgsPointCloudBlock> takeBlock();

    //! Returns the error message string of the request
    QString errorStr();

  signals:
    //! Emitted when the request processing has finished
    void finished();

  protected:
    QgsPointCloudNodeId mNode;
    QString mUri;
    QgsPointCloudAttributeCollection mAttributes;
    QgsPointCloudAttributeCollection mRequestedAttributes;
    std::unique_ptr<QgsTileDownloadManagerReply> mTileDownloadManagerReply = nullptr;
    std::unique_ptr<QgsPointCloudBlock> mBlock;
    QString mErrorStr;
    QgsVector3D mScale, mOffset;
    QgsPointCloudExpression mFilterExpression;
    QgsRectangle mFilterRect;
};

#endif // QGSPOINTCLOUDBLOCKREQUEST_H
