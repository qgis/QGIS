/***************************************************************************
                         qgsdxfpallabeling.cpp
                         ---------------------
    begin                : January 2014
    copyright            : (C) 2014 by Marco Hugentobler
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

#include "qgsdxfpallabeling.h"
#include "qgsdxfexport.h"
#include "qgspalgeometry.h"
#include "qgspallabeling.h"
#include "qgsmapsettings.h"

#include "pal/pointset.h"
#include "pal/labelposition.h"


QgsDxfLabelProvider::QgsDxfLabelProvider( QgsVectorLayer* layer , QgsDxfExport* dxf )
    : QgsVectorLayerLabelProvider( layer, false )
    , mDxfExport( dxf )
{
}

void QgsDxfLabelProvider::drawLabel( QgsRenderContext& context, pal::LabelPosition* label ) const
{
  Q_UNUSED( context );

  //debug: print label infos
  if ( mDxfExport )
  {
    QgsTextLabelFeature* lf = dynamic_cast<QgsTextLabelFeature*>( label->getFeaturePart()->feature() );
    if ( !lf )
      return;

    const QgsPalLayerSettings& tmpLyr = mSettings;

    //label text
    QString txt = lf->text( label->getPartId() );

    //angle
    double angle = label->getAlpha() * 180 / M_PI;

    QgsFeatureId fid = label->getFeaturePart()->featureId();
    QString dxfLayer = mDxfLayerNames[fid];

    //debug: show label rectangle
#if 0
    QgsPolyline line;
    for ( int i = 0; i < 4; ++i )
    {
      line.append( QgsPoint( label->getX( i ), label->getY( i ) ) );
    }
    mDxfExport->writePolyline( line, dxfLayer, "CONTINUOUS", 1, 0.01, true );
#endif

    QString wrapchr = tmpLyr.wrapChar.isEmpty() ? "\n" : tmpLyr.wrapChar;

    //add the direction symbol if needed
    if ( !txt.isEmpty() && tmpLyr.placement == QgsPalLayerSettings::Line && tmpLyr.addDirectionSymbol )
    {
      bool prependSymb = false;
      QString symb = tmpLyr.rightDirectionSymbol;

      if ( label->getReversed() )
      {
        prependSymb = true;
        symb = tmpLyr.leftDirectionSymbol;
      }

      if ( tmpLyr.reverseDirectionSymbol )
      {
        if ( symb == tmpLyr.rightDirectionSymbol )
        {
          prependSymb = true;
          symb = tmpLyr.leftDirectionSymbol;
        }
        else
        {
          prependSymb = false;
          symb = tmpLyr.rightDirectionSymbol;
        }
      }

      if ( tmpLyr.placeDirectionSymbol == QgsPalLayerSettings::SymbolAbove )
      {
        prependSymb = true;
        symb = symb + wrapchr;
      }
      else if ( tmpLyr.placeDirectionSymbol == QgsPalLayerSettings::SymbolBelow )
      {
        prependSymb = false;
        symb = wrapchr + symb;
      }

      if ( prependSymb )
      {
        txt.prepend( symb );
      }
      else
      {
        txt.append( symb );
      }
    }

    txt = txt.replace( wrapchr, "\\P" );

    if ( tmpLyr.textFont.underline() )
    {
      txt.prepend( "\\L" ).append( "\\l" );
    }

    if ( tmpLyr.textFont.overline() )
    {
      txt.prepend( "\\O" ).append( "\\o" );
    }

    if ( tmpLyr.textFont.strikeOut() )
    {
      txt.prepend( "\\K" ).append( "\\k" );
    }

    txt.prepend( QString( "\\f%1|i%2|b%3;\\H%4;\\W0.75;" )
                 .arg( tmpLyr.textFont.family() )
                 .arg( tmpLyr.textFont.italic() ? 1 : 0 )
                 .arg( tmpLyr.textFont.bold() ? 1 : 0 )
                 .arg( label->getHeight() / ( 1 + txt.count( "\\P" ) ) * 0.75 ) );

    mDxfExport->writeMText( dxfLayer, txt, QgsPoint( label->getX(), label->getY() ), label->getWidth() * 1.1, angle, tmpLyr.textColor );
  }
}

void QgsDxfLabelProvider::registerDxfFeature( QgsFeature& feature, const QgsRenderContext& context, const QString& dxfLayerName )
{
  registerFeature( feature, context );
  mDxfLayerNames[feature.id()] = dxfLayerName;
}
