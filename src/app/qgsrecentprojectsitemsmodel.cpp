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

#include "qgsrecentprojectsitemsmodel.h"

#include "qgsapplication.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsmessagelog.h"
#include "qgsprojectstorageregistry.h"

#include <QApplication>
#include <QAbstractTextDocumentLayout>
#include <QPixmap>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QTextDocument>
#include <QDir>

QgsRecentProjectItemDelegate::QgsRecentProjectItemDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
  , mRoundedRectSizePixels( Qgis::UI_SCALE_FACTOR * QApplication::fontMetrics().height() * 0.5 )
{

}

void QgsRecentProjectItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  painter->save();

  QTextDocument doc;
  QPixmap icon = qvariant_cast<QPixmap>( index.data( Qt::DecorationRole ) );

  QAbstractTextDocumentLayout::PaintContext ctx;
  QStyleOptionViewItem optionV4 = option;

  QColor color = optionV4.palette.color( QPalette::Active, QPalette::Window );
  if ( option.state & QStyle::State_Selected && option.state & QStyle::State_HasFocus )
  {
    color.setAlpha( 40 );
    ctx.palette.setColor( QPalette::Text, optionV4.palette.color( QPalette::Active, QPalette::HighlightedText ) );

    QStyle *style = QApplication::style();
    style->drawPrimitive( QStyle::PE_PanelItemViewItem, &option, painter, nullptr );
  }
  else if ( option.state & QStyle::State_Enabled )
  {
    if ( option.state & QStyle::State_Selected )
    {
      color.setAlpha( 40 );
    }
    ctx.palette.setColor( QPalette::Text, optionV4.palette.color( QPalette::Active, QPalette::Text ) );

    QStyle *style = QApplication::style();
    style->drawPrimitive( QStyle::PE_PanelItemViewItem, &option, painter, nullptr );
  }
  else
  {
    ctx.palette.setColor( QPalette::Text, optionV4.palette.color( QPalette::Disabled, QPalette::Text ) );
  }

  painter->setRenderHint( QPainter::Antialiasing );
  painter->setPen( QColor( 0, 0, 0, 0 ) );
  painter->setBrush( QBrush( color ) );
  painter->drawRoundedRect( option.rect.left() + 0.625 * mRoundedRectSizePixels, option.rect.top() + 0.625 * mRoundedRectSizePixels,
                            option.rect.width() - 2 * 0.625 * mRoundedRectSizePixels, option.rect.height() - 2 * 0.625 * mRoundedRectSizePixels, mRoundedRectSizePixels, mRoundedRectSizePixels );

  int titleSize = QApplication::fontMetrics().height() * 1.1;
  int textSize = titleSize * 0.85;

  doc.setHtml( QStringLiteral( "<div style='font-size:%1px;'><span style='font-size:%2px;font-weight:bold;'>%3%4</span><br>%5<br>%6</div>" ).arg( textSize ).arg( titleSize )
               .arg( index.data( QgsRecentProjectItemsModel::TitleRole ).toString(),
                     index.data( QgsRecentProjectItemsModel::PinRole ).toBool() ? QStringLiteral( "<img src=\"qrc:/images/themes/default/pin.svg\">" ) : QString(),
                     index.data( QgsRecentProjectItemsModel::NativePathRole ).toString(),
                     index.data( QgsRecentProjectItemsModel::CrsRole ).toString() ) );
  doc.setTextWidth( option.rect.width() - ( !icon.isNull() ? icon.width() + 4.375 * mRoundedRectSizePixels : 4.375 * mRoundedRectSizePixels ) );

  if ( !icon.isNull() )
  {
    painter->drawPixmap( option.rect.left() + 1.25 * mRoundedRectSizePixels, option.rect.top() + 1.25 * mRoundedRectSizePixels, icon );
  }

  painter->translate( option.rect.left() + ( !icon.isNull() ? icon.width() + 3.125 * mRoundedRectSizePixels : 1.875 * mRoundedRectSizePixels ), option.rect.top() + 1.875 * mRoundedRectSizePixels );
  ctx.clip = QRect( 0, 0, option.rect.width() - ( !icon.isNull() ? icon.width() - 4.375 * mRoundedRectSizePixels : 3.125 *  mRoundedRectSizePixels ), option.rect.height() - 3.125 * mRoundedRectSizePixels );
  doc.documentLayout()->draw( painter, ctx );

  painter->restore();
}

