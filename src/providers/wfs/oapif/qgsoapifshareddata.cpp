/***************************************************************************
    qgsoapifhareddata.cpp
    ---------------------
    begin                : October 2019
    copyright            : (C) 2019 by Even Rouault
    email                : even.rouault at spatialys.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoapifshareddata.h"

#include "qgsexpressionnodeimpl.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsoapifcql2textexpressioncompiler.h"
#include "qgsoapiffeaturedownloaderimpl.h"
#include "qgsoapifutils.h"

#include <QUrl>
#include <QUrlQuery>

#include "moc_qgsoapifshareddata.cpp"

QgsOapifSharedData::QgsOapifSharedData( const QString &uri )
  : QgsBackgroundCachedSharedData( uri, "oapif", tr( "OAPIF" ) )
{
  mHideProgressDialog = mURI.hideDownloadProgressDialog();
}

QgsOapifSharedData::~QgsOapifSharedData()
{
  QgsDebugMsgLevel( u"~QgsOapifSharedData()"_s, 4 );

  cleanup();
}

QString QgsOapifSharedData::appendExtraQueryParameters( const QString &url ) const
{
  if ( mExtraQueryParameters.isEmpty() || url.indexOf( mExtraQueryParameters ) > 0 )
    return url;
  const int nPos = static_cast<int>( url.indexOf( '?' ) );
  if ( nPos < 0 )
    return url + '?' + mExtraQueryParameters;
  return url + '&' + mExtraQueryParameters;
}

bool QgsOapifSharedData::isRestrictedToRequestBBOX() const
{
  return mURI.isRestrictedToRequestBBOX();
}


std::unique_ptr<QgsFeatureDownloaderImpl> QgsOapifSharedData::newFeatureDownloaderImpl( QgsFeatureDownloader *downloader, bool requestMadeFromMainThread )
{
  return std::unique_ptr<QgsFeatureDownloaderImpl>( new QgsOapifFeatureDownloaderImpl( this, downloader, requestMadeFromMainThread ) );
}


void QgsOapifSharedData::invalidateCacheBaseUnderLock()
{
}

QgsOapifSharedData *QgsOapifSharedData::clone() const
{
  QgsOapifSharedData *copy = new QgsOapifSharedData( mURI.uri( true ) );
  copy->mWKBType = mWKBType;
  copy->mPageSize = mPageSize;
  copy->mExtraQueryParameters = mExtraQueryParameters;
  copy->mCollectionUrl = mCollectionUrl;
  copy->mItemsUrl = mItemsUrl;
  copy->mFeatureFormat = mFeatureFormat;
  copy->mServerFilter = mServerFilter;
  copy->mFoundIdTopLevel = mFoundIdTopLevel;
  copy->mFoundIdInProperties = mFoundIdInProperties;
  copy->mSimpleQueryables = mSimpleQueryables;
  copy->mServerSupportsFilterCql2Text = mServerSupportsFilterCql2Text;
  copy->mServerSupportsLikeBetweenIn = mServerSupportsLikeBetweenIn;
  copy->mServerSupportsCaseI = mServerSupportsCaseI;
  copy->mServerSupportsBasicSpatialFunctions = mServerSupportsBasicSpatialFunctions;
  copy->mQueryables = mQueryables;
  QgsBackgroundCachedSharedData::copyStateToClone( copy );

  return copy;
}

static QDateTime getDateTimeValue( const QVariant &v )
{
  if ( v.userType() == QMetaType::Type::QString )
    return QDateTime::fromString( v.toString(), Qt::ISODateWithMs );
  else if ( v.userType() == QMetaType::Type::QDateTime )
    return v.toDateTime();
  return QDateTime();
}

static bool isDateTime( const QVariant &v )
{
  return getDateTimeValue( v ).isValid();
}

static QString getDateTimeValueAsString( const QVariant &v )
{
  if ( v.userType() == QMetaType::Type::QString )
    return v.toString();
  else if ( v.userType() == QMetaType::Type::QDateTime )
    return v.toDateTime().toOffsetFromUtc( 0 ).toString( Qt::ISODateWithMs );
  return QString();
}

static bool isDateTimeField( const QgsFields &fields, const QString &fieldName )
{
  const int idx = fields.indexOf( fieldName );
  if ( idx >= 0 )
  {
    const auto type = fields.at( idx ).type();
    return type == QMetaType::Type::QDateTime || type == QMetaType::Type::QDate;
  }
  return false;
}

static QString getEncodedQueryParam( const QString &key, const QString &value )
{
  QUrlQuery query;
  query.addQueryItem( key, value );
  return query.toString( QUrl::FullyEncoded );
}

static void collectTopLevelAndNodes( const QgsExpressionNode *node, std::vector<const QgsExpressionNode *> &topAndNodes )
{
  if ( node->nodeType() == QgsExpressionNode::ntBinaryOperator )
  {
    const auto binNode = static_cast<const QgsExpressionNodeBinaryOperator *>( node );
    const auto op = binNode->op();
    if ( op == QgsExpressionNodeBinaryOperator::boAnd )
    {
      collectTopLevelAndNodes( binNode->opLeft(), topAndNodes );
      collectTopLevelAndNodes( binNode->opRight(), topAndNodes );
      return;
    }
  }
  topAndNodes.push_back( node );
}

QString QgsOapifSharedData::compileExpressionNodeUsingPart1(
  const QgsExpressionNode *rootNode,
  QgsOapifFilterTranslationState &translationState,
  QString &untranslatedPart
) const
{
  std::vector<const QgsExpressionNode *> topAndNodes;
  collectTopLevelAndNodes( rootNode, topAndNodes );
  QDateTime minDate;
  QDateTime maxDate;
  QString minDateStr;
  QString maxDateStr;
  QStringList equalityComparisons;
  bool hasTranslatedParts = false;
  for ( size_t i = 0; i < topAndNodes.size(); /* do not increment here */ )
  {
    bool removeMe = false;
    const auto node = topAndNodes[i];
    if ( node->nodeType() == QgsExpressionNode::ntBinaryOperator )
    {
      const auto binNode = static_cast<const QgsExpressionNodeBinaryOperator *>( node );
      const auto op = binNode->op();
      if ( binNode->opLeft()->nodeType() == QgsExpressionNode::ntColumnRef && binNode->opRight()->nodeType() == QgsExpressionNode::ntLiteral )
      {
        const auto left = static_cast<const QgsExpressionNodeColumnRef *>( binNode->opLeft() );
        const auto right = static_cast<const QgsExpressionNodeLiteral *>( binNode->opRight() );
        if ( isDateTimeField( mFields, left->name() ) && isDateTime( right->value() ) )
        {
          if ( op == QgsExpressionNodeBinaryOperator::boGE || op == QgsExpressionNodeBinaryOperator::boGT || op == QgsExpressionNodeBinaryOperator::boEQ )
          {
            removeMe = true;
            if ( !minDate.isValid() || getDateTimeValue( right->value() ) > minDate )
            {
              minDate = getDateTimeValue( right->value() );
              minDateStr = getDateTimeValueAsString( right->value() );
            }
          }
          if ( op == QgsExpressionNodeBinaryOperator::boLE || op == QgsExpressionNodeBinaryOperator::boLT || op == QgsExpressionNodeBinaryOperator::boEQ )
          {
            removeMe = true;
            if ( !maxDate.isValid() || getDateTimeValue( right->value() ) < maxDate )
            {
              maxDate = getDateTimeValue( right->value() );
              maxDateStr = getDateTimeValueAsString( right->value() );
            }
          }
        }
        else if ( op == QgsExpressionNodeBinaryOperator::boEQ && mFields.indexOf( left->name() ) >= 0 )
        {
          // Filtering based on Part 1 /rec/core/fc-filters recommendation.
          const auto iter = mSimpleQueryables.find( left->name() );
          if ( iter != mSimpleQueryables.end() )
          {
            if ( iter->mType == "string"_L1 && right->value().userType() == QMetaType::Type::QString )
            {
              equalityComparisons << getEncodedQueryParam( left->name(), right->value().toString() );
              removeMe = true;
            }
            else if ( ( iter->mType == "integer"_L1 || iter->mType == "number"_L1 ) && right->value().userType() == QMetaType::Type::Int )
            {
              equalityComparisons << getEncodedQueryParam( left->name(), QString::number( right->value().toInt() ) );
              removeMe = true;
            }
            else if ( iter->mType == "number"_L1 && right->value().userType() == QMetaType::Type::Double )
            {
              equalityComparisons << getEncodedQueryParam( left->name(), QString::number( right->value().toDouble() ) );
              removeMe = true;
            }
            else if ( iter->mType == "boolean"_L1 && right->value().userType() == QMetaType::Type::Bool )
            {
              equalityComparisons << getEncodedQueryParam( left->name(), right->value().toBool() ? "true"_L1 : "false"_L1 );
              removeMe = true;
            }
          }
        }
      }
    }
    if ( removeMe )
    {
      hasTranslatedParts = true;
      topAndNodes.erase( topAndNodes.begin() + static_cast<int>( i ) );
    }
    else
      ++i;
  }

  QString ret;
  if ( minDate.isValid() && maxDate.isValid() )
  {
    if ( minDate == maxDate )
    {
      ret = u"datetime="_s + minDateStr;
    }
    else
    {
      ret = u"datetime="_s + minDateStr + u"%2F"_s + maxDateStr;
    }
  }
  else if ( minDate.isValid() )
  {
    // TODO: use ellipsis '..' instead of dummy upper bound once more servers are compliant
    ret = u"datetime="_s + minDateStr + u"%2F9999-12-31T00:00:00Z"_s;
  }
  else if ( maxDate.isValid() )
  {
    // TODO: use ellipsis '..' instead of dummy upper bound once more servers are compliant
    ret = u"datetime=0000-01-01T00:00:00Z%2F"_s + maxDateStr;
  }

  for ( const QString &equalityComparison : equalityComparisons )
  {
    if ( !ret.isEmpty() )
      ret += QLatin1Char( '&' );
    ret += equalityComparison;
  }

  if ( !hasTranslatedParts )
  {
    untranslatedPart = rootNode->dump();
    translationState = QgsOapifFilterTranslationState::FULLY_CLIENT;
  }
  else if ( topAndNodes.empty() )
  {
    untranslatedPart.clear();
    translationState = QgsOapifFilterTranslationState::FULLY_SERVER;
  }
  else
  {
    translationState = QgsOapifFilterTranslationState::PARTIAL;

    // Collect part(s) of the filter to be evaluated on client-side
    if ( topAndNodes.size() == 1 )
    {
      untranslatedPart = topAndNodes[0]->dump();
    }
    else
    {
      for ( size_t i = 0; i < topAndNodes.size(); ++i )
      {
        if ( i == 0 )
          untranslatedPart = u"("_s;
        else
          untranslatedPart += " AND ("_L1;
        untranslatedPart += topAndNodes[i]->dump();
        untranslatedPart += QLatin1Char( ')' );
      }
    }
  }

  return ret;
}

