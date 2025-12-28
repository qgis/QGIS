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
#include "qgsprocessingmodelalgorithm.h"
#include "qgsprocessingregistry.h"

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
  , mComment( other.mComment )
{
  setAlgorithmId( other.algorithmId() );
}

QgsProcessingModelChildAlgorithm &QgsProcessingModelChildAlgorithm::operator=( const QgsProcessingModelChildAlgorithm &other )
{
  if ( &other == this )
    return *this;

  QgsProcessingModelComponent::operator =( other );
  mId = other.mId;
  mConfiguration = other.mConfiguration;
  setAlgorithmId( other.algorithmId() );
  mParams = other.mParams;
  mModelOutputs = other.mModelOutputs;
  mActive = other.mActive;
  mDependencies = other.mDependencies;
  mComment = other.mComment;
  return *this;
}

QgsProcessingModelChildAlgorithm *QgsProcessingModelChildAlgorithm::clone() const
{
  return new QgsProcessingModelChildAlgorithm( *this );
}

void QgsProcessingModelChildAlgorithm::copyNonDefinitionPropertiesFromModel( QgsProcessingModelAlgorithm *model )
{
  const QgsProcessingModelChildAlgorithm existingChild = model->childAlgorithm( mId );
  copyNonDefinitionProperties( existingChild );

  int i = 0;
  for ( auto it = mModelOutputs.begin(); it != mModelOutputs.end(); ++it )
  {
    const QMap<QString, QgsProcessingModelOutput> existingChildModelOutputs = existingChild.modelOutputs();
    auto existingOutputIt = existingChildModelOutputs.find( it.key() );
    if ( existingOutputIt == existingChildModelOutputs.end() )
      continue;

    if ( !existingOutputIt->position().isNull() )
    {
      it.value().setPosition( existingOutputIt->position() );
      it.value().setSize( existingOutputIt->size() );
    }
    else
      it.value().setPosition( position() + QPointF( size().width(), ( i + 1.5 ) * size().height() ) );

    if ( QgsProcessingModelComment *comment = it.value().comment() )
    {
      if ( const QgsProcessingModelComment *existingComment = existingOutputIt->comment() )
      {
        comment->setDescription( existingComment->description() );
        comment->setSize( existingComment->size() );
        comment->setPosition( existingComment->position() );
        comment->setColor( existingComment->color() );
      }
    }
    i++;
  }
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
  map.insert( u"id"_s, mId );
  map.insert( u"alg_id"_s, mAlgorithmId );
  map.insert( u"alg_config"_s, mConfiguration );
  map.insert( u"active"_s, mActive );

  QVariantList dependencies;
  for ( const QgsProcessingModelChildDependency &dependency : mDependencies )
  {
    dependencies << dependency.toVariant();
  }
  map.insert( u"dependencies"_s, dependencies );

  saveCommonProperties( map );

  QVariantMap paramMap;
  QMap< QString, QgsProcessingModelChildParameterSources >::const_iterator paramIt = mParams.constBegin();
  for ( ; paramIt != mParams.constEnd(); ++paramIt )
  {
    QVariantList sources;
    const auto constValue = paramIt.value();
    for ( const QgsProcessingModelChildParameterSource &source : constValue )
    {
      sources << source.toVariant();
    }
    paramMap.insert( paramIt.key(), sources );
  }
  map.insert( u"params"_s, paramMap );

  QVariantMap outputMap;
  QMap< QString, QgsProcessingModelOutput >::const_iterator outputIt = mModelOutputs.constBegin();
  for ( ; outputIt != mModelOutputs.constEnd(); ++outputIt )
  {
    outputMap.insert( outputIt.key(), outputIt.value().toVariant() );
  }
  map.insert( u"outputs"_s, outputMap );

  return map;
}

