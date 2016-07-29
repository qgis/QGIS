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
    , mFeatureSelectionModel( nullptr )
    , mEditSelectionModel( nullptr )
    , mListModel( listModel )
    , mCurrentFeatureEdited( false )
{
}

QgsFeatureListViewDelegate::Element QgsFeatureListViewDelegate::positionToElement( QPoint pos )
{
  if ( pos.x() > sIconSize )
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

void QgsFeatureListViewDelegate::setEditSelectionModel( QItemSelectionModel* editSelectionModel )
{
  mEditSelectionModel = editSelectionModel;
}

QSize QgsFeatureListViewDelegate::sizeHint( const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
  Q_UNUSED( index )
  int height = sIconSize;
  return QSize( option.rect.width(), qMax( height, option.fontMetrics.height() ) );
}

void QgsFeatureListViewDelegate::paint( QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  static QPixmap selectedIcon;
  if ( selectedIcon.isNull() )
    selectedIcon = QgsApplication::getThemePixmap( "/mIconSelected.svg" );
  static QPixmap deselectedIcon;
  if ( deselectedIcon.isNull() )
    deselectedIcon = QgsApplication::getThemePixmap( "/mIconDeselected.svg" );

  QString text = index.model()->data( index, Qt::EditRole ).toString();
  QgsFeatureListModel::FeatureInfo featInfo = index.model()->data( index, Qt::UserRole ).value<QgsFeatureListModel::FeatureInfo>();

  bool isEditSelection = mEditSelectionModel && mEditSelectionModel->isSelected( mListModel->mapToMaster( index ) );

  // Icon layout options
  QStyleOptionViewItem iconOption;

  QRect iconLayoutBounds( option.rect.x(), option.rect.y(), option.rect.height(), option.rect.height() );

  QPixmap icon = mFeatureSelectionModel->isSelected( index ) ? selectedIcon : deselectedIcon;

  // Scale up the icon if needed
  if ( option.rect.height() > sIconSize )
  {
    icon = icon.scaledToHeight( option.rect.height(), Qt::SmoothTransformation );
  }

  // Text layout options
  QRect textLayoutBounds( iconLayoutBounds.x() + iconLayoutBounds.width(), option.rect.y(), option.rect.width() - ( iconLayoutBounds.x() + iconLayoutBounds.width() ), option.rect.height() );

  QStyleOptionViewItem textOption;
  textOption.state |= QStyle::State_Enabled;
  if ( isEditSelection )
  {
    textOption.state |= QStyle::State_Selected;
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
  drawDecoration( painter, iconOption, iconLayoutBounds, icon );
}
