/***************************************************************************

               ----------------------------------------------------
              date                 : 22.5.2019
              copyright            : (C) 2019 by Matthias Kuhn
              email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsprojectlistitemdelegate.h"
#include "qgis.h"
#include "qgsnewsfeedmodel.h"
#include "qgswebframe.h"
#include "qgsapplication.h"
#include "qgsrendercontext.h"

#include <QApplication>
#include <QPainter>
#include <QTextDocument>
#include <QAbstractTextDocumentLayout>

QgsProjectListItemDelegate::QgsProjectListItemDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
  , mRoundedRectSizePixels( static_cast<int>( Qgis::UI_SCALE_FACTOR * QApplication::fontMetrics().height() * 0.5 ) )
{

}

void QgsProjectListItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  const QgsScopedQPainterState painterState( painter );

  QTextDocument doc;
  const QPixmap icon = qvariant_cast<QPixmap>( index.data( Qt::DecorationRole ) );

  QAbstractTextDocumentLayout::PaintContext ctx;
  const QStyleOptionViewItem optionV4 = option;

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

  const int titleSize = static_cast<int>( QApplication::fontMetrics().height() * 1.1 );
  const int textSize = static_cast<int>( titleSize * 0.85 );

  doc.setHtml( QStringLiteral( "<div style='font-size:%1px'><span style='font-size:%2px;font-weight:bold;'>%3%4</span><br>%5<br>%6</div>" ).arg( textSize ).arg( QString::number( titleSize ),
               index.data( QgsProjectListItemDelegate::TitleRole ).toString(),
               index.data( QgsProjectListItemDelegate::PinRole ).toBool() ? QStringLiteral( "<img src=\":/images/themes/default/pin.svg\">" ) : QString(),
               mShowPath ? index.data( QgsProjectListItemDelegate::NativePathRole ).toString() : QString(),
               index.data( QgsProjectListItemDelegate::CrsRole ).toString() ) );
  doc.setTextWidth( option.rect.width() - ( !icon.isNull() ? icon.width() + 4.375 * mRoundedRectSizePixels : 4.375 * mRoundedRectSizePixels ) );

  if ( !icon.isNull() )
  {
    painter->drawPixmap( option.rect.left() + 1.25 * mRoundedRectSizePixels, option.rect.top() + 1.25 * mRoundedRectSizePixels, icon );
  }

  painter->translate( option.rect.left() + ( !icon.isNull() ? icon.width() + 3.125 * mRoundedRectSizePixels : 1.875 * mRoundedRectSizePixels ), option.rect.top() + 1.875 * mRoundedRectSizePixels );
  ctx.clip = QRect( 0, 0, option.rect.width() - ( !icon.isNull() ? icon.width() - 4.375 * mRoundedRectSizePixels : 3.125 *  mRoundedRectSizePixels ), option.rect.height() - 3.125 * mRoundedRectSizePixels );
  doc.documentLayout()->draw( painter, ctx );
}

QSize QgsProjectListItemDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QTextDocument doc;
  const QPixmap icon = qvariant_cast<QPixmap>( index.data( Qt::DecorationRole ) );

  int width;
  if ( option.rect.width() < 450 )
  {
    width = 450;
  }
  else
  {
    width = option.rect.width();
  }

  const int titleSize = QApplication::fontMetrics().height() * 1.1;
  const int textSize = titleSize * 0.85;

  doc.setHtml( QStringLiteral( "<div style='font-size:%1px;'><span style='font-size:%2px;font-weight:bold;'>%3%4</span><br>%5<br>%6</div>" ).arg( textSize ).arg( titleSize )
               .arg( index.data( QgsProjectListItemDelegate::TitleRole ).toString(),
                     index.data( QgsProjectListItemDelegate::PinRole ).toBool() ? QStringLiteral( "<img src=\"qrc:/images/themes/default/pin.svg\">" ) : QString(),
                     index.data( QgsProjectListItemDelegate::NativePathRole ).toString(),
                     index.data( QgsProjectListItemDelegate::CrsRole ).toString() ) );
  doc.setTextWidth( width - ( !icon.isNull() ? icon.width() + 4.375 * mRoundedRectSizePixels : 4.375 * mRoundedRectSizePixels ) );

  return QSize( width, std::max( ( double ) doc.size().height() + 1.25 * mRoundedRectSizePixels, static_cast<double>( icon.height() ) ) + 2.5 * mRoundedRectSizePixels );
}

bool QgsProjectListItemDelegate::showPath() const
{
  return mShowPath;
}

void QgsProjectListItemDelegate::setShowPath( bool value )
{
  mShowPath = value;
}

QgsProjectPreviewImage::QgsProjectPreviewImage() = default;

QgsProjectPreviewImage::QgsProjectPreviewImage( const QString &path )
{
  loadImageFromFile( path );
}

QgsProjectPreviewImage::QgsProjectPreviewImage( const QImage &image )
{
  mImage = image;
}

void QgsProjectPreviewImage::loadImageFromFile( const QString &path )
{
  mImage = QImage( path );
}

void QgsProjectPreviewImage::setImage( const QImage &image )
{
  mImage = image;
}

QPixmap QgsProjectPreviewImage::pixmap() const
{
  //nicely round corners so users don't get paper cuts
  QImage previewImage( mImage.size(), QImage::Format_ARGB32 );
  previewImage.fill( Qt::transparent );
  QPainter previewPainter( &previewImage );
  previewPainter.setRenderHint( QPainter::Antialiasing, true );
  previewPainter.setPen( Qt::NoPen );
  previewPainter.setBrush( Qt::black );
  previewPainter.drawRoundedRect( 0, 0, previewImage.width(), previewImage.height(), 8, 8 );
  previewPainter.setCompositionMode( QPainter::CompositionMode_SourceIn );
  previewPainter.drawImage( 0, 0, mImage );
  previewPainter.end();
  return QPixmap::fromImage( previewImage );
}

bool QgsProjectPreviewImage::isNull() const
{
  return mImage.isNull();
}

//
// QgsNewsItemListItemDelegate
//

QgsNewsItemListItemDelegate::QgsNewsItemListItemDelegate( QObject *parent )
  : QStyledItemDelegate( parent )
  , mRoundedRectSizePixels( static_cast<int>( Qgis::UI_SCALE_FACTOR * QApplication::fontMetrics().height() * 0.5 ) )
  , mDismissRectSize( 20, 20 ) // TODO - hidpi friendly
{

}

void QgsNewsItemListItemDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  const QgsScopedQPainterState painterState( painter );

  QTextDocument doc;
  const QPixmap icon = qvariant_cast<QPixmap>( index.data( Qt::DecorationRole ) );

  QAbstractTextDocumentLayout::PaintContext ctx;
  const QStyleOptionViewItem optionV4 = option;

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

  const int titleSize = static_cast<int>( QApplication::fontMetrics().height() * 1.1 );
  const int textSize = static_cast<int>( titleSize * 0.85 );

  doc.setHtml( QStringLiteral( "<div style='font-size:%1px'><span style='font-size:%2px;font-weight:bold;'>%3%4</span>%5</div>" ).arg( textSize ).arg( QString::number( titleSize ),
               index.data( QgsNewsFeedModel::Title ).toString(),
               index.data( QgsNewsFeedModel::Sticky ).toBool() ? QStringLiteral( "<img src=\"qrc:/images/themes/default/pin.svg\">" ) : QString(),
               index.data( QgsNewsFeedModel::Content ).toString() ) );


  doc.setTextWidth( option.rect.width() - ( !icon.isNull() ? icon.width() + 4.375 * mRoundedRectSizePixels : 4.375 * mRoundedRectSizePixels ) );

  if ( !icon.isNull() )
  {
    painter->drawPixmap( option.rect.left() + 1.25 * mRoundedRectSizePixels, option.rect.top() + 1.25 * mRoundedRectSizePixels, icon );
  }

  // Gross, but not well supported in Qt
  mDismissRect = QRect( option.rect.width() - 32, option.rect.top() + 10, mDismissRectSize.width(), mDismissRectSize.height() );
  const QPixmap pixmap = QgsApplication::getThemeIcon( QStringLiteral( "/mIconClearItem.svg" ) ).pixmap( mDismissRectSize, QIcon::Normal );
  painter->drawPixmap( mDismissRect.topLeft(), pixmap );
  mDismissRect.setTop( 10 );

  painter->translate( option.rect.left() + ( !icon.isNull() ? icon.width() + 3.125 * mRoundedRectSizePixels : 1.875 * mRoundedRectSizePixels ), option.rect.top() + 1.875 * mRoundedRectSizePixels );
  ctx.clip = QRect( 0, 0, option.rect.width() - ( !icon.isNull() ? icon.width() - 4.375 * mRoundedRectSizePixels : 3.125 *  mRoundedRectSizePixels ), option.rect.height() - 3.125 * mRoundedRectSizePixels );
  doc.documentLayout()->draw( painter, ctx );
}

QSize QgsNewsItemListItemDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QTextDocument doc;
  const QPixmap icon = qvariant_cast<QPixmap>( index.data( Qt::DecorationRole ) );

  int width;
  if ( option.rect.width() < 450 )
  {
    width = 450;
  }
  else
  {
    width = option.rect.width();
  }

  const int titleSize = QApplication::fontMetrics().height() * 1.1;
  const int textSize = titleSize * 0.85;
  doc.setHtml( QStringLiteral( "<div style='font-size:%1px'><span style='font-size:%2px;font-weight:bold;'>%3%4</span>%5</div>" ).arg( textSize ).arg( QString::number( titleSize ),
               index.data( QgsNewsFeedModel::Title ).toString(),
               index.data( QgsNewsFeedModel::Sticky ).toBool() ? QStringLiteral( "<img src=\"qrc:/images/themes/default/pin.svg\">" ) : QString(),
               index.data( QgsNewsFeedModel::Content ).toString() ) );
  doc.setTextWidth( width - ( !icon.isNull() ? icon.width() + 4.375 * mRoundedRectSizePixels : 4.375 * mRoundedRectSizePixels ) );

  return QSize( width, std::max( ( double ) doc.size().height() + 1.25 * mRoundedRectSizePixels, static_cast<double>( icon.height() ) ) + 2.5 * mRoundedRectSizePixels );
}


