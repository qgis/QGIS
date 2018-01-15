/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsbrowsermodel.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/




class QgsBrowserModel : QAbstractItemModel
{

%TypeHeaderCode
#include "qgsbrowsermodel.h"
%End
  public:
    explicit QgsBrowserModel( QObject *parent = 0 );
    ~QgsBrowserModel();

    enum ItemDataRole
    {
      PathRole,
      CommentRole,
      SortRole,
    };

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

%Docstring
Used by other components to obtain information about each item provided by the model.
In many models, the combination of flags should include Qt.ItemIsEnabled and Qt.ItemIsSelectable. *
%End

    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

%Docstring
Used to supply item data to views and delegates. Generally, models only need to supply data
for Qt.DisplayRole and any application-specific user roles, but it is also good practice
to provide data for Qt.ToolTipRole, Qt.AccessibleTextRole, and Qt.AccessibleDescriptionRole.
See the Qt.ItemDataRole enum documentation for information about the types associated with each role. *
%End

    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

%Docstring
Provides views with information to show in their headers. The information is only retrieved
by views that can display header information. *
%End

    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

%Docstring
Provides the number of rows of data exposed by the model.
%End

    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

%Docstring
Provides the number of columns of data exposed by the model. List models do not provide this function
because it is already implemented in QAbstractListModel. *
%End

    virtual QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const;

%Docstring
Returns the index of the item in the model specified by the given row, column and parent index.
%End

    QModelIndex findItem( QgsDataItem *item, QgsDataItem *parent = 0 ) const;

    virtual QModelIndex parent( const QModelIndex &index ) const;

%Docstring
Returns the parent of the model item with the given index.
If the item has no parent, an invalid QModelIndex is returned.
%End

    virtual QStringList mimeTypes() const;

%Docstring
Returns a list of mime that can describe model indexes
%End

    virtual QMimeData *mimeData( const QModelIndexList &indexes ) const;

%Docstring
Returns an object that contains serialized items of data corresponding to the list of indexes specified
%End

    virtual bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent );

%Docstring
Handles the data supplied by a drag and drop operation that ended with the given action
%End

    QgsDataItem *dataItem( const QModelIndex &idx ) const;

    virtual bool hasChildren( const QModelIndex &parent = QModelIndex() ) const;


    void refresh( const QString &path );
%Docstring
Refresh item specified by path
%End

    void refresh( const QModelIndex &index = QModelIndex() );
%Docstring
Refresh item children
%End

    QModelIndex findPath( const QString &path, Qt::MatchFlag matchFlag = Qt::MatchExactly );
%Docstring
Return index of item with given path. It only searches in currently fetched
items, i.e. it does not fetch children.

:param path: item path
:param matchFlag: supported is Qt.MatchExactly and Qt.MatchStartsWith which has reverse meaning, i.e. find
item with the longest match from start with path (to get as close/deep as possible to deleted item).

:return: model index, invalid if item not found *
%End


    void connectItem( QgsDataItem *item );

    virtual bool canFetchMore( const QModelIndex &parent ) const;

    virtual void fetchMore( const QModelIndex &parent );


    bool initialized( ) const;
%Docstring
Returns true if the model has been initialized
%End

  signals:
    void stateChanged( const QModelIndex &index, QgsDataItem::State oldState );
%Docstring
Emitted when item children fetch was finished
%End

    void connectionsChanged();
%Docstring
Connections changed in the browser, forwarded to the widget and used to
notify the provider dialogs of a changed connection
%End

  public slots:
    void reload();
%Docstring
Reload the whole model
%End
    void beginInsertItems( QgsDataItem *parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsDataItem *parent, int first, int last );
    void endRemoveItems();
    void itemDataChanged( QgsDataItem *item );
    void itemStateChanged( QgsDataItem *item, QgsDataItem::State oldState );

    void addFavoriteDirectory( const QString &directory, const QString &name = QString() );
%Docstring
Adds a ``directory`` to the favorites group.

If ``name`` is specified, it will be used as the favorite's name. Otherwise
the name will be set to match ``directory``.

.. versionadded:: 3.0

.. seealso:: :py:func:`removeFavorite`
%End

    void removeFavorite( const QModelIndex &index );
%Docstring
Removes a favorite directory from its corresponding model index.

.. versionadded:: 3.0

.. seealso:: :py:func:`addFavoriteDirectory`
%End

    void updateProjectHome();

    void hidePath( QgsDataItem *item );
%Docstring
Hide the given path in the browser model
%End

    void initialize();
%Docstring
Delayed initialization, needed because the provider registry must be already populated
%End

  protected:
    void addRootItems();
%Docstring
Populates the model
%End
    void removeRootItems();


};

/************************************************************************
 * This file has been generated automatically from                      *
 *                                                                      *
 * src/core/qgsbrowsermodel.h                                           *
 *                                                                      *
 * Do not edit manually ! Edit header and run scripts/sipify.pl again   *
 ************************************************************************/
