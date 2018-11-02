/***************************************************************************
    qgsappbrowserproviders.h
    -------------------------
    begin                : September 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSAPPBROWSERPROVIDERS_H
#define QGSAPPBROWSERPROVIDERS_H

#include "qgis_app.h"
#include "qgsdataitemprovider.h"
#include "qgsdataprovider.h"
#include "qgscustomdrophandler.h"

/**
 * Custom data item for QLR files.
 */
class QgsQlrDataItem : public QgsLayerItem
{
    Q_OBJECT

  public:

    QgsQlrDataItem( QgsDataItem *parent, const QString &name, const QString &path );
    bool hasDragEnabled() const override;
    QgsMimeDataUtils::Uri mimeUri() const override;

};

/**
 * Data item provider for showing QLR layer files in the browser.
 */
class QgsQlrDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    int capabilities() override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

/**
 * Handles drag and drop of QLR files to app.
 */
class QgsQlrDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT

  public:

    QString customUriProviderKey() const override;
    void handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const override;
};

/**
 * Custom data item for QPT print template files.
 */
class QgsQptDataItem : public QgsDataItem
{
    Q_OBJECT

  public:

    QgsQptDataItem( QgsDataItem *parent, const QString &name, const QString &path );
    bool hasDragEnabled() const override;
    QgsMimeDataUtils::Uri mimeUri() const override;
    bool handleDoubleClick() override;
    QList< QAction * > actions( QWidget *parent ) override;


};

/**
 * Data item provider for showing QPT print templates in the browser.
 */
class QgsQptDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    int capabilities() override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

/**
 * Handles drag and drop of QPT print templates to app.
 */
class QgsQptDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT

  public:

    QString customUriProviderKey() const override;
    void handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const override;
    bool handleFileDrop( const QString &file ) override;
};



/**
 * Custom data item for py Python scripts.
 */
class QgsPyDataItem : public QgsDataItem
{
    Q_OBJECT

  public:

    QgsPyDataItem( QgsDataItem *parent, const QString &name, const QString &path );
    bool hasDragEnabled() const override;
    QgsMimeDataUtils::Uri mimeUri() const override;
    bool handleDoubleClick() override;
    QList< QAction * > actions( QWidget *parent ) override;


};

/**
 * Data item provider for showing Python py scripts in the browser.
 */
class QgsPyDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    int capabilities() override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

/**
 * Handles drag and drop of Python py scripts to app.
 */
class QgsPyDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT

  public:

    QString customUriProviderKey() const override;
    void handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const override;
    bool handleFileDrop( const QString &file ) override;
};


/**
 * Custom data item for XML style libraries.
 */
class QgsStyleXmlDataItem : public QgsDataItem
{
    Q_OBJECT

  public:

    QgsStyleXmlDataItem( QgsDataItem *parent, const QString &name, const QString &path );
    bool hasDragEnabled() const override;
    QgsMimeDataUtils::Uri mimeUri() const override;
    bool handleDoubleClick() override;
    QList< QAction * > actions( QWidget *parent ) override;

};

/**
 * Data item provider for showing style XML libraries in the browser.
 */
class QgsStyleXmlDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    int capabilities() override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

/**
 * Handles drag and drop of style XML libraries to app.
 */
class QgsStyleXmlDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT

  public:

    QString customUriProviderKey() const override;
    void handleCustomUriDrop( const QgsMimeDataUtils::Uri &uri ) const override;
    bool handleFileDrop( const QString &file ) override;
};

/**
 * Custom data item for qgs/qgz QGIS project files, with more functionality than default browser project
 * file handling. Specifically allows browsing of the project's layer structure within the browser
 */
class APP_EXPORT QgsProjectRootDataItem : public QgsProjectItem
{
  public:

    /**
     * Constructor for QgsProjectRootDataItem, with the specified
     * project \a path.
     */
    QgsProjectRootDataItem( QgsDataItem *parent, const QString &path );
    QVector<QgsDataItem *> createChildren() override;

};

/**
 * Represents a layer tree group node within a QGIS project file.
 */
class APP_EXPORT QgsProjectLayerTreeGroupItem : public QgsDataCollectionItem
{
  public:

    /**
     * Constructor for QgsProjectLayerTreeGroupItem, with the specified group \a name.
     */
    QgsProjectLayerTreeGroupItem( QgsDataItem *parent, const QString &name );

};

/**
 * Custom data item provider for showing qgs/qgz QGIS project files within the browser,
 * including the ability to browser the whole project's layer tree structure directly
 * within the browser.
 */
class APP_EXPORT QgsProjectDataItemProvider : public QgsDataItemProvider
{
  public:
    QString name() override;
    int capabilities() override;
    QgsDataItem *createDataItem( const QString &path, QgsDataItem *parentItem ) override;
};

#endif // QGSAPPBROWSERPROVIDERS_H