bool QgsOapifSharedData::computeFilter( const QgsExpression &expr, QgsOapifFilterTranslationState &translationState, QString &serverSideParameters, QString &clientSideFilterExpression ) const
{
  const auto rootNode = expr.rootNode();
  if ( !rootNode )
    return false;

  if ( mServerSupportsFilterCql2Text )
  {
    const bool invertAxisOrientation = mSourceCrs.hasAxisInverted();
    QgsOapifCql2TextExpressionCompiler compiler(
      mQueryables, mServerSupportsLikeBetweenIn, mServerSupportsCaseI,
      mServerSupportsBasicSpatialFunctions, invertAxisOrientation
    );
    QgsOapifCql2TextExpressionCompiler::Result res = compiler.compile( &expr );
    if ( res == QgsOapifCql2TextExpressionCompiler::Fail )
    {
      clientSideFilterExpression = expr.rootNode()->dump();
      translationState = QgsOapifFilterTranslationState::FULLY_CLIENT;
      return true;
    }
    serverSideParameters = getEncodedQueryParam( u"filter"_s, compiler.result() );
    serverSideParameters += "&filter-lang=cql2-text"_L1;
    if ( compiler.geometryLiteralUsed() )
    {
      if ( mSourceCrs
           != QgsCoordinateReferenceSystem::fromOgcWmsCrs( OAPIF_PROVIDER_DEFAULT_CRS ) )
      {
        serverSideParameters += u"&filter-crs=%1"_s.arg( mSourceCrs.toOgcUri() );
      }
    }

    clientSideFilterExpression.clear();
    if ( res == QgsOapifCql2TextExpressionCompiler::Partial )
    {
      translationState = QgsOapifFilterTranslationState::PARTIAL;
    }
    else
    {
      translationState = QgsOapifFilterTranslationState::FULLY_SERVER;
    }
    return true;
  }

  serverSideParameters = compileExpressionNodeUsingPart1( rootNode, translationState, clientSideFilterExpression );
  return true;
}

