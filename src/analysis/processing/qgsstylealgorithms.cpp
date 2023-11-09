/***************************************************************************
                         qgsstylealgorithms.cpp
                         ---------------------
    begin                : July 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgsstylealgorithms.h"
#include "qgsstyle.h"

///@cond PRIVATE

QgsCombineStylesAlgorithm::QgsCombineStylesAlgorithm() = default;

QgsCombineStylesAlgorithm::~QgsCombineStylesAlgorithm() = default;

void QgsCombineStylesAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterMultipleLayers( QStringLiteral( "INPUT" ), QObject::tr( "Input databases" ), QgsProcessing::TypeFile ) );

  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output style database" ),
                QObject::tr( "Style files (*.xml)" ) ) );

  const QStringList options = QStringList()
                              << QObject::tr( "Symbols" )
                              << QObject::tr( "Color ramps" )
                              << QObject::tr( "Text formats" )
                              << QObject::tr( "Label settings" );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "OBJECTS" ), QObject::tr( "Objects to combine" ), options, true, QVariantList() << 0 << 1 << 2 << 3 ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "SYMBOLS" ), QObject::tr( "Symbol count" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "COLORRAMPS" ), QObject::tr( "Color ramp count" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "TEXTFORMATS" ), QObject::tr( "Text format count" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "LABELSETTINGS" ), QObject::tr( "Label settings count" ) ) );
}

QString QgsCombineStylesAlgorithm::name() const
{
  return QStringLiteral( "combinestyles" );
}

QString QgsCombineStylesAlgorithm::displayName() const
{
  return QObject::tr( "Combine style databases" );
}

QStringList QgsCombineStylesAlgorithm::tags() const
{
  return QObject::tr( "symbols,colors,ramps,formats,labels,text,fonts,merge" ).split( ',' );
}

QString QgsCombineStylesAlgorithm::group() const
{
  return QObject::tr( "Cartography" );
}

QString QgsCombineStylesAlgorithm::groupId() const
{
  return QStringLiteral( "cartography" );
}

QString QgsCombineStylesAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm combines multiple QGIS style databases into a single style database. If any symbols exist with duplicate names between the different "
                      "source databases these will be renamed to have unique names in the output combined database." );
}

QString QgsCombineStylesAlgorithm::shortDescription() const
{
  return QObject::tr( "Combines multiple style databases into a single database." );
}

QgsCombineStylesAlgorithm *QgsCombineStylesAlgorithm::createInstance() const
{
  return new QgsCombineStylesAlgorithm();
}

QVariantMap QgsCombineStylesAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  const QStringList inputs = parameterAsFileList( parameters, QStringLiteral( "INPUT" ), context );

  QList< QgsStyle::StyleEntity > objects;
  const QList< int > selectedObjects = parameterAsEnums( parameters, QStringLiteral( "OBJECTS" ), context );
  if ( selectedObjects.contains( 0 ) )
    objects << QgsStyle::SymbolEntity;
  if ( selectedObjects.contains( 1 ) )
    objects << QgsStyle::ColorrampEntity;
  if ( selectedObjects.contains( 2 ) )
    objects << QgsStyle::TextFormatEntity;
  if ( selectedObjects.contains( 3 ) )
    objects << QgsStyle::LabelSettingsEntity;

  QgsStyle style;
  style.createMemoryDatabase();

  int i = 0;
  QMap< QgsStyle::StyleEntity, QSet< QString> > usedNames;
  auto makeUniqueName = [&usedNames]( const QString & sourceName, QgsStyle::StyleEntity type )->QString
  {
    QString candidate = sourceName;
    int i = 1;
    bool exists = true;
    while ( exists )
    {
      exists = usedNames[ type ].contains( candidate );
      if ( !exists )
        break;

      i++;
      candidate = sourceName + QStringLiteral( " (%1)" ).arg( i );
    }

    usedNames[ type ].insert( candidate );
    return candidate;
  };

  for ( const QString &source : inputs )
  {
    if ( feedback )
    {
      feedback->setProgress( 100 * i / static_cast< double >( inputs.count() ) );
      feedback->pushInfo( QObject::tr( "Importing %1" ).arg( source ) );
    }

    QgsStyle sourceStyle;
    sourceStyle.createMemoryDatabase();
    if ( !sourceStyle.importXml( source ) )
    {
      feedback->reportError( QObject::tr( "Could not read %1" ).arg( source ) );
      i++;
      continue;
    }

    if ( objects.contains( QgsStyle::SymbolEntity ) )
    {
      const QStringList symbolNames = sourceStyle.symbolNames();
      for ( const QString &name : symbolNames )
      {
        const QString newName = makeUniqueName( name, QgsStyle::SymbolEntity );
        style.addSymbol( newName, sourceStyle.symbol( name ), true );
        style.tagSymbol( QgsStyle::SymbolEntity, newName, sourceStyle.tagsOfSymbol( QgsStyle::SymbolEntity, name ) );
      }
    }
    if ( objects.contains( QgsStyle::ColorrampEntity ) )
    {
      const QStringList colorRampNames = sourceStyle.colorRampNames();
      for ( const QString &name : colorRampNames )
      {
        const QString newName = makeUniqueName( name, QgsStyle::ColorrampEntity );
        style.addColorRamp( newName, sourceStyle.colorRamp( name ), true );
        style.tagSymbol( QgsStyle::ColorrampEntity, newName, sourceStyle.tagsOfSymbol( QgsStyle::ColorrampEntity, name ) );
      }
    }
    if ( objects.contains( QgsStyle::TextFormatEntity ) )
    {
      const QStringList formatNames = sourceStyle.textFormatNames();
      for ( const QString &name : formatNames )
      {
        const QString newName = makeUniqueName( name, QgsStyle::TextFormatEntity );
        style.addTextFormat( newName, sourceStyle.textFormat( name ), true );
        style.tagSymbol( QgsStyle::TextFormatEntity, newName, sourceStyle.tagsOfSymbol( QgsStyle::TextFormatEntity, name ) );
      }
    }
    if ( objects.contains( QgsStyle::LabelSettingsEntity ) )
    {
      const QStringList formatNames = sourceStyle.labelSettingsNames();
      for ( const QString &name : formatNames )
      {
        const QString newName = makeUniqueName( name, QgsStyle::LabelSettingsEntity );
        style.addLabelSettings( newName, sourceStyle.labelSettings( name ), true );
        style.tagSymbol( QgsStyle::LabelSettingsEntity, newName, sourceStyle.tagsOfSymbol( QgsStyle::LabelSettingsEntity, name ) );
      }
    }

    i++;
  }
  if ( feedback )
  {
    feedback->setProgress( 100 );
    feedback->pushInfo( QObject::tr( "Writing output file" ) );
  }

  const QString file = parameterAsString( parameters, QStringLiteral( "OUTPUT" ), context );
  if ( !style.exportXml( file ) )
  {
    throw QgsProcessingException( QObject::tr( "Error saving style database as %1" ).arg( file ) );
  }

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), file );
  results.insert( QStringLiteral( "SYMBOLS" ), style.symbolCount() );
  results.insert( QStringLiteral( "COLORRAMPS" ), style.colorRampCount() );
  results.insert( QStringLiteral( "TEXTFORMATS" ), style.textFormatCount() );
  results.insert( QStringLiteral( "LABELSETTINGS" ), style.labelSettingsCount() );
  return results;
}

///@endcond




