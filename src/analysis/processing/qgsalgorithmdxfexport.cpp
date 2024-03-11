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

#include "qgsprocessingparameterdxflayers.h"
#include "qgsdxfexport.h"

///@cond PRIVATE

QString QgsDxfExportAlgorithm::name() const
{
  return QStringLiteral( "dxfexport" );
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
  return QStringLiteral( "vectorgeneral" );
}

QString QgsDxfExportAlgorithm::shortHelpString() const
{
  return QObject::tr( "Exports layers to DXF file. For each layer, you can choose a field whose values are used to split features in generated destination layers in the DXF output." );
}

QgsDxfExportAlgorithm *QgsDxfExportAlgorithm::createInstance() const
{
  return new QgsDxfExportAlgorithm();
}

void QgsDxfExportAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterDxfLayers( QStringLiteral( "LAYERS" ), QObject::tr( "Input layers" ) ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "SYMBOLOGY_MODE" ), QObject::tr( "Symbology mode" ), QStringList() << QObject::tr( "No Symbology" ) << QObject::tr( "Feature Symbology" ) << QObject::tr( "Symbol Layer Symbology" ), false, 0 ) );
  addParameter( new QgsProcessingParameterScale( QStringLiteral( "SYMBOLOGY_SCALE" ), QObject::tr( "Symbology scale" ), 1000000 ) );
  std::unique_ptr<QgsProcessingParameterMapTheme> mapThemeParam = std::make_unique<QgsProcessingParameterMapTheme>( QStringLiteral( "MAP_THEME" ), QObject::tr( "Map theme" ), QVariant(), true );
  mapThemeParam->setHelp( QObject::tr( "Match layer styling to the provided map theme" ) );
  addParameter( mapThemeParam.release() );
  const QStringList encodings = QgsDxfExport::encodings();
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "ENCODING" ), QObject::tr( "Encoding" ), encodings, false, encodings.at( 0 ), false, true ) );
  addParameter( new QgsProcessingParameterCrs( QStringLiteral( "CRS" ), QObject::tr( "CRS" ), QStringLiteral( "EPSG:4326" ) ) );
  std::unique_ptr<QgsProcessingParameterExtent> extentParam = std::make_unique<QgsProcessingParameterExtent>( QStringLiteral( "EXTENT" ), QObject::tr( "Extent" ), QVariant(), true );
  extentParam->setHelp( QObject::tr( "Limit exported features to those with geometries intersecting the provided extent" ) );
  addParameter( extentParam.release() );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "SELECTED_FEATURES_ONLY" ), QObject::tr( "Use only selected features" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "USE_LAYER_TITLE" ), QObject::tr( "Use layer title as name" ), false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "FORCE_2D" ), QObject::tr( "Force 2D output" ),  false ) );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "MTEXT" ), QObject::tr( "Export labels as MTEXT elements" ),  true ) );
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "DXF" ), QObject::tr( "DXF Files" ) + " (*.dxf *.DXF)" ) );
}

bool QgsDxfExportAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  // Retrieve and clone layers
  const QString mapTheme = parameterAsString( parameters, QStringLiteral( "MAP_THEME" ), context );
  if ( !mapTheme.isEmpty() && context.project()->mapThemeCollection()->hasMapTheme( mapTheme ) )
  {
    mMapThemeStyleOverrides = context.project()->mapThemeCollection( )->mapThemeStyleOverrides( mapTheme );
  }
  return true;
}

QVariantMap QgsDxfExportAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QgsMapSettings mapSettings;
  mapSettings.setTransformContext( context.transformContext() );
  mapSettings.setLayerStyleOverrides( mMapThemeStyleOverrides );

  QList<QgsVectorLayer *> mapLayers;

  const QVariant layersVariant = parameters.value( parameterDefinition( QStringLiteral( "LAYERS" ) )->name() );
  const QList<QgsDxfExport::DxfLayer> layers = QgsProcessingParameterDxfLayers::parameterAsLayers( layersVariant, context );
  for ( const QgsDxfExport::DxfLayer &layer : layers )
  {
    if ( !layer.layer() )
      throw QgsProcessingException( QObject::tr( "Unknown input layer" ) );

    mapLayers.push_back( layer.layer() );
  }

  const Qgis::FeatureSymbologyExport symbologyMode = static_cast< Qgis::FeatureSymbologyExport >( parameterAsInt( parameters, QStringLiteral( "SYMBOLOGY_MODE" ), context ) );
  const double symbologyScale = parameterAsDouble( parameters, QStringLiteral( "SYMBOLOGY_SCALE" ), context );
  const QString encoding = parameterAsEnumString( parameters, QStringLiteral( "ENCODING" ), context );
  const QgsCoordinateReferenceSystem crs = parameterAsCrs( parameters, QStringLiteral( "CRS" ), context );
  const bool selectedFeaturesOnly = parameterAsBool( parameters, QStringLiteral( "SELECTED_FEATURES_ONLY" ), context );
  const bool useLayerTitle = parameterAsBool( parameters, QStringLiteral( "USE_LAYER_TITLE" ), context );
  const bool useMText = parameterAsBool( parameters, QStringLiteral( "MTEXT" ), context );
  const bool force2D = parameterAsBool( parameters, QStringLiteral( "FORCE_2D" ), context );

  QgsRectangle extent;
  if ( parameters.value( QStringLiteral( "EXTENT" ) ).isValid() )
  {
    extent = parameterAsExtent( parameters, QStringLiteral( "EXTENT" ), context, crs );
  }

  const QString outputFile = parameterAsFileOutput( parameters, QStringLiteral( "OUTPUT" ), context );

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
  outputs.insert( QStringLiteral( "OUTPUT" ), outputFile );
  return outputs;
}

///@endcond
