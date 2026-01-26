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

double QgsMeshRendererScalarSettings::classificationMinimum() const { return mClassificationMinimum; }

double QgsMeshRendererScalarSettings::classificationMaximum() const { return mClassificationMaximum; }

void QgsMeshRendererScalarSettings::setClassificationMinimumMaximum( double minimum, double maximum )
{
  mClassificationMinimum = minimum;
  mClassificationMaximum = maximum;
  updateShader();
}

double QgsMeshRendererScalarSettings::opacity() const { return mOpacity; }

void QgsMeshRendererScalarSettings::setOpacity( double opacity ) { mOpacity = opacity; }

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
  mEdgeStrokeWidthUnit = static_cast<Qgis::RenderUnit>(
                           elemEdge.attribute( u"stroke-width-unit"_s ).toInt() );
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

double QgsMeshRendererVectorSettings::lineWidth() const
{
  return mLineWidth;
}

void QgsMeshRendererVectorSettings::setLineWidth( double lineWidth )
{
  mLineWidth = lineWidth;
}

QColor QgsMeshRendererVectorSettings::color() const
{
  return mColor;
}

void QgsMeshRendererVectorSettings::setColor( const QColor &vectorColor )
{
  mColor = vectorColor;
}

double QgsMeshRendererVectorSettings::filterMin() const
{
  return mFilterMin;
}

void QgsMeshRendererVectorSettings::setFilterMin( double vectorFilterMin )
{
  mFilterMin = vectorFilterMin;
}

double QgsMeshRendererVectorSettings::filterMax() const
{
  return mFilterMax;
}

void QgsMeshRendererVectorSettings::setFilterMax( double vectorFilterMax )
{
  mFilterMax = vectorFilterMax;
}

bool QgsMeshRendererVectorSettings::isOnUserDefinedGrid() const
{
  return mOnUserDefinedGrid;
}

void QgsMeshRendererVectorSettings::setOnUserDefinedGrid( bool enabled )
{
  mOnUserDefinedGrid = enabled;
}

int QgsMeshRendererVectorSettings::userGridCellWidth() const
{
  return mUserGridCellWidth;
}

void QgsMeshRendererVectorSettings::setUserGridCellWidth( int width )
{
  mUserGridCellWidth = width;
}

int QgsMeshRendererVectorSettings::userGridCellHeight() const
{
  return mUserGridCellHeight;
}

void QgsMeshRendererVectorSettings::setUserGridCellHeight( int height )
{
  mUserGridCellHeight = height;
}

QgsMeshRendererVectorArrowSettings::ArrowScalingMethod QgsMeshRendererVectorArrowSettings::shaftLengthMethod() const
{
  return mShaftLengthMethod;
}

void QgsMeshRendererVectorArrowSettings::setShaftLengthMethod( QgsMeshRendererVectorArrowSettings::ArrowScalingMethod shaftLengthMethod )
{
  mShaftLengthMethod = shaftLengthMethod;
}

double QgsMeshRendererVectorArrowSettings::minShaftLength() const
{
  return mMinShaftLength;
}

void QgsMeshRendererVectorArrowSettings::setMinShaftLength( double minShaftLength )
{
  mMinShaftLength = minShaftLength;
}

double QgsMeshRendererVectorArrowSettings::maxShaftLength() const
{
  return mMaxShaftLength;
}

void QgsMeshRendererVectorArrowSettings::setMaxShaftLength( double maxShaftLength )
{
  mMaxShaftLength = maxShaftLength;
}

double QgsMeshRendererVectorArrowSettings::scaleFactor() const
{
  return mScaleFactor;
}

void QgsMeshRendererVectorArrowSettings::setScaleFactor( double scaleFactor )
{
  mScaleFactor = scaleFactor;
}

double QgsMeshRendererVectorArrowSettings::fixedShaftLength() const
{
  return mFixedShaftLength;
}

void QgsMeshRendererVectorArrowSettings::setFixedShaftLength( double fixedShaftLength )
{
  mFixedShaftLength = fixedShaftLength;
}

double QgsMeshRendererVectorArrowSettings::arrowHeadWidthRatio() const
{
  return mArrowHeadWidthRatio;
}

