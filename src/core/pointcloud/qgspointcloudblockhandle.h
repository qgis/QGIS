/***************************************************************************
                         qgspointcloudblockhandle.h
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

#ifndef QGSPOINTCLOUDBLOCKHANDLE_H
#define QGSPOINTCLOUDBLOCKHANDLE_H

#include <QObject>

#include "qgspointcloudattribute.h"
#include "qgstiledownloadmanager.h"

#define SIP_NO_FILE

class QgsPointCloudAttributeCollection;
class QgsPointCloudBlock;

/**
 * \ingroup core
 * \brief Base class for handling loading QgsPointCloudBlock asynchronously
 *
 * \note The API is considered EXPERIMENTAL and can be changed without a notice
 *
 * \since QGIS 3.20
 */
class CORE_EXPORT QgsPointCloudBlockHandle : public QObject
{
    Q_OBJECT
  public:
    //! Constructor
    QgsPointCloudBlockHandle( const QString &dataType, const QgsPointCloudAttributeCollection &attributes, const QgsPointCloudAttributeCollection &requestedAttributes, QgsTileDownloadManagerReply *tileDownloadManagerReply );
  signals:
    //! Emitted when the block is loaded successfully
    void blockLoadingSucceeded( QgsPointCloudBlock *block );
    //! Emitted when the block loading has failed
    void blockLoadingFailed( const QString &errorStr );
  private:
    QString mDataType;
    QgsPointCloudAttributeCollection mAttributes;
    QgsPointCloudAttributeCollection mRequestedAttributes;
    QgsTileDownloadManagerReply *mTileDownloadManagetReply = nullptr;
  private slots:
    void blockFinishedLoading();
};

#endif // QGSPOINTCLOUDBLOCKHANDLE_H
