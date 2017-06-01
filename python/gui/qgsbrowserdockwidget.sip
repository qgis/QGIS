/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsbrowserdockwidget.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsBrowserPropertiesWrapLabel : QTextEdit
{
%Docstring
 Hack to show wrapped text without spaces
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsbrowserdockwidget.h"
%End
  public:

    QgsBrowserPropertiesWrapLabel( const QString &text, QWidget *parent = 0 );
%Docstring
 Constructor for QgsBrowserPropertiesWrapLabel
 \param text label text
 \param parent parent widget
%End

};

class QgsBrowserPropertiesWidget : QWidget
{
%Docstring
 The QgsBrowserPropertiesWidget base class
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsbrowserdockwidget.h"
%End
  public:

    explicit QgsBrowserPropertiesWidget( QWidget *parent = 0 );
%Docstring
 Constructor for QgsBrowserPropertiesWidget
 \param parent parent widget
%End
    static QgsBrowserPropertiesWidget *createWidget( QgsDataItem *item, QWidget *parent = 0 ) /Factory/;
%Docstring
Factory method to create a new browser widget
 :rtype: QgsBrowserPropertiesWidget
%End
    virtual void setItem( QgsDataItem *item );
%Docstring
Stub
%End
    virtual void setWidget( QWidget *widget );
%Docstring
Set content widget, usually item paramWidget. Takes ownership.
%End

    virtual void setCondensedMode( bool condensedMode );
%Docstring
 Sets whether the properties widget should display in condensed mode, ie, for display in a dock
 widget rather than it's own separate dialog.
 \param condensedMode set to true to enable condensed mode
.. versionadded:: 2.10
%End
};

class QgsBrowserLayerProperties : QgsBrowserPropertiesWidget
{
%Docstring
 The QgsBrowserLayerProperties class
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsbrowserdockwidget.h"
%End
  public:

    explicit QgsBrowserLayerProperties( QWidget *parent = 0 );
%Docstring
 Constructor for QgsBrowserLayerProperties
 \param parent parent widget
%End
    virtual void setItem( QgsDataItem *item );

%Docstring
Set item
%End

    virtual void setCondensedMode( bool condensedMode );
%Docstring
 Sets whether the properties widget should display in condensed mode, ie, for display in a dock
 widget rather than it's own separate dialog.
 \param condensedMode set to true to enable condensed mode
.. versionadded:: 2.10
%End

};

class QgsBrowserDirectoryProperties : QgsBrowserPropertiesWidget
{
%Docstring
 The QgsBrowserDirectoryProperties class
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsbrowserdockwidget.h"
%End
  public:

    explicit QgsBrowserDirectoryProperties( QWidget *parent = 0 );
%Docstring
 Constructor for QgsBrowserDirectoryProperties
 \param parent parent widget
%End

    virtual void setItem( QgsDataItem *item );

%Docstring
Create widget from the given item and add it
%End
};

class QgsBrowserPropertiesDialog : QDialog
{
%Docstring
 The QgsBrowserPropertiesDialog class
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsbrowserdockwidget.h"
%End
  public:

    QgsBrowserPropertiesDialog( const QString &settingsSection, QWidget *parent = 0 );
%Docstring
 Constructor for QgsBrowserPropertiesDialog
 \param settingsSection prefix for settings (from the object name)
 \param parent parent widget
%End
    ~QgsBrowserPropertiesDialog();

    void setItem( QgsDataItem *item );
%Docstring
Create dialog from the given item and add it
%End

};

class QgsBrowserDockWidget : QgsDockWidget
{
%Docstring
 The QgsBrowserDockWidget class
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsbrowserdockwidget.h"
%End
  public:

    explicit QgsBrowserDockWidget( const QString &name, QWidget *parent = 0 );
%Docstring
 Constructor for QgsBrowserDockWidget
 \param name name of the widget
 \param parent parent widget
%End
    ~QgsBrowserDockWidget();
    void addFavoriteDirectory( const QString &favDir );
%Docstring
Add directory to favorites
%End

  public slots:
    void addLayerAtIndex( const QModelIndex &index );
%Docstring
Add layer at index
%End
    void showContextMenu( QPoint );
%Docstring
Show context menu
%End

    void addFavorite();
%Docstring
Add current item to favorite
%End
    void addFavoriteDirectory();
%Docstring
Add directory from file dialog to favorite
%End
    void removeFavorite();
%Docstring
Remove from favorite
%End

    void refresh();
%Docstring
Refresh browser view model (and view)
%End

