/***************************************************************************

               ----------------------------------------------------
              date                 : 17.8.2015
              copyright            : (C) 2015 by Matthias Kuhn
              email                : matthias (at) opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswelcomepageitemsmodel.h"
#include "qgsmessagelog.h"

#include <QApplication>
#include <QAbstractTextDocumentLayout>
#include <QPixmap>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QTextDocument>

QgsWelcomePageItemDelegate::QgsWelcomePageItemDelegate( QObject * parent )
    : QStyledItemDelegate( parent )
{

}

void QgsWelcomePageItemDelegate::paint( QPainter* painter, const QStyleOptionViewItem & option, const QModelIndex &index ) const
{
  painter->save();

  QTextDocument doc;
  QAbstractTextDocumentLayout::PaintContext ctx;

  QColor color;
  if ( option.state & QStyle::State_Selected )
  {
    color = QColor( 255, 255, 255, 60 );
    QStyle *style = QApplication::style();
    style->drawPrimitive( QStyle::PE_PanelItemViewItem, &option, painter, NULL );
  }
  else if ( option.state & QStyle::State_Enabled )
  {
    color = QColor( 100, 100, 100, 30 );
  }
  else
  {
    color = QColor( 100, 100, 100, 30 );
    ctx.palette.setColor( QPalette::Text, QColor( 150, 150, 150, 255 ) );
  }

  painter->setRenderHint( QPainter::Antialiasing );
  painter->setPen( QColor( 0, 0, 0, 0 ) );
  painter->setBrush( QBrush( color ) );
  painter->drawRoundedRect( option.rect.left() + 5, option.rect.top() + 5, option.rect.width() - 10, option.rect.height() - 10, 8, 8 );

  doc.setHtml( QString( "<span style='font-size:18px;font-weight:bold;'>%1</span><br>%2" ).arg( index.data( QgsWelcomePageItemsModel::TitleRole ).toString() ).arg( index.data( QgsWelcomePageItemsModel::PathRole ).toString() ) );
  doc.setTextWidth( 800 );

  QPixmap icon = qvariant_cast<QPixmap>( index.data( Qt::DecorationRole ) );
  if ( !icon.isNull() )
  {
    painter->drawPixmap( option.rect.left() + 10, option.rect.top()  + 10, icon );
  }

  painter->translate( option.rect.left() + ( !icon.isNull() ? icon.width() + 25 : 15 ), option.rect.top() + 15 );
  ctx.clip = QRect( 0, 0, option.rect.width() - ( !icon.isNull() ? icon.width() - 35 : 25 ), option.rect.height() - 25 );
  doc.documentLayout()->draw( painter, ctx );

  painter->restore();
}

QSize QgsWelcomePageItemDelegate::sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
  QTextDocument doc;

  doc.setHtml( QString( "<span style='font-size:18px;font-weight:bold;'>%1</span><br>%2" ).arg( index.data( QgsWelcomePageItemsModel::TitleRole ).toString() ).arg( index.data( QgsWelcomePageItemsModel::PathRole ).toString() ) );
  doc.setTextWidth( 800 );

  QPixmap icon = qvariant_cast<QPixmap>( index.data( Qt::DecorationRole ) );

  return QSize( option.rect.width(), qMax( doc.size().height() + 10, ( double )icon.height() ) + 20 );
}

QgsWelcomePageItemsModel::QgsWelcomePageItemsModel( QObject* parent )
    : QAbstractListModel( parent )
{

}

void QgsWelcomePageItemsModel::setRecentProjects( const QList<RecentProjectData>& recentProjects )
{
  beginResetModel();
  mRecentProjects = recentProjects;
  endResetModel();
}


int QgsWelcomePageItemsModel::rowCount( const QModelIndex& parent ) const
{
  Q_UNUSED( parent )
  return mRecentProjects.size();
}

QVariant QgsWelcomePageItemsModel::data( const QModelIndex& index, int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
    case TitleRole:
      return mRecentProjects.at( index.row() ).title != mRecentProjects.at( index.row() ).path ? mRecentProjects.at( index.row() ).title : QFileInfo( mRecentProjects.at( index.row() ).path ).baseName();
      break;
    case PathRole:
      return mRecentProjects.at( index.row() ).path;
      break;
    case Qt::DecorationRole:
    {
      QImage thumbnail( mRecentProjects.at( index.row() ).previewImagePath );
      if ( thumbnail.isNull() )
        return QVariant();

      //nicely round corners so users don't get paper cuts
      QImage previewImage( thumbnail.size(), QImage::Format_ARGB32 );
      previewImage.fill( Qt::transparent );
      QPainter previewPainter( &previewImage );
      previewPainter.setRenderHint( QPainter::Antialiasing, true );
      previewPainter.setPen( Qt::NoPen );
      previewPainter.setBrush( Qt::black );
      previewPainter.drawRoundedRect( 0, 0, previewImage.width(), previewImage.height(), 8, 8 );
      previewPainter.setCompositionMode( QPainter::CompositionMode_SourceIn );
      previewPainter.drawImage( 0, 0, thumbnail );
      previewPainter.end();

      return QPixmap::fromImage( previewImage );
    }

    case Qt::ToolTipRole:
      return mRecentProjects.at( index.row() ).path;

    default:
      return QVariant();
  }
}


Qt::ItemFlags QgsWelcomePageItemsModel::flags( const QModelIndex& index ) const
{
  Qt::ItemFlags flags = QAbstractItemModel::flags( index );

  const RecentProjectData& projectData = mRecentProjects.at( index.row() );

  if ( !QFile::exists(( projectData.path ) ) )
    flags &= ~Qt::ItemIsEnabled;

  return flags;
}