bool QgsOapifSharedData::computeServerFilter( QString &errorMsg )
{
  errorMsg.clear();
  mClientSideFilterExpression = mURI.filter();
  mServerFilter.clear();
  if ( mClientSideFilterExpression.isEmpty() )
  {
    mFilterTranslationState = QgsOapifFilterTranslationState::FULLY_SERVER;
    return true;
  }

  const QgsExpression expr( mClientSideFilterExpression );
  bool ret = computeFilter( expr, mFilterTranslationState, mServerFilter, mClientSideFilterExpression );
  if ( ret )
  {
    if ( mFilterTranslationState == QgsOapifFilterTranslationState::PARTIAL )
    {
      QgsDebugMsgLevel( u"Part of the filter will be evaluated on client-side: %1"_s.arg( mClientSideFilterExpression ), 2 );
    }
    else if ( mFilterTranslationState == QgsOapifFilterTranslationState::FULLY_CLIENT )
    {
      QgsDebugMsgLevel( u"Whole filter will be evaluated on client-side"_s, 2 );
    }
  }
  return ret;
}

QString QgsOapifSharedData::computedExpression( const QgsExpression &expression ) const
{
  if ( !expression.isValid() )
    return QString();
  QgsOapifFilterTranslationState translationState;
  QString serverParameters;
  QString clientSideFilterExpression;
  computeFilter( expression, translationState, serverParameters, clientSideFilterExpression );
  return serverParameters;
}

void QgsOapifSharedData::pushError( const QString &errorMsg ) const
{
  QgsMessageLog::logMessage( errorMsg, tr( "OAPIF" ) );
  emit raiseError( errorMsg );
}
