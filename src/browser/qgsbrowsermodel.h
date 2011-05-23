#ifndef QGSBROWSERMODEL_H
#define QGSBROWSERMODEL_H

#include <QAbstractItemModel>
#include <QIcon>

#include "qgsdataitem.h"

class QgsBrowserModel : public QAbstractItemModel
{
    Q_OBJECT
  public:
    explicit QgsBrowserModel( QObject *parent = 0 );
    ~QgsBrowserModel();

    // implemented methods from QAbstractItemModel for read-only access

    /** Used by other components to obtain information about each item provided by the model.
      In many models, the combination of flags should include Qt::ItemIsEnabled and Qt::ItemIsSelectable. */
    virtual Qt::ItemFlags flags( const QModelIndex & index ) const;
    /** Used to supply item data to views and delegates. Generally, models only need to supply data
      for Qt::DisplayRole and any application-specific user roles, but it is also good practice
      to provide data for Qt::ToolTipRole, Qt::AccessibleTextRole, and Qt::AccessibleDescriptionRole.
      See the Qt::ItemDataRole enum documentation for information about the types associated with each role. */
    virtual QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const;
    /** Provides views with information to show in their headers. The information is only retrieved
      by views that can display header information. */
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role = Qt::DisplayRole ) const;

    /** Provides the number of rows of data exposed by the model. */
    virtual int rowCount( const QModelIndex & parent = QModelIndex() ) const;
    /** Provides the number of columns of data exposed by the model. List models do not provide this function
      because it is already implemented in QAbstractListModel. */
    virtual int columnCount( const QModelIndex & parent = QModelIndex() ) const;

    /** Returns the index of the item in the model specified by the given row, column and parent index. */
    virtual QModelIndex index( int row, int column, const QModelIndex & parent = QModelIndex() ) const;

    QModelIndex index( QgsDataItem *item );

    /** Returns the parent of the model item with the given index. If the item has no parent, an invalid QModelIndex is returned. */
    virtual QModelIndex parent( const QModelIndex & index ) const;


    bool hasChildren( const QModelIndex & parent = QModelIndex() ) const;

    // Refresh item specified by path
    void refresh( QString path, const QModelIndex& index = QModelIndex() );
    // Refresh item childs
    void refresh( const QModelIndex& index = QModelIndex() );

    void connectItem( QgsDataItem * item );
  signals:

  public slots:
    //void removeItems( QgsDataItem * parent, QVector<QgsDataItem *>items );
    //void addItems( QgsDataItem * parent, QVector<QgsDataItem *>items );
    //void refreshItems( QgsDataItem * parent, QVector<QgsDataItem *>items );

    void beginInsertItems( QgsDataItem* parent, int first, int last );
    void endInsertItems();
    void beginRemoveItems( QgsDataItem* parent, int first, int last );
    void endRemoveItems();

  protected:
    QVector<QgsDataItem*> mRootItems;
    QIcon mIconDirectory;
};

#endif // QGSBROWSERMODEL_H
