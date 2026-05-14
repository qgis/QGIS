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
#include "qgsvariantutils.h"

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
template<typename RendererType> class QgsTemplatedCategorizedRendererModel : public QAbstractItemModel
{
  public:
    using Category = typename RendererType::Category;

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
    {}

    virtual int symbolColumn() const { return 0; }

    virtual int valueColumn() const { return 1; }

    virtual QVariant headerData( int section, Qt::Orientation orientation, int role ) const override = 0;

    virtual void sort( int column, Qt::SortOrder order = Qt::AscendingOrder ) override = 0;

    virtual Qt::ItemFlags flags( const QModelIndex &index ) const override
    {
      // Flat list, to ease drop handling valid indexes are not dropEnabled
      if ( !index.isValid() || !mRenderer )
      {
        return Qt::ItemIsDropEnabled;
      }

      Qt::ItemFlags flags = Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable;
      if ( index.column() == valueColumn() )
      {
        const Category category = mRenderer->categories().value( index.row() );
        if ( category.value().userType() != QMetaType::Type::QVariantList )
        {
          flags |= Qt::ItemIsEditable;
        }
      }

      // allow subclasses to extend flags
      flags |= extraFlags( index );

      return flags;
    }

    virtual QVariant data( const QModelIndex &index, int role ) const override
    {
      if ( !index.isValid() || !mRenderer )
      {
        return QVariant();
      }

      const Category category = mRenderer->categories().value( index.row() );

      switch ( role )
      {
        case Qt::CheckStateRole:
        {
          if ( index.column() == symbolColumn() )
          {
            return category.renderState() ? Qt::Checked : Qt::Unchecked;
          }
          break;
        }

        case Qt::DisplayRole:
        case Qt::ToolTipRole:
        {
          if ( index.column() == valueColumn() )
          {
            if ( category.value().userType() == QMetaType::Type::QVariantList )
            {
              QStringList res;
              const QVariantList list = category.value().toList();
              res.reserve( list.size() );
              for ( const QVariant &variant : list )
                res << QgsVariantUtils::displayString( variant );

              if ( role == Qt::DisplayRole )
                return res.join( ';' );
              else // tooltip
                return res.join( '\n' );
            }
            else if ( QgsVariantUtils::isNull( category.value() ) || category.value().toString().isEmpty() )
            {
              return tr( "all other values" );
            }
            else
            {
              return QgsVariantUtils::displayString( category.value() );
            }
          }
          break;
        }

        case Qt::FontRole:
        {
          if ( index.column() == valueColumn() && category.value().userType() != QMetaType::Type::QVariantList && ( QgsVariantUtils::isNull( category.value() ) || category.value().toString().isEmpty() ) )
          {
            QFont italicFont;
            italicFont.setItalic( true );
            return italicFont;
          }
          return QVariant();
        }

        case Qt::DecorationRole:
        {
          if ( index.column() == symbolColumn() && category.symbol() )
          {
            return symbolIcon( category );
          }
          break;
        }

        case Qt::ForegroundRole:
        {
          QBrush brush( qApp->palette().color( QPalette::Text ), Qt::SolidPattern );
          if ( index.column() == valueColumn() && ( category.value().userType() == QMetaType::Type::QVariantList || QgsVariantUtils::isNull( category.value() ) || category.value().toString().isEmpty() ) )
          {
            QColor fadedTextColor = brush.color();
            fadedTextColor.setAlpha( 128 );
            brush.setColor( fadedTextColor );
          }
          return brush;
        }

        case Qt::TextAlignmentRole:
        {
          return ( index.column() == valueColumn() ) ? static_cast<Qt::Alignment::Int>( Qt::AlignHCenter ) : static_cast<Qt::Alignment::Int>( Qt::AlignLeft );
        }

        case Qt::EditRole:
        {
          if ( index.column() == valueColumn() )
          {
            if ( category.value().userType() == QMetaType::Type::QVariantList )
            {
              QStringList res;
              const QVariantList list = category.value().toList();
              res.reserve( list.size() );
              for ( const QVariant &variant : list )
                res << variant.toString();

              return res.join( ';' );
            }
            else
            {
              return category.value();
            }
          }
          break;
        }
        case static_cast<int>( Qt::UserRole + 1 ):
        {
          if ( index.column() == valueColumn() )
          {
            return category.value();
          }
          break;
        }
        default:
          break;
      }

      // allow subclasses to return data
      return extraData( index, role );
    }

    virtual bool setData( const QModelIndex &index, const QVariant &value, int role ) override
    {
      if ( !index.isValid() )
        return false;

      if ( index.column() == symbolColumn() && role == Qt::CheckStateRole )
      {
        mRenderer->updateCategoryRenderState( index.row(), value == Qt::Checked );
        emit dataChanged( index, index );
        return true;
      }

      if ( role != Qt::EditRole )
      {
        return false;
      }

      if ( index.column() == valueColumn() )
      {
        // try to preserve variant type for this value, unless it was an empty string (other values)
        QVariant val = value;
        const QVariant previousValue = mRenderer->categories().value( index.row() ).value();
        if ( previousValue.userType() != QMetaType::Type::QString && !previousValue.toString().isEmpty() )
        {
          switch ( previousValue.userType() )
          {
            case QMetaType::Type::Int:
              val = value.toInt();
              break;
            case QMetaType::Type::Double:
              val = value.toDouble();
              break;
            case QMetaType::Type::QVariantList:
            {
              const QStringList parts = value.toString().split( ';' );
              QVariantList list;
              list.reserve( parts.count() );
              for ( const QString &part : parts )
                list << part;

              if ( list.count() == 1 )
                val = list.at( 0 );
              else
                val = list;
              break;
            }
            default:
              val = value.toString();
              break;
          }
        }
        mRenderer->updateCategoryValue( index.row(), val );
        emit dataChanged( index, index );
        return true;
      }

      return setExtraData( index, value );
    }

    int rowCount( const QModelIndex &parent = QModelIndex() ) const override
    {
      if ( parent.isValid() || !mRenderer )
      {
        return 0;
      }
      return static_cast<int>( mRenderer->categories().size() );
    }

    int columnCount( const QModelIndex & = QModelIndex() ) const override = 0;

    Qt::DropActions supportedDropActions() const override { return Qt::MoveAction; }

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
    void addCategory( const Category &cat )
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
    Category category( const QModelIndex &index )
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
    void updateSymbology() { emit dataChanged( createIndex( 0, 0 ), createIndex( static_cast<int>( mRenderer->categories().size() ), 0 ) ); }

  protected:
    /**
     * Called when rows have been moved in the model.
     */
    virtual void onRowsMoved() = 0;

    /**
     * Called by flags() to handle additional columns
     */
    virtual Qt::ItemFlags extraFlags( const QModelIndex &index ) const
    {
      Q_UNUSED( index )
      return Qt::NoItemFlags;
    }

    virtual QIcon symbolIcon( const RendererType::Category &category ) const = 0;

    /**
     * Called by setData() to handle additional editable columns
     *
     */
    virtual bool setExtraData( const QModelIndex &index, const QVariant &value )
    {
      Q_UNUSED( index )
      Q_UNUSED( value )
      return false;
    }

    /**
     * Called by data() to handle additional columns
     *
     */
    virtual QVariant extraData( const QModelIndex &index, int role ) const
    {
      Q_UNUSED( index )
      Q_UNUSED( role )
      return QVariant();
    }

  protected:
    RendererType *mRenderer = nullptr;
    QString mMimeFormat;
    QPointer<QScreen> mScreen;
};


/// @endcond

#endif // QGSTEMPLATEDCATEGORIZEDRENDERERMODEL_H
