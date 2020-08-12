/***************************************************************************
    qgsfeaturelistviewdelegate.cpp
    ---------------------
    begin                : February 2013
    copyright            : (C) 2013 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsfeaturelistviewdelegate.h"
#include "qgsvectorlayer.h"
#include "qgsattributetablemodel.h"
#include "qgsfeaturelistmodel.h"
#include "qgsapplication.h"
#include "qgsvectorlayereditbuffer.h"
#include "qgsfeatureselectionmodel.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QApplication>
#include <QMouseEvent>
#include <QObject>

QgsFeatureListViewDelegate::QgsFeatureListViewDelegate( QgsFeatureListModel *listModel, QObject *parent )
  : QItemDelegate( parent )
  , mListModel( listModel )
  , mCurrentFeatureEdited( false )
{
}

QgsFeatureListViewDelegate::Element QgsFeatureListViewDelegate::positionToElement( QPoint pos )
{
  if ( pos.x() > ICON_SIZE )
  {
    return EditElement;
  }
  else
  {
    return SelectionElement;
  }
}

void QgsFeatureListViewDelegate::setFeatureSelectionModel( QgsFeatureSelectionModel *featureSelectionModel )
{
  mFeatureSelectionModel = featureSelectionModel;
}

void QgsFeatureListViewDelegate::setCurrentFeatureEdited( bool state )
{
  mCurrentFeatureEdited = state;
}

void QgsFeatureListViewDelegate::setEditSelectionModel( QItemSelectionModel *editSelectionModel )
{
  mEditSelectionModel = editSelectionModel;
}

QSize QgsFeatureListViewDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( index )
  int height = ICON_SIZE;
  return QSize( option.rect.width(), std::max( height, option.fontMetrics.height() ) );
}

void QgsFeatureListViewDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  const bool isEditSelection = mEditSelectionModel && mEditSelectionModel->isSelected( mListModel->mapToMaster( index ) );

  QStyleOptionViewItem textOption = option;
  textOption.state |= QStyle::State_Enabled;
  if ( isEditSelection )
  {
    textOption.state |= QStyle::State_Selected;
  }
  drawBackground( painter, textOption, index );


  static QPixmap sSelectedIcon;
  if ( sSelectedIcon.isNull() )
    sSelectedIcon = QgsApplication::getThemePixmap( QStringLiteral( "/mIconSelected.svg" ) );
  static QPixmap sDeselectedIcon;
  if ( sDeselectedIcon.isNull() )
    sDeselectedIcon = QgsApplication::getThemePixmap( QStringLiteral( "/mIconDeselected.svg" ) );

  QString text = index.model()->data( index, Qt::EditRole ).toString();
  QgsFeatureListModel::FeatureInfo featInfo = index.model()->data( index, Qt::UserRole ).value<QgsFeatureListModel::FeatureInfo>();

  // Icon layout options
  QStyleOptionViewItem iconOption;

  QRect iconLayoutBounds( option.rect.x(), option.rect.y(), option.rect.height(), option.rect.height() );

  QPixmap icon = mFeatureSelectionModel->isSelected( index ) ? sSelectedIcon : sDeselectedIcon;

  // Scale up the icon if needed
  if ( option.rect.height() > ICON_SIZE )
  {
    icon = icon.scaledToHeight( option.rect.height(), Qt::SmoothTransformation );
  }

  drawDecoration( painter, iconOption, iconLayoutBounds, icon );

  // if conditional style also has an icon, draw that too
  const QVariant conditionalIcon = index.model()->data( index, Qt::DecorationRole );
  if ( conditionalIcon.isValid() )
  {
    const QPixmap pixmap = conditionalIcon.value< QPixmap >();
#if QT_VERSION < QT_VERSION_CHECK(5, 11, 0)
    iconLayoutBounds.moveLeft( iconLayoutBounds.x() + icon.width() + QFontMetrics( textOption.font ).width( 'X' ) );
#else
    iconLayoutBounds.moveLeft( iconLayoutBounds.x() + icon.width() + QFontMetrics( textOption.font ).horizontalAdvance( 'X' ) );
#endif
    iconLayoutBounds.setTop( option.rect.y() + ( option.rect.height() - pixmap.height() ) / 2.0 );
    iconLayoutBounds.setHeight( pixmap.height() );
    drawDecoration( painter, iconOption, iconLayoutBounds, pixmap );
  }

  // Text layout options
  QRect textLayoutBounds( iconLayoutBounds.x() + iconLayoutBounds.width(), option.rect.y(), option.rect.width() - ( iconLayoutBounds.x() + iconLayoutBounds.width() ), option.rect.height() );

  // start with font and foreground color from model's FontRole
  const QVariant font = index.model()->data( index, Qt::FontRole );
  if ( font.isValid() )
  {
    textOption.font = font.value< QFont >();
  }
  const QVariant textColor = index.model()->data( index, Qt::TextColorRole );
  if ( textColor.isValid() )
  {
    textOption.palette.setColor( QPalette::Text, textColor.value< QColor >() );
  }

  if ( featInfo.isNew )
  {
    textOption.font.setStyle( QFont::StyleItalic );
    textOption.palette.setColor( QPalette::Text, Qt::darkGreen );
    textOption.palette.setColor( QPalette::HighlightedText, Qt::darkGreen );
  }
  else if ( featInfo.isEdited || ( mCurrentFeatureEdited && isEditSelection ) )
  {
    textOption.font.setStyle( QFont::StyleItalic );
    textOption.palette.setColor( QPalette::Text, Qt::red );
    textOption.palette.setColor( QPalette::HighlightedText, Qt::red );
  }

  drawDisplay( painter, textOption, textLayoutBounds, text );
}
