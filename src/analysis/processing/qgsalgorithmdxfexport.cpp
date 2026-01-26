/***************************************************************************
  qgsalgorithmdxfexport.cpp
  ---------------------
  Date                 : September 2020
  Copyright            : (C) 2020 by Alexander Bruy
  Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmdxfexport.h"

#include "qgsdxfexport.h"
#include "qgsprocessingparameterdxflayers.h"

///@cond PRIVATE

QString QgsDxfExportAlgorithm::name() const
{
  return u"dxfexport"_s;
}

QString QgsDxfExportAlgorithm::displayName() const
{
  return QObject::tr( "Export layers to DXF" );
}

QStringList QgsDxfExportAlgorithm::tags() const
{
  return QObject::tr( "layer,export,dxf,cad,dwg" ).split( ',' );
}

QString QgsDxfExportAlgorithm::group() const
{
  return QObject::tr( "Vector general" );
}

QString QgsDxfExportAlgorithm::groupId() const
{
  return u"vectorgeneral"_s;
}

QString QgsDxfExportAlgorithm::shortHelpString() const
{
  return QObject::tr( "Exports layers to a DXF file. For each layer, you can choose a field whose values are used to split features in generated destination layers in the DXF output.\n\n"
                      "If no field is chosen, you can still override the output layer name by directly entering a new output layer name in the Configure Layer panel or by preferring layer title (set in layer properties) to layer name." );
}

QString QgsDxfExportAlgorithm::shortDescription() const
{
  return QObject::tr( "Exports layers to a DXF file." );
}

QgsDxfExportAlgorithm *QgsDxfExportAlgorithm::createInstance() const
{
  return new QgsDxfExportAlgorithm();
}

void QgsDxfExportAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterDxfLayers( u"LAYERS"_s, QObject::tr( "Input layers" ) ) );
  addParameter( new QgsProcessingParameterEnum( u"SYMBOLOGY_MODE"_s, QObject::tr( "Symbology mode" ), QStringList() << QObject::tr( "No Symbology" ) << QObject::tr( "Feature Symbology" ) << QObject::tr( "Symbol Layer Symbology" ), false, 0 ) );
  addParameter( new QgsProcessingParameterScale( u"SYMBOLOGY_SCALE"_s, QObject::tr( "Symbology scale" ), 1000000 ) );
  auto mapThemeParam = std::make_unique<QgsProcessingParameterMapTheme>( u"MAP_THEME"_s, QObject::tr( "Map theme" ), QVariant(), true );
  mapThemeParam->setHelp( QObject::tr( "Match layer styling to the provided map theme" ) );
  addParameter( mapThemeParam.release() );
  const QStringList encodings = QgsDxfExport::encodings();
  addParameter( new QgsProcessingParameterEnum( u"ENCODING"_s, QObject::tr( "Encoding" ), encodings, false, encodings.at( 0 ), false, true ) );
  addParameter( new QgsProcessingParameterCrs( u"CRS"_s, QObject::tr( "CRS" ), u"EPSG:4326"_s ) );
  auto extentParam = std::make_unique<QgsProcessingParameterExtent>( u"EXTENT"_s, QObject::tr( "Extent" ), QVariant(), true );
  extentParam->setHelp( QObject::tr( "Limit exported features to those with geometries intersecting the provided extent" ) );
  addParameter( extentParam.release() );
  addParameter( new QgsProcessingParameterBoolean( u"SELECTED_FEATURES_ONLY"_s, QObject::tr( "Use only selected features" ), false ) );
  auto useTitleParam = std::make_unique<QgsProcessingParameterBoolean>( u"USE_LAYER_TITLE"_s, QObject::tr( "Use layer title as name" ), false );
  useTitleParam->setHelp( QObject::tr( "If no attribute is chosen and layer name is not being overridden, prefer layer title (set in layer properties) to layer name" ) );
  addParameter( useTitleParam.release() );
  addParameter( new QgsProcessingParameterBoolean( u"FORCE_2D"_s, QObject::tr( "Force 2D output" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( u"MTEXT"_s, QObject::tr( "Export labels as MTEXT elements" ), true ) );
  addParameter( new QgsProcessingParameterBoolean( u"EXPORT_LINES_WITH_ZERO_WIDTH"_s, QObject::tr( "Export lines with zero width" ) ), false );
  addParameter( new QgsProcessingParameterFileDestination( u"OUTPUT"_s, QObject::tr( "DXF" ), QObject::tr( "DXF Files" ) + " (*.dxf *.DXF)" ) );
}

bool QgsDxfExportAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  // Retrieve and clone layers
  const QString mapTheme = parameterAsString( parameters, u"MAP_THEME"_s, context );
  if ( !mapTheme.isEmpty() && context.project()->mapThemeCollection()->hasMapTheme( mapTheme ) )
  {
    mMapThemeStyleOverrides = context.project()->mapThemeCollection()->mapThemeStyleOverrides( mapTheme );
  }
  mScaleMethod = context.project()->scaleMethod();
  return true;
}

QVariantMap QgsDxfExportAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsMapSettings mapSettings;
  mapSettings.setTransformContext( context.transformContext() );
  mapSettings.setLayerStyleOverrides( mMapThemeStyleOverrides );
  mapSettings.setScaleMethod( mScaleMethod );

  QList<QgsVectorLayer *> mapLayers;

  const QVariant layersVariant = parameters.value( parameterDefinition( u"LAYERS"_s )->name() );
  const QList<QgsDxfExport::DxfLayer> layers = QgsProcessingParameterDxfLayers::parameterAsLayers( layersVariant, context );
  for ( const QgsDxfExport::DxfLayer &layer : layers )
  {
    if ( !layer.layer() )
      throw QgsProcessingException( QObject::tr( "Unknown input layer" ) );

    mapLayers.push_back( layer.layer() );
  }

  const Qgis::FeatureSymbologyExport symbologyMode = static_cast<Qgis::FeatureSymbologyExport>( parameterAsInt( parameters, u"SYMBOLOGY_MODE"_s, context ) );
  const double symbologyScale = parameterAsDouble( parameters, u"SYMBOLOGY_SCALE"_s, context );
  const QString encoding = parameterAsEnumString( parameters, u"ENCODING"_s, context );
  const QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, u"CRS"_s, context );
  const bool selectedFeaturesOnly = parameterAsBool( parameters, u"SELECTED_FEATURES_ONLY"_s, context );
  const bool useLayerTitle = parameterAsBool( parameters, u"USE_LAYER_TITLE"_s, context );
  const bool useMText = parameterAsBool( parameters, u"MTEXT"_s, context );
  const bool force2D = parameterAsBool( parameters, u"FORCE_2D"_s, context );
  const bool exportLinesWithZeroWidth = parameterAsBool( parameters, u"EXPORT_LINES_WITH_ZERO_WIDTH"_s, context );

  QgsRectangle extent;
  if ( parameters.value( u"EXTENT"_s ).isValid() )
  {
    extent = parameterAsExtent( parameters, u"EXTENT"_s, context, crs );
  }

  const QString outputFile = parameterAsFileOutput( parameters, u"OUTPUT"_s, context );

  QgsDxfExport dxfExport;

  dxfExport.setMapSettings( mapSettings );
  dxfExport.addLayers( layers );
  dxfExport.setSymbologyScale( symbologyScale );
  dxfExport.setSymbologyExport( symbologyMode );
  dxfExport.setLayerTitleAsName( useLayerTitle );
  dxfExport.setDestinationCrs( crs );
  dxfExport.setForce2d( force2D );

  if ( !extent.isEmpty() )
  {
    dxfExport.setExtent( extent );
  }

  QgsDxfExport::Flags flags = QgsDxfExport::Flags();
  if ( !useMText )
    flags = flags | QgsDxfExport::FlagNoMText;
  if ( selectedFeaturesOnly )
    flags = flags | QgsDxfExport::FlagOnlySelectedFeatures;
  if ( exportLinesWithZeroWidth )
    flags = flags | QgsDxfExport::FlagHairlineWidthExport;
  dxfExport.setFlags( flags );

  QFile dxfFile( outputFile );
  switch ( dxfExport.writeToFile( &dxfFile, encoding ) )
  {
    case QgsDxfExport::ExportResult::Success:
      if ( !dxfExport.feedbackMessage().isEmpty() )
      {
        feedback->pushInfo( dxfExport.feedbackMessage() );
      }
      feedback->pushInfo( QObject::tr( "DXF export completed" ) );
      break;

    case QgsDxfExport::ExportResult::DeviceNotWritableError:
      throw QgsProcessingException( QObject::tr( "DXF export failed, device is not writable" ) );
      break;

    case QgsDxfExport::ExportResult::InvalidDeviceError:
      throw QgsProcessingException( QObject::tr( "DXF export failed, the device is invalid" ) );
      break;

    case QgsDxfExport::ExportResult::EmptyExtentError:
      throw QgsProcessingException( QObject::tr( "DXF export failed, the extent could not be determined" ) );
      break;
  }

  QVariantMap outputs;
  outputs.insert( u"OUTPUT"_s, outputFile );
  return outputs;
}

///@endcond
