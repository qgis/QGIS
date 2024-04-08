/***************************************************************************
                         qgsrastersinglecolorrendererwidget.cpp
                         ---------------------------------
    begin                : April 2024
    copyright            : (C) 2024 by Mathieu Pellerin
    email                : mathieu at opengis dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsrastersinglecolorrendererwidget.h"
#include "qgsrastersinglecolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"

QgsRasterSingleColorRendererWidget::QgsRasterSingleColorRendererWidget( QgsRasterLayer *layer, const QgsRectangle &extent )
  : QgsRasterRendererWidget( layer, extent )
{
  setupUi( this );

  if ( mRasterLayer )
  {
    QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
    if ( !provider )
    {
      return;
    }

    connect( mColor, &QgsColorButton::colorChanged, this, &QgsRasterSingleColorRendererWidget::colorChanged );

    setFromRenderer( layer->renderer() );
  }
}

QgsRasterRenderer *QgsRasterSingleColorRendererWidget::renderer()
{
  if ( !mRasterLayer )
  {
    return nullptr;
  }

  QgsRasterDataProvider *provider = mRasterLayer->dataProvider();
  if ( !provider )
  {
    return nullptr;
  }

  QgsRasterSingleColorRenderer *renderer = new QgsRasterSingleColorRenderer( provider, mColor->color() );
  return renderer;
}

void QgsRasterSingleColorRendererWidget::colorChanged( const QColor & )
{
  emit widgetChanged();
}

void QgsRasterSingleColorRendererWidget::setFromRenderer( const QgsRasterRenderer *r )
{
  const QgsRasterSingleColorRenderer *scr = dynamic_cast<const QgsRasterSingleColorRenderer *>( r );
  if ( scr )
  {
    mColor->setColor( scr->color() );
  }
  else
  {
    mColor->setColor( QColor( 0, 0, 0 ) );
  }
}
