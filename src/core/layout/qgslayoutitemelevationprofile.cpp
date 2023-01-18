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
#include "qgsplot.h"
#include "qgslayout.h"
#include "qgsmessagelog.h"
#include "qgsmaplayerlistutils_p.h"

///@cond PRIVATE
class QgsLayoutItemElevationProfilePlot : public Qgs2DPlot
{
  public:

    QgsLayoutItemElevationProfilePlot()
    {
    }

    void renderContent( QgsRenderContext &rc, const QRectF &plotArea ) override
    {
      Q_UNUSED( rc );
      Q_UNUSED( plotArea )
    }

  private:

};
///@endcond PRIVATE

QgsLayoutItemElevationProfile::QgsLayoutItemElevationProfile( QgsLayout *layout )
  : QgsLayoutItem( layout )
  , mPlot( std::make_unique< QgsLayoutItemElevationProfilePlot >() )
{
  //default to no background
  setBackgroundEnabled( false );
}

QgsLayoutItemElevationProfile::~QgsLayoutItemElevationProfile() = default;

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
  return QgsApplication::getThemeIcon( QStringLiteral( "mLayoutItemElevationProfile.svg" ) );
}

void QgsLayoutItemElevationProfile::refreshDataDefinedProperty( DataDefinedProperty property )
{
  const QgsExpressionContext context = createExpressionContext();

  bool forceUpdate = false;

  if ( ( property == QgsLayoutObject::ElevationProfileMinimumDistance || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileMinimumDistance ) ) )
  {
    double value = mPlot->xMinimum();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileMinimumDistance, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile minimum distance expression eval error" ) );
    }
    else
    {
      mPlot->setXMinimum( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileMaximumDistance || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileMaximumDistance ) ) )
  {
    double value = mPlot->xMaximum();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileMaximumDistance, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile maximum distance expression eval error" ) );
    }
    else
    {
      mPlot->setXMaximum( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileMinimumElevation || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileMinimumElevation ) ) )
  {
    double value = mPlot->yMinimum();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileMinimumElevation, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile minimum elevation expression eval error" ) );
    }
    else
    {
      mPlot->setYMinimum( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileMaximumElevation || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileMaximumElevation ) ) )
  {
    double value = mPlot->yMaximum();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileMaximumElevation, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile maximum elevation expression eval error" ) );
    }
    else
    {
      mPlot->setYMaximum( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileDistanceMajorInterval || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileDistanceMajorInterval ) ) )
  {
    double value = mPlot->xAxis().gridIntervalMajor();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileDistanceMajorInterval, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile distance axis major interval expression eval error" ) );
    }
    else
    {
      mPlot->xAxis().setGridIntervalMajor( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileDistanceMinorInterval || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileDistanceMinorInterval ) ) )
  {
    double value = mPlot->xAxis().gridIntervalMinor();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileDistanceMinorInterval, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile distance axis minor interval expression eval error" ) );
    }
    else
    {
      mPlot->xAxis().setGridIntervalMinor( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileDistanceLabelInterval || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileDistanceLabelInterval ) ) )
  {
    double value = mPlot->xAxis().labelInterval();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileDistanceLabelInterval, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile distance axis label interval expression eval error" ) );
    }
    else
    {
      mPlot->xAxis().setLabelInterval( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileElevationMajorInterval || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileElevationMajorInterval ) ) )
  {
    double value = mPlot->yAxis().gridIntervalMajor();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileElevationMajorInterval, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile elevation axis major interval expression eval error" ) );
    }
    else
    {
      mPlot->yAxis().setGridIntervalMajor( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileElevationMinorInterval || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileElevationMinorInterval ) ) )
  {
    double value = mPlot->yAxis().gridIntervalMinor();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileElevationMinorInterval, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile elevation axis minor interval expression eval error" ) );
    }
    else
    {
      mPlot->yAxis().setGridIntervalMinor( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::ElevationProfileElevationLabelInterval || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::ElevationProfileElevationLabelInterval ) ) )
  {
    double value = mPlot->yAxis().labelInterval();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ElevationProfileElevationLabelInterval, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile elevation axis label interval expression eval error" ) );
    }
    else
    {
      mPlot->yAxis().setLabelInterval( value );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::MarginLeft || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::MarginLeft ) ) )
  {
    double value = mPlot->margins().left();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MarginLeft, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile left margin expression eval error" ) );
    }
    else
    {
      QgsMargins margins = mPlot->margins();
      margins.setLeft( value );
      mPlot->setMargins( margins );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::MarginRight || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::MarginRight ) ) )
  {
    double value = mPlot->margins().right();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MarginRight, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile right margin expression eval error" ) );
    }
    else
    {
      QgsMargins margins = mPlot->margins();
      margins.setRight( value );
      mPlot->setMargins( margins );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::MarginTop || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::MarginTop ) ) )
  {
    double value = mPlot->margins().top();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MarginTop, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile top margin expression eval error" ) );
    }
    else
    {
      QgsMargins margins = mPlot->margins();
      margins.setTop( value );
      mPlot->setMargins( margins );
    }

    forceUpdate = true;
  }

  if ( ( property == QgsLayoutObject::MarginBottom || property == QgsLayoutObject::AllProperties )
       && ( mDataDefinedProperties.isActive( QgsLayoutObject::MarginBottom ) ) )
  {
    double value = mPlot->margins().bottom();

    bool ok = false;
    value = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::MarginBottom, context, value, &ok );

    if ( !ok )
    {
      QgsMessageLog::logMessage( tr( "Elevation profile bottom margin expression eval error" ) );
    }
    else
    {
      QgsMargins margins = mPlot->margins();
      margins.setBottom( value );
      mPlot->setMargins( margins );
    }

    forceUpdate = true;
  }


  if ( forceUpdate )
  {
    refreshItemSize();
    update();
  }

  QgsLayoutItem::refreshDataDefinedProperty( property );
}

