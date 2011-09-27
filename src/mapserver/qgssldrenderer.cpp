/***************************************************************************
                              qgssldrenderer.cpp
                 A renderer flexible enough for sld specified symbolisation
                              -------------------
  begin                : May 12, 2006
  copyright            : (C) 2006 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgssldrenderer.h"
#include "qgsfeature.h"
#include "qgsfilter.h"
#include "qgssldrule.h"
#include "qgssymbol.h"
#include "qgslogger.h"
#include <QPainter>
#include <QPixmap>
#include <QSet>

QgsSLDRenderer::QgsSLDRenderer( QGis::GeometryType type ): QgsRenderer(), mScaleDenominator( 0 ), mUseTransparencyOnClassLevel( false )
{
  mGeometryType = type;
}

QgsSLDRenderer::~QgsSLDRenderer()
{
  //free all the pending rules
  for ( QList<QgsSLDRule*>::iterator it = mRules.begin(); it != mRules.end(); ++it )
  {
    delete( *it );
  }
}

void QgsSLDRenderer::addRule( QgsSLDRule* rule )
{
  mRules.push_back( rule );
}

void QgsSLDRenderer::renderFeature( QgsRenderContext &renderContext, QgsFeature& f, QImage* pic, bool selected, double opacity )
{
  Q_UNUSED( opacity );
  //find a rule that matches the feature
  QList<QgsSLDRule*>::const_iterator rule_it = mRules.constBegin();
  for ( ; rule_it != mRules.constEnd(); ++rule_it )
  {
    if (( *rule_it )->evaluate( f, mScaleDenominator ) )
    {
      //we found a matching rule to apply symbology
      if (( *rule_it )->applySymbology( renderContext.painter(), f, pic, selected, renderContext.scaleFactor(), renderContext.rasterScaleFactor(), mGeometryType, mSelectionColor ) == 0 )
      {
        return;
      }
    }
  }

  //no rule applies. Set Qt::NoBrush, Qt::NoPen and a transparent pixmap
  renderContext.painter()->setPen( QPen( Qt::NoPen ) );
  renderContext.painter()->setBrush( QBrush( Qt::NoBrush ) );
  pic->fill( 0 );
}

bool QgsSLDRenderer::needsAttributes() const
{
  //todo: parse the entries to find out
  return true;
}

QgsAttributeList QgsSLDRenderer::classificationAttributes() const
{
  QSet<int> attributeSet;

  QList<QgsSLDRule*>::const_iterator rule_it = mRules.constBegin();
  for ( ; rule_it != mRules.constEnd(); ++rule_it )
  {
    QSet<int> ruleAttributes = ( *rule_it )->attributeIndices();
    QSet<int>::const_iterator att_it = ruleAttributes.constBegin();
    for ( ; att_it != ruleAttributes.constEnd(); ++att_it )
    {
      attributeSet.insert( *att_it );
    }
  }

  QgsAttributeList attributeList;

  QSet<int>::const_iterator setIt = attributeSet.constBegin();
  for ( ; setIt != attributeSet.constEnd(); ++setIt )
  {
    attributeList.push_back( *setIt );
  }
  return attributeList;
}

const QList<QgsSymbol*> QgsSLDRenderer::symbols() const
{
  return QList<QgsSymbol*>(); //soon...
#if 0
  return mSymbols;
#endif //0
}

void QgsSLDRenderer::refreshLegend( std::list< std::pair<QString, QPixmap> >* symbologyList ) const
{
  Q_UNUSED( symbologyList );
  //soon...
}

QgsRenderer* QgsSLDRenderer::clone() const
{
  //soon...
  return 0;
}

bool QgsSLDRenderer::containsPixmap() const
{
  return false; //later with diagrams maybe true...
}

bool QgsSLDRenderer::usesTransparency() const
{
  return mUseTransparencyOnClassLevel;
}

