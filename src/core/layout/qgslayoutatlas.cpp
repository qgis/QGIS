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

QgsLayoutAtlas::QgsLayoutAtlas( QgsLayout *layout )
  : QObject( layout )
  , mLayout( layout )
  , mFilenameExpressionString( QStringLiteral( "'output_'||@atlas_featurenumber" ) )
{

  //listen out for layer removal
  connect( mLayout->project(), static_cast < void ( QgsProject::* )( const QStringList & ) >( &QgsProject::layersWillBeRemoved ), this, &QgsLayoutAtlas::removeLayers );
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

QString QgsLayoutAtlas::nameForPage( int pageNumber ) const
{
#if 0 //TODO
  if ( pageNumber < 0 || pageNumber >= mFeatureIds.count() )
    return QString();

  return mFeatureIds.at( pageNumber ).second;
#endif
}

bool QgsLayoutAtlas::setFilterExpression( const QString &expression, QString &errorString )
{
  mFilterExpression = expression;
  return true;
}

/// @cond PRIVATE
class AtlasFieldSorter
{
  public:
    AtlasFieldSorter( QgsLayoutAtlas::SorterKeys &keys, bool ascending = true )
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

void QgsLayoutAtlas::setHideCoverage( bool hide )
{
  mHideCoverage = hide;

#if 0 //TODO
  if ( mComposition->atlasMode() == QgsComposition::PreviewAtlas )
  {
    //an atlas preview is enabled, so reflect changes in coverage layer visibility immediately
    updateAtlasMaps();
    mComposition->update();
  }
#endif
}

bool QgsLayoutAtlas::setFilenameExpression( const QString &pattern, QString &errorString )
{
  mFilenameExpressionString = pattern;
  return updateFilenameExpression( errorString );
}

QgsExpressionContext QgsLayoutAtlas::createExpressionContext()
{
  QgsExpressionContext expressionContext;
  expressionContext << QgsExpressionContextUtils::globalScope();
  if ( mLayout )
    expressionContext << QgsExpressionContextUtils::projectScope( mLayout->project() );
#if 0 //TODO
      << QgsExpressionContextUtils::compositionScope( mLayout );

  expressionContext.appendScope( QgsExpressionContextUtils::atlasScope( this ) );
#endif

  if ( mCoverageLayer )
    expressionContext.lastScope()->setFields( mCoverageLayer->fields() );
#if 0 //TODO
  if ( mLayout && mEnabled )
    expressionContext.lastScope()->setFeature( mCurrentFeature );
#endif

  return expressionContext;
}

bool QgsLayoutAtlas::updateFilenameExpression( QString &error )
{
  if ( !mCoverageLayer )
  {
    return false;
  }

  QgsExpressionContext expressionContext = createExpressionContext();

  if ( !mFilenameExpressionString.isEmpty() )
  {
    mFilenameExpression = QgsExpression( mFilenameExpressionString );
    // expression used to evaluate each filename
    // test for evaluation errors
    if ( mFilenameExpression.hasParserError() )
    {
      error = mFilenameExpression.parserErrorString();
      return false;
    }

    // prepare the filename expression
    mFilenameExpression.prepare( &expressionContext );
  }

#if 0 //TODO
  //if atlas preview is currently enabled, regenerate filename for current feature
  if ( mComposition->atlasMode() == QgsComposition::PreviewAtlas )
  {
    evalFeatureFilename( expressionContext );
  }
#endif
  return true;
}

