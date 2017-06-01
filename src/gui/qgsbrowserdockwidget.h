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
#include "qgis_gui.h"
#include <QSortFilterProxyModel>

class QgsBrowserModel;
class QModelIndex;
class QgsDockBrowserTreeView;
class QgsLayerItem;
class QgsDataItem;
class QgsBrowserTreeFilterProxyModel;

/**
 * \ingroup gui
 * Hack to show wrapped text without spaces
 * \since QGIS 2.10
 */
class GUI_EXPORT QgsBrowserPropertiesWrapLabel : public QTextEdit
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsBrowserPropertiesWrapLabel
      * \param text label text
      * \param parent parent widget
      */
    QgsBrowserPropertiesWrapLabel( const QString &text, QWidget *parent = nullptr );

  private slots:
    void adjustHeight( QSizeF size );
};

/**
 * \ingroup gui
 * The QgsBrowserPropertiesWidget base class
 * \since QGIS 2.10
 */
class GUI_EXPORT QgsBrowserPropertiesWidget : public QWidget
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsBrowserPropertiesWidget
      * \param parent parent widget
      */
    explicit QgsBrowserPropertiesWidget( QWidget *parent = nullptr );
    //! Factory method to create a new browser widget
    static QgsBrowserPropertiesWidget *createWidget( QgsDataItem *item, QWidget *parent = nullptr ) SIP_FACTORY;
    //! Stub
    virtual void setItem( QgsDataItem *item ) { Q_UNUSED( item ) }
    //! Set content widget, usually item paramWidget. Takes ownership.
    virtual void setWidget( QWidget *widget );

    /** Sets whether the properties widget should display in condensed mode, ie, for display in a dock
     * widget rather than it's own separate dialog.
     * \param condensedMode set to true to enable condensed mode
     * \since QGIS 2.10
     */
    virtual void setCondensedMode( bool condensedMode ) { Q_UNUSED( condensedMode ); }
};

/**
 * \ingroup gui
 * The QgsBrowserLayerProperties class
 * \since QGIS 2.10
 */
class GUI_EXPORT QgsBrowserLayerProperties : public QgsBrowserPropertiesWidget, private Ui::QgsBrowserLayerPropertiesBase
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsBrowserLayerProperties
      * \param parent parent widget
      */
    explicit QgsBrowserLayerProperties( QWidget *parent = nullptr );
    //! Set item
    void setItem( QgsDataItem *item ) override;

    /** Sets whether the properties widget should display in condensed mode, ie, for display in a dock
     * widget rather than it's own separate dialog.
     * \param condensedMode set to true to enable condensed mode
     * \since QGIS 2.10
     */
    virtual void setCondensedMode( bool condensedMode ) override;

  private:
    QgsBrowserPropertiesWrapLabel *mUriLabel = nullptr;
};

/**
 * \ingroup gui
 * The QgsBrowserDirectoryProperties class
 * \since QGIS 2.10
 */
class GUI_EXPORT QgsBrowserDirectoryProperties : public QgsBrowserPropertiesWidget, private Ui::QgsBrowserDirectoryPropertiesBase
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsBrowserDirectoryProperties
      * \param parent parent widget
      */
    explicit QgsBrowserDirectoryProperties( QWidget *parent = nullptr );

    //! Create widget from the given item and add it
    void setItem( QgsDataItem *item ) override;
  private:
    QgsDirectoryParamWidget *mDirectoryWidget = nullptr;
    QgsBrowserPropertiesWrapLabel *mPathLabel = nullptr;
};

/**
 * \ingroup gui
 * The QgsBrowserPropertiesDialog class
 * \since QGIS 2.10
 */
class GUI_EXPORT QgsBrowserPropertiesDialog : public QDialog, private Ui::QgsBrowserPropertiesDialogBase
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsBrowserPropertiesDialog
      * \param settingsSection prefix for settings (from the object name)
      * \param parent parent widget
      */
    QgsBrowserPropertiesDialog( const QString &settingsSection, QWidget *parent = nullptr );
    ~QgsBrowserPropertiesDialog();

    //! Create dialog from the given item and add it
    void setItem( QgsDataItem *item );

  private:
    QgsBrowserPropertiesWidget *mPropertiesWidget = nullptr;
    QString mSettingsSection;
};

/**
 * \ingroup gui
 * The QgsBrowserDockWidget class
 * \since QGIS 2.10
 */