void QgsMeshRendererVectorArrowSettings::setArrowHeadWidthRatio( double vectorHeadWidthRatio )
{
  mArrowHeadWidthRatio = vectorHeadWidthRatio;
}

double QgsMeshRendererVectorArrowSettings::arrowHeadLengthRatio() const
{
  return mArrowHeadLengthRatio;
}

void QgsMeshRendererVectorArrowSettings::setArrowHeadLengthRatio( double vectorHeadLengthRatio )
{
  mArrowHeadLengthRatio = vectorHeadLengthRatio;
}

QDomElement QgsMeshRendererVectorArrowSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( u"vector-arrow-settings"_s );
  elem.setAttribute( u"arrow-head-width-ratio"_s, mArrowHeadWidthRatio );
  elem.setAttribute( u"arrow-head-length-ratio"_s, mArrowHeadLengthRatio );

  QDomElement elemShaft = doc.createElement( u"shaft-length"_s );
  QString methodTxt;
  switch ( mShaftLengthMethod )
  {
    case MinMax:
      methodTxt = u"minmax"_s;
      elemShaft.setAttribute( u"min"_s, mMinShaftLength );
      elemShaft.setAttribute( u"max"_s, mMaxShaftLength );
      break;
    case Scaled:
      methodTxt = u"scaled"_s;
      elemShaft.setAttribute( u"scale-factor"_s, mScaleFactor );
      break;
    case Fixed:
      methodTxt = u"fixed"_s ;
      elemShaft.setAttribute( u"fixed-length"_s, mFixedShaftLength );
      break;
  }
  elemShaft.setAttribute( u"method"_s, methodTxt );
  elem.appendChild( elemShaft );
  return elem;
}

void QgsMeshRendererVectorArrowSettings::readXml( const QDomElement &elem )
{
  mArrowHeadWidthRatio = elem.attribute( u"arrow-head-width-ratio"_s ).toDouble();
  mArrowHeadLengthRatio = elem.attribute( u"arrow-head-length-ratio"_s ).toDouble();

  const QDomElement elemShaft = elem.firstChildElement( u"shaft-length"_s );
  const QString methodTxt = elemShaft.attribute( u"method"_s );
  if ( u"minmax"_s == methodTxt )
  {
    mShaftLengthMethod = MinMax;
    mMinShaftLength = elemShaft.attribute( u"min"_s ).toDouble();
    mMaxShaftLength = elemShaft.attribute( u"max"_s ).toDouble();
  }
  else if ( u"scaled"_s == methodTxt )
  {
    mShaftLengthMethod = Scaled;
    mScaleFactor = elemShaft.attribute( u"scale-factor"_s ).toDouble();
  }
  else  // fixed
  {
    mShaftLengthMethod = Fixed;
    mFixedShaftLength = elemShaft.attribute( u"fixed-length"_s ).toDouble();
  }
}

// ---------------------------------------------------------------------

QgsMeshRendererSettings::QgsMeshRendererSettings()
  : mAveragingMethod( new QgsMeshSigmaAveragingMethod() )
{
}

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
{
}

QgsMeshRendererSettings::QgsMeshRendererSettings( QgsMeshRendererSettings &&other )
  : mRendererNativeMeshSettings( std::move( other.mRendererNativeMeshSettings ) )
  , mRendererTriangularMeshSettings( std::move( other.mRendererTriangularMeshSettings ) )
  , mRendererEdgeMeshSettings( std::move( other.mRendererEdgeMeshSettings ) )
  , mRendererScalarSettings( std::move( other.mRendererScalarSettings ) )
  , mRendererVectorSettings( std::move( other.mRendererVectorSettings ) )
  , mActiveScalarDatasetGroup( other.mActiveScalarDatasetGroup )
  , mActiveVectorDatasetGroup( other.mActiveVectorDatasetGroup )
  , mAveragingMethod( std::move( other.mAveragingMethod ) )
{
}

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
    const QgsMeshRendererVectorSettings &vectorSettings = mRendererVectorSettings[*groupIndex];
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
    elemAveraging.setAttribute( u"method"_s, QString::number( mAveragingMethod->method() ) ) ;
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
    QgsMeshRendererVectorSettings vectorSettings;
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

QgsMeshRendererVectorStreamlineSettings::SeedingStartPointsMethod QgsMeshRendererVectorStreamlineSettings::seedingMethod() const
{
  return mSeedingMethod;
}

