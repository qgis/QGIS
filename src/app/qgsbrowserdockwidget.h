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

#include <ui_qgsbrowserdockwidgetbase.h>
#include <ui_qgsbrowserlayerpropertiesbase.h>
#include <ui_qgsbrowserdirectorypropertiesbase.h>
#include <ui_qgsbrowserpropertiesdialogbase.h>

#include "qgsdataitem.h"
#include "qgsbrowsertreeview.h"
#include "qgsdockwidget.h"
#include <QSortFilterProxyModel>

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
    QgsBrowserPropertiesWrapLabel( const QString& text, QWidget* parent = nullptr );

  private slots:
    void adjustHeight( QSizeF size );
};

class QgsBrowserPropertiesWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit QgsBrowserPropertiesWidget( QWidget* parent = nullptr );
    static QgsBrowserPropertiesWidget* createWidget( QgsDataItem* item, QWidget* parent = nullptr );
    virtual void setItem( QgsDataItem* item ) { Q_UNUSED( item ) }
    /** Set content widget, usually item paramWidget. Takes ownership. */
    virtual void setWidget( QWidget* widget );

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
    explicit QgsBrowserLayerProperties( QWidget* parent = nullptr );
    void setItem( QgsDataItem* item ) override;

    virtual void setCondensedMode( bool condensedMode ) override;

  private:
    QgsBrowserPropertiesWrapLabel *mUriLabel;
};

class QgsBrowserDirectoryProperties : public QgsBrowserPropertiesWidget , private Ui::QgsBrowserDirectoryPropertiesBase
{
    Q_OBJECT
  public:
    explicit QgsBrowserDirectoryProperties( QWidget* parent = nullptr );

    void setItem( QgsDataItem* item ) override;
  private:
    QgsDirectoryParamWidget* mDirectoryWidget;
    QgsBrowserPropertiesWrapLabel *mPathLabel;
};

class QgsBrowserPropertiesDialog : public QDialog , private Ui::QgsBrowserPropertiesDialogBase
{
    Q_OBJECT
  public:
    QgsBrowserPropertiesDialog( const QString& settingsSection, QWidget* parent = nullptr );
    ~QgsBrowserPropertiesDialog();

    void setItem( QgsDataItem* item );

  private:
    QgsBrowserPropertiesWidget* mPropertiesWidget;
    QString mSettingsSection;
};

class APP_EXPORT QgsBrowserDockWidget : public QgsDockWidget, private Ui::QgsBrowserDockWidgetBase
{
    Q_OBJECT
  public:
    explicit QgsBrowserDockWidget( const QString& name, QWidget *parent = nullptr );
    ~QgsBrowserDockWidget();
    void addFavouriteDirectory( const QString& favDir );

  public slots:
    void addLayerAtIndex( const QModelIndex& index );
    void showContextMenu( QPoint );

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
    void hideItem();
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


/**
Utility class for correct drag&drop handling.

We want to allow user to drag layers to qgis window. At the same time we do not
accept drops of the items on our view - but if we ignore the drag enter action
then qgis application consumes the drag events and it is possible to drop the
items on the tree view although the drop is actually managed by qgis app.
 */
class QgsDockBrowserTreeView : public QgsBrowserTreeView
{
    Q_OBJECT

  public:
    explicit QgsDockBrowserTreeView( QWidget* parent );

    void dragEnterEvent( QDragEnterEvent* e ) override;
    void dragMoveEvent( QDragMoveEvent* e ) override;
};

/**
Utility class for filtering browser items
 */
class QgsBrowserTreeFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
  public:
    explicit QgsBrowserTreeFilterProxyModel( QObject *parent );

    void setBrowserModel( QgsBrowserModel* model );

    void setFilterSyntax( const QString & syntax );

    void setFilter( const QString & filter );

    void setCaseSensitive( bool caseSensitive );

    void updateFilter();

  protected:

    QgsBrowserModel* mModel;
    QString mFilter; //filter string provided
    QVector<QRegExp> mREList; //list of filters, separated by "|"
    QString mPatternSyntax;
    Qt::CaseSensitivity mCaseSensitivity;

    bool filterAcceptsString( const QString & value ) const;

    // It would be better to apply the filer only to expanded (visible) items, but using mapFromSource() + view here was causing strange errors
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

    // returns true if at least one ancestor is accepted by filter
    bool filterAcceptsAncestor( const QModelIndex &sourceIndex ) const;

    // returns true if at least one descendant s accepted by filter
    bool filterAcceptsDescendant( const QModelIndex &sourceIndex ) const;

    // filter accepts item name
    bool filterAcceptsItem( const QModelIndex &sourceIndex ) const;
};


#endif // QGSBROWSERDOCKWIDGET_H
