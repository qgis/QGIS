/***************************************************************************
    qgsdigitizingguide.cpp
    ----------------------
    begin                : December 2023
    copyright            : (C) Denis Rouzaud
    email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdigitizingguidemodel.h"

#include "qgsdigitizingguidelayer.h"
#include "qgsannotationitem.h"
#include "qgsannotationpointtextitem.h"
#include "qgsannotationlinetextitem.h"

#include <QItemSelection>
#include <QModelIndexList>


QgsDigitizingGuideModel::QgsDigitizingGuideModel( QgsDigitizingGuideLayer *guideLayer )
  : QAbstractTableModel( guideLayer )
  , mGuideLayer( guideLayer )
{
}

void QgsDigitizingGuideModel::addPointGuide( const QString &guideItemId,
    const QString &title,
    const QString &titleItemId,
    QStringList details,
    const QDateTime &creation )
{
  beginInsertRows( QModelIndex(), mGuides.count(), mGuides.count() );
  GuideInfo item( guideItemId, creation );
  item.mType = QStringLiteral( "point-guide" );
  item.mTitle = title;
  item.mTitleItemId = titleItemId;
  item.mDetails = details;
  mGuides << item;
  endInsertRows();
}

void QgsDigitizingGuideModel::addLineGuide( const QString &guideItemId,
    const QString &title,
    const QString &titleItemId,
    QStringList details,
    const QDateTime &creation )
{
  beginInsertRows( QModelIndex(), mGuides.count(), mGuides.count() );
  GuideInfo item( guideItemId, creation );
  item.mType = QStringLiteral( "line-guide" );
  item.mTitle = title;
  item.mTitleItemId = titleItemId;
  item.mDetails = details;
  mGuides << item;
  endInsertRows();
}

void QgsDigitizingGuideModel::clear()
{
  beginResetModel();
  mGuides.clear();
  endResetModel();
}

bool QgsDigitizingGuideModel::removeGuides( const QModelIndexList &indexList )
{
  beginResetModel();
  bool ok = false;

  // so that rows are removed from highest index
  QModelIndexList sortedIndexes = indexList;
  std::sort( sortedIndexes.begin(), sortedIndexes.end(), []( const QModelIndex & a, const QModelIndex & b ) {return a.row() > b.row();} );

  for ( const QModelIndex &index : sortedIndexes )
  {
    if ( !index.isValid() )
      continue;

    ok = true;

    GuideInfo guide = mGuides.at( index.row() );

    mGuideLayer->removeItem( guide.mGuideItemId );
    mGuideLayer->removeItem( guide.mTitleItemId );

    for ( const QString &detailId : std::as_const( guide.mDetails ) )
    {
      mGuideLayer->removeItem( detailId );
    }

    mGuides.removeAt( index.row() );
  }
  endResetModel();
  return ok;
}

int QgsDigitizingGuideModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mGuides.count();
}

int QgsDigitizingGuideModel::columnCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return 1;
}

QVariant QgsDigitizingGuideModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  const GuideInfo &item = mGuides.at( index.row() );

  if ( index.column() == 0 && role == Qt::DisplayRole )
    return item.mTitle;

  if ( index.column() == 0 && role == Qt::CheckStateRole )
    return item.mEnabled ? Qt::Checked : Qt::Unchecked;

  if ( index.column() == 0 && role == Qt::UserRole )
    return item.mGuideItemId;

  return QVariant();
}

Qt::ItemFlags QgsDigitizingGuideModel::flags( const QModelIndex &index ) const
{
  if ( !index.isValid() )
    return Qt::ItemFlags();

  if ( index.column() == 0 )
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsUserCheckable | Qt::ItemIsEditable;

  return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

bool QgsDigitizingGuideModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() )
    return false;

  if ( index.column() == 0 && role == static_cast<int>( Qt::EditRole ) )
  {
    mGuides[index.row()].mTitle = value.toString();
    QgsAnnotationItem *titleItem = mGuideLayer->item( mGuides[index.row()].mTitleItemId );

    if ( mGuides[index.row()].mType == QStringLiteral( "point-guide" ) )
    {
      QgsAnnotationPointTextItem *pointTextItem = dynamic_cast<QgsAnnotationPointTextItem *>( titleItem );
      if ( pointTextItem )
      {
        pointTextItem->setText( value.toString() );
        mGuideLayer->triggerRepaint();
        emit dataChanged( index, index );
        return true;
      }
    }
    else if ( mGuides[index.row()].mType == QStringLiteral( "line-guide" ) )
    {
      QgsAnnotationLineTextItem *lineTextItem = dynamic_cast<QgsAnnotationLineTextItem *>( titleItem );
      if ( lineTextItem )
      {
        lineTextItem->setText( value.toString() );
        mGuideLayer->triggerRepaint();
        emit dataChanged( index, index );
        return true;
      }
    }
  }

  if ( index.column() == 0 && role == static_cast<int>( Qt::CheckStateRole ) )
  {
    bool enabled = value == Qt::Checked ? true : false;
    QgsAnnotationItem *item = mGuideLayer->item( mGuides[index.row()].mGuideItemId );
    if ( item )
    {
      item->setEnabled( enabled );
      mGuideLayer->triggerRepaint();
    }
    mGuides[index.row()].mEnabled = enabled;
    emit dataChanged( index, index, {role} );
    return true;
  }

  return false;
}


void QgsDigitizingGuideModel::selectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  const QModelIndexList deselectedIndexes = deselected.indexes();
  for ( const QModelIndex &index : deselectedIndexes )
  {
    if ( index.isValid() )
    {
      GuideInfo guide = mGuides.at( index.row() );
      QgsAnnotationItem *titleItem = mGuideLayer->item( guide.mTitleItemId );
      if ( titleItem )
        titleItem->setEnabled( false );
      for ( const QString &detailId : std::as_const( guide.mDetails ) )
      {
        QgsAnnotationItem *detailItem = mGuideLayer->item( detailId );
        if ( detailItem )
          detailItem->setEnabled( false );
      }
    }
  }

  const QModelIndexList selectedIndexes = selected.indexes();
  if ( selectedIndexes.count() )
  {
    for ( const QModelIndex &index : selectedIndexes )
    {
      if ( index.isValid() )
      {
        GuideInfo guide = mGuides.at( index.row() );
        QgsAnnotationItem *titleItem = mGuideLayer->item( guide.mTitleItemId );
        if ( titleItem )
          titleItem->setEnabled( true );
        for ( const QString &detailId : std::as_const( guide.mDetails ) )
        {
          QgsAnnotationItem *detailItem = mGuideLayer->item( detailId );
          if ( detailItem )
            detailItem->setEnabled( true );
        }
      }
    }
  }
  else
  {
    mGuideLayer->setGuideHighlight();
  }

  mGuideLayer->triggerRepaint();
}