void QgsMeshRendererVectorStreamlineSettings::setSeedingMethod( const SeedingStartPointsMethod &seedingMethod )
{
  mSeedingMethod = seedingMethod;
}

double QgsMeshRendererVectorStreamlineSettings::seedingDensity() const
{
  return mSeedingDensity;
}

void QgsMeshRendererVectorStreamlineSettings::setSeedingDensity( double seedingDensity )
{
  mSeedingDensity = seedingDensity;
}

QDomElement QgsMeshRendererVectorStreamlineSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( u"vector-streamline-settings"_s );

  elem.setAttribute( u"seeding-method"_s, mSeedingMethod );
  elem.setAttribute( u"seeding-density"_s, mSeedingDensity );

  return elem;
}

void QgsMeshRendererVectorStreamlineSettings::readXml( const QDomElement &elem )
{
  mSeedingMethod =
    static_cast<QgsMeshRendererVectorStreamlineSettings::SeedingStartPointsMethod>(
      elem.attribute( u"seeding-method"_s ).toInt() );
  mSeedingDensity = elem.attribute( u"seeding-density"_s ).toDouble();
}

QgsMeshRendererVectorSettings::Symbology QgsMeshRendererVectorSettings::symbology() const
{
  return mDisplayingMethod;
}

void QgsMeshRendererVectorSettings::setSymbology( const Symbology &displayingMethod )
{
  mDisplayingMethod = displayingMethod;
}

QgsMeshRendererVectorArrowSettings QgsMeshRendererVectorSettings::arrowSettings() const
{
  return mArrowsSettings;
}

void QgsMeshRendererVectorSettings::setArrowsSettings( const QgsMeshRendererVectorArrowSettings &arrowSettings )
{
  mArrowsSettings = arrowSettings;
}

QgsMeshRendererVectorStreamlineSettings QgsMeshRendererVectorSettings::streamLinesSettings() const
{
  return mStreamLinesSettings;
}

void QgsMeshRendererVectorSettings::setStreamLinesSettings( const QgsMeshRendererVectorStreamlineSettings &streamLinesSettings )
{
  mStreamLinesSettings = streamLinesSettings;
}

QDomElement QgsMeshRendererVectorSettings::writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  QDomElement elem = doc.createElement( u"vector-settings"_s );
  elem.setAttribute( u"symbology"_s, mDisplayingMethod );

  elem.setAttribute( u"line-width"_s, mLineWidth );
  elem.setAttribute( u"coloring-method"_s, coloringMethod() );
  elem.setAttribute( u"color"_s, QgsColorUtils::colorToString( mColor ) );
  const QDomElement elemShader = mColorRampShader.writeXml( doc, context );
  elem.appendChild( elemShader );
  elem.setAttribute( u"filter-min"_s, mFilterMin );
  elem.setAttribute( u"filter-max"_s, mFilterMax );

  elem.setAttribute( u"user-grid-enabled"_s, mOnUserDefinedGrid ? u"1"_s : u"0"_s );
  elem.setAttribute( u"user-grid-width"_s, mUserGridCellWidth );
  elem.setAttribute( u"user-grid-height"_s, mUserGridCellHeight );

  elem.appendChild( mArrowsSettings.writeXml( doc ) );
  elem.appendChild( mStreamLinesSettings.writeXml( doc ) );
  elem.appendChild( mTracesSettings.writeXml( doc ) );
  elem.appendChild( mWindBarbSettings.writeXml( doc ) );

  return elem;
}

