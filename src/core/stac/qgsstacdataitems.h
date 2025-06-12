/***************************************************************************
    qgsstacdataitems.h
    ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTACDATAITEMS_H
#define QGSSTACDATAITEMS_H

#include "qgis_core.h"
#include "qgsdataitemprovider.h"
#include "qgsconnectionsitem.h"
#include "qgsstaccatalog.h"
#include "qgsstacitem.h"

#include <QUrl>

class QgsStacController;
class QgsStacCollection;


/**
 * \ingroup core
 * \brief Item to display that there are additional STAC items which are not loaded.
 * \since QGIS 3.44
*/
class CORE_EXPORT QgsStacFetchMoreItem : public QgsDataItem
{
    Q_OBJECT
  public:
    QgsStacFetchMoreItem( QgsDataItem *parent, const QString &name );

    bool handleDoubleClick() override;
    QVariant sortKey() const override { return QStringLiteral( "3" ); }

};

/**
 * \ingroup core
 * \brief Item for STAC Items within a catalog or collection.
 * \since QGIS 3.44
*/
class CORE_EXPORT QgsStacItemItem : public QgsDataItem
{
    Q_OBJECT
  public:
    QgsStacItemItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;
    bool hasDragEnabled() const override;
    QgsMimeDataUtils::UriList mimeUris() const override;
    bool equal( const QgsDataItem *other ) override;
    QVariant sortKey() const override { return QStringLiteral( "2 %1" ).arg( mName ); }

    void updateToolTip();
    QgsStacController *stacController();

#ifndef SIP_RUN
    //! takes ownership
    void setStacItem( std::unique_ptr< QgsStacItem > item );
#else
    void setStacItem( QgsStacItem *item SIP_TRANSFER );
    % MethodCode
    sipCpp-> setStacItem( std::unique_ptr< QgsStacItem >( a0 ) );
    % End
#endif


    //! does not transfer ownership
    QgsStacItem *stacItem() const;

  public slots:
    void itemRequestFinished( int requestId, QString error );

  private:
    std::unique_ptr< QgsStacItem > mStacItem;
    QString mUri;
    QString mConnName;
};

/**
 * \ingroup core
 * \brief Item for catalogs and collections.
 * \since QGIS 3.44
*/
class CORE_EXPORT QgsStacCatalogItem : public QgsDataCollectionItem
{
    Q_OBJECT
  public:
    QgsStacCatalogItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;
    bool equal( const QgsDataItem *other ) override;
    QVariant sortKey() const override { return QStringLiteral( "1 %1" ).arg( mName ); }

    void updateToolTip();

    //! takes ownership
#ifndef SIP_RUN
    void setStacCatalog( std::unique_ptr< QgsStacCatalog > object );
#else
    void setStacCatalog( QgsStacCatalog *object SIP_TRANSFER ) SIP_HOLDGIL;
    % MethodCode
    sipCpp->setStacCatalog( std::unique_ptr< QgsStacCatalog>( a0 ) );
    % End
#endif

    //! does not transfer ownership
    QgsStacCatalog *stacCatalog() const;

    void fetchMoreChildren();

    bool isCatalog() const;
    bool isCollection() const;
    QgsStacController *stacController() const;
    QgsStacCatalog *rootCatalog() const;
    QgsStacFetchMoreItem *fetchMoreItem() const;

  public slots:
    void childrenCreated() override;

  private slots:
    void onControllerFinished( int requestId, const QString &error );

  private:
    //! takes ownership
    QVector< QgsDataItem * > createItems( const QVector<QgsStacItem *> items );
    QVector< QgsDataItem * > createCollections( const QVector<QgsStacCollection *> collections );

    //! The URI
    QString mUri;
    QString mConnName;
    bool mIsCollection = false;
    std::unique_ptr< QgsStacCatalog > mStacCatalog;
    QUrl mFetchMoreUrl;
};

/**
 * \ingroup core
 * \brief Item for STAC connections, is also a catalog itself.
 * \since QGIS 3.44
*/
class CORE_EXPORT QgsStacConnectionItem : public QgsStacCatalogItem
{
    Q_OBJECT
  public:
    QgsStacConnectionItem( QgsDataItem *parent, const QString &connectionName );

    QgsStacController *controller() const;

  private:
    //! The URI
    QString mUri;
    QString mConnName;
    std::unique_ptr<QgsStacController> mController;
};

/**
 * \ingroup core
 * \brief Root item for STAC connections.
 * \since QGIS 3.44
*/
class CORE_EXPORT QgsStacRootItem : public QgsConnectionsRootItem
{
    Q_OBJECT
  public:
    QgsStacRootItem( QgsDataItem *parent, const QString &name, const QString &path );

    QVector<QgsDataItem *> createChildren() override;

    QVariant sortKey() const override { return 3; }

  public slots:
    void onConnectionsChanged();
};

/**
 * \ingroup core
 * \brief Provider for STAC root data item.
 * \since QGIS 3.44
*/
class CORE_EXPORT QgsStacDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    QString dataProviderKey() const override;
    Qgis::DataItemProviderCapabilities capabilities() const override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};


#endif // QGSSTACDATAITEMS_H
