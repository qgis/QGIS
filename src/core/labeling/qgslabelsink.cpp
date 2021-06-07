/***************************************************************************
                         qgslabelsink.cpp
                         ---------------------
    begin                : January 2014
    copyright            : (C) 2014 by Marco Hugentobler
    email                : marco at sourcepole dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgslabelsink.h"
#include "qgspallabeling.h"
#include "qgsmapsettings.h"
#include "qgslogger.h"

QgsLabelSinkProvider::QgsLabelSinkProvider( QgsVectorLayer *layer, const QString &providerId, QgsLabelSink *dxf, const QgsPalLayerSettings *settings )
  : QgsVectorLayerLabelProvider( layer, providerId, false, settings )
  , mLabelSink( dxf )
{
}

void QgsLabelSinkProvider::drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const
{
  Q_ASSERT( mLabelSink );
  mLabelSink->drawLabel( layerId(), context, label, mSettings );
}

QgsRuleBasedLabelSinkProvider::QgsRuleBasedLabelSinkProvider( const QgsRuleBasedLabeling &rules, QgsVectorLayer *layer, QgsLabelSink *dxf )
  : QgsRuleBasedLabelProvider( rules, layer, false )
  , mLabelSink( dxf )
{
  mRules->rootRule()->createSubProviders( layer, mSubProviders, this );
}

void QgsRuleBasedLabelSinkProvider::reinit( QgsVectorLayer *layer )
{
  mRules->rootRule()->createSubProviders( layer, mSubProviders, this );
}

QgsVectorLayerLabelProvider *QgsRuleBasedLabelSinkProvider::createProvider( QgsVectorLayer *layer, const QString &providerId, bool withFeatureLoop, const QgsPalLayerSettings *settings )
{
  Q_UNUSED( withFeatureLoop )
  return new QgsLabelSinkProvider( layer, providerId, mLabelSink, settings );
}

void QgsRuleBasedLabelSinkProvider::drawLabel( QgsRenderContext &context, pal::LabelPosition *label ) const
{
  Q_ASSERT( mLabelSink );
  mLabelSink->drawLabel( layerId(), context, label, mSettings );
}