Qgs2DPlot *QgsLayoutItemElevationProfile::plot()
{
  return mPlot.get();
}

const Qgs2DPlot *QgsLayoutItemElevationProfile::plot() const
{
  return mPlot.get();
}

QList<QgsMapLayer *> QgsLayoutItemElevationProfile::layers() const
{
  return _qgis_listRefToRaw( mLayers );
}

void QgsLayoutItemElevationProfile::setLayers( const QList<QgsMapLayer *> &layers )
{
  if ( layers == _qgis_listRefToRaw( mLayers ) )
    return;

  mLayers = _qgis_listRawToRef( layers );
}

void QgsLayoutItemElevationProfile::draw( QgsLayoutItemRenderContext &context )
{
  // size must be in pixels, not layout units
  mPlot->setSize( rect().size() * context.renderContext().scaleFactor() );
  mPlot->render( context.renderContext() );
}

bool QgsLayoutItemElevationProfile::writePropertiesToElement( QDomElement &layoutProfileElem, QDomDocument &doc, const QgsReadWriteContext &rwContext ) const
{
  {
    QDomElement plotElement = doc.createElement( QStringLiteral( "plot" ) );
    mPlot->writeXml( plotElement, doc, rwContext );
    layoutProfileElem.appendChild( plotElement );
  }

  {
    QDomElement layersElement = doc.createElement( QStringLiteral( "layers" ) );
    for ( const QgsMapLayerRef &layer : mLayers )
    {
      QDomElement layerElement = doc.createElement( QStringLiteral( "layer" ) );
      layer.writeXml( layerElement, rwContext );
      layersElement.appendChild( layerElement );
    }
    layoutProfileElem.appendChild( layersElement );
  }

  return true;
}

bool QgsLayoutItemElevationProfile::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &, const QgsReadWriteContext &context )
{
  const QDomElement plotElement = itemElem.firstChildElement( QStringLiteral( "plot" ) );
  if ( !plotElement.isNull() )
  {
    mPlot->readXml( plotElement, context );
  }

  {
    mLayers.clear();
    const QDomElement layersElement = itemElem.firstChildElement( QStringLiteral( "layers" ) );
    QDomElement layerElement = layersElement.firstChildElement( QStringLiteral( "layer" ) );
    while ( !layerElement.isNull() )
    {
      QgsMapLayerRef ref;
      ref.readXml( layerElement, context );
      ref.resolveWeakly( mLayout->project() );
      mLayers.append( ref );

      layerElement = layerElement.nextSiblingElement( QStringLiteral( "layer" ) );
    }
  }

  return true;
}

