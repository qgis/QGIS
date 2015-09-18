/***************************************************************************
    qgsbrowserdockwidget.h
    ---------------------
    begin                : July 2011
    copyright            : (C) 2011 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSBROWSERDOCKWIDGET_H
#define QGSBROWSERDOCKWIDGET_H

#include <QDockWidget>
#include <ui_qgsbrowserdockwidgetbase.h>
#include <ui_qgsbrowserlayerpropertiesbase.h>
#include <ui_qgsbrowserdirectorypropertiesbase.h>
#include <ui_qgsbrowserpropertiesdialogbase.h>

#include "qgsdataitem.h"

class QgsBrowserModel;
class QModelIndex;
class QgsDockBrowserTreeView;
class QgsLayerItem;
class QgsDataItem;
class QgsBrowserTreeFilterProxyModel;

// hack to show wrapped text without spaces
class QgsBrowserPropertiesWrapLabel : public QTextEdit
{
    Q_OBJECT
  public:
    QgsBrowserPropertiesWrapLabel( const QString& text, QWidget* parent = 0 );

  private slots:
    void adjustHeight( const QSizeF& size );
};

class QgsBrowserPropertiesWidget : public QWidget
{
    Q_OBJECT
  public:
    QgsBrowserPropertiesWidget( QWidget* parent = 0 );
    static QgsBrowserPropertiesWidget* createWidget( QgsDataItem* item, QWidget* parent = 0 );
    virtual void setItem( QgsDataItem* item ) = 0;

    /** Sets whether the properties widget should display in condensed mode, ie, for display in a dock
     * widget rather than it's own separate dialog.
     * @param condensedMode set to true to enable condensed mode
     * @note added in QGIS 2.10
     */
    virtual void setCondensedMode( bool condensedMode ) { Q_UNUSED( condensedMode ); }
};

class QgsBrowserLayerProperties : public QgsBrowserPropertiesWidget, private Ui::QgsBrowserLayerPropertiesBase
{
    Q_OBJECT
  public:
    QgsBrowserLayerProperties( QWidget* parent = 0 );
    void setItem( QgsDataItem* item ) override;

    virtual void setCondensedMode( bool condensedMode ) override;

  private:
    QgsBrowserPropertiesWrapLabel *mUriLabel;
};

class QgsBrowserDirectoryProperties : public QgsBrowserPropertiesWidget , private Ui::QgsBrowserDirectoryPropertiesBase
{
    Q_OBJECT
  public:
    QgsBrowserDirectoryProperties( QWidget* parent = 0 );

    void setItem( QgsDataItem* item ) override;
  private:
    QgsDirectoryParamWidget* mDirectoryWidget;
    QgsBrowserPropertiesWrapLabel *mPathLabel;
};

class QgsBrowserPropertiesDialog : public QDialog , private Ui::QgsBrowserPropertiesDialogBase
{
    Q_OBJECT
  public:
    QgsBrowserPropertiesDialog( QString settingsSection, QWidget* parent = 0 );
    ~QgsBrowserPropertiesDialog();

    void setItem( QgsDataItem* item );

  private:
    QgsBrowserPropertiesWidget* mPropertiesWidget;
    QString mSettingsSection;
};

class APP_EXPORT QgsBrowserDockWidget : public QDockWidget, private Ui::QgsBrowserDockWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsBrowserDockWidget( QString name, QWidget *parent = 0 );
    ~QgsBrowserDockWidget();
    void addFavouriteDirectory( QString favDir );

  public slots:
    void addLayerAtIndex( const QModelIndex& index );
    void showContextMenu( const QPoint & );

    void addFavourite();
    void addFavouriteDirectory();
    void removeFavourite();

    void refresh();

    void showFilterWidget( bool visible );
    void enablePropertiesWidget( bool enable );
    void setFilterSyntax( QAction * );
    void setCaseSensitive( bool caseSensitive );
    void setFilter();

    // layer menu items
    void addCurrentLayer();
    void addSelectedLayers();
    void showProperties();
    void toggleFastScan();

    void selectionChanged( const QItemSelection & selected, const QItemSelection & deselected );
    void splitterMoved();

  protected:
    void refreshModel( const QModelIndex& index );
    void showEvent( QShowEvent * event ) override;
    void addLayer( QgsLayerItem *layerItem );
    void clearPropertiesWidget();
    void setPropertiesWidget();

    int selectedItemsCount();
    QString settingsSection() { return objectName().toLower(); }

    QgsDockBrowserTreeView* mBrowserView;
    QgsBrowserModel* mModel;
    QgsBrowserTreeFilterProxyModel* mProxyModel;
    QString mInitPath;
    bool mPropertiesWidgetEnabled;
    // height fraction
    float mPropertiesWidgetHeight;

  private:
};

#endif // QGSBROWSERDOCKWIDGET_H
