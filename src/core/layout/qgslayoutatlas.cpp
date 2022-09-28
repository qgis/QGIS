/***************************************************************************
                             qgslayoutatlas.cpp
                             ----------------
    begin                : December 2017
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
#include <algorithm>
#include <stdexcept>
#include <QtAlgorithms>

#include "qgslayoutatlas.h"
#include "qgslayout.h"
#include "qgsmessagelog.h"
#include "qgsfeaturerequest.h"
#include "qgsfeatureiterator.h"
#include "qgsvectorlayer.h"
#include "qgsexpressioncontextutils.h"
#include "qgsvariantutils.h"

QgsLayoutAtlas::QgsLayoutAtlas( QgsLayout *layout )
  : QObject( layout )
  , mLayout( layout )
  , mFilenameExpressionString( QStringLiteral( "'output_'||@atlas_featurenumber" ) )
{

  //listen out for layer removal
  connect( mLayout->project(), static_cast < void ( QgsProject::* )( const QStringList & ) >( &QgsProject::layersWillBeRemoved ), this, &QgsLayoutAtlas::removeLayers );

  if ( QgsVariantUtils::isNull( mLayout->customProperty( QStringLiteral( "singleFile" ) ) ) )
    mLayout->setCustomProperty( QStringLiteral( "singleFile" ), true );
}

QString QgsLayoutAtlas::stringType() const
{
  return QStringLiteral( "atlas" );
}

QgsLayout *QgsLayoutAtlas::layout()
{
  return mLayout;
}

const QgsLayout *QgsLayoutAtlas::layout() const
{
  return mLayout.data();
}

bool QgsLayoutAtlas::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement atlasElem = document.createElement( QStringLiteral( "Atlas" ) );
  atlasElem.setAttribute( QStringLiteral( "enabled" ), mEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );

  if ( mCoverageLayer )
  {
    atlasElem.setAttribute( QStringLiteral( "coverageLayer" ), mCoverageLayer.layerId );
    atlasElem.setAttribute( QStringLiteral( "coverageLayerName" ), mCoverageLayer.name );
    atlasElem.setAttribute( QStringLiteral( "coverageLayerSource" ), mCoverageLayer.source );
    atlasElem.setAttribute( QStringLiteral( "coverageLayerProvider" ), mCoverageLayer.provider );
  }
  else
  {
    atlasElem.setAttribute( QStringLiteral( "coverageLayer" ), QString() );
  }

  atlasElem.setAttribute( QStringLiteral( "hideCoverage" ), mHideCoverage ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  atlasElem.setAttribute( QStringLiteral( "filenamePattern" ), mFilenameExpressionString );
  atlasElem.setAttribute( QStringLiteral( "pageNameExpression" ), mPageNameExpression );

  atlasElem.setAttribute( QStringLiteral( "sortFeatures" ), mSortFeatures ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  if ( mSortFeatures )
  {
    atlasElem.setAttribute( QStringLiteral( "sortKey" ), mSortExpression );
    atlasElem.setAttribute( QStringLiteral( "sortAscending" ), mSortAscending ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  }
  atlasElem.setAttribute( QStringLiteral( "filterFeatures" ), mFilterFeatures ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  if ( mFilterFeatures )
  {
    atlasElem.setAttribute( QStringLiteral( "featureFilter" ), mFilterExpression );
  }

  parentElement.appendChild( atlasElem );

  return true;
}

bool QgsLayoutAtlas::readXml( const QDomElement &atlasElem, const QDomDocument &, const QgsReadWriteContext & )
{
  mEnabled = atlasElem.attribute( QStringLiteral( "enabled" ), QStringLiteral( "0" ) ).toInt();

  // look for stored layer name
  const QString layerId = atlasElem.attribute( QStringLiteral( "coverageLayer" ) );
  const QString layerName = atlasElem.attribute( QStringLiteral( "coverageLayerName" ) );
  const QString layerSource = atlasElem.attribute( QStringLiteral( "coverageLayerSource" ) );
  const QString layerProvider = atlasElem.attribute( QStringLiteral( "coverageLayerProvider" ) );

  mCoverageLayer = QgsVectorLayerRef( layerId, layerName, layerSource, layerProvider );
  mCoverageLayer.resolveWeakly( mLayout->project() );
  mLayout->reportContext().setLayer( mCoverageLayer.get() );

  mPageNameExpression = atlasElem.attribute( QStringLiteral( "pageNameExpression" ), QString() );
  QString error;
  setFilenameExpression( atlasElem.attribute( QStringLiteral( "filenamePattern" ), QString() ), error );

  mSortFeatures = atlasElem.attribute( QStringLiteral( "sortFeatures" ), QStringLiteral( "0" ) ).toInt();
  mSortExpression = atlasElem.attribute( QStringLiteral( "sortKey" ) );
  mSortAscending = atlasElem.attribute( QStringLiteral( "sortAscending" ), QStringLiteral( "1" ) ).toInt();
  mFilterFeatures = atlasElem.attribute( QStringLiteral( "filterFeatures" ), QStringLiteral( "0" ) ).toInt();
  mFilterExpression = atlasElem.attribute( QStringLiteral( "featureFilter" ) );

  mHideCoverage = atlasElem.attribute( QStringLiteral( "hideCoverage" ), QStringLiteral( "0" ) ).toInt();

  emit toggled( mEnabled );
  emit changed();
  return true;
}

void QgsLayoutAtlas::setEnabled( bool enabled )
{
  if ( enabled == mEnabled )
  {
    return;
  }

  mEnabled = enabled;
  emit toggled( enabled );
  emit changed();
}

void QgsLayoutAtlas::removeLayers( const QStringList &layers )
{
  if ( !mCoverageLayer )
  {
    return;
  }

  for ( const QString &layerId : layers )
  {
    if ( layerId == mCoverageLayer.layerId )
    {
      //current coverage layer removed
      mCoverageLayer.setLayer( nullptr );
      setEnabled( false );
      break;
    }
  }
}

void QgsLayoutAtlas::setCoverageLayer( QgsVectorLayer *layer )
{
  if ( layer == mCoverageLayer.get() )
  {
    return;
  }

  mCoverageLayer.setLayer( layer );
  emit coverageLayerChanged( layer );
}

void QgsLayoutAtlas::setPageNameExpression( const QString &expression )
{
  if ( mPageNameExpression == expression )
    return;

  mPageNameExpression = expression;
  emit changed();
}

QString QgsLayoutAtlas::nameForPage( int pageNumber ) const
{
  if ( pageNumber < 0 || pageNumber >= mFeatureIds.count() )
    return QString();

  return mFeatureIds.at( pageNumber ).second;
}

void QgsLayoutAtlas::setSortFeatures( bool enabled )
{
  if ( mSortFeatures == enabled )
    return;

  mSortFeatures = enabled;
  emit changed();
}

void QgsLayoutAtlas::setSortAscending( bool ascending )
{
  if ( mSortAscending == ascending )
    return;

  mSortAscending = ascending;
  emit changed();
}

void QgsLayoutAtlas::setSortExpression( const QString &expression )
{
  if ( mSortExpression == expression )
    return;

  mSortExpression = expression;
  emit changed();
}

void QgsLayoutAtlas::setFilterFeatures( bool filtered )
{
  if ( mFilterFeatures == filtered )
    return;

  mFilterFeatures = filtered;
  emit changed();
}

bool QgsLayoutAtlas::setFilterExpression( const QString &expression, QString &errorString )
{
  errorString.clear();
  const bool hasChanged = mFilterExpression != expression;
  mFilterExpression = expression;

  const QgsExpression filterExpression( mFilterExpression );
  if ( hasChanged )
    emit changed();
  if ( filterExpression.hasParserError() )
  {
    errorString = filterExpression.parserErrorString();
    return false;
  }

  return true;
}


/// @cond PRIVATE
class AtlasFeatureSorter
{
  public:
    AtlasFeatureSorter( QgsLayoutAtlas::SorterKeys &keys, bool ascending = true )
      : mKeys( keys )
      , mAscending( ascending )
    {}

    bool operator()( const QPair< QgsFeatureId, QString > &id1, const QPair< QgsFeatureId, QString > &id2 )
    {
      return mAscending ? qgsVariantLessThan( mKeys.value( id1.first ), mKeys.value( id2.first ) )
             : qgsVariantGreaterThan( mKeys.value( id1.first ), mKeys.value( id2.first ) );
    }

  private:
    QgsLayoutAtlas::SorterKeys &mKeys;
    bool mAscending;
};

/// @endcond

int QgsLayoutAtlas::updateFeatures()
{
  mCurrentFeatureNo = -1;
  if ( !mCoverageLayer )
  {
    return 0;
  }

  QgsExpressionContext expressionContext = createExpressionContext();

  QString error;
  updateFilenameExpression( error );

  // select all features with all attributes
  QgsFeatureRequest req;

  req.setExpressionContext( expressionContext );

  mFilterParserError.clear();
  if ( mFilterFeatures && !mFilterExpression.isEmpty() )
  {
    const QgsExpression filterExpression( mFilterExpression );
    if ( filterExpression.hasParserError() )
    {
      mFilterParserError = filterExpression.parserErrorString();
      return 0;
    }

    //filter good to go
    req.setFilterExpression( mFilterExpression );
  }

#ifdef HAVE_SERVER_PYTHON_PLUGINS
  if ( mLayout->renderContext().featureFilterProvider() )
  {
    mLayout->renderContext().featureFilterProvider()->filterFeatures( mCoverageLayer.get(), req );
  }
#endif

  QgsFeatureIterator fit = mCoverageLayer->getFeatures( req );

  std::unique_ptr<QgsExpression> nameExpression;
  if ( !mPageNameExpression.isEmpty() )
  {
    nameExpression = std::make_unique< QgsExpression >( mPageNameExpression );
    if ( nameExpression->hasParserError() )
    {
      nameExpression.reset( nullptr );
    }
    else
    {
      nameExpression->prepare( &expressionContext );
    }
  }

  // We cannot use nextFeature() directly since the feature pointer is rewinded by the rendering process
  // We thus store the feature ids for future extraction
  QgsFeature feat;
  mFeatureIds.clear();
  mFeatureKeys.clear();

  std::unique_ptr<QgsExpression> sortExpression;
  if ( mSortFeatures && !mSortExpression.isEmpty() )
  {
    sortExpression = std::make_unique< QgsExpression >( mSortExpression );
    if ( sortExpression->hasParserError() )
    {
      sortExpression.reset( nullptr );
    }
    else
    {
      sortExpression->prepare( &expressionContext );
    }
  }

  while ( fit.nextFeature( feat ) )
  {
    expressionContext.setFeature( feat );

    QString pageName;
    if ( nameExpression )
    {
      const QVariant result = nameExpression->evaluate( &expressionContext );
      if ( nameExpression->hasEvalError() )
      {
        QgsMessageLog::logMessage( tr( "Atlas name eval error: %1" ).arg( nameExpression->evalErrorString() ), tr( "Layout" ) );
      }
      pageName = result.toString();
    }

    mFeatureIds.push_back( qMakePair( feat.id(), pageName ) );

    if ( sortExpression )
    {
      const QVariant result = sortExpression->evaluate( &expressionContext );
      if ( sortExpression->hasEvalError() )
      {
        QgsMessageLog::logMessage( tr( "Atlas sort eval error: %1" ).arg( sortExpression->evalErrorString() ), tr( "Layout" ) );
      }
      mFeatureKeys.insert( feat.id(), result );
    }
  }

  // sort features, if asked for
  if ( !mFeatureKeys.isEmpty() )
  {
    const AtlasFeatureSorter sorter( mFeatureKeys, mSortAscending );
    std::sort( mFeatureIds.begin(), mFeatureIds.end(), sorter ); // clazy:exclude=detaching-member
  }

  emit numberFeaturesChanged( mFeatureIds.size() );
  return mFeatureIds.size();
}

bool QgsLayoutAtlas::beginRender()
{
  if ( !mCoverageLayer )
  {
    return false;
  }

  emit renderBegun();

  if ( !updateFeatures() )
  {
    //no matching features found
    return false;
  }

  return true;
}

bool QgsLayoutAtlas::endRender()
{
  emit featureChanged( QgsFeature() );
  emit renderEnded();
  return true;
}

int QgsLayoutAtlas::count() const
{
  return mFeatureIds.size();
}

QString QgsLayoutAtlas::filePath( const QString &baseFilePath, const QString &extension )
{
  const QFileInfo fi( baseFilePath );
  const QDir dir = fi.dir(); // ignore everything except the directory
  QString base = dir.filePath( mCurrentFilename );
  if ( !extension.startsWith( '.' ) )
    base += '.';
  base += extension;
  return base;
}

bool QgsLayoutAtlas::next()
{
  const int newFeatureNo = mCurrentFeatureNo + 1;
  if ( newFeatureNo >= mFeatureIds.size() )
  {
    return false;
  }

  return prepareForFeature( newFeatureNo );
}

bool QgsLayoutAtlas::previous()
{
  const int newFeatureNo = mCurrentFeatureNo - 1;
  if ( newFeatureNo < 0 )
  {
    return false;
  }

  return prepareForFeature( newFeatureNo );
}

bool QgsLayoutAtlas::first()
{
  return prepareForFeature( 0 );
}

bool QgsLayoutAtlas::last()
{
  return prepareForFeature( mFeatureIds.size() - 1 );
}

bool QgsLayoutAtlas::seekTo( int feature )
{
  return prepareForFeature( feature );
}

bool QgsLayoutAtlas::seekTo( const QgsFeature &feature )
{
  int i = -1;
  auto it = mFeatureIds.constBegin();
  for ( int currentIdx = 0; it != mFeatureIds.constEnd(); ++it, ++currentIdx )
  {
    if ( ( *it ).first == feature.id() )
    {
      i = currentIdx;
      break;
    }
  }

  if ( i < 0 )
  {
    //feature not found
    return false;
  }

  return seekTo( i );
}

void QgsLayoutAtlas::refreshCurrentFeature()
{
  prepareForFeature( mCurrentFeatureNo );
}

void QgsLayoutAtlas::setHideCoverage( bool hide )
{
  mLayout->renderContext().setFlag( QgsLayoutRenderContext::FlagHideCoverageLayer, hide );
  if ( hide == mHideCoverage )
    return;

  mHideCoverage = hide;
  mLayout->refresh();
  emit changed();
}

bool QgsLayoutAtlas::setFilenameExpression( const QString &pattern, QString &errorString )
{
  const bool hasChanged = mFilenameExpressionString != pattern;
  mFilenameExpressionString = pattern;

  if ( hasChanged )
    emit changed();

  return updateFilenameExpression( errorString );
}

QString QgsLayoutAtlas::currentFilename() const
{
  return mCurrentFilename;
}

QgsExpressionContext QgsLayoutAtlas::createExpressionContext() const
{
  QgsExpressionContext expressionContext;
  expressionContext << QgsExpressionContextUtils::globalScope();
  if ( mLayout )
    expressionContext << QgsExpressionContextUtils::projectScope( mLayout->project() )
                      << QgsExpressionContextUtils::layoutScope( mLayout );

  expressionContext.appendScope( QgsExpressionContextUtils::atlasScope( this ) );

  if ( mCoverageLayer )
    expressionContext.appendScope( mCoverageLayer->createExpressionContextScope() );

  if ( mLayout && mEnabled )
  {
    if ( mCurrentFeature.isValid() )
    {
      expressionContext.lastScope()->setFeature( mCurrentFeature );
    }
    else if ( mCoverageLayer )  // Create an empty feature for the expression validation
    {
      QgsFeature feature{ mCoverageLayer->fields() };
      feature.setValid( true );
      expressionContext.lastScope()->setFeature( feature );
    }
  }
  return expressionContext;
}

bool QgsLayoutAtlas::updateFilenameExpression( QString &error )
{
  if ( !mCoverageLayer )
  {
    return false;
  }

  const QgsExpressionContext expressionContext = createExpressionContext();
  bool evalResult { true };

  if ( !mFilenameExpressionString.isEmpty() )
  {
    QgsExpression filenameExpression( mFilenameExpressionString );
    // expression used to evaluate each filename
    // test for evaluation errors
    if ( filenameExpression.hasParserError() )
    {
      error = filenameExpression.parserErrorString();
      return false;
    }

    // prepare the filename expression
    evalResult = filenameExpression.prepare( &expressionContext );
  }

  // regenerate current filename
  if ( evalResult )
  {
    evalResult = evalFeatureFilename( expressionContext );
  }

  if ( ! evalResult )
  {
    error = mFilenameExpressionError;
  }

  return evalResult;
}

bool QgsLayoutAtlas::evalFeatureFilename( const QgsExpressionContext &context )
{
  //generate filename for current atlas feature
  mFilenameExpressionError.clear();
  if ( !mFilenameExpressionString.isEmpty() )
  {
    QgsExpression filenameExpression( mFilenameExpressionString );
    filenameExpression.prepare( &context );
    const QVariant filenameRes = filenameExpression.evaluate( &context );
    if ( filenameExpression.hasEvalError() )
    {
      mFilenameExpressionError = filenameExpression.evalErrorString();
      QgsMessageLog::logMessage( tr( "Atlas filename evaluation error: %1" ).arg( filenameExpression.evalErrorString() ), tr( "Layout" ) );
      return false;
    }

    mCurrentFilename = filenameRes.toString();
  }
  return true;
}

bool QgsLayoutAtlas::prepareForFeature( const int featureI )
{
  if ( !mCoverageLayer )
  {
    return false;
  }

  if ( mFeatureIds.isEmpty() )
  {
    emit messagePushed( tr( "No matching atlas features" ) );
    return false;
  }

  if ( featureI >= mFeatureIds.size() )
  {
    return false;
  }

  mCurrentFeatureNo = featureI;

  // retrieve the next feature, based on its id
  if ( !mCoverageLayer->getFeatures( QgsFeatureRequest().setFilterFid( mFeatureIds[ featureI ].first ) ).nextFeature( mCurrentFeature ) )
    return false;

  mLayout->reportContext().blockSignals( true ); // setFeature emits changed, we don't want 2 signals
  mLayout->reportContext().setLayer( mCoverageLayer.get() );
  mLayout->reportContext().blockSignals( false );
  mLayout->reportContext().setFeature( mCurrentFeature );

  // must come after we've set the report context feature, or the expression context will have an outdated atlas feature
  const QgsExpressionContext expressionContext = createExpressionContext();

  // generate filename for current feature
  if ( !evalFeatureFilename( expressionContext ) )
  {
    //error evaluating filename
    return false;
  }

  emit featureChanged( mCurrentFeature );
  emit messagePushed( tr( "Atlas feature %1 of %2" ).arg( featureI + 1 ).arg( mFeatureIds.size() ) );

  return mCurrentFeature.isValid();
}