QSize QgsRecentProjectItemDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
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

  doc.setHtml( QStringLiteral( "<div style='font-size:%1px;'><span style='font-size:%2px;font-weight:bold;'>%3%4</span><br>%5<br>%6</div>" ).arg( textSize ).arg( titleSize )
               .arg( index.data( QgsRecentProjectItemsModel::TitleRole ).toString(),
                     index.data( QgsRecentProjectItemsModel::PinRole ).toBool() ? QStringLiteral( "<img src=\"qrc:/images/themes/default/pin.svg\">" ) : QString(),
                     index.data( QgsRecentProjectItemsModel::NativePathRole ).toString(),
                     index.data( QgsRecentProjectItemsModel::CrsRole ).toString() ) );
  doc.setTextWidth( width - ( !icon.isNull() ? icon.width() + 4.375 * mRoundedRectSizePixels : 4.375 * mRoundedRectSizePixels ) );

  return QSize( width, std::max( ( double ) doc.size().height() + 1.25 * mRoundedRectSizePixels, static_cast<double>( icon.height() ) ) + 2.5 * mRoundedRectSizePixels );
}

QgsRecentProjectItemsModel::QgsRecentProjectItemsModel( QObject *parent )
  : QAbstractListModel( parent )
{

}

void QgsRecentProjectItemsModel::setRecentProjects( const QList<RecentProjectData> &recentProjects )
{
  beginResetModel();
  mRecentProjects = recentProjects;
  endResetModel();
}


int QgsRecentProjectItemsModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mRecentProjects.size();
}

QVariant QgsRecentProjectItemsModel::data( const QModelIndex &index, int role ) const
{
  switch ( role )
  {
    case Qt::DisplayRole:
    case TitleRole:
      return mRecentProjects.at( index.row() ).title != mRecentProjects.at( index.row() ).path ? mRecentProjects.at( index.row() ).title : QFileInfo( mRecentProjects.at( index.row() ).path ).completeBaseName();
    case PathRole:
      return mRecentProjects.at( index.row() ).path;
    case NativePathRole:
      return QDir::toNativeSeparators( mRecentProjects.at( index.row() ).path );
    case CrsRole:
      if ( !mRecentProjects.at( index.row() ).crs.isEmpty() )
      {
        QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( mRecentProjects.at( index.row() ).crs );
        return  QStringLiteral( "%1 (%2)" ).arg( mRecentProjects.at( index.row() ).crs, crs.description() );
      }
      else
      {
        return QString();
      }
    case PinRole:
      return mRecentProjects.at( index.row() ).pin;
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


Qt::ItemFlags QgsRecentProjectItemsModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() || !rowCount( index.parent() ) )
    return Qt::NoItemFlags;

  Qt::ItemFlags flags = QAbstractListModel::flags( index );

  const RecentProjectData &projectData = mRecentProjects.at( index.row() );

  // This check can be slow for network based projects, so only run it the first time
  if ( !projectData.checkedExists )
  {
    if ( QgsApplication::projectStorageRegistry()->projectStorageFromUri( projectData.path ) )
      // we could check whether a project exists in a custom project storage by checking its metadata,
      // but that may be slow (e.g. doing some network queries) so for now we always assume such projects exist
      projectData.exists = true;
    else
      projectData.exists = QFile::exists( ( projectData.path ) );
    projectData.checkedExists = true;
  }

  if ( !projectData.exists )
    flags &= ~Qt::ItemIsEnabled;

  return flags;
}

void QgsRecentProjectItemsModel::pinProject( const QModelIndex &index )
{
  mRecentProjects.at( index.row() ).pin = true;
}

void QgsRecentProjectItemsModel::unpinProject( const QModelIndex &index )
{
  mRecentProjects.at( index.row() ).pin = false;
}

void QgsRecentProjectItemsModel::removeProject( const QModelIndex &index )
{
  beginRemoveRows( index, index.row(), index.row() );
  mRecentProjects.removeAt( index.row() );
  endRemoveRows();
}

void QgsRecentProjectItemsModel::recheckProject( const QModelIndex &index )
{
  const RecentProjectData &projectData = mRecentProjects.at( index.row() );
  projectData.exists = QFile::exists( ( projectData.path ) );
  projectData.checkedExists = true;
}
