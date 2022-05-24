/***************************************************************************
    qgscombinedstylemodel.cpp
    ---------------
    begin                : May 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscombinedstylemodel.h"
#include "qgsstyle.h"
#include "qgsstylemodel.h"
#include "qgssingleitemmodel.h"
#include "qgsapplication.h"

#if QT_VERSION >= QT_VERSION_CHECK(5, 13, 0)


QgsCombinedStyleModel::QgsCombinedStyleModel( QObject *parent )
  : QConcatenateTablesProxyModel( parent )
{

}

QVariant QgsCombinedStyleModel::headerData( int section, Qt::Orientation orientation, int role ) const
{
  return QgsStyleModel::headerDataStatic( section, orientation, role );
}

void QgsCombinedStyleModel::addStyle( QgsStyle *style )
{
  connect( style, &QgsStyle::destroyed, this, [this, style]()
  {
    if ( QgsSingleItemModel *model = mTitleModels.value( style ) )
    {
      removeSourceModel( model );
      mTitleModels.remove( style );
      delete model;
    }

    if ( QgsStyleModel *model = mOwnedStyleModels.value( style ) )
    {
      removeSourceModel( model );
      mOwnedStyleModels.remove( style );
      delete model;
    }
    mStyles.removeAll( style );
  } );

  mStyles.append( style );

  QgsSingleItemModel *titleModel = new QgsSingleItemModel( this,
  {
    {
      { Qt::DisplayRole, style->name() },
      { Qt::ToolTipRole, style->name() },
      { QgsStyleModel::IsTitleRole, true },
      { QgsStyleModel::StyleFileName, style->fileName() },
      { QgsStyleModel::StyleName, style->name() },
    }, {
      { QgsStyleModel::IsTitleRole, true },
      { QgsStyleModel::StyleFileName, style->fileName() },
      { QgsStyleModel::StyleName, style->name() }
    }
  } );
  addSourceModel( titleModel );
  mTitleModels.insert( style, titleModel );

  QgsStyleModel *styleModel = new QgsStyleModel( style );

  for ( QSize size : std::as_const( mAdditionalSizes ) )
  {
    styleModel->addDesiredIconSize( size );
  }

  addSourceModel( styleModel );
  mOwnedStyleModels.insert( style, styleModel );
}

void QgsCombinedStyleModel::removeStyle( QgsStyle *style )
{
  if ( QgsSingleItemModel *model = mTitleModels.value( style ) )
  {
    removeSourceModel( model );
    mTitleModels.remove( style );
    delete model;
  }

  if ( QgsStyleModel *model = mOwnedStyleModels.value( style ) )
  {
    removeSourceModel( model );
    mOwnedStyleModels.remove( style );
    delete model;
  }
  mStyles.removeAll( style );
}

void QgsCombinedStyleModel::addDefaultStyle()
{
  QgsStyle *defaultStyle = QgsStyle::defaultStyle();
  QgsStyleModel *styleModel = QgsApplication::defaultStyleModel();
  if ( !defaultStyle || !styleModel )
    return;

  mStyles.append( defaultStyle );

  QgsSingleItemModel *titleModel = new QgsSingleItemModel( this,
  {
    {
      { Qt::DisplayRole, defaultStyle->name() },
      { Qt::ToolTipRole, defaultStyle->name() },
      { QgsStyleModel::IsTitleRole, true },
      { QgsStyleModel::StyleFileName, defaultStyle->fileName() },
      { QgsStyleModel::StyleName, defaultStyle->name() },
    }, {
      { QgsStyleModel::IsTitleRole, true },
      { QgsStyleModel::StyleFileName, defaultStyle->fileName() },
      { QgsStyleModel::StyleName, defaultStyle->name() }
    }
  } );

  addSourceModel( titleModel );
  mTitleModels.insert( defaultStyle, titleModel );

  for ( QSize size : std::as_const( mAdditionalSizes ) )
  {
    styleModel->addDesiredIconSize( size );
  }

  addSourceModel( styleModel );
}

QList< QgsStyle * > QgsCombinedStyleModel::styles() const
{
  return mStyles;
}

void QgsCombinedStyleModel::addDesiredIconSize( QSize size )
{
  if ( !mAdditionalSizes.contains( size ) )
    mAdditionalSizes.append( size );

  for ( auto it = mOwnedStyleModels.constBegin(); it != mOwnedStyleModels.constEnd(); ++it )
  {
    it.value()->addDesiredIconSize( size );
  }

  if ( mStyles.contains( QgsStyle::defaultStyle() ) )
  {
    QgsApplication::defaultStyleModel()->addDesiredIconSize( size );
  }
}

#endif
