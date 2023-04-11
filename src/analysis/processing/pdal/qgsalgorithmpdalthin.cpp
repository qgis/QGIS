/***************************************************************************
                         qgsalgorithmpdalthin.cpp
                         ---------------------
    begin                : February 2023
    copyright            : (C) 2023 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsalgorithmpdalthin.h"

#include "qgsrunprocess.h"
#include "qgspointcloudlayer.h"

///@cond PRIVATE

QString QgsPdalThinAlgorithm::name() const
{
  return QStringLiteral( "thin" );
}

QString QgsPdalThinAlgorithm::displayName() const
{
  return QObject::tr( "Thin" );
}

QString QgsPdalThinAlgorithm::group() const
{
  return QObject::tr( "Point cloud data management" );
}

QString QgsPdalThinAlgorithm::groupId() const
{
  return QStringLiteral( "pointclouddatamanagement" );
}

QStringList QgsPdalThinAlgorithm::tags() const
{
  return QObject::tr( "thin,reduce,decrease,size" ).split( ',' );
}

QString QgsPdalThinAlgorithm::shortHelpString() const
{
  return QObject::tr( "This algorithm creates a thinned version of the point cloud by only keeping every N-th or by performing sampling by distance point." );
}

QgsPdalThinAlgorithm *QgsPdalThinAlgorithm::createInstance() const
{
  return new QgsPdalThinAlgorithm();
}

void QgsPdalThinAlgorithm::initAlgorithm( const QVariantMap & )
{
  addParameter( new QgsProcessingParameterPointCloudLayer( QStringLiteral( "INPUT" ), QObject::tr( "Input layer" ) ) );
  addParameter( new QgsProcessingParameterEnum( QStringLiteral( "MODE" ), QObject::tr( "Mode" ), QStringList() << QObject::tr( "Every N-th" ) << QObject::tr( "Sample" ), false, 0 ) );
  addParameter( new QgsProcessingParameterNumber( QStringLiteral( "STEP" ), QObject::tr( "Step" ), QgsProcessingParameterNumber::Integer, 20, false, 1 ) );
  addParameter( new QgsProcessingParameterPointCloudDestination( QStringLiteral( "OUTPUT" ), QObject::tr( "Output layer" ) ) );
}

QStringList QgsPdalThinAlgorithm::createArgumentLists( const QVariantMap &parameters, QgsProcessingContext &context, QgsProcessingFeedback *feedback )
{
  Q_UNUSED( feedback );

  QgsPointCloudLayer *layer = parameterAsPointCloudLayer( parameters, QStringLiteral( "INPUT" ), context, QgsProcessing::LayerOptionsFlag::SkipIndexGeneration );
  if ( !layer )
    throw QgsProcessingException( invalidPointCloudError( parameters, QStringLiteral( "INPUT" ) ) );

  const QString outputFile = parameterAsOutputLayer( parameters, QStringLiteral( "OUTPUT" ), context );
  setOutputValue( QStringLiteral( "OUTPUT" ), outputFile );

  int mode = parameterAsInt( parameters, QStringLiteral( "MODE" ), context );
  int step = parameterAsInt( parameters, QStringLiteral( "STEP" ), context );

  QStringList args = { QStringLiteral( "thin" ),
                       QStringLiteral( "--input=%1" ).arg( layer->source() ),
                       QStringLiteral( "--output=%1" ).arg( outputFile ),
                       QStringLiteral( "--mode=%1" ).arg( mode == 0 ? QStringLiteral( "every-nth" ) : QStringLiteral( "sample" ) ),
                       QStringLiteral( "--%1=%2" ).arg( mode == 0 ? QStringLiteral( "step-every-nth" ) : QStringLiteral( "step-sample" ) ).arg( step )
                     };

  addThreadsParameter( args );
  return args;
}

///@endcond
