/***************************************************************************
                             qgslayoutitemelevationprofile.cpp
                             ---------------------------------
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

#include "qgslayoutitemelevationprofile.h"
#include "qgslayoutitemregistry.h"


QgsLayoutItemElevationProfile::QgsLayoutItemElevationProfile( QgsLayout *layout )
  : QgsLayoutItem( layout )
{
  //get default layout font from settings
  const QgsSettings settings;
  const QString defaultFontString = settings.value( QStringLiteral( "LayoutDesigner/defaultFont" ), QVariant(), QgsSettings::Gui ).toString();
  if ( !defaultFontString.isEmpty() )
  {
//    QFont f = mFormat.font();
//   f.setFamily( defaultFontString );
    //  mFormat.setFont( f );
  }

  //default to a 10 point font size
  //mFormat.setSize( 10 );
  //mFormat.setSizeUnit( QgsUnitTypes::RenderPoints );

  //default to no background
  setBackgroundEnabled( false );
}

QgsLayoutItemElevationProfile *QgsLayoutItemElevationProfile::create( QgsLayout *layout )
{
  return new QgsLayoutItemElevationProfile( layout );
}

int QgsLayoutItemElevationProfile::type() const
{
  return QgsLayoutItemRegistry::LayoutElevationProfile;
}

QIcon QgsLayoutItemElevationProfile::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemLabel.svg" ) );
}

void QgsLayoutItemElevationProfile::draw( QgsLayoutItemRenderContext &context )
{

}

bool QgsLayoutItemElevationProfile::writePropertiesToElement( QDomElement &layoutLabelElem, QDomDocument &doc, const QgsReadWriteContext &rwContext ) const
{


  return true;
}

bool QgsLayoutItemElevationProfile::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &, const QgsReadWriteContext &context )
{

  return true;
}

