class QgsBrowserModel : QAbstractItemModel
{
%TypeHeaderCode
#include <qgsbrowsermodel.h>
%End

  public:
    explicit QgsBrowserModel( QObject *parent = 0 );
    ~QgsBrowserModel();

    enum ItemDataRole
    {
      // item path used to access path in the tree, see QgsDataItem::mPath
      PathRole
    };
    // implemented methods from QAbstractItemModel for read-only access

    /** Used by other components to obtain information about each item provided by the model.
      In many models, the combination of flags should include Qt::ItemIsEnabled and Qt::ItemIsSelectable. */
    virtual Qt::ItemFlags flags( const QModelIndex &index ) const;

    /** Used to supply item data to views and delegates. Generally, models only need to supply data
      for Qt::DisplayRole and any application-specific user roles, but it is also good practice
      to provide data for Qt::ToolTipRole, Qt::AccessibleTextRole, and Qt::AccessibleDescriptionRole.
      See the Qt::ItemDataRole enum documentation for information about the types associated with each role. */
    virtual QVariant data( const QModelIndex &index, int role = Qt::DisplayRole ) const;

    /** Provides views with information to show in their headers. The information is only retrieved
      by views that can display header information. */
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    /** Provides the number of rows of data exposed by the model. */
    virtual int rowCount( const QModelIndex &parent = QModelIndex() ) const;

    /** Provides the number of columns of data exposed by the model. List models do not provide this function
      because it is already implemented in QAbstractListModel. */
    virtual int columnCount( const QModelIndex &parent = QModelIndex() ) const;

    /** Returns the index of the item in the model specified by the given row, column and parent index. */
    virtual QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;

    QModelIndex findItem( QgsDataItem *item, QgsDataItem *parent = 0 ) const;

    /** Returns the parent of the model item with the given index.
     * If the item has no parent, an invalid QModelIndex is returned.
     */
    virtual QModelIndex parent( const QModelIndex &index ) const;

    /** Returns a list of mime that can describe model indexes */
    virtual QStringList mimeTypes() const;

    /** Returns an object that contains serialized items of data corresponding to the list of indexes specified */
    virtual QMimeData * mimeData( const QModelIndexList &indexes ) const;

    /** Handles the data supplied by a drag and drop operation that ended with the given action */
    virtual bool dropMimeData( const QMimeData * data, Qt::DropAction action, int row, int column, const QModelIndex & parent );

    QgsDataItem *dataItem( const QModelIndex &idx ) const;

    bool hasChildren( const QModelIndex &parent = QModelIndex() ) const;

    // Refresh item specified by path
    void refresh( QString path );

    // Refresh item childs
    void refresh( const QModelIndex &index = QModelIndex() );

    /** Return index of item with given path. It only searches in currently fetched
     * items, i.e. it does not fetch children.
     * @param path item path
     * @param matchFlag supported is Qt::MatchExactly and Qt::MatchStartsWith which has reverse meaning, i.e. find
     *        item with the longest match from start with path (to get as close/deep as possible to deleted item).
     * @return model index, invalid if item not found */
    QModelIndex findPath( QString path, Qt::MatchFlag matchFlag = Qt::MatchExactly );

    void connectItem( QgsDataItem *item );

    bool canFetchMore( const QModelIndex & parent ) const;
    void fetchMore( const QModelIndex & parent );

  public slots:
    // Reload the whole model
    void reload();
    void beginInsertItems( QgsDataItem *parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsDataItem *parent, int first, int last );
    void endRemoveItems();
    void itemDataChanged( QgsDataItem * item );
    void itemStateChanged( QgsDataItem * item, QgsDataItem::State oldState );

    void addFavouriteDirectory( QString favDir );
    void removeFavourite( const QModelIndex &index );
    void updateProjectHome();

  protected:
    // populates the model
    void addRootItems();
    void removeRootItems();
};
