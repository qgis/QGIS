/***************************************************************************
                         qgsprocessingmodelchildalgorithm.cpp
                         ------------------------------------
    begin                : June 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgsprocessingmodelchildalgorithm.h"
#include "qgsapplication.h"
#include "qgsprocessingregistry.h"
#include "qgsprocessingmodelalgorithm.h"

///@cond NOT_STABLE

QgsProcessingModelChildAlgorithm::QgsProcessingModelChildAlgorithm( const QString &algorithmId )
{
  setAlgorithmId( algorithmId );
}

QgsProcessingModelChildAlgorithm::QgsProcessingModelChildAlgorithm( const QgsProcessingModelChildAlgorithm &other )
  : QgsProcessingModelComponent( other )
  , mId( other.mId )
  , mConfiguration( other.mConfiguration )
  , mParams( other.mParams )
  , mModelOutputs( other.mModelOutputs )
  , mActive( other.mActive )
  , mDependencies( other.mDependencies )
  , mParametersCollapsed( other.mParametersCollapsed )
  , mOutputsCollapsed( other.mOutputsCollapsed )
{
  setAlgorithmId( other.algorithmId() );
}

QgsProcessingModelChildAlgorithm &QgsProcessingModelChildAlgorithm::operator=( const QgsProcessingModelChildAlgorithm &other )
{
  QgsProcessingModelComponent::operator =( other );
  mId = other.mId;
  mConfiguration = other.mConfiguration;
  setAlgorithmId( other.algorithmId() );
  mParams = other.mParams;
  mModelOutputs = other.mModelOutputs;
  mActive = other.mActive;
  mDependencies = other.mDependencies;
  mParametersCollapsed = other.mParametersCollapsed;
  mOutputsCollapsed = other.mOutputsCollapsed;
  return *this;
}

const QgsProcessingAlgorithm *QgsProcessingModelChildAlgorithm::algorithm() const
{
  return mAlgorithm.get();
}

void QgsProcessingModelChildAlgorithm::setModelOutputs( const QMap<QString, QgsProcessingModelOutput> &modelOutputs )
{
  mModelOutputs = modelOutputs;

  QMap<QString, QgsProcessingModelOutput>::iterator outputIt = mModelOutputs.begin();
  for ( ; outputIt != mModelOutputs.end(); ++outputIt )
  {
    // make sure values are consistent
    outputIt->setName( outputIt.key() );
    outputIt->setChildId( mId );
  }
}

bool QgsProcessingModelChildAlgorithm::removeModelOutput( const QString &name )
{
  mModelOutputs.remove( name );
  return true;
}

QVariant QgsProcessingModelChildAlgorithm::toVariant() const
{
  QVariantMap map;
  map.insert( QStringLiteral( "id" ), mId );
  map.insert( QStringLiteral( "alg_id" ), mAlgorithmId );
  map.insert( QStringLiteral( "alg_config" ), mConfiguration );
  map.insert( QStringLiteral( "active" ), mActive );
  map.insert( QStringLiteral( "dependencies" ), mDependencies );
  map.insert( QStringLiteral( "parameters_collapsed" ), mParametersCollapsed );
  map.insert( QStringLiteral( "outputs_collapsed" ), mOutputsCollapsed );

  saveCommonProperties( map );

  QVariantMap paramMap;
  QMap< QString, QgsProcessingModelChildParameterSources >::const_iterator paramIt = mParams.constBegin();
  for ( ; paramIt != mParams.constEnd(); ++paramIt )
  {
    QVariantList sources;
    Q_FOREACH ( const QgsProcessingModelChildParameterSource &source, paramIt.value() )
    {
      sources << source.toVariant();
    }
    paramMap.insert( paramIt.key(), sources );
  }
  map.insert( QStringLiteral( "params" ), paramMap );

  QVariantMap outputMap;
  QMap< QString, QgsProcessingModelOutput >::const_iterator outputIt = mModelOutputs.constBegin();
  for ( ; outputIt != mModelOutputs.constEnd(); ++outputIt )
  {
    outputMap.insert( outputIt.key(), outputIt.value().toVariant() );
  }
  map.insert( QStringLiteral( "outputs" ), outputMap );

  return map;
}

bool QgsProcessingModelChildAlgorithm::loadVariant( const QVariant &child )
{
  QVariantMap map = child.toMap();

  mId = map.value( QStringLiteral( "id" ) ).toString();
  mConfiguration = map.value( QStringLiteral( "alg_config" ) ).toMap();
  setAlgorithmId( map.value( QStringLiteral( "alg_id" ) ).toString() );
  mActive = map.value( QStringLiteral( "active" ) ).toBool();
  mDependencies = map.value( QStringLiteral( "dependencies" ) ).toStringList();
  mParametersCollapsed = map.value( QStringLiteral( "parameters_collapsed" ) ).toBool();
  mOutputsCollapsed = map.value( QStringLiteral( "outputs_collapsed" ) ).toBool();

  restoreCommonProperties( map );

  mParams.clear();
  QVariantMap paramMap = map.value( QStringLiteral( "params" ) ).toMap();
  QVariantMap::const_iterator paramIt = paramMap.constBegin();
  for ( ; paramIt != paramMap.constEnd(); ++paramIt )
  {
    QgsProcessingModelChildParameterSources sources;
    Q_FOREACH ( const QVariant &sourceVar, paramIt->toList() )
    {
      QgsProcessingModelChildParameterSource param;
      if ( !param.loadVariant( sourceVar.toMap() ) )
        return false;
      sources << param;
    }
    mParams.insert( paramIt.key(), sources );
  }

  mModelOutputs.clear();
  QVariantMap outputMap = map.value( QStringLiteral( "outputs" ) ).toMap();
  QVariantMap::const_iterator outputIt = outputMap.constBegin();
  for ( ; outputIt != outputMap.constEnd(); ++outputIt )
  {
    QgsProcessingModelOutput output;
    if ( !output.loadVariant( outputIt.value().toMap() ) )
      return false;

    mModelOutputs.insert( outputIt.key(), output );
  }

  return true;
}

QString QgsProcessingModelChildAlgorithm::asPythonCode() const
{
  QStringList lines;

  if ( !algorithm() )
    return QString();

  QStringList paramParts;
  QMap< QString, QgsProcessingModelChildParameterSources >::const_iterator paramIt = mParams.constBegin();
  for ( ; paramIt != mParams.constEnd(); ++paramIt )
  {
    QStringList sourceParts;
    Q_FOREACH ( const QgsProcessingModelChildParameterSource &source, paramIt.value() )
    {
      QString part = source.asPythonCode();
      if ( !part.isEmpty() )
        sourceParts << QStringLiteral( "'%1':%2" ).arg( paramIt.key(), part );
    }
    if ( sourceParts.count() == 1 )
      paramParts << sourceParts.at( 0 );
    else
      paramParts << QStringLiteral( "[%1]" ).arg( paramParts.join( ',' ) );
  }

  lines << QStringLiteral( "outputs['%1']=processing.run('%2', {%3}, context=context, feedback=feedback)" ).arg( mId, mAlgorithmId, paramParts.join( ',' ) );

  QMap< QString, QgsProcessingModelOutput >::const_iterator outputIt = mModelOutputs.constBegin();
  for ( ; outputIt != mModelOutputs.constEnd(); ++outputIt )
  {
    lines << QStringLiteral( "results['%1']=outputs['%2']['%3']" ).arg( outputIt.key(), mId, outputIt.value().childOutputName() );
  }

  return lines.join( '\n' );
}

QVariantMap QgsProcessingModelChildAlgorithm::configuration() const
{
  return mConfiguration;
}

void QgsProcessingModelChildAlgorithm::setConfiguration( const QVariantMap &configuration )
{
  mConfiguration = configuration;
  mAlgorithm.reset( QgsApplication::processingRegistry()->createAlgorithmById( mAlgorithmId, mConfiguration ) );
}

void QgsProcessingModelChildAlgorithm::generateChildId( const QgsProcessingModelAlgorithm &model )
{
  int i = 1;
  QString id;
  while ( true )
  {
    id = QStringLiteral( "%1_%2" ).arg( mAlgorithmId ).arg( i );
    if ( !model.childAlgorithms().contains( id ) )
      break;
    i++;
  }
  mId = id;
}

bool QgsProcessingModelChildAlgorithm::setAlgorithmId( const QString &algorithmId )
{
  mAlgorithmId = algorithmId;
  mAlgorithm.reset( QgsApplication::processingRegistry()->createAlgorithmById( mAlgorithmId, mConfiguration ) );
  return static_cast< bool >( mAlgorithm.get() );
}

bool QgsProcessingModelChildAlgorithm::reattach() const
{
  return const_cast< QgsProcessingModelChildAlgorithm * >( this )->setAlgorithmId( mAlgorithmId );
}

///@endcond
