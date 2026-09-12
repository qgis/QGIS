/***************************************************************************
                         qgsmeshrenderersettings.cpp
                         ---------------------------
    begin                : May 2018
    copyright            : (C) 2018 by Peter Petrik
    email                : zilolv at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmeshrenderersettings.h"

#include "qgscolorramp.h"
#include "qgscolorutils.h"
#include "qgsunittypes.h"

#include <QString>

using namespace Qt::StringLiterals;

bool QgsMeshRendererMeshSettings::isEnabled() const
{
  return mEnabled;
}

void QgsMeshRendererMeshSettings::setEnabled( bool on )
{
  mEnabled = on;
}

double QgsMeshRendererMeshSettings::lineWidth() const
{
  return mLineWidth;
}

void QgsMeshRendererMeshSettings::setLineWidth( double lineWidth )
{
  mLineWidth = lineWidth;
}

QColor QgsMeshRendererMeshSettings::color() const
{
  return mColor;
}

void QgsMeshRendererMeshSettings::setColor( const QColor &color )
{
  mColor = color;
}

Qgis::RenderUnit QgsMeshRendererMeshSettings::lineWidthUnit() const
{
  return mLineWidthUnit;
}

void QgsMeshRendererMeshSettings::setLineWidthUnit( Qgis::RenderUnit lineWidthUnit )
{
  mLineWidthUnit = lineWidthUnit;
}

QDomElement QgsMeshRendererMeshSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( u"mesh-settings"_s );
  elem.setAttribute( u"enabled"_s, mEnabled ? u"1"_s : u"0"_s );
  elem.setAttribute( u"line-width"_s, mLineWidth );
  elem.setAttribute( u"color"_s, QgsColorUtils::colorToString( mColor ) );
  elem.setAttribute( u"line-width-unit"_s, QgsUnitTypes::encodeUnit( mLineWidthUnit ) );
  return elem;
}

void QgsMeshRendererMeshSettings::readXml( const QDomElement &elem )
{
  mEnabled = elem.attribute( u"enabled"_s ).toInt();
  mLineWidth = elem.attribute( u"line-width"_s ).toDouble();
  mColor = QgsColorUtils::colorFromString( elem.attribute( u"color"_s ) );
  mLineWidthUnit = QgsUnitTypes::decodeRenderUnit( elem.attribute( u"line-width-unit"_s ) );
}

// ---------------------------------------------------------------------

QgsColorRampShader QgsMeshRendererScalarSettings::colorRampShader() const
{
  return mColorRampShader;
}

void QgsMeshRendererScalarSettings::setColorRampShader( const QgsColorRampShader &shader )
{
  mColorRampShader = shader;
}

double QgsMeshRendererScalarSettings::classificationMinimum() const
{
  return mClassificationMinimum;
}

double QgsMeshRendererScalarSettings::classificationMaximum() const
{
  return mClassificationMaximum;
}

void QgsMeshRendererScalarSettings::setClassificationMinimumMaximum( double minimum, double maximum )
{
  mClassificationMinimum = minimum;
  mClassificationMaximum = maximum;
  updateShader();
}

double QgsMeshRendererScalarSettings::opacity() const
{
  return mOpacity;
}

void QgsMeshRendererScalarSettings::setOpacity( double opacity )
{
  mOpacity = opacity;
}

QgsMeshRendererScalarSettings::DataResamplingMethod QgsMeshRendererScalarSettings::dataResamplingMethod() const
{
  return mDataResamplingMethod;
}

void QgsMeshRendererScalarSettings::setDataResamplingMethod( const QgsMeshRendererScalarSettings::DataResamplingMethod &dataInterpolationMethod )
{
  mDataResamplingMethod = dataInterpolationMethod;
}

QDomElement QgsMeshRendererScalarSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( u"scalar-settings"_s );
  elem.setAttribute( u"min-val"_s, mClassificationMinimum );
  elem.setAttribute( u"max-val"_s, mClassificationMaximum );
  elem.setAttribute( u"opacity"_s, mOpacity );

  QString methodTxt;
  switch ( mDataResamplingMethod )
  {
    case NoResampling:
      methodTxt = u"no-resampling"_s;
      break;
    case NeighbourAverage:
      methodTxt = u"neighbour-average"_s;
      break;
  }
  elem.setAttribute( u"interpolation-method"_s, methodTxt );

  if ( mRangeExtent != Qgis::MeshRangeExtent::WholeMesh )
    elem.setAttribute( u"range-extent"_s, qgsEnumValueToKey( mRangeExtent ) );
  if ( mRangeLimit != Qgis::MeshRangeLimit::NotSet )
    elem.setAttribute( u"range-limit"_s, qgsEnumValueToKey( mRangeLimit ) );

  const QDomElement elemShader = mColorRampShader.writeXml( doc, context );
  elem.appendChild( elemShader );

  QDomElement elemEdge = doc.createElement( u"edge-settings"_s );
  elemEdge.appendChild( mEdgeStrokeWidth.writeXml( doc, context ) );
  elemEdge.setAttribute( u"stroke-width-unit"_s, static_cast< int >( mEdgeStrokeWidthUnit ) );
  elem.appendChild( elemEdge );

  return elem;
}

void QgsMeshRendererScalarSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mClassificationMinimum = elem.attribute( u"min-val"_s ).toDouble();
  mClassificationMaximum = elem.attribute( u"max-val"_s ).toDouble();
  mOpacity = elem.attribute( u"opacity"_s ).toDouble();

  const QString methodTxt = elem.attribute( u"interpolation-method"_s );
  if ( u"neighbour-average"_s == methodTxt )
  {
    mDataResamplingMethod = DataResamplingMethod::NeighbourAverage;
  }
  else
  {
    mDataResamplingMethod = DataResamplingMethod::NoResampling;
  }

  mRangeExtent = qgsEnumKeyToValue( elem.attribute( "range-extent" ), Qgis::MeshRangeExtent::WholeMesh );
  mRangeLimit = qgsEnumKeyToValue( elem.attribute( "range-limit" ), Qgis::MeshRangeLimit::NotSet );

  const QDomElement elemShader = elem.firstChildElement( u"colorrampshader"_s );
  mColorRampShader.readXml( elemShader, context );

  const QDomElement elemEdge = elem.firstChildElement( u"edge-settings"_s );
  const QDomElement elemEdgeStrokeWidth = elemEdge.firstChildElement( u"mesh-stroke-width"_s );
  mEdgeStrokeWidth.readXml( elemEdgeStrokeWidth, context );
  mEdgeStrokeWidthUnit = static_cast<Qgis::RenderUnit>( elemEdge.attribute( u"stroke-width-unit"_s ).toInt() );
}

QgsInterpolatedLineWidth QgsMeshRendererScalarSettings::edgeStrokeWidth() const
{
  return mEdgeStrokeWidth;
}

void QgsMeshRendererScalarSettings::setEdgeStrokeWidth( const QgsInterpolatedLineWidth &strokeWidth )
{
  mEdgeStrokeWidth = strokeWidth;
}

Qgis::RenderUnit QgsMeshRendererScalarSettings::edgeStrokeWidthUnit() const
{
  return mEdgeStrokeWidthUnit;
}

void QgsMeshRendererScalarSettings::setEdgeStrokeWidthUnit( Qgis::RenderUnit edgeStrokeWidthUnit )
{
  mEdgeStrokeWidthUnit = edgeStrokeWidthUnit;
}

void QgsMeshRendererScalarSettings::updateShader()
{
  mColorRampShader.setMinimumValue( mClassificationMinimum );
  mColorRampShader.setMaximumValue( mClassificationMaximum );

  if ( !mColorRampShader.isEmpty() )
    mColorRampShader.classifyColorRamp( mColorRampShader.sourceColorRamp()->count(), 1, QgsRectangle(), nullptr );
}

// ---------------------------------------------------------------------

QgsMeshRendererSettings::QgsMeshRendererSettings()
  : mAveragingMethod( new QgsMeshSigmaAveragingMethod() )
{}

QgsMeshRendererSettings::QgsMeshRendererSettings( const QgsMeshRendererSettings &other )
  //****** IMPORTANT! editing this? make sure you update the move constructor too! *****
  : mRendererNativeMeshSettings( other.mRendererNativeMeshSettings )
  , mRendererTriangularMeshSettings( other.mRendererTriangularMeshSettings )
  , mRendererEdgeMeshSettings( other.mRendererEdgeMeshSettings )
  , mRendererScalarSettings( other.mRendererScalarSettings )
  , mRendererVectorSettings( other.mRendererVectorSettings )
  , mActiveScalarDatasetGroup( other.mActiveScalarDatasetGroup )
  , mActiveVectorDatasetGroup( other.mActiveVectorDatasetGroup )
  , mAveragingMethod( other.mAveragingMethod )
//****** IMPORTANT! editing this? make sure you update the move constructor too! *****
{}

QgsMeshRendererSettings::QgsMeshRendererSettings( QgsMeshRendererSettings &&other )
  : mRendererNativeMeshSettings( std::move( other.mRendererNativeMeshSettings ) )
  , mRendererTriangularMeshSettings( std::move( other.mRendererTriangularMeshSettings ) )
  , mRendererEdgeMeshSettings( std::move( other.mRendererEdgeMeshSettings ) )
  , mRendererScalarSettings( std::move( other.mRendererScalarSettings ) )
  , mRendererVectorSettings( std::move( other.mRendererVectorSettings ) )
  , mActiveScalarDatasetGroup( other.mActiveScalarDatasetGroup )
  , mActiveVectorDatasetGroup( other.mActiveVectorDatasetGroup )
  , mAveragingMethod( std::move( other.mAveragingMethod ) )
{}

QgsMeshRendererSettings &QgsMeshRendererSettings::operator=( const QgsMeshRendererSettings &other )
{
  if ( &other == this )
    return *this;

  //****** IMPORTANT! editing this? make sure you update the move assignment operator too! *****
  mRendererNativeMeshSettings = other.mRendererNativeMeshSettings;
  mRendererTriangularMeshSettings = other.mRendererTriangularMeshSettings;
  mRendererEdgeMeshSettings = other.mRendererEdgeMeshSettings;
  mRendererScalarSettings = other.mRendererScalarSettings;
  mRendererVectorSettings = other.mRendererVectorSettings;
  mActiveScalarDatasetGroup = other.mActiveScalarDatasetGroup;
  mActiveVectorDatasetGroup = other.mActiveVectorDatasetGroup;
  mAveragingMethod = other.mAveragingMethod;
  //****** IMPORTANT! editing this? make sure you update the move assignment operator too! *****
  return *this;
}

QgsMeshRendererSettings &QgsMeshRendererSettings::operator=( QgsMeshRendererSettings &&other )
{
  if ( &other == this )
    return *this;

  mRendererNativeMeshSettings = std::move( other.mRendererNativeMeshSettings );
  mRendererTriangularMeshSettings = std::move( other.mRendererTriangularMeshSettings );
  mRendererEdgeMeshSettings = std::move( other.mRendererEdgeMeshSettings );
  mRendererScalarSettings = std::move( other.mRendererScalarSettings );
  mRendererVectorSettings = std::move( other.mRendererVectorSettings );
  mActiveScalarDatasetGroup = other.mActiveScalarDatasetGroup;
  mActiveVectorDatasetGroup = other.mActiveVectorDatasetGroup;
  mAveragingMethod = std::move( other.mAveragingMethod );
  return *this;
}

QgsMeshRendererSettings::~QgsMeshRendererSettings() = default;

QgsMesh3DAveragingMethod *QgsMeshRendererSettings::averagingMethod() const
{
  return mAveragingMethod.get();
}

void QgsMeshRendererSettings::setAveragingMethod( QgsMesh3DAveragingMethod *method )
{
  if ( method )
    mAveragingMethod.reset( method->clone() );
  else
    mAveragingMethod.reset();
}

QDomElement QgsMeshRendererSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( u"mesh-renderer-settings"_s );

  QDomElement elemActiveDatasetGroup = doc.createElement( u"active-dataset-group"_s );
  elemActiveDatasetGroup.setAttribute( u"scalar"_s, mActiveScalarDatasetGroup );
  elemActiveDatasetGroup.setAttribute( u"vector"_s, mActiveVectorDatasetGroup );
  elem.appendChild( elemActiveDatasetGroup );

  for ( auto groupIndex = mRendererScalarSettings.keyBegin(); groupIndex != mRendererScalarSettings.keyEnd(); groupIndex++ )
  {
    const QgsMeshRendererScalarSettings &scalarSettings = mRendererScalarSettings[*groupIndex];
    QDomElement elemScalar = scalarSettings.writeXml( doc, context );
    elemScalar.setAttribute( u"group"_s, *groupIndex );
    elem.appendChild( elemScalar );
  }

  for ( auto groupIndex = mRendererVectorSettings.keyBegin(); groupIndex != mRendererVectorSettings.keyEnd(); groupIndex++ )
  {
    const QgsVectorFieldSettings &vectorSettings = mRendererVectorSettings[*groupIndex];
    QDomElement elemVector = vectorSettings.writeXml( doc, context );
    elemVector.setAttribute( u"group"_s, *groupIndex );
    elem.appendChild( elemVector );
  }

  QDomElement elemNativeMesh = mRendererNativeMeshSettings.writeXml( doc );
  elemNativeMesh.setTagName( u"mesh-settings-native"_s );
  elem.appendChild( elemNativeMesh );

  QDomElement elemEdgeMesh = mRendererEdgeMeshSettings.writeXml( doc );
  elemEdgeMesh.setTagName( u"mesh-settings-edge"_s );
  elem.appendChild( elemEdgeMesh );

  QDomElement elemTriangularMesh = mRendererTriangularMeshSettings.writeXml( doc );
  elemTriangularMesh.setTagName( u"mesh-settings-triangular"_s );
  elem.appendChild( elemTriangularMesh );

  if ( mAveragingMethod )
  {
    QDomElement elemAveraging = doc.createElement( u"averaging-3d"_s );
    elemAveraging.setAttribute( u"method"_s, QString::number( mAveragingMethod->method() ) );
    const QDomElement elemAveragingParams = mAveragingMethod->writeXml( doc );
    elemAveraging.appendChild( elemAveragingParams );
    elem.appendChild( elemAveraging );
  }

  return elem;
}

void QgsMeshRendererSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mRendererScalarSettings.clear();
  mRendererVectorSettings.clear();
  mAveragingMethod.reset();

  const QDomElement elemActiveDataset = elem.firstChildElement( u"active-dataset-group"_s );
  if ( elemActiveDataset.hasAttribute( u"scalar"_s ) )
    mActiveScalarDatasetGroup = elemActiveDataset.attribute( u"scalar"_s ).toInt();

  if ( elemActiveDataset.hasAttribute( u"vector"_s ) )
    mActiveVectorDatasetGroup = elemActiveDataset.attribute( u"vector"_s ).toInt();

  QDomElement elemScalar = elem.firstChildElement( u"scalar-settings"_s );
  while ( !elemScalar.isNull() )
  {
    const int groupIndex = elemScalar.attribute( u"group"_s ).toInt();
    QgsMeshRendererScalarSettings scalarSettings;
    scalarSettings.readXml( elemScalar, context );
    mRendererScalarSettings.insert( groupIndex, scalarSettings );

    elemScalar = elemScalar.nextSiblingElement( u"scalar-settings"_s );
  }

  QDomElement elemVector = elem.firstChildElement( u"vector-settings"_s );
  while ( !elemVector.isNull() )
  {
    const int groupIndex = elemVector.attribute( u"group"_s ).toInt();
    QgsVectorFieldSettings vectorSettings;
    vectorSettings.readXml( elemVector, context );
    mRendererVectorSettings.insert( groupIndex, vectorSettings );

    elemVector = elemVector.nextSiblingElement( u"vector-settings"_s );
  }

  const QDomElement elemNativeMesh = elem.firstChildElement( u"mesh-settings-native"_s );
  mRendererNativeMeshSettings.readXml( elemNativeMesh );

  const QDomElement elemEdgeMesh = elem.firstChildElement( u"mesh-settings-edge"_s );
  mRendererEdgeMeshSettings.readXml( elemEdgeMesh );

  const QDomElement elemTriangularMesh = elem.firstChildElement( u"mesh-settings-triangular"_s );
  mRendererTriangularMeshSettings.readXml( elemTriangularMesh );

  const QDomElement elemAveraging = elem.firstChildElement( u"averaging-3d"_s );
  if ( !elemAveraging.isNull() )
  {
    mAveragingMethod.reset( QgsMesh3DAveragingMethod::createFromXml( elemAveraging ) );
  }
}

int QgsMeshRendererSettings::activeScalarDatasetGroup() const
{
  return mActiveScalarDatasetGroup;
}

void QgsMeshRendererSettings::setActiveScalarDatasetGroup( int activeScalarDatasetGroup )
{
  mActiveScalarDatasetGroup = activeScalarDatasetGroup;
}

int QgsMeshRendererSettings::activeVectorDatasetGroup() const
{
  return mActiveVectorDatasetGroup;
}

void QgsMeshRendererSettings::setActiveVectorDatasetGroup( int activeVectorDatasetGroup )
{
  mActiveVectorDatasetGroup = activeVectorDatasetGroup;
}

bool QgsMeshRendererSettings::hasSettings( int datasetGroupIndex ) const
{
  return mRendererScalarSettings.contains( datasetGroupIndex ) || mRendererVectorSettings.contains( datasetGroupIndex );
}
