/***************************************************************************
    qgscolortooltip_p.h
    ---------------------
    begin                : 2024/08/21
    copyright            : (C) 2024 by Julien Cabieces
    email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOLORTOOLTIP_P_H
#define QGSCOLORTOOLTIP_P_H

#include <QPainter>
#include <QBuffer>

#include "qgssymbollayerutils.h"

//! helper class to generate color tooltip

/**
 * \ingroup gui
 * \since QGIS 3.40
 */
class QgsColorTooltip
{
  public:
    //! Returns an HTML desciption given a \a color with a preview image of the color
    template<typename T>
    static QString htmlDescription( QColor color, T *widget )
    {
      // create very large preview swatch
      const int width = static_cast<int>( Qgis::UI_SCALE_FACTOR * widget->fontMetrics().horizontalAdvance( 'X' ) * 23 );
      const int height = static_cast<int>( width / 1.61803398875 ); // golden ratio

      const int margin = static_cast<int>( height * 0.1 );
      QImage icon = QImage( width + 2 * margin, height + 2 * margin, QImage::Format_ARGB32 );
      icon.fill( Qt::transparent );

      QPainter p;
      p.begin( &icon );

      //start with checkboard pattern
      const QBrush checkBrush = QBrush( widget->transparentBackground() );
      p.setPen( Qt::NoPen );
      p.setBrush( checkBrush );
      p.drawRect( margin, margin, width, height );

      //draw color over pattern
      p.setBrush( QBrush( color ) );
      p.drawRect( margin, margin, width, height );

      if ( color.alpha() < 255 )
      {
        //draw fully opaque color over half of the area
        color.setAlpha( 255 );
        p.setBrush( QBrush( color ) );
        p.drawRect( margin, margin, width / 2, height );
      }

      //draw border
      p.setPen( QColor( 197, 197, 197 ) );
      p.setBrush( Qt::NoBrush );
      p.drawRect( margin, margin, width, height );
      p.end();

      QByteArray data;
      QBuffer buffer( &data );
      icon.save( &buffer, "PNG", 100 );

      QString info = QStringLiteral( "<b>HEX</b> %1<br>" ).arg( color.name() );

      if ( color.spec() == QColor::Spec::Cmyk )
      {
        const double cyan = color.cyanF() * 100.;
        const double magenta = color.magentaF() * 100.;
        const double yellow = color.yellowF() * 100.;
        const double black = color.blackF() * 100.;
        const double alpha = color.alphaF() * 100.;

        info += QStringLiteral( "<b>CMYKA</b> %1,%2,%3,%4,%5<p>" )
                  .arg( cyan, 0, 'f', 2 )
                  .arg( magenta, 0, 'f', 2 )
                  .arg( yellow, 0, 'f', 2 )
                  .arg( black, 0, 'f', 2 )
                  .arg( alpha, 0, 'f', 2 );
      }
      else
      {
        const int hue = color.hue();
        const int value = color.value();
        const int saturation = color.saturation();

        info += QStringLiteral( "<b>RGBA</b> %1<br>"
                                "<b>HSV</b> %2,%3,%4<p>" )
                  .arg( QgsSymbolLayerUtils::encodeColor( color ) )
                  .arg( hue )
                  .arg( saturation )
                  .arg( value );
      }

      info += QStringLiteral( "<img src='data:image/png;base64, %1'>" ).arg( QString( data.toBase64() ) );

      return info;
    }
};

#endif
