/***************************************************************************
    qgsgeocmsdataitems.h
    ---------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSGEOCMSDATAITEMS_H
#define QGSGEOCMSDATAITEMS_H

#include "qgsdataitem.h"
#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"
#include "qgsdatasourceuri.h"
#include "qgsgeocmsconnection.h"

class QgsGeoCMSConnectionItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsGeoCMSConnectionItem( QgsDataItem *parent, QString name, QString path, QgsGeoCMSConnection *conn );
    QVector<QgsDataItem *> createChildren() override;
    virtual QList<QAction *> actions() override;

    QString mGeoCMSName;

  private:
    void editConnection();
    void deleteConnection()
    {
      QgsGeoCMSConnection::deleteConnection( mParent->name(), mConnection->mConnName );
      mParent->refresh();
    };

    QString mUri;
    QgsGeoCMSConnection *mConnection = nullptr;
};

class QgsGeoCMSServiceItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsGeoCMSServiceItem( QgsDataItem *parent, QgsGeoCMSConnection *conn, QString serviceName, QString path );
    QVector<QgsDataItem *> createChildren() override;

  private:
    void replacePath( QgsDataItem *item, QString before, QString after );
    QString mName;
    QString mServiceName;
    QString mUri;
    QgsGeoCMSConnection *mConnection = nullptr;
};

class QgsGeoCMSRootItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsGeoCMSRootItem( QgsDataItem *parent, QString name, QString path );

    QVector<QgsDataItem *> createChildren() override;

    virtual QList<QAction *> actions() override;

  private slots:
    void newConnection();
};

#endif //QGSGEOCMSDATAITEMS_H