bool QgsProcessingModelChildAlgorithm::loadVariant( const QVariant &child )
{
  QVariantMap map = child.toMap();

  mId = map.value( u"id"_s ).toString();
  if ( mId.isEmpty() )
    return false;

  mConfiguration = map.value( u"alg_config"_s ).toMap();
  setAlgorithmId( map.value( u"alg_id"_s ).toString() );
  if ( algorithmId().isEmpty() )
    return false;
  mActive = map.value( u"active"_s ).toBool();

  mDependencies.clear();
  if ( map.value( u"dependencies"_s ).userType() == QMetaType::Type::QStringList )
  {
    const QStringList dependencies = map.value( u"dependencies"_s ).toStringList();
    mDependencies.reserve( dependencies.size() );
    for ( const QString &dependency : dependencies )
    {
      QgsProcessingModelChildDependency dep;
      dep.childId = dependency;
      mDependencies << dep;
    }
  }
  else
  {
    const QVariantList dependencies = map.value( u"dependencies"_s ).toList();
    mDependencies.reserve( dependencies.size() );
    for ( const QVariant &dependency : dependencies )
    {
      QgsProcessingModelChildDependency dep;
      dep.loadVariant( dependency.toMap() );
      mDependencies << dep;
    }
  }

  restoreCommonProperties( map );

  mParams.clear();
  QVariantMap paramMap = map.value( u"params"_s ).toMap();
  QVariantMap::const_iterator paramIt = paramMap.constBegin();
  for ( ; paramIt != paramMap.constEnd(); ++paramIt )
  {
    QgsProcessingModelChildParameterSources sources;
    const auto constToList = paramIt->toList();
    sources.reserve( constToList.size() );
    for ( const QVariant &sourceVar : constToList )
    {
      QgsProcessingModelChildParameterSource param;
      if ( !param.loadVariant( sourceVar.toMap() ) )
        return false;
      sources << param;
    }
    mParams.insert( paramIt.key(), sources );
  }

  mModelOutputs.clear();
  QVariantMap outputMap = map.value( u"outputs"_s ).toMap();
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

QStringList QgsProcessingModelChildAlgorithm::asPythonCode( const QgsProcessing::PythonOutputType outputType, const QgsStringMap &extraParameters,
    int currentIndent, int indentSize, const QMap<QString, QString> &friendlyChildNames, const QMap<QString, QString> &friendlyOutputNames ) const
{
  QStringList lines;
  const QString baseIndent = QString( ' ' ).repeated( currentIndent );
  const QString lineIndent = QString( ' ' ).repeated( indentSize );

  if ( !algorithm() )
    return QStringList();

  if ( !description().isEmpty() )
    lines << baseIndent + u"# %1"_s.arg( description() );
  if ( !mComment.description().isEmpty() )
  {
    const QStringList parts = mComment.description().split( u"\n"_s );
    for ( const QString &part : parts )
    {
      lines << baseIndent + u"# %1"_s.arg( part );
    }
  }

  QStringList paramParts;
  QStringList paramComments;
  for ( auto paramIt = mParams.constBegin(); paramIt != mParams.constEnd(); ++paramIt )
  {
    QStringList sourceParts;
    QStringList sourceComments;
    const QgsProcessingParameterDefinition *def = algorithm() ? algorithm()->parameterDefinition( paramIt.key() ) : nullptr;
    const auto parts = paramIt.value();
    sourceParts.reserve( parts.size() );
    sourceComments.reserve( parts.size() );
    for ( const QgsProcessingModelChildParameterSource &source : parts )
    {
      QString part = source.asPythonCode( outputType, def, friendlyChildNames );
      if ( !part.isEmpty() )
      {
        sourceParts << part;
        sourceComments << source.asPythonComment( def );
      }
    }
    if ( sourceParts.count() == 1 )
    {
      paramParts << u"'%1': %2"_s.arg( paramIt.key(), sourceParts.at( 0 ) );
      paramComments << sourceComments.at( 0 );
    }
    else
    {
      paramParts << u"'%1': [%2]"_s.arg( paramIt.key(), sourceParts.join( ',' ) );
      paramComments << QString();
    }
  }

  lines << baseIndent + u"alg_params = {"_s;
  lines.reserve( lines.size() + paramParts.size() );
  int i = 0;
  for ( const QString &p : std::as_const( paramParts ) )
  {
    QString line = baseIndent + lineIndent + p + ',';
    if ( !paramComments.value( i ).isEmpty() )
    {
      line += u"  # %1"_s.arg( paramComments.value( i ) );
    }
    lines << line;
    i++;
  }
  for ( auto it = extraParameters.constBegin(); it != extraParameters.constEnd(); ++it )
  {
    lines << baseIndent + lineIndent + u"%1: %2,"_s.arg( QgsProcessingUtils::stringToPythonLiteral( it.key() ), it.value() );
  }
  if ( lines.constLast().endsWith( ',' ) )
  {
    lines[ lines.count() - 1 ].truncate( lines.constLast().length() - 1 );
  }
  lines << baseIndent + u"}"_s;

  lines << baseIndent + u"outputs['%1'] = processing.run('%2', alg_params, context=context, feedback=feedback, is_child_algorithm=True)"_s.arg( friendlyChildNames.value( mId, mId ), mAlgorithmId );

  for ( auto outputIt = mModelOutputs.constBegin(); outputIt != mModelOutputs.constEnd(); ++outputIt )
  {
    QString outputName = u"%1:%2"_s.arg( mId, outputIt.key() );
    outputName = friendlyOutputNames.value( outputName, outputName );
    lines << baseIndent + u"results['%1'] = outputs['%2']['%3']"_s.arg( outputName, friendlyChildNames.value( mId, mId ), outputIt.value().childOutputName() );
  }

  return lines;
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
    id = u"%1_%2"_s.arg( mAlgorithmId ).arg( i );
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
