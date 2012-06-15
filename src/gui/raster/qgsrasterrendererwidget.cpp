/***************************************************************************
                         qgsrasterrendererwidget.cpp
                         ---------------------------
    begin                : June 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrasterrendererwidget.h"
#include "qgsrasterdataprovider.h"
#include "qgsrasterlayer.h"


QString QgsRasterRendererWidget::displayBandName( int band ) const
{
  QString name;
  if ( !mRasterLayer )
  {
    return name;
  }

  const QgsRasterDataProvider* provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return name;
  }

  name = provider->colorInterpretationName( band );
  if ( name == "Undefined" )
  {
    name = provider->generateBandName( band );
  }
  return name;
}
