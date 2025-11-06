/***************************************************************************
    qgstemplatedcategorizedrendererwidget_p.h
    ---------------------
    begin                : November 2025
    copyright            : (C) 2025 by Jean Felder
    email                : jean dot felder at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSTEMPLATEDCATEGORIZEDRENDERERMODEL_H
#define QGSTEMPLATEDCATEGORIZEDRENDERERMODEL_H


#include <algorithm>

#include "qgslogger.h"

#include <QAbstractItemModel>
#include <QIODevice>
#include <QMimeData>
#include <QPointer>
#include <QScreen>
#include <QString>

#define SIP_NO_FILE

using namespace Qt::StringLiterals;


///@cond PRIVATE

/**
 * \ingroup gui
 * \brief A model for managing categories in a 2D or 3D categorized renderer widget
 * displayed in a tree view.
 *
 * \tparam RendererType The type of categorized renderer (2D or 3D).
 */
template<typename RendererType>
class QgsTemplatedCategorizedRendererModel : public QAbstractItemModel
{
  public:
    /**
     * Constructs a new categorized renderer model.
     *
     * \param parent Parent object
     * \param screen Screen for scaling calculations.
     */
    QgsTemplatedCategorizedRendererModel( QObject *parent = nullptr, QScreen *screen = nullptr )
      : QAbstractItemModel( parent )
      , mMimeFormat( u"application/x-qgscategorizedsymbolrendererv2model"_s )
      , mScreen( screen )
    {
    }

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const override = 0;

    virtual QVariant data( const QModelIndex &index, int role ) const override = 0;
    virtual bool setData( const QModelIndex &index, const QVariant &value, int role ) override = 0;
    virtual QVariant headerData( int section, Qt::Orientation orientation, int role ) const override = 0;

    virtual void sort( int column, Qt::SortOrder order = Qt::AscendingOrder ) override = 0;

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override
    {
      if ( parent.isValid() || !mRenderer )
      {
        return 0;
      }
      return static_cast<int>( mRenderer->categories().size() );
    }

    int columnCount( const QModelIndex & = QModelIndex() ) const override = 0;

    Qt::DropActions supportedDropActions() const override
    {
      return Qt::MoveAction;
    }

    QModelIndex index( int row, int column, const QModelIndex &parent = QModelIndex() ) const override
    {
      if ( hasIndex( row, column, parent ) )
      {
        return createIndex( row, column );
      }
      return QModelIndex();
    }

    QModelIndex parent( const QModelIndex &index ) const override
    {
      Q_UNUSED( index )
      return QModelIndex();
    }

    QStringList mimeTypes() const override
    {
      QStringList types;
      types << mMimeFormat;
      return types;
    }

    QMimeData *mimeData( const QModelIndexList &indexes ) const override
    {
      QMimeData *mimeData = new QMimeData();
      QByteArray encodedData;

      QDataStream stream( &encodedData, QIODevice::WriteOnly );

      const auto constIndexes = indexes;
      for ( const QModelIndex &index : constIndexes )
      {
        if ( !index.isValid() || index.column() != 0 )
          continue;

        stream << index.row();
      }
      mimeData->setData( mMimeFormat, encodedData );
      return mimeData;
    }

    bool dropMimeData( const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent ) override
    {
      Q_UNUSED( column )
      Q_UNUSED( parent )
      if ( action != Qt::MoveAction )
        return true;

      if ( !data->hasFormat( mMimeFormat ) )
        return false;

      QByteArray encodedData = data->data( mMimeFormat );
      QDataStream stream( &encodedData, QIODevice::ReadOnly );

      QVector<int> rows;
      while ( !stream.atEnd() )
      {
        int r;
        stream >> r;
        rows.append( r );
      }

      std::sort( rows.begin(), rows.end() );

      int to = row;

      if ( to == -1 )
        to = static_cast<int>( mRenderer->categories().size() );
      for ( int i = static_cast<int>( rows.size() ) - 1; i >= 0; i-- )
      {
        QgsDebugMsgLevel( u"move %1 to %2"_s.arg( rows[i] ).arg( to ), 2 );
        int t = to;
        if ( rows[i] < t )
          t--;
        mRenderer->moveCategory( rows[i], t );
        for ( int j = 0; j < i; j++ )
        {
          if ( to < rows[j] && rows[i] > rows[j] )
            rows[j] += 1;
        }
        if ( rows[i] < to )
          to--;
      }
      emit dataChanged( createIndex( 0, 0 ), createIndex( static_cast<int>( mRenderer->categories().size() ), 0 ) );
      onRowsMoved();
      return false;
    }

    /**
     * Sets the renderer for this model.
     *
     * \param renderer The new renderer. The model does not take ownership of the renderer.
     */
    void setRenderer( RendererType *renderer )
    {
      if ( mRenderer )
      {
        beginRemoveRows( QModelIndex(), 0, std::max<int>( static_cast<int>( mRenderer->categories().size() ) - 1, 0 ) );
        mRenderer = nullptr;
        endRemoveRows();
      }
      if ( renderer )
      {
        mRenderer = renderer;
        if ( renderer->categories().size() > 0 )
        {
          beginInsertRows( QModelIndex(), 0, static_cast<int>( renderer->categories().size() ) - 1 );
          endInsertRows();
        }
      }
    }

    /**
     * Adds a category to the renderer.
     *
     * \param cat The category to add.
     */
    void addCategory( const typename RendererType::Category &cat )
    {
      if ( !mRenderer )
        return;
      const int idx = static_cast<int>( mRenderer->categories().size() );
      beginInsertRows( QModelIndex(), idx, idx );
      mRenderer->addCategory( cat );
      endInsertRows();
    }

    /**
     * Returns the category at the given model index.
     *
     * \param index The model index to retrieve the category from.
     * \return The category at the specified index, or a default empty category in case of error.
     */
    typename RendererType::Category category( const QModelIndex &index )
    {
      if ( !mRenderer )
      {
        return typename RendererType::Category();
      }
      const auto &catList = mRenderer->categories();
      const int row = index.row();
      if ( row >= catList.size() )
      {
        return typename RendererType::Category();
      }
      return catList.at( row );
    }

    /**
     * Deletes the categories at the specified rows.
     *
     * \param rows List of row indices to delete.
     */
    void deleteRows( QList<int> rows )
    {
      std::sort( rows.begin(), rows.end() );
      for ( int i = static_cast<int>( rows.size() ) - 1; i >= 0; i-- )
      {
        beginRemoveRows( QModelIndex(), rows[i], rows[i] );
        mRenderer->deleteCategory( rows[i] );
        endRemoveRows();
      }
    }

    /**
     * Removes all categories from the renderer.
     */
    void removeAllRows()
    {
      beginRemoveRows( QModelIndex(), 0, static_cast<int>( mRenderer->categories().size() ) - 1 );
      mRenderer->deleteAllCategories();
      endRemoveRows();
    }

    /**
     * Updates the symbology of all categories.
     */
    void updateSymbology()
    {
      emit dataChanged( createIndex( 0, 0 ), createIndex( static_cast<int>( mRenderer->categories().size() ), 0 ) );
    }

  protected:
    /**
     * Called when rows have been moved in the model.
     */
    virtual void onRowsMoved() = 0;

  protected:
    RendererType *mRenderer = nullptr;
    QString mMimeFormat;
    QPointer<QScreen> mScreen;
};

/// @endcond

#endif // QGSTEMPLATEDCATEGORIZEDRENDERERMODEL_H
