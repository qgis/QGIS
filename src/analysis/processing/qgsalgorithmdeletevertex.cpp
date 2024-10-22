/***************************************************************************
                        qgsalgorithmdeletevertex.cpp
                        ---------------------
   begin                : June 2024
   copyright            : (C) 2024 by Jacky Volpes
   email                : jacky dot volpes at oslandia dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmdeletevertex.h"
#include "qgsgeometrycheckerutils.h"
#include "qgsgeometryutils.h"
#include <algorithm>

///@cond PRIVATE

auto QgsDeleteVertexAlgorithm::name() const -> QString
{
  return QStringLiteral( "deletevertex" );
}

auto QgsDeleteVertexAlgorithm::displayName() const -> QString
{
  return QObject::tr( "Delete vertices" );
}

auto QgsDeleteVertexAlgorithm::tags() const -> QStringList
{
  return QObject::tr( "delete,vertices" ).split( ',' );
}

auto QgsDeleteVertexAlgorithm::group() const -> QString
{
  return QObject::tr( "Fix geometry" );
}

auto QgsDeleteVertexAlgorithm::groupId() const -> QString
{
  return QStringLiteral( "fixgeometry" );
}

auto QgsDeleteVertexAlgorithm::shortHelpString() const -> QString
{
  return QObject::tr( "This algorithm deletes vertices based on an input layer describing the vertices.\n\n"
                      "If topological deletion is enabled (default), when deletion of a vertex results in a duplicate vertex"
                      "(when there is a spike vertex), the duplicate vertex is deleted to keep a single verted." );
}

auto QgsDeleteVertexAlgorithm::createInstance() const -> QgsDeleteVertexAlgorithm *
{
  return new QgsDeleteVertexAlgorithm();
}

void QgsDeleteVertexAlgorithm::initAlgorithm( const QVariantMap &configuration )
{
  Q_UNUSED( configuration )

  // Inputs
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ),
                QList< int >() << static_cast<int>( Qgis::ProcessingSourceType::VectorPolygon ) << static_cast<int>( Qgis::ProcessingSourceType::VectorLine ) )
              );
  addParameter( new QgsProcessingParameterFeatureSource( QStringLiteral( "ERRORS" ), QObject::tr( "Errors layer" ),
                QList< int >() << static_cast<int>( Qgis::ProcessingSourceType::VectorPoint ) )
              );
  addParameter( new QgsProcessingParameterField(
                  QStringLiteral( "FEAT_ID" ), QObject::tr( "Field of feature ID" ),
                  QStringLiteral( "gc_featid" ), QStringLiteral( "ERRORS" ),
                  Qgis::ProcessingFieldParameterDataType::Numeric, false, true )
              );
  addParameter( new QgsProcessingParameterField(
                  QStringLiteral( "PART_IDX" ), QObject::tr( "Field of part index" ),
                  QStringLiteral( "gc_partidx" ), QStringLiteral( "ERRORS" ),
                  Qgis::ProcessingFieldParameterDataType::Numeric, false, true )
              );
  addParameter( new QgsProcessingParameterField(
                  QStringLiteral( "RING_IDX" ), QObject::tr( "Field of ring index" ),
                  QStringLiteral( "gc_ringidx" ), QStringLiteral( "ERRORS" ),
                  Qgis::ProcessingFieldParameterDataType::Numeric, false, true )
              );
  addParameter( new QgsProcessingParameterField(
                  QStringLiteral( "VERTEX_IDX" ), QObject::tr( "Field of vertex index" ),
                  QStringLiteral( "gc_vertidx" ), QStringLiteral( "ERRORS" ),
                  Qgis::ProcessingFieldParameterDataType::Numeric, false, true )
              );
  addParameter( new QgsProcessingParameterBoolean( QStringLiteral( "TOPOLOGICAL" ), QObject::tr( "Topological deletion" ), true ) );

  // Outputs
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "OUTPUT" ), QObject::tr( "Output layer" ), Qgis::ProcessingSourceType::VectorAnyGeometry ) );
  addParameter( new QgsProcessingParameterFeatureSink( QStringLiteral( "REPORT" ), QObject::tr( "Report layer" ), Qgis::ProcessingSourceType::VectorPoint ) );

  std::unique_ptr< QgsProcessingParameterNumber > tolerance = std::make_unique< QgsProcessingParameterNumber >( QStringLiteral( "TOLERANCE" ),
      QObject::tr( "Tolerance" ), Qgis::ProcessingNumberParameterType::Integer, 8, false, 1, 13 );
  tolerance->setFlags( tolerance->flags() | Qgis::ProcessingParameterFlag::Advanced );
  addParameter( tolerance.release() );
}

auto QgsDeleteVertexAlgorithm::processAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback ) -> QVariantMap
{
  std::unique_ptr< QgsProcessingFeatureSource > source( parameterAsSource( parameters, QStringLiteral( "INPUT" ), context ) );
  if ( !source )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "INPUT" ) ) );

  std::unique_ptr< QgsProcessingFeatureSource > errors( parameterAsSource( parameters, QStringLiteral( "ERRORS" ), context ) );
  if ( !errors )
    throw QgsProcessingException( invalidSourceError( parameters, QStringLiteral( "ERRORS" ) ) );

  QgsProcessingMultiStepFeedback multiStepFeedback( 2, feedback );

  QString featIdFieldName = parameterAsString( parameters, QStringLiteral( "FEAT_ID" ), context );
  QString partIdxFieldName = parameterAsString( parameters, QStringLiteral( "PART_IDX" ), context );
  QString ringIdxFieldName = parameterAsString( parameters, QStringLiteral( "RING_IDX" ), context );
  QString vertexIdxFieldName = parameterAsString( parameters, QStringLiteral( "VERTEX_IDX" ), context );

  QString dest_output;
  std::unique_ptr< QgsFeatureSink > sink_output( parameterAsSink( parameters, QStringLiteral( "OUTPUT" ), context, dest_output, source->fields(), source->wkbType(), source->sourceCrs() ) );
  if ( !sink_output )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "OUTPUT" ) ) );

  QString dest_report;
  QgsFields reportFields = errors->fields();
  reportFields.append( QgsField( QStringLiteral( "report" ), QMetaType::QString ) );
  std::unique_ptr< QgsFeatureSink > sink_report( parameterAsSink( parameters, QStringLiteral( "REPORT" ), context, dest_report, reportFields, errors->wkbType(), errors->sourceCrs() ) );
  if ( !sink_report )
    throw QgsProcessingException( invalidSinkError( parameters, QStringLiteral( "REPORT" ) ) );

  // **Preparation step**
  //
  // Go through every feature in the errors layer to build the list of vertices to delete.
  //
  // We want to have the list ordered by descending feature id, descending part, descending ring, and descending vertex number.
  // So in the next step we can remove the vertices in an order such that no index will change (always from the last vertex of the geometry to the first).
  //
  // To achieve that, when adding a new vertex to the list we search for the first element of the list that has a smaller index than
  // the vertex we want to insert, ant insert it right before it.
  //
  // Our list is in fact a map named verticesMap, whose key is the line/polygon feature on which we delete the vertices,
  // and whose value is a list of feature id from the input errors layer and the corresponding vertex object build from it.

  multiStepFeedback.setCurrentStep( 1 );
  multiStepFeedback.setProgressText( QObject::tr( "Preparing deletion…" ) );
  double step{errors->featureCount() > 0 ? 100.0 / ( double )errors->featureCount() : 1};
  long i = 0;
  multiStepFeedback.setProgress( 0.0 );
  QgsFeature reportFeature;
  reportFeature.setFields( reportFields );
  QgsFeature errorFeature;
  QgsFeatureIterator featureIt = errors->getFeatures();
  QMap<QgsFeatureId, QList<QPair<QgsFeatureId, QgsVertexId>>> verticesMap;  // see explanation above
  QgsFeatureIds sourceFeatureIds = source->allFeatureIds();
  while ( featureIt.nextFeature( errorFeature ) )
  {
    const QgsFeatureId errorFeatureId = errorFeature.id();
    const QgsFeatureId featureId = errorFeature.attribute( featIdFieldName ).toInt();
    const QgsVertexId vidx = QgsVertexId( errorFeature.attribute( partIdxFieldName ).toInt(),
                                          errorFeature.attribute( ringIdxFieldName ).toInt(),
                                          errorFeature.attribute( vertexIdxFieldName ).toInt() );

    if ( !sourceFeatureIds.contains( featureId ) )
    {
      reportFeature.setAttributes( errorFeature.attributes() << QObject::tr( "Source feature not found" ) );
      reportFeature.setGeometry( errorFeature.geometry() );
      if ( !sink_report->addFeature( reportFeature, QgsFeatureSink::FastInsert ) )
        throw QgsProcessingException( writeFeatureError( sink_report.get(), parameters, QStringLiteral( "REPORT" ) ) );
      continue;
    }

    if ( !verticesMap.contains( featureId ) )  // no vertex of this feature id yet
      verticesMap[featureId] = QList<QPair<QgsFeatureId, QgsVertexId>>() << QPair<QgsFeatureId, QgsVertexId>( errorFeatureId, vidx );
    else  // there already is a vertex (or vertices) for this feature: insert the new one at the right place (see explanation above)
    {
      QList<QPair<QgsFeatureId, QgsVertexId>> &vertices = verticesMap[featureId];
      auto it = std::find_if( vertices.begin(), vertices.end(), [&vidx]( QPair<QgsFeatureId, QgsVertexId> pair )
      {
        QgsVertexId v = pair.second;
        if ( v.part == vidx.part )
        {
          if ( v.ring == vidx.ring )
            return v.vertex <= vidx.vertex;
          return v.ring < vidx.ring;
        }
        return v.part < vidx.part;
      } );
      vertices.insert( it, QPair<QgsFeatureId, QgsVertexId>( errorFeatureId, vidx ) );
    }
    multiStepFeedback.setProgress( 100.0 * step * static_cast<double>( i ) );
  }

  QgsFeature sourceFeature;
  QgsFeatureIterator sourceFeatureIt = source->getFeatures();
  step = source->featureCount() > 0 ? 100.0 / ( double )source->featureCount() : 1;
  i = 0;
  multiStepFeedback.setCurrentStep( 2 );
  multiStepFeedback.setProgressText( QObject::tr( "Deleting vertices…" ) );
  multiStepFeedback.setProgress( 0.0 );
  while ( sourceFeatureIt.nextFeature( sourceFeature ) )
  {
    QgsFeatureId sourceFeatureId = sourceFeature.id();
    QgsGeometry sourceGeometry = sourceFeature.geometry();
    QgsAbstractGeometry *geometry = sourceGeometry.get();
    if ( verticesMap.contains( sourceFeatureId ) )
    {
      QList<QPair<QgsFeatureId, QgsVertexId>> verticesList = verticesMap[sourceFeatureId];
      for ( auto errorIt = verticesList.cbegin(); errorIt != verticesList.cend(); ++errorIt )
      {
        QString reportMessage;
        QgsVertexId vidx = errorIt->second;
        QgsFeatureId errorFeatureId = errorIt->first;
        errors->getFeatures( QgsFeatureRequest( errorFeatureId ) ).nextFeature( errorFeature );

        try
        {
          if ( !vidx.isValid( geometry ) )
            throw QObject::tr( "vidx not valid" );

          // Check if error still applies
          const int n = QgsGeometryCheckerUtils::polyLineSize( geometry, vidx.part, vidx.ring );
          if ( n == 0 )
            throw QObject::tr( "No more vertex in geometry" );

          if ( !QgsGeometryCheckerUtils::canDeleteVertex( geometry, vidx.part, vidx.ring ) )
            throw QObject::tr( "Resulting geometry would be degenerate" );
          else if ( !geometry->deleteVertex( vidx ) )
            throw QObject::tr( "Failed to delete vertex" );
          else
          {
            reportMessage = QObject::tr( "Vertex removed" );

            if ( parameterAsBool( parameters, QStringLiteral( "TOPOLOGICAL" ), context ) )  // Avoid duplicate nodes as result of deleting spike vertex
            {
              const QgsPoint &p1 = geometry->vertexAt( QgsVertexId( vidx.part, vidx.ring, ( vidx.vertex - 1 + n ) % n ) );
              const QgsPoint &p3 = geometry->vertexAt( QgsVertexId( vidx.part, vidx.ring, vidx.vertex ) );
              if ( QgsGeometryUtils::sqrDistance2D( p1, p3 ) < ( std::pow( 10, -mTolerance ) * std::pow( 10, -mTolerance ) ) &&
                   QgsGeometryCheckerUtils::canDeleteVertex( geometry, vidx.part, vidx.ring ) &&
                   geometry->deleteVertex( vidx ) )
                reportMessage = QObject::tr( "Double vertex removed" );
            }
          }
        }
        catch ( QString errorMessage )
        {
          reportMessage = errorMessage;
        }
        reportFeature.setAttributes( errorFeature.attributes() << reportMessage );
        reportFeature.setGeometry( errorFeature.geometry() );
        if ( !sink_report->addFeature( reportFeature, QgsFeatureSink::FastInsert ) )
          throw QgsProcessingException( writeFeatureError( sink_report.get(), parameters, QStringLiteral( "REPORT" ) ) );
      }
    }
    sourceFeature.setGeometry( sourceGeometry );
    if ( !sink_output->addFeature( sourceFeature, QgsFeatureSink::FastInsert ) )
      throw QgsProcessingException( writeFeatureError( sink_output.get(), parameters, QStringLiteral( "OUTPUT" ) ) );
    multiStepFeedback.setProgress( 100.0 * step * static_cast<double>( i ) );
  }

  QVariantMap outputs;
  outputs.insert( QStringLiteral( "OUTPUT" ), dest_output );
  outputs.insert( QStringLiteral( "REPORT" ), dest_report );

  return outputs;
}

auto QgsDeleteVertexAlgorithm::prepareAlgorithm( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback * ) -> bool
{
  mTolerance = parameterAsInt( parameters, QStringLiteral( "TOLERANCE" ), context );

  return true;
}

///@endcond