void QgsMeshRendererVectorSettings::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  mDisplayingMethod = static_cast<QgsMeshRendererVectorSettings::Symbology>(
                        elem.attribute( u"symbology"_s ).toInt() );

  mLineWidth = elem.attribute( u"line-width"_s ).toDouble();
  mColoringMethod = static_cast<QgsInterpolatedLineColor::ColoringMethod>(
                      elem.attribute( u"coloring-method"_s ).toInt() );
  mColor = QgsColorUtils::colorFromString( elem.attribute( u"color"_s ) );
  mColorRampShader.readXml( elem.firstChildElement( "colorrampshader" ), context );
  mFilterMin = elem.attribute( u"filter-min"_s ).toDouble();
  mFilterMax = elem.attribute( u"filter-max"_s ).toDouble();

  mOnUserDefinedGrid = elem.attribute( u"user-grid-enabled"_s ).toInt(); //bool
  mUserGridCellWidth = elem.attribute( u"user-grid-width"_s ).toInt();
  mUserGridCellHeight = elem.attribute( u"user-grid-height"_s ).toInt();

  const QDomElement elemVector = elem.firstChildElement( u"vector-arrow-settings"_s );
  if ( ! elemVector.isNull() )
    mArrowsSettings.readXml( elemVector );

  const QDomElement elemStreamLine = elem.firstChildElement( u"vector-streamline-settings"_s );
  if ( ! elemStreamLine.isNull() )
    mStreamLinesSettings.readXml( elemStreamLine );

  const QDomElement elemTraces = elem.firstChildElement( u"vector-traces-settings"_s );
  if ( ! elemTraces.isNull() )
    mTracesSettings.readXml( elemTraces );

  const QDomElement elemWindBarb = elem.firstChildElement( u"vector-windbarb-settings"_s );
  if ( ! elemWindBarb.isNull() )
    mWindBarbSettings.readXml( elemWindBarb );
}

QgsInterpolatedLineColor::ColoringMethod QgsMeshRendererVectorSettings::coloringMethod() const
{
  return mColoringMethod;
}

void QgsMeshRendererVectorSettings::setColoringMethod( const QgsInterpolatedLineColor::ColoringMethod &coloringMethod )
{
  mColoringMethod = coloringMethod;
}

QgsColorRampShader QgsMeshRendererVectorSettings::colorRampShader() const
{
  return mColorRampShader;
}

void QgsMeshRendererVectorSettings::setColorRampShader( const QgsColorRampShader &colorRampShader )
{
  mColorRampShader = colorRampShader;
}

QgsInterpolatedLineColor QgsMeshRendererVectorSettings::vectorStrokeColoring() const
{
  QgsInterpolatedLineColor strokeColoring;
  switch ( mColoringMethod )
  {
    case QgsInterpolatedLineColor::SingleColor:
      strokeColoring = QgsInterpolatedLineColor( mColor );
      break;
    case QgsInterpolatedLineColor::ColorRamp:
      strokeColoring = QgsInterpolatedLineColor( mColorRampShader );
      break;
  }

  return strokeColoring;
}

QgsMeshRendererVectorTracesSettings QgsMeshRendererVectorSettings::tracesSettings() const
{
  return mTracesSettings;
}

void QgsMeshRendererVectorSettings::setTracesSettings( const QgsMeshRendererVectorTracesSettings &tracesSettings )
{
  mTracesSettings = tracesSettings;
}

void QgsMeshRendererVectorTracesSettings::readXml( const QDomElement &elem )
{
  mMaximumTailLength = elem.attribute( u"maximum-tail-length"_s ).toInt();
  mMaximumTailLengthUnit = static_cast<Qgis::RenderUnit>(
                             elem.attribute( u"maximum-tail-length-unit"_s ).toInt() );
  mParticlesCount = elem.attribute( u"particles-count"_s ).toInt();
}

QDomElement QgsMeshRendererVectorTracesSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( u"vector-traces-settings"_s );
  elem.setAttribute( u"maximum-tail-length"_s, mMaximumTailLength );
  elem.setAttribute( u"maximum-tail-length-unit"_s, static_cast< int >( mMaximumTailLengthUnit ) );
  elem.setAttribute( u"particles-count"_s, mParticlesCount );

  return elem;
}

Qgis::RenderUnit QgsMeshRendererVectorTracesSettings::maximumTailLengthUnit() const
{
  return mMaximumTailLengthUnit;
}

void QgsMeshRendererVectorTracesSettings::setMaximumTailLengthUnit( Qgis::RenderUnit maximumTailLengthUnit )
{
  mMaximumTailLengthUnit = maximumTailLengthUnit;
}

double QgsMeshRendererVectorTracesSettings::maximumTailLength() const
{
  return mMaximumTailLength;
}

void QgsMeshRendererVectorTracesSettings::setMaximumTailLength( double maximumTailLength )
{
  mMaximumTailLength = maximumTailLength;
}

