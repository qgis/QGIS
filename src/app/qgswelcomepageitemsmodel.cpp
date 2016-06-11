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
#include "qgscoordinatereferencesystem.h"
#include "qgsmessagelog.h"
#include "qgscrscache.h"

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
  QPixmap icon = qvariant_cast<QPixmap>( index.data( Qt::DecorationRole ) );

  QAbstractTextDocumentLayout::PaintContext ctx;
  QStyleOptionViewItemV4 optionV4 = option;

  QColor color;
  if ( option.state & QStyle::State_Selected && option.state & QStyle::State_HasFocus )
  {
    color = QColor( 255, 255, 255, 60 );
    ctx.palette.setColor( QPalette::Text, optionV4.palette.color( QPalette::Active, QPalette::HighlightedText ) );

    QStyle *style = QApplication::style();
    style->drawPrimitive( QStyle::PE_PanelItemViewItem, &option, painter, nullptr );
  }
  else if ( option.state & QStyle::State_Enabled )
  {
    color = QColor( 100, 100, 100, 30 );
    ctx.palette.setColor( QPalette::Text, optionV4.palette.color( QPalette::Active, QPalette::Text ) );

    QStyle *style = QApplication::style();
    style->drawPrimitive( QStyle::PE_PanelItemViewItem, &option, painter, nullptr );
  }
  else
  {
    color = QColor( 100, 100, 100, 30 );
    ctx.palette.setColor( QPalette::Text, optionV4.palette.color( QPalette::Disabled, QPalette::Text ) );
  }

  painter->setRenderHint( QPainter::Antialiasing );
  painter->setPen( QColor( 0, 0, 0, 0 ) );
  painter->setBrush( QBrush( color ) );
  painter->drawRoundedRect( option.rect.left() + 5, option.rect.top() + 5, option.rect.width() - 10, option.rect.height() - 10, 8, 8 );

  int titleSize = QApplication::fontMetrics().height() * 1.1;
  int textSize = titleSize * 0.85;

  doc.setHtml( QString( "<div style='font-size:%1px;'><span style='font-size:%2px;font-weight:bold;'>%3</span><br>%4<br>%5</div>" ).arg( textSize ).arg( titleSize )
               .arg( index.data( QgsWelcomePageItemsModel::TitleRole ).toString(),
                     index.data( QgsWelcomePageItemsModel::PathRole ).toString(),
                     index.data( QgsWelcomePageItemsModel::CrsRole ).toString() ) );
  doc.setTextWidth( option.rect.width() - ( !icon.isNull() ? icon.width() + 35 : 35 ) );

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
  QPixmap icon = qvariant_cast<QPixmap>( index.data( Qt::DecorationRole ) );

  int width;
  if ( option.rect.width() < 450 )
  {
    width = 450;
  }
  else
  {
    width = option.rect.width();
  }

  int titleSize = QApplication::fontMetrics().height() * 1.1;
  int textSize = titleSize * 0.85;

  doc.setHtml( QString( "<div style='font-size:%1px;'><span style='font-size:%2px;font-weight:bold;'>%3</span><br>%4<br>%5</div>" ).arg( textSize ).arg( titleSize )
               .arg( index.data( QgsWelcomePageItemsModel::TitleRole ).toString(),
                     index.data( QgsWelcomePageItemsModel::PathRole ).toString(),
                     index.data( QgsWelcomePageItemsModel::CrsRole ).toString() ) );
  doc.setTextWidth( width - ( !icon.isNull() ? icon.width() + 35 : 35 ) );

  return QSize( width, qMax(( double ) doc.size().height() + 10, ( double )icon.height() ) + 20 );
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
    case PathRole:
      return mRecentProjects.at( index.row() ).path;
    case CrsRole:
      if ( mRecentProjects.at( index.row() ).crs != "" )
      {
        QgsCoordinateReferenceSystem crs = QgsCRSCache::instance()->crsByOgcWmsCrs( mRecentProjects.at( index.row() ).crs );
        return  QString( "%1 (%2)" ).arg( mRecentProjects.at( index.row() ).crs, crs.description() );
      }
      else
      {
        return QString();
      }
    case Qt::DecorationRole:
    {
      QString filename( mRecentProjects.at( index.row() ).previewImagePath );
      if ( filename.isEmpty() )
        return QVariant();

      QImage thumbnail( filename );
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
