/***************************************************************************
    qgsstacitemlistmodel.cpp
    ---------------------
    begin                : November 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacitemlistmodel.h"
#include "moc_qgsstacitemlistmodel.cpp"
#include "qgsstacitem.h"
#include "qgsstaccollection.h"
#include "qgsnetworkcontentfetcher.h"

#include <QAbstractItemView>
#include <QScrollBar>
#include <QPainter>
#include <QApplication>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

///@cond PRIVATE

QgsStacItemListModel::QgsStacItemListModel( QObject *parent )
  : QAbstractListModel( parent )
{
}

int QgsStacItemListModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return mItems.size();
}

QVariant QgsStacItemListModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() >= mItems.size() )
    return QVariant();

  switch ( role )
  {
    case Qt::DecorationRole:
    {
      const QMap<QString, QgsStacAsset> assets = mItems.at( index.row() )->assets();
      for ( auto it = assets.constBegin(); it != assets.constEnd(); ++it )
      {
        if ( it->roles().contains( QLatin1String( "thumbnail" ) ) )
        {
          return mThumbnails[it->href()];
        }
      }
      return QVariant();
    }
    case Role::Title:
    {
      if ( mItems.at( index.row() )->title().isEmpty() )
        return mItems.at( index.row() )->id();
      else
        return mItems.at( index.row() )->title();
    }
    case Role::StacObject:
      return QVariant::fromValue( static_cast<QgsStacObject *>( mItems.at( index.row() ) ) );
    case Role::Collection:
    {
      const QString col = mItems.at( index.row() )->collection();
      return mCollections.value( col, col );
    }
    case Role::MediaTypes:
    {
      QSet<QString> types;
      const QMap<QString, QgsStacAsset> assets = mItems.at( index.row() )->assets();
      for ( auto it = assets.constBegin(); it != assets.constEnd(); ++it )
      {
        if ( it->isCloudOptimized() )
        {
          types.insert( it->mediaType() );
        }
      }
      return QStringList( types.cbegin(), types.cend() );
    }
    case Role::Uris:
    {
      const QgsMimeDataUtils::UriList uris = mItems.at( index.row() )->uris();
      return QVariant::fromValue( uris );
    }
    case Role::Formats:
    {
      QSet<QString> formats;
      const QMap<QString, QgsStacAsset> assets = mItems.at( index.row() )->assets();
      for ( auto it = assets.constBegin(); it != assets.constEnd(); ++it )
      {
        if ( it->isCloudOptimized() )
        {
          formats.insert( it->formatName() );
        }
      }
      return QStringList( formats.cbegin(), formats.cend() );
    }
    case Role::Geometry:
    {
      return QVariant::fromValue( mItems.at( index.row() )->geometry() );
    }
  }

  return QVariant();
}

void QgsStacItemListModel::clear()
{
  beginResetModel();
  qDeleteAll( mItems );
  mItems.clear();
  mThumbnails.clear();
  mCollections.clear();
  endResetModel();
}

void QgsStacItemListModel::setCollections( const QVector<QgsStacCollection *> &collections )
{
  for ( QgsStacCollection *col : collections )
  {
    if ( !col->title().isEmpty() )
      mCollections[col->id()] = col->title();
  }
}

void QgsStacItemListModel::addItems( const QVector<QgsStacItem *> &items )
{
  int nextItemIndex = mItems.count();
  beginInsertRows( QModelIndex(), mItems.size(), mItems.size() + items.size() - 1 );
  mItems.append( items );
  endInsertRows();

  for ( auto item : items )
  {
    const QMap<QString, QgsStacAsset> assets = item->assets();
    for ( auto it = assets.constBegin(); it != assets.constEnd(); ++it )
    {
      if ( it->roles().contains( QLatin1String( "thumbnail" ) ) )
      {
        const QString href = it->href();
        QgsNetworkContentFetcher *f = new QgsNetworkContentFetcher();
        f->fetchContent( href );
        connect( f, &QgsNetworkContentFetcher::finished, this, [this, f, href, nextItemIndex] {
          if ( f->reply()->error() == QNetworkReply::NoError )
          {
            const QImage img = QImage::fromData( f->reply()->readAll() );
            QImage previewImage( img.size(), QImage::Format_ARGB32 );
            previewImage.fill( Qt::transparent );
            QPainter previewPainter( &previewImage );
            previewPainter.setRenderHint( QPainter::Antialiasing, true );
            previewPainter.setPen( Qt::NoPen );
            previewPainter.setBrush( Qt::black );
            previewPainter.drawRoundedRect( 0, 0, previewImage.width(), previewImage.height(), static_cast<double>( previewImage.width() ) / 10., static_cast<double>( previewImage.width() ) / 10. );
            previewPainter.setCompositionMode( QPainter::CompositionMode_SourceIn );
            previewPainter.drawImage( 0, 0, img );
            previewPainter.end();
            mThumbnails[href] = QPixmap::fromImage( previewImage );
            const QModelIndex idx = index( nextItemIndex );
            emit dataChanged( idx, idx, { Qt::DecorationRole } );
          }
          f->deleteLater();
        } );
      }
      nextItemIndex++;
    }
  }
}

QVector<QgsStacItem *> QgsStacItemListModel::items() const
{
  return mItems;
}


QgsStacItemDelegate::QgsStacItemDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
{
}

QSize QgsStacItemDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index )

  const QFontMetrics fm( option.font );
  int width = option.rect.width();
  if ( const QAbstractItemView *view = qobject_cast<const QAbstractItemView *>( option.widget ) )
  {
    width -= view->verticalScrollBar()->width();
  }
  return QSize( width, fm.lineSpacing() * 8 );
}

void QgsStacItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  painter->save();
  QTextDocument doc;
  const QPixmap icon = qvariant_cast<QPixmap>( index.data( Qt::DecorationRole ) );

  QAbstractTextDocumentLayout::PaintContext ctx;

  QColor color = option.palette.color( QPalette::Active, QPalette::Window );
  if ( option.state & QStyle::State_Selected )
  {
    color.setAlpha( 40 );
    ctx.palette.setColor( QPalette::Text, option.palette.color( QPalette::Active, QPalette::HighlightedText ) );

    QStyle *style = QApplication::style();
    style->drawPrimitive( QStyle::PE_PanelItemViewItem, &option, painter, nullptr );
  }
  else
  {
    ctx.palette.setColor( QPalette::Text, option.palette.color( QPalette::Active, QPalette::Text ) );

    QStyle *style = QApplication::style();
    style->drawPrimitive( QStyle::PE_PanelItemViewItem, &option, painter, nullptr );
  }

  painter->setRenderHint( QPainter::Antialiasing, true );
  painter->setRenderHint( QPainter::SmoothPixmapTransform, true );
  painter->setPen( QColor( 0, 0, 0, 0 ) );
  painter->setBrush( QBrush( color ) );
  painter->drawRoundedRect( option.rect.left() + static_cast<int>( 0.625 * mRoundedRectSizePixels ), option.rect.top() + static_cast<int>( 0.625 * mRoundedRectSizePixels ), option.rect.width() - static_cast<int>( 2 * 0.625 * mRoundedRectSizePixels ), option.rect.height() - static_cast<int>( 2 * 0.625 * mRoundedRectSizePixels ), mRoundedRectSizePixels, mRoundedRectSizePixels );

  const QFontMetrics fm( option.font );
  const int textSize = static_cast<int>( fm.height() * 0.85 );
  const int hintHeight = sizeHint( option, index ).height();
  const int h = static_cast<int>( hintHeight - 2.5 * mRoundedRectSizePixels );
  QSize iconSize( h, h );
  if ( QWidget *w = qobject_cast<QWidget *>( option.styleObject ) )
  {
    iconSize /= w->devicePixelRatioF();
  }

  doc.setHtml( QStringLiteral( "<div style='font-size:%1px'><span style='font-weight:bold;'>%2</span><br>%3<br><br><i>%4</i></div>" )
                 .arg( QString::number( textSize ), index.data( QgsStacItemListModel::Role::Title ).toString(), index.data( QgsStacItemListModel::Role::Collection ).toString(), index.data( QgsStacItemListModel::Role::Formats ).toStringList().join( QLatin1String( ", " ) ) ) );
  doc.setTextWidth( option.rect.width() - ( !icon.isNull() ? iconSize.width() + 4.375 * mRoundedRectSizePixels : 4.375 * mRoundedRectSizePixels ) );

  if ( !icon.isNull() )
  {
    painter->drawPixmap( option.rect.left() + static_cast<int>( 1.25 * mRoundedRectSizePixels ), option.rect.top() + static_cast<int>( 1.25 * mRoundedRectSizePixels ), iconSize.width(), iconSize.height(), icon );
  }

  painter->translate( option.rect.left() + ( !icon.isNull() ? iconSize.width() + 3.125 * mRoundedRectSizePixels : 1.875 * mRoundedRectSizePixels ), option.rect.top() + 1.875 * mRoundedRectSizePixels );
  ctx.clip = QRectF( 0, 0, option.rect.width() - ( !icon.isNull() ? iconSize.width() - 4.375 * mRoundedRectSizePixels : 3.125 * mRoundedRectSizePixels ), option.rect.height() - 3.125 * mRoundedRectSizePixels );
  doc.documentLayout()->draw( painter, ctx );
  painter->restore();
}

///@endcond