int QgsMeshRendererVectorTracesSettings::particlesCount() const
{
  return mParticlesCount;
}

void QgsMeshRendererVectorTracesSettings::setParticlesCount( int value )
{
  mParticlesCount = value;
}

bool QgsMeshRendererSettings::hasSettings( int datasetGroupIndex ) const
{
  return mRendererScalarSettings.contains( datasetGroupIndex ) || mRendererVectorSettings.contains( datasetGroupIndex );
}

QgsMeshRendererVectorWindBarbSettings QgsMeshRendererVectorSettings::windBarbSettings() const
{
  return mWindBarbSettings;
}

void QgsMeshRendererVectorSettings::setWindBarbSettings( const QgsMeshRendererVectorWindBarbSettings &windBarbSettings )
{
  mWindBarbSettings = windBarbSettings;
}

void QgsMeshRendererVectorWindBarbSettings::readXml( const QDomElement &elem )
{
  mShaftLength = elem.attribute( u"shaft-length"_s, u"10"_s ).toDouble();
  mShaftLengthUnits = static_cast<Qgis::RenderUnit>(
                        elem.attribute( u"shaft-length-units"_s ).toInt() );
  mMagnitudeMultiplier = elem.attribute( u"magnitude-multiplier"_s, u"1"_s ).toDouble();
  mMagnitudeUnits = static_cast<WindSpeedUnit>(
                      elem.attribute( u"magnitude-units"_s, u"0"_s ).toInt() );
}

QDomElement QgsMeshRendererVectorWindBarbSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( u"vector-windbarb-settings"_s );
  elem.setAttribute( u"shaft-length"_s, mShaftLength );
  elem.setAttribute( u"shaft-length-units"_s, static_cast< int >( mShaftLengthUnits ) );
  elem.setAttribute( u"magnitude-multiplier"_s, mMagnitudeMultiplier );
  elem.setAttribute( u"magnitude-units"_s, static_cast< int >( mMagnitudeUnits ) );
  return elem;
}

double QgsMeshRendererVectorWindBarbSettings::magnitudeMultiplier() const
{
  switch ( mMagnitudeUnits )
  {
    case QgsMeshRendererVectorWindBarbSettings::WindSpeedUnit::Knots:
      return 1.0;
    case QgsMeshRendererVectorWindBarbSettings::WindSpeedUnit::MetersPerSecond:
      return 3600.0 / 1852.0;
    case QgsMeshRendererVectorWindBarbSettings::WindSpeedUnit::KilometersPerHour:
      return 1.0 / 1.852;
    case QgsMeshRendererVectorWindBarbSettings::WindSpeedUnit::MilesPerHour:
      return 1.609344 / 1.852;
    case QgsMeshRendererVectorWindBarbSettings::WindSpeedUnit::FeetPerSecond:
      return 3600.0 / 1.852 / 5280.0 * 1.609344 ;
    case QgsMeshRendererVectorWindBarbSettings::WindSpeedUnit::OtherUnit:
      return mMagnitudeMultiplier;
  }
  return 1.0; // should not reach
}

void QgsMeshRendererVectorWindBarbSettings::setMagnitudeMultiplier( double magnitudeMultiplier )
{
  mMagnitudeMultiplier = magnitudeMultiplier;
}

double QgsMeshRendererVectorWindBarbSettings::shaftLength() const
{
  return mShaftLength;
}

void QgsMeshRendererVectorWindBarbSettings::setShaftLength( double shaftLength )
{
  mShaftLength = shaftLength;
}

Qgis::RenderUnit QgsMeshRendererVectorWindBarbSettings::shaftLengthUnits() const
{
  return mShaftLengthUnits;
}

void QgsMeshRendererVectorWindBarbSettings::setShaftLengthUnits( Qgis::RenderUnit shaftLengthUnit )
{
  mShaftLengthUnits = shaftLengthUnit;
}

QgsMeshRendererVectorWindBarbSettings::WindSpeedUnit QgsMeshRendererVectorWindBarbSettings::magnitudeUnits() const
{
  return mMagnitudeUnits;
}

void QgsMeshRendererVectorWindBarbSettings::setMagnitudeUnits( WindSpeedUnit units )
{
  mMagnitudeUnits = units;
}
