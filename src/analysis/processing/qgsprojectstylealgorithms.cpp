/***************************************************************************
                         qgsprojectstylealgorithms.cpp
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

#include "qgsprojectstylealgorithms.h"
#include "qgsstyle.h"
#include "qgsstyleentityvisitor.h"
///@cond PRIVATE

class StyleVisitor : public QgsStyleEntityVisitorInterface
{
  public:

    StyleVisitor( QgsStyle *style, const QList< QgsStyle::StyleEntity > &objects )
      : mStyle( style )
      , mObjects( objects )
    {

    }

    bool visit( const QgsStyleEntityVisitorInterface::StyleLeaf &entity ) override
    {
      if ( mObjects.contains( entity.entity->type() ) )
      {
        const QString name = mParentNames.join( ' ' ) + ' ' + entity.description;
        QString candidate = name;
        int i = 1;
        bool exists = true;
        while ( exists )
        {
          exists = mStyle->allNames( entity.entity->type() ).contains( candidate );
          if ( !exists )
            break;

          i++;
          candidate = name + QStringLiteral( " (%1)" ).arg( i );
        }
        mStyle->addEntity( candidate, entity.entity, true );
      }
      return true;
    }

    bool visitEnter( const QgsStyleEntityVisitorInterface::Node &node ) override
    {
      switch ( node.type )
      {
        case QgsStyleEntityVisitorInterface::NodeType::Project:
        case QgsStyleEntityVisitorInterface::NodeType::Layouts:
        case QgsStyleEntityVisitorInterface::NodeType::LayoutItem:
        case QgsStyleEntityVisitorInterface::NodeType::ReportHeader:
        case QgsStyleEntityVisitorInterface::NodeType::ReportFooter:
        case QgsStyleEntityVisitorInterface::NodeType::ReportSection:
        case QgsStyleEntityVisitorInterface::NodeType::Annotations:
          break;

        case QgsStyleEntityVisitorInterface::NodeType::Layer:
        case QgsStyleEntityVisitorInterface::NodeType::PrintLayout:
        case QgsStyleEntityVisitorInterface::NodeType::Report:
        case QgsStyleEntityVisitorInterface::NodeType::Annotation:
        case QgsStyleEntityVisitorInterface::NodeType::SymbolRule:
          mParentNames << node.description;
          break;
      }
      return true;
    }

    bool visitExit( const QgsStyleEntityVisitorInterface::Node &node ) override
    {
      switch ( node.type )
      {
        case QgsStyleEntityVisitorInterface::NodeType::Project:
        case QgsStyleEntityVisitorInterface::NodeType::Layouts:
        case QgsStyleEntityVisitorInterface::NodeType::LayoutItem:
        case QgsStyleEntityVisitorInterface::NodeType::ReportHeader:
        case QgsStyleEntityVisitorInterface::NodeType::ReportFooter:
        case QgsStyleEntityVisitorInterface::NodeType::ReportSection:
        case QgsStyleEntityVisitorInterface::NodeType::Annotations:
          break;

        case QgsStyleEntityVisitorInterface::NodeType::Layer:
        case QgsStyleEntityVisitorInterface::NodeType::PrintLayout:
        case QgsStyleEntityVisitorInterface::NodeType::Report:
        case QgsStyleEntityVisitorInterface::NodeType::Annotation:
        case QgsStyleEntityVisitorInterface::NodeType::SymbolRule:
          mParentNames.pop_back();
          break;
      }
      return true;
    }

  private:

    QgsStyle *mStyle = nullptr;
    QList< QgsStyle::StyleEntity > mObjects;
    QStringList mParentNames;

};

QgsStyleFromProjectAlgorithm::QgsStyleFromProjectAlgorithm() = default;

QgsStyleFromProjectAlgorithm::~QgsStyleFromProjectAlgorithm() = default;

void QgsStyleFromProjectAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterFileDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output style database" ),
                QObject::tr( "Style files (*.xml)" ) ) );

  QStringList options = QStringList()
                        << QObject::tr( "Symbols" )
                        << QObject::tr( "Color ramps" )
                        << QObject::tr( "Text formats" )
                        << QObject::tr( "Label settings" );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "OBJECTS" ), QObject::tr( "Objects to extract" ), options, true, QVariantList() << 0 << 1 << 2 << 3 ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "SYMBOLS" ), QObject::tr( "Symbol count" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "COLORRAMPS" ), QObject::tr( "Color ramp count" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "TEXTFORMATS" ), QObject::tr( "Text format count" ) ) );
  addOutput( new QgsProcessingOutputNumber( QStringLiteral( "LABELSETTINGS" ), QObject::tr( "Label settings count" ) ) );
}

QString QgsStyleFromProjectAlgorithm::name() const
{
  return QStringLiteral( "stylefromproject" );
}

QString QgsStyleFromProjectAlgorithm::displayName() const
{
  return QObject::tr( "Create style database from project" );
}

QStringList QgsStyleFromProjectAlgorithm::tags() const
{
  return QObject::tr( "symbols,color,ramps,colors,formats,labels,text,fonts" ).split( ',' );
}

QString QgsStyleFromProjectAlgorithm::group() const
{
  return QObject::tr( "Cartography" );
}

QString QgsStyleFromProjectAlgorithm::groupId() const
{
  return QStringLiteral( "cartography" );
}

QString QgsStyleFromProjectAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm extracts all style objects (including symbols, color ramps, text formats and label settings) from a QGIS project.\n\n"
                      "The extract symbols are saved to a QGIS style database (XML format), which can be managed and imported via the Style Manager dialog." );
}

QString QgsStyleFromProjectAlgorithm::shortDescription() const
{
  return QObject::tr( "Creates a style database by extracting all symbols, color ramps, text formats and label settings from a QGIS project." );
}

QgsStyleFromProjectAlgorithm *QgsStyleFromProjectAlgorithm::createInstance() const
{
  return new QgsStyleFromProjectAlgorithm();
}

bool QgsStyleFromProjectAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  if ( !context.project() )
    return false;

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

  // not thread safe, so prepare in the main thread
  mStyle = qgis::make_unique< QgsStyle >();
  mStyle->createMemoryDatabase();

  StyleVisitor visitor( mStyle.get(), objects );
  context.project()->accept( &visitor );
  return true;
}

QVariantMap QgsStyleFromProjectAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * )
{
  const QString file = parameterAsString( parameters, QStringLiteral( "OUTPUT" ), context );
  if ( !mStyle->exportXml( file ) )
  {
    throw QgsProcessingException( QObject::tr( "Error saving style database as %1" ).arg( file ) );
  }

  QVariantMap results;
  results.insert( QStringLiteral( "OUTPUT" ), file );
  results.insert( QStringLiteral( "SYMBOLS" ), mStyle->symbolCount() );
  results.insert( QStringLiteral( "COLORRAMPS" ), mStyle->colorRampCount() );
  results.insert( QStringLiteral( "TEXTFORMATS" ), mStyle->textFormatCount() );
  results.insert( QStringLiteral( "LABELSETTINGS" ), mStyle->labelSettingsCount() );
  return results;
}

///@endcond



