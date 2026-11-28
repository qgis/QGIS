/***************************************************************************
                         qgsalgorithmapplylayerstyles.cpp
                         ---------------------
    begin                : November 2026
    copyright            : (C) 2026 by Nyall Dawson
    email                : nyall.dawson@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmapplylayerstyles.h"

#include "qgsfilefiltergenerator.h"
#include "qgsmaplayerstylemanager.h"

#include <QString>

using namespace Qt::StringLiterals;

///@cond PRIVATE

QString QgsApplyLayerStylesAlgorithm::name() const
{
  return u"setlayerstyles"_s;
}

QString QgsApplyLayerStylesAlgorithm::displayName() const
{
  return QObject::tr( "Set multiple layer styles" );
}

QStringList QgsApplyLayerStylesAlgorithm::tags() const
{
  return QObject::tr( "change,layer,style,qml" ).split( ',' );
}

QString QgsApplyLayerStylesAlgorithm::group() const
{
  return QObject::tr( "Cartography" );
}

QString QgsApplyLayerStylesAlgorithm::groupId() const
{
  return u"cartography"_s;
}

QString QgsApplyLayerStylesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm applies multiple styles to a layer. The styles must be defined as QML files." )
         + u"\n\n"
         + QObject::tr( "A style preset will be created for the layer corresponding to each of the selected QML files." )
         + u"\n\n"
         + QObject::tr(
           "Optionally, all existing styles can be removed prior to loading the new styles. If this option is not selected then the new styles will be added to all the existing layer styles."
         );
}

QString QgsApplyLayerStylesAlgorithm::shortDescription() const
{
  return QObject::tr( "Applies multiple styles from QML files to a layer." );
}

QgsApplyLayerStylesAlgorithm *QgsApplyLayerStylesAlgorithm::createInstance() const
{
  return new QgsApplyLayerStylesAlgorithm();
}

void QgsApplyLayerStylesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMapLayer( u"INPUT"_s, QObject::tr( "Layer" ) ) );
  auto stylesParam = new QgsProcessingParameterMultipleLayers( u"STYLES"_s, QObject::tr( "Style files" ), Qgis::ProcessingSourceType::File, QVariant(), true );
  stylesParam->setFileFilter( QgsFileFilterGenerator::fileFilter( QgsFileFilterGenerator::StandardFileFormat::QML ) );
  addParameter( stylesParam );

  auto removeExisting = new QgsProcessingParameterBoolean( u"REMOVE_EXISTING"_s, QObject::tr( "Remove existing styles" ), false );
  removeExisting->setHelp( QObject::tr( "If checked, any existing style presets for the layer will be removed." ) );
  addParameter( removeExisting );

  addOutput( new QgsProcessingOutputMapLayer( u"OUTPUT"_s, QObject::tr( "Styled" ) ) );
}

bool QgsApplyLayerStylesAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  QGS_MARK_ALGORITHM_SOURCE

  QgsMapLayer *layer = parameterAsLayer( parameters, u"INPUT"_s, context );
  const QStringList styles = parameterAsFileList( parameters, u"STYLES"_s, context );
  QStringList validPaths;
  for ( const QString &qmlPath : styles )
  {
    if ( qmlPath.isEmpty() )
      continue;

    if ( !QFile::exists( qmlPath ) )
    {
      throw QgsProcessingException( QObject::tr( "The file %1 does not exist" ).arg( QDir::toNativeSeparators( qmlPath ) ) );
    }
    validPaths << qmlPath;
  }

  const bool removeExisting = parameterAsBool( parameters, u"REMOVE_EXISTING"_s, context );

  if ( !layer )
    throw QgsProcessingException( QObject::tr( "Invalid input layer" ) );

  mLayerId = layer->id();

  QgsMapLayerStyleManager *styleManager = layer->styleManager();

  const QString DUMMY_STYLE_NAME = u"!!!!!!!REMAINING_STYLE_TO_REMOVE!!!!!!!"_s;
  QString finalRemainingStyleName;
  if ( removeExisting )
  {
    const QStringList existingStyles = styleManager->styles();
    for ( const QString &style : existingStyles )
    {
      if ( feedback->isCanceled() )
        return false;

      if ( styleManager->removeStyle( style ) )
      {
        feedback->pushInfo( QObject::tr( "Removed existing style '%1'" ).arg( style ) );
      }
    }
    // last style cannot be removed, so we defer this until after we've loaded the new styles
    if ( !validPaths.empty() && !styleManager->styles().isEmpty() )
    {
      finalRemainingStyleName = styleManager->styles().at( 0 );
      feedback->pushInfo( QObject::tr( "Removed existing style '%1'" ).arg( finalRemainingStyleName ) );
      styleManager->renameStyle( finalRemainingStyleName, DUMMY_STYLE_NAME );
    }
    if ( validPaths.empty() )
    {
      // removed all existing styles (except the last), nothing to load => all done
      layer->triggerRepaint();
      return true;
    }
  }

  QSet< QString > styleNames = qgis::listToSet( styleManager->styles() );
  QString firstStyleName;
  const double step = !validPaths.empty() ? 100.0 / static_cast< double >( validPaths.size() ) : 1;
  int index = 0;
  for ( const QString &qmlPath : validPaths )
  {
    if ( feedback->isCanceled() )
    {
      if ( removeExisting )
      {
        styleManager->renameStyle( DUMMY_STYLE_NAME, finalRemainingStyleName );
      }
      return false;
    }

    const QFileInfo qmlFileInfo( qmlPath );
    const QString baseName = qmlFileInfo.completeBaseName();

    QString styleName = baseName;
    int counter = 1;
    while ( styleNames.contains( styleName ) )
    {
      counter++;
      styleName = u"%1 (%2)"_s.arg( baseName ).arg( counter );
    }

    feedback->pushInfo( QObject::tr( "Loading %1 as layer style '%2'" ).arg( QDir::toNativeSeparators( qmlPath ), styleName ) );
    if ( !styleManager->addStyleFromLayer( styleName ) )
    {
      if ( removeExisting )
      {
        styleManager->renameStyle( DUMMY_STYLE_NAME, finalRemainingStyleName );
      }
      throw QgsProcessingException( QObject::tr( "Could not create style %1" ).arg( styleName ) );
    }
    styleNames.insert( styleName );

    styleManager->setCurrentStyle( styleName );
    bool ok = false;
    const QString msg = layer->loadNamedStyle( qmlPath, ok );
    if ( !ok )
    {
      if ( removeExisting )
      {
        styleManager->renameStyle( DUMMY_STYLE_NAME, finalRemainingStyleName );
      }
      throw QgsProcessingException( QObject::tr( "Failed to load style %1. Error: %2" ).arg( QDir::toNativeSeparators( qmlPath ), msg ) );
    }

    if ( firstStyleName.isEmpty() )
    {
      firstStyleName = styleName;
    }
    index++;
    feedback->setProgress( index * step );
  }

  if ( removeExisting )
  {
    styleManager->removeStyle( DUMMY_STYLE_NAME );
    styleManager->setCurrentStyle( firstStyleName );
  }

  layer->triggerRepaint();

  return true;
}

QVariantMap QgsApplyLayerStylesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  Q_UNUSED( parameters );
  Q_UNUSED( context );

  QVariantMap results;
  results.insert( u"OUTPUT"_s, mLayerId );
  return results;
}

///@endcond
