/***************************************************************************
                         qgslayoutelevationprofilewidget.cpp
                         ----------------------
    begin                : January 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslayoutelevationprofilewidget.h"
#include "qgslayoutitemelevationprofile.h"
#include "qgslayoutitemwidget.h"
#include "qgslayoutitemregistry.h"

QgsLayoutElevationProfileWidget::QgsLayoutElevationProfileWidget( QgsLayoutItemElevationProfile *profile )
  : QgsLayoutItemBaseWidget( nullptr, profile )
  , mProfile( profile )
{
  Q_ASSERT( mProfile );

  setupUi( this );
  setPanelTitle( tr( "Elevation Profile Properties" ) );

  //add widget for general composer item properties
  mItemPropertiesWidget = new QgsLayoutItemPropertiesWidget( this, profile );
  mainLayout->addWidget( mItemPropertiesWidget );
}

void QgsLayoutElevationProfileWidget::setMasterLayout( QgsMasterLayoutInterface *masterLayout )
{
  if ( mItemPropertiesWidget )
    mItemPropertiesWidget->setMasterLayout( masterLayout );
}

QgsExpressionContext QgsLayoutElevationProfileWidget::createExpressionContext() const
{
  return mProfile->createExpressionContext();
}

bool QgsLayoutElevationProfileWidget::setNewItem( QgsLayoutItem *item )
{
  if ( item->type() != QgsLayoutItemRegistry::LayoutElevationProfile )
    return false;

  if ( mProfile )
  {
    disconnect( mProfile, &QgsLayoutObject::changed, this, &QgsLayoutElevationProfileWidget::setGuiElementValues );
  }

  mProfile = qobject_cast< QgsLayoutItemElevationProfile * >( item );
  mItemPropertiesWidget->setItem( mProfile );

  if ( mProfile )
  {
    connect( mProfile, &QgsLayoutObject::changed, this, &QgsLayoutElevationProfileWidget::setGuiElementValues );
  }

  setGuiElementValues();

  return true;
}

void QgsLayoutElevationProfileWidget::setGuiElementValues()
{

}