    void showFilterWidget( bool visible );
%Docstring
Show/hide filter widget
%End
    void enablePropertiesWidget( bool enable );
%Docstring
Enable/disable properties widget
%End
    void setFilterSyntax( QAction * );
%Docstring
Set filter syntax
%End
    void setCaseSensitive( bool caseSensitive );
%Docstring
Set filter case sensitivity
%End
    void setFilter();
%Docstring
Apply filter to the model
%End
    void updateProjectHome();
%Docstring
Update project home directory
%End


    void addSelectedLayers();
%Docstring
Add selected layers to the project
%End
    void showProperties();
%Docstring
Show the layer properties
%End
    void hideItem();
%Docstring
Hide current item
%End
    void toggleFastScan();
%Docstring
Toggle fast scan
%End

    void selectionChanged( const QItemSelection &selected, const QItemSelection &deselected );
%Docstring
Selection hass changed
%End
    void splitterMoved();
%Docstring
Splitter has been moved
%End

  signals:
    void openFile( const QString & );
%Docstring
Emitted when a file needs to be opened
%End
    void handleDropUriList( const QgsMimeDataUtils::UriList & );
%Docstring
Emitted when drop uri list needs to be handled
%End

  protected:
    void refreshModel( const QModelIndex &index );
%Docstring
Refresh the model
%End
    virtual void showEvent( QShowEvent *event );

%Docstring
Show event override
%End
    void addLayer( QgsLayerItem *layerItem );
%Docstring
Add a layer
%End
    void clearPropertiesWidget();
%Docstring
Clear the properties widget
%End
    void setPropertiesWidget();
%Docstring
Set the properties widget
%End

    int selectedItemsCount();
%Docstring
Count selected items
 :rtype: int
%End
    QString settingsSection();
%Docstring
Settings prefix (the object name)
 :rtype: str
%End


};


class QgsDockBrowserTreeView : QgsBrowserTreeView
{
%Docstring
 Utility class for correct drag&drop handling.

 We want to allow user to drag layers to qgis window. At the same time we do not
 accept drops of the items on our view - but if we ignore the drag enter action
 then qgis application consumes the drag events and it is possible to drop the
 items on the tree view although the drop is actually managed by qgis app.
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsbrowserdockwidget.h"
%End
  public:

    explicit QgsDockBrowserTreeView( QWidget *parent );
%Docstring
 Constructor for QgsDockBrowserTreeView
 \param parent parent widget
%End
    virtual void dragEnterEvent( QDragEnterEvent *e );

%Docstring
Overrides drag enter event
%End
    virtual void dragMoveEvent( QDragMoveEvent *e );

%Docstring
Overrides drag move event
%End
    virtual void dropEvent( QDropEvent *e );

%Docstring
Overrides drag stop event
%End
};

class QgsBrowserTreeFilterProxyModel : QSortFilterProxyModel
{
%Docstring
 Utility class for filtering browser items
.. versionadded:: 2.10
%End

%TypeHeaderCode
#include "qgsbrowserdockwidget.h"
%End
  public:

    explicit QgsBrowserTreeFilterProxyModel( QObject *parent );
%Docstring
 Constructor for QgsBrowserTreeFilterProxyModel
 @param parent parent widget
%End
    void setBrowserModel( QgsBrowserModel *model );
%Docstring
Set the browser model
%End
    void setFilterSyntax( const QString &syntax );
%Docstring
Set the filter syntax
%End
    void setFilter( const QString &filter );
%Docstring
Set the filter
%End
    void setCaseSensitive( bool caseSensitive );
%Docstring
Set case sensitivity
%End
    void updateFilter();
%Docstring
Update filter
%End

  protected:


    bool filterAcceptsString( const QString &value ) const;
%Docstring
Filter accepts string
 :rtype: bool
%End

    virtual bool filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const;

%Docstring
It would be better to apply the filer only to expanded (visible) items, but using mapFromSource() + view here was causing strange errors
 :rtype: bool
%End

    bool filterAcceptsAncestor( const QModelIndex &sourceIndex ) const;
%Docstring
Returns true if at least one ancestor is accepted by filter
 :rtype: bool
%End

    bool filterAcceptsDescendant( const QModelIndex &sourceIndex ) const;
%Docstring
Returns true if at least one descendant s accepted by filter
 :rtype: bool
%End

    bool filterAcceptsItem( const QModelIndex &sourceIndex ) const;
%Docstring
Filter accepts item name
 :rtype: bool
%End
};


/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/gui/qgsbrowserdockwidget.h                                       *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