class GUI_EXPORT QgsBrowserDockWidget : public QgsDockWidget, private Ui::QgsBrowserDockWidgetBase
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsBrowserDockWidget
      * \param name name of the widget
      * \param parent parent widget
      */
    explicit QgsBrowserDockWidget( const QString &name, QWidget *parent = nullptr );
    ~QgsBrowserDockWidget();
    //! Add directory to favorites
    void addFavoriteDirectory( const QString &favDir );

  public slots:
    //! Add layer at index
    void addLayerAtIndex( const QModelIndex &index );
    //! Show context menu
    void showContextMenu( QPoint );

    //! Add current item to favorite
    void addFavorite();
    //! Add directory from file dialog to favorite
    void addFavoriteDirectory();
    //! Remove from favorite
    void removeFavorite();

    //! Refresh browser view model (and view)
    void refresh();

    //! Show/hide filter widget
    void showFilterWidget( bool visible );
    //! Enable/disable properties widget
    void enablePropertiesWidget( bool enable );
    //! Set filter syntax
    void setFilterSyntax( QAction * );
    //! Set filter case sensitivity
    void setCaseSensitive( bool caseSensitive );
    //! Apply filter to the model
    void setFilter();
    //! Update project home directory
    void updateProjectHome();


    //! Add selected layers to the project
    void addSelectedLayers();
    //! Show the layer properties
    void showProperties();
    //! Hide current item
    void hideItem();
    //! Toggle fast scan
    void toggleFastScan();

    //! Selection hass changed
    void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
    //! Splitter has been moved
    void splitterMoved();

  signals:
    //! Emitted when a file needs to be opened
    void openFile( const QString & );
    //! Emitted when drop uri list needs to be handled
    void handleDropUriList( const QgsMimeDataUtils::UriList & );

  protected:
    //! Refresh the model
    void refreshModel( const QModelIndex &index );
    //! Show event override
    void showEvent( QShowEvent *event ) override;
    //! Add a layer
    void addLayer( QgsLayerItem *layerItem );
    //! Clear the properties widget
    void clearPropertiesWidget();
    //! Set the properties widget
    void setPropertiesWidget();

    //! Count selected items
    int selectedItemsCount();
    //! Settings prefix (the object name)
    QString settingsSection() { return objectName().toLower(); }

    QgsDockBrowserTreeView *mBrowserView = nullptr;
    QgsBrowserModel *mModel = nullptr;
    QgsBrowserTreeFilterProxyModel *mProxyModel = nullptr;
    QString mInitPath;
    bool mPropertiesWidgetEnabled;
    // height fraction
    float mPropertiesWidgetHeight;

  private:
};


/**
 * \ingroup gui
 * Utility class for correct drag&drop handling.
 *
 * We want to allow user to drag layers to qgis window. At the same time we do not
 * accept drops of the items on our view - but if we ignore the drag enter action
 * then qgis application consumes the drag events and it is possible to drop the
 * items on the tree view although the drop is actually managed by qgis app.
 * \since QGIS 2.10
 */
class GUI_EXPORT QgsDockBrowserTreeView : public QgsBrowserTreeView
{
    Q_OBJECT

  public:

    /**
      * Constructor for QgsDockBrowserTreeView
      * \param parent parent widget
      */
    explicit QgsDockBrowserTreeView( QWidget *parent );
    //! Overrides drag enter event
    void dragEnterEvent( QDragEnterEvent *e ) override;
    //! Overrides drag move event
    void dragMoveEvent( QDragMoveEvent *e ) override;
    //! Overrides drag stop event
    void dropEvent( QDropEvent *e ) override;
};

/**
 * \ingroup gui
 * Utility class for filtering browser items
 * \since QGIS 2.10
 */
class GUI_EXPORT QgsBrowserTreeFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
  public:

    /**
      * Constructor for QgsBrowserTreeFilterProxyModel
      * @param parent parent widget
      */
    explicit QgsBrowserTreeFilterProxyModel( QObject *parent );
    //! Set the browser model
    void setBrowserModel( QgsBrowserModel *model );
    //! Set the filter syntax
    void setFilterSyntax( const QString &syntax );
    //! Set the filter
    void setFilter( const QString &filter );
    //! Set case sensitivity
    void setCaseSensitive( bool caseSensitive );
    //! Update filter
    void updateFilter();

  protected:

    QgsBrowserModel *mModel = nullptr;
    QString mFilter; //filter string provided
    QVector<QRegExp> mREList; //list of filters, separated by "|"
    QString mPatternSyntax;
    Qt::CaseSensitivity mCaseSensitivity;

    //! Filter accepts string
    bool filterAcceptsString( const QString &value ) const;

    //! It would be better to apply the filer only to expanded (visible) items, but using mapFromSource() + view here was causing strange errors
    bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const override;

    //! Returns true if at least one ancestor is accepted by filter
    bool filterAcceptsAncestor( const QModelIndex &sourceIndex ) const;

    //! Returns true if at least one descendant s accepted by filter
    bool filterAcceptsDescendant( const QModelIndex &sourceIndex ) const;

    //! Filter accepts item name
    bool filterAcceptsItem( const QModelIndex &sourceIndex ) const;
};


#endif // QGSBROWSERDOCKWIDGET_H
