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
#include "qgsmessagelog.h"

///@cond PRIVATE
class QgsLayoutItemElevationProfilePlot : public Qgs2DPlot
{
  public:

    QgsLayoutItemElevationProfilePlot()
    {
    }

    void renderContent( QgsRenderContext &rc, const QRectF &plotArea ) override
    {

    }

  private:

};
///@endcond PRIVATE

QgsLayoutItemElevationProfile::QgsLayoutItemElevationProfile( QgsLayout *layout )
  : QgsLayoutItem( layout )
  , mPlot( std::make_unique< QgsLayoutItemElevationProfilePlot >() )
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
  return QgsApplication::getThemeIcon( QStringLiteral( "/mLayoutItemLabel.svg" ) );
}

void QgsLayoutItemElevationProfile::refreshDataDefinedProperty( DataDefinedProperty property )
{
  const QgsExpressionContext context = createExpressionContext();

  bool forceUpdate = false;

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

void QgsLayoutItemElevationProfile::draw( QgsLayoutItemRenderContext &context )
{
  // size must be in pixels, not layout units
  mPlot->setSize( rect().size() * context.renderContext().scaleFactor() );
  mPlot->render( context.renderContext() );
}

bool QgsLayoutItemElevationProfile::writePropertiesToElement( QDomElement &layoutProfileElem, QDomDocument &doc, const QgsReadWriteContext &rwContext ) const
{
  QDomElement plotElement = doc.createElement( QStringLiteral( "plot" ) );
  mPlot->writeXml( plotElement, doc, rwContext );
  layoutProfileElem.appendChild( plotElement );
  return true;
}

bool QgsLayoutItemElevationProfile::readPropertiesFromElement( const QDomElement &itemElem, const QDomDocument &, const QgsReadWriteContext &context )
{
  const QDomElement plotElement = itemElem.firstChildElement( QStringLiteral( "plot" ) );
  if ( !plotElement.isNull() )
  {
    mPlot->readXml( plotElement, context );
  }
  return true;
}

