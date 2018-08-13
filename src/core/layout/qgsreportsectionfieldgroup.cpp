/***************************************************************************
                             qgsreportsectionfieldgroup.cpp
                             --------------------
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

#include "qgsreportsectionfieldgroup.h"
#include "qgslayout.h"

///@cond NOT_STABLE

QgsReportSectionFieldGroup::QgsReportSectionFieldGroup( QgsAbstractReportSection *parent )
  : QgsAbstractReportSection( parent )
{

}

QString QgsReportSectionFieldGroup::description() const
{
  if ( mCoverageLayer.get() )
    return QObject::tr( "Group: %1 - %2" ).arg( mCoverageLayer->name(), mField );
  else
    return QObject::tr( "Group" );
}

QIcon QgsReportSectionFieldGroup::icon() const
{
  return QgsApplication::getThemeIcon( QStringLiteral( "/mIconFieldText.svg" ) );
}

QgsReportSectionFieldGroup *QgsReportSectionFieldGroup::clone() const
{
  std::unique_ptr< QgsReportSectionFieldGroup > copy = qgis::make_unique< QgsReportSectionFieldGroup >( nullptr );
  copyCommonProperties( copy.get() );

  if ( mBody )
  {
    copy->mBody.reset( mBody->clone() );
  }
  else
    copy->mBody.reset();

  copy->setLayer( mCoverageLayer.get() );
  copy->setField( mField );
  copy->setSortAscending( mSortAscending );
  copy->setBodyEnabled( mBodyEnabled );

  return copy.release();
}

bool QgsReportSectionFieldGroup::beginRender()
{
  if ( !mCoverageLayer.get() )
    return false;

  if ( !mField.isEmpty() )
  {
    mFieldIndex = mCoverageLayer->fields().lookupField( mField );
    if ( mFieldIndex < 0 )
      return false;

    if ( mBody )
      mBody->reportContext().setLayer( mCoverageLayer.get() );

    mFeatures = QgsFeatureIterator();
  }
  return QgsAbstractReportSection::beginRender();
}

bool QgsReportSectionFieldGroup::prepareHeader()
{
  if ( !header() )
    return false;

  if ( !mFeatures.isValid() )
  {
    mFeatures = mCoverageLayer->getFeatures( buildFeatureRequest() );
  }

  mHeaderFeature = getNextFeature();
  header()->reportContext().blockSignals( true );
  header()->reportContext().setLayer( mCoverageLayer.get() );
  header()->reportContext().blockSignals( false );
  header()->reportContext().setFeature( mHeaderFeature );
  mSkipNextRequest = true;
  mNoFeatures = !mHeaderFeature.isValid();
  return mHeaderVisibility == AlwaysInclude || !mNoFeatures;
}

bool QgsReportSectionFieldGroup::prepareFooter()
{
  return mFooterVisibility == AlwaysInclude || !mNoFeatures;
}

QgsLayout *QgsReportSectionFieldGroup::nextBody( bool &ok )
{
  if ( !mFeatures.isValid() )
  {
    mFeatures = mCoverageLayer->getFeatures( buildFeatureRequest() );
  }

  QgsFeature f;
  if ( !mSkipNextRequest )
  {
    f = getNextFeature();
  }
  else
  {
    f = mHeaderFeature;
    mSkipNextRequest = false;
  }

  if ( !f.isValid() )
  {
    // no features left for this iteration
    mFeatures = QgsFeatureIterator();

    if ( footer() )
    {
      footer()->reportContext().blockSignals( true );
      footer()->reportContext().setLayer( mCoverageLayer.get() );
      footer()->reportContext().blockSignals( false );
      footer()->reportContext().setFeature( mLastFeature );
    }
    ok = false;
    return nullptr;
  }

  mLastFeature = f;

  updateChildContexts( f );

  ok = true;
  if ( mBody && mBodyEnabled )
  {
    mBody->reportContext().blockSignals( true );
    mBody->reportContext().setLayer( mCoverageLayer.get() );
    mBody->reportContext().blockSignals( false );
    mBody->reportContext().setFeature( f );
  }

  return mBodyEnabled ? mBody.get() : nullptr;
}

void QgsReportSectionFieldGroup::reset()
{
  QgsAbstractReportSection::reset();
  mEncounteredValues.clear();
  mSkipNextRequest = false;
  mHeaderFeature = QgsFeature();
  mLastFeature = QgsFeature();
  mFeatures = QgsFeatureIterator();
  mNoFeatures = false;
}

void QgsReportSectionFieldGroup::setParentSection( QgsAbstractReportSection *parent )
{
  QgsAbstractReportSection::setParentSection( parent );
  if ( !mCoverageLayer )
    mCoverageLayer.resolveWeakly( project() );
}

void QgsReportSectionFieldGroup::reloadSettings()
{
  QgsAbstractReportSection::reloadSettings();
  if ( mBody )
    mBody->reloadSettings();
}

bool QgsReportSectionFieldGroup::writePropertiesToElement( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  element.setAttribute( QStringLiteral( "headerVisibility" ), static_cast< int >( mHeaderVisibility ) );
  element.setAttribute( QStringLiteral( "footerVisibility" ), static_cast< int >( mFooterVisibility ) );
  element.setAttribute( QStringLiteral( "field" ), mField );
  element.setAttribute( QStringLiteral( "ascending" ), mSortAscending ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  element.setAttribute( QStringLiteral( "bodyEnabled" ), mBodyEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  if ( mCoverageLayer )
  {
    element.setAttribute( QStringLiteral( "coverageLayer" ), mCoverageLayer.layerId );
    element.setAttribute( QStringLiteral( "coverageLayerName" ), mCoverageLayer.name );
    element.setAttribute( QStringLiteral( "coverageLayerSource" ), mCoverageLayer.source );
    element.setAttribute( QStringLiteral( "coverageLayerProvider" ), mCoverageLayer.provider );
  }

  if ( mBody )
  {
    QDomElement bodyElement = doc.createElement( QStringLiteral( "body" ) );
    bodyElement.appendChild( mBody->writeXml( doc, context ) );
    element.appendChild( bodyElement );
  }
  return true;
}

bool QgsReportSectionFieldGroup::readPropertiesFromElement( const QDomElement &element, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  mHeaderVisibility = static_cast< SectionVisibility >( element.attribute( QStringLiteral( "headerVisibility" ) ).toInt() );
  mFooterVisibility = static_cast< SectionVisibility >( element.attribute( QStringLiteral( "footerVisibility" ) ).toInt() );
  mField = element.attribute( QStringLiteral( "field" ) );
  mSortAscending = element.attribute( QStringLiteral( "ascending" ) ).toInt();
  mBodyEnabled = element.attribute( QStringLiteral( "bodyEnabled" ) ).toInt();
  QString layerId = element.attribute( QStringLiteral( "coverageLayer" ) );
  QString layerName = element.attribute( QStringLiteral( "coverageLayerName" ) );
  QString layerSource = element.attribute( QStringLiteral( "coverageLayerSource" ) );
  QString layerProvider = element.attribute( QStringLiteral( "coverageLayerProvider" ) );
  mCoverageLayer = QgsVectorLayerRef( layerId, layerName, layerSource, layerProvider );
  mCoverageLayer.resolveWeakly( project() );

  const QDomElement bodyElement = element.firstChildElement( QStringLiteral( "body" ) );
  if ( !bodyElement.isNull() )
  {
    const QDomElement bodyLayoutElem = bodyElement.firstChild().toElement();
    std::unique_ptr< QgsLayout > body = qgis::make_unique< QgsLayout >( project() );
    body->readXml( bodyLayoutElem, doc, context );
    mBody = std::move( body );
  }
  return true;
}

bool QgsReportSectionFieldGroup::sortAscending() const
{
  return mSortAscending;
}

void QgsReportSectionFieldGroup::setSortAscending( bool sortAscending )
{
  mSortAscending = sortAscending;
}

QgsFeatureRequest QgsReportSectionFieldGroup::buildFeatureRequest() const
{
  QgsFeatureRequest request;
  QVariantMap filter = context().fieldFilters;

  QStringList filterParts;
  for ( auto filterIt = filter.constBegin(); filterIt != filter.constEnd(); ++filterIt )
  {
    // use lookupField since we don't want case sensitivity
    int fieldIndex = mCoverageLayer->fields().lookupField( filterIt.key() );
    if ( fieldIndex >= 0 )
    {
      // layer has a matching field, so we need to filter by it
      filterParts << QgsExpression::createFieldEqualityExpression( mCoverageLayer->fields().at( fieldIndex ).name(), filterIt.value() );
    }
  }
  if ( !filterParts.empty() )
  {
    QString filterString = QStringLiteral( "(%1)" ).arg( filterParts.join( QStringLiteral( ") AND (" ) ) );
    request.setFilterExpression( filterString );
  }

  request.addOrderBy( mField, mSortAscending );
  return request;
}

QgsFeature QgsReportSectionFieldGroup::getNextFeature()
{
  QgsFeature f;
  QVariant currentValue;
  bool first = true;
  while ( first || ( ( !mBody || !mBodyEnabled ) && mEncounteredValues.contains( currentValue ) ) )
  {
    if ( !mFeatures.nextFeature( f ) )
    {
      return QgsFeature();
    }

    first = false;
    currentValue = f.attribute( mFieldIndex );
  }

  mEncounteredValues.insert( currentValue );
  return f;
}

void QgsReportSectionFieldGroup::updateChildContexts( const QgsFeature &feature )
{
  QgsReportSectionContext c = context();
  c.feature = feature;
  if ( mCoverageLayer )
    c.currentLayer = mCoverageLayer.get();

  QVariantMap currentFilter = c.fieldFilters;
  currentFilter.insert( mField, feature.attribute( mFieldIndex ) );
  c.fieldFilters = currentFilter;

  const QList< QgsAbstractReportSection * > sections = childSections();
  for ( QgsAbstractReportSection *section : qgis::as_const( sections ) )
  {
    section->setContext( c );
  }
}

///@endcond

