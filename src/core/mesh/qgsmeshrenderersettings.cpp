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
#include "qgssymbollayerutils.h"
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

QgsUnitTypes::RenderUnit QgsMeshRendererMeshSettings::lineWidthUnit() const
{
  return mLineWidthUnit;
}

void QgsMeshRendererMeshSettings::setLineWidthUnit( const QgsUnitTypes::RenderUnit &lineWidthUnit )
{
  mLineWidthUnit = lineWidthUnit;
}

QDomElement QgsMeshRendererMeshSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "mesh-settings" ) );
  elem.setAttribute( QStringLiteral( "enabled" ), mEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elem.setAttribute( QStringLiteral( "line-width" ), mLineWidth );
  elem.setAttribute( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( mColor ) );
  elem.setAttribute( QStringLiteral( "line-width-unit" ), QgsUnitTypes::encodeUnit( mLineWidthUnit ) );
  return elem;
}

void QgsMeshRendererMeshSettings::readXml( const QDomElement &elem )
{
  mEnabled = elem.attribute( QStringLiteral( "enabled" ) ).toInt();
  mLineWidth = elem.attribute( QStringLiteral( "line-width" ) ).toDouble();
  mColor = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "color" ) ) );
  mLineWidthUnit = QgsUnitTypes::decodeRenderUnit( elem.attribute( QStringLiteral( "line-width-unit" ) ) );
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

QDomElement QgsMeshRendererScalarSettings::writeXml( QDomDocument &doc ) const
{
  QgsReadWriteContext readWriteContext;
  QDomElement elem = doc.createElement( QStringLiteral( "scalar-settings" ) );
  elem.setAttribute( QStringLiteral( "min-val" ), mClassificationMinimum );
  elem.setAttribute( QStringLiteral( "max-val" ), mClassificationMaximum );
  elem.setAttribute( QStringLiteral( "opacity" ), mOpacity );

  QString methodTxt;
  switch ( mDataResamplingMethod )
  {
    case None:
      methodTxt = QStringLiteral( "none" );
      break;
    case NeighbourAverage:
      methodTxt = QStringLiteral( "neighbour-average" );
      break;
  }
  elem.setAttribute( QStringLiteral( "interpolation-method" ), methodTxt );
  QDomElement elemShader = mColorRampShader.writeXml( doc );
  elem.appendChild( elemShader );

  QDomElement elemEdge = doc.createElement( QStringLiteral( "edge-settings" ) );
  elemEdge.appendChild( mEdgeStrokeWidth.writeXml( doc, readWriteContext ) );
  elemEdge.setAttribute( QStringLiteral( "stroke-width-unit" ), mEdgeStrokeWidthUnit );
  elem.appendChild( elemEdge );

  return elem;
}

void QgsMeshRendererScalarSettings::readXml( const QDomElement &elem )
{
  QgsReadWriteContext readWriteContext;
  mClassificationMinimum = elem.attribute( QStringLiteral( "min-val" ) ).toDouble();
  mClassificationMaximum = elem.attribute( QStringLiteral( "max-val" ) ).toDouble();
  mOpacity = elem.attribute( QStringLiteral( "opacity" ) ).toDouble();

  QString methodTxt = elem.attribute( QStringLiteral( "interpolation-method" ) );
  if ( QStringLiteral( "neighbour-average" ) == methodTxt )
  {
    mDataResamplingMethod = DataResamplingMethod::NeighbourAverage;
  }
  else
  {
    mDataResamplingMethod = DataResamplingMethod::None;
  }
  QDomElement elemShader = elem.firstChildElement( QStringLiteral( "colorrampshader" ) );
  mColorRampShader.readXml( elemShader );

  QDomElement elemEdge = elem.firstChildElement( QStringLiteral( "edge-settings" ) );
  QDomElement elemEdgeStrokeWidth = elemEdge.firstChildElement( QStringLiteral( "mesh-stroke-width" ) );
  mEdgeStrokeWidth.readXml( elemEdgeStrokeWidth, readWriteContext );
  mEdgeStrokeWidthUnit = static_cast<QgsUnitTypes::RenderUnit>(
                           elemEdge.attribute( QStringLiteral( "stroke-width-unit" ) ).toInt() );
}

QgsInterpolatedLineWidth QgsMeshRendererScalarSettings::edgeStrokeWidth() const
{
  return mEdgeStrokeWidth;
}

void QgsMeshRendererScalarSettings::setEdgeStrokeWidth( const QgsInterpolatedLineWidth &strokeWidth )
{
  mEdgeStrokeWidth = strokeWidth;
}

QgsUnitTypes::RenderUnit QgsMeshRendererScalarSettings::edgeStrokeWidthUnit() const
{
  return mEdgeStrokeWidthUnit;
}

void QgsMeshRendererScalarSettings::setEdgeStrokeWidthUnit( const QgsUnitTypes::RenderUnit &edgeStrokeWidthUnit )
{
  mEdgeStrokeWidthUnit = edgeStrokeWidthUnit;
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
  QDomElement elem = doc.createElement( QStringLiteral( "vector-arrow-settings" ) );
  elem.setAttribute( QStringLiteral( "arrow-head-width-ratio" ), mArrowHeadWidthRatio );
  elem.setAttribute( QStringLiteral( "arrow-head-length-ratio" ), mArrowHeadLengthRatio );

  QDomElement elemShaft = doc.createElement( QStringLiteral( "shaft-length" ) );
  QString methodTxt;
  switch ( mShaftLengthMethod )
  {
    case MinMax:
      methodTxt = QStringLiteral( "minmax" );
      elemShaft.setAttribute( QStringLiteral( "min" ), mMinShaftLength );
      elemShaft.setAttribute( QStringLiteral( "max" ), mMaxShaftLength );
      break;
    case Scaled:
      methodTxt = QStringLiteral( "scaled" );
      elemShaft.setAttribute( QStringLiteral( "scale-factor" ), mScaleFactor );
      break;
    case Fixed:
      methodTxt = QStringLiteral( "fixed" ) ;
      elemShaft.setAttribute( QStringLiteral( "fixed-length" ), mFixedShaftLength );
      break;
  }
  elemShaft.setAttribute( QStringLiteral( "method" ), methodTxt );
  elem.appendChild( elemShaft );
  return elem;
}

void QgsMeshRendererVectorArrowSettings::readXml( const QDomElement &elem )
{
  mArrowHeadWidthRatio = elem.attribute( QStringLiteral( "arrow-head-width-ratio" ) ).toDouble();
  mArrowHeadLengthRatio = elem.attribute( QStringLiteral( "arrow-head-length-ratio" ) ).toDouble();

  QDomElement elemShaft = elem.firstChildElement( QStringLiteral( "shaft-length" ) );
  QString methodTxt = elemShaft.attribute( QStringLiteral( "method" ) );
  if ( QStringLiteral( "minmax" ) == methodTxt )
  {
    mShaftLengthMethod = MinMax;
    mMinShaftLength = elemShaft.attribute( QStringLiteral( "min" ) ).toDouble();
    mMaxShaftLength = elemShaft.attribute( QStringLiteral( "max" ) ).toDouble();
  }
  else if ( QStringLiteral( "scaled" ) == methodTxt )
  {
    mShaftLengthMethod = Scaled;
    mScaleFactor = elemShaft.attribute( QStringLiteral( "scale-factor" ) ).toDouble();
  }
  else  // fixed
  {
    mShaftLengthMethod = Fixed;
    mFixedShaftLength = elemShaft.attribute( QStringLiteral( "fixed-length" ) ).toDouble();
  }
}

// ---------------------------------------------------------------------

QgsMeshRendererSettings::QgsMeshRendererSettings()
  : mAveragingMethod( new QgsMeshMultiLevelsAveragingMethod() )
{
}

QgsMeshRendererSettings::~QgsMeshRendererSettings() = default;

QgsMesh3dAveragingMethod *QgsMeshRendererSettings::averagingMethod() const
{
  return mAveragingMethod.get();
}

void QgsMeshRendererSettings::setAveragingMethod( QgsMesh3dAveragingMethod *method )
{
  if ( method )
    mAveragingMethod.reset( method->clone() );
  else
    mAveragingMethod.reset();
}

QDomElement QgsMeshRendererSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "mesh-renderer-settings" ) );

  QDomElement elemActiveDatasetGroup = doc.createElement( QStringLiteral( "active-dataset-group" ) );
  elemActiveDatasetGroup.setAttribute( QStringLiteral( "scalar" ), mActiveScalarDatasetGroup );
  elemActiveDatasetGroup.setAttribute( QStringLiteral( "vector" ), mActiveVectorDatasetGroup );
  elem.appendChild( elemActiveDatasetGroup );

  for ( int groupIndex : mRendererScalarSettings.keys() )
  {
    const QgsMeshRendererScalarSettings &scalarSettings = mRendererScalarSettings[groupIndex];
    QDomElement elemScalar = scalarSettings.writeXml( doc );
    elemScalar.setAttribute( QStringLiteral( "group" ), groupIndex );
    elem.appendChild( elemScalar );
  }

  for ( int groupIndex : mRendererVectorSettings.keys() )
  {
    const QgsMeshRendererVectorSettings &vectorSettings = mRendererVectorSettings[groupIndex];
    QDomElement elemVector = vectorSettings.writeXml( doc );
    elemVector.setAttribute( QStringLiteral( "group" ), groupIndex );
    elem.appendChild( elemVector );
  }

  QDomElement elemNativeMesh = mRendererNativeMeshSettings.writeXml( doc );
  elemNativeMesh.setTagName( QStringLiteral( "mesh-settings-native" ) );
  elem.appendChild( elemNativeMesh );

  QDomElement elemEdgeMesh = mRendererEdgeMeshSettings.writeXml( doc );
  elemEdgeMesh.setTagName( QStringLiteral( "mesh-settings-edge" ) );
  elem.appendChild( elemEdgeMesh );

  QDomElement elemTriangularMesh = mRendererTriangularMeshSettings.writeXml( doc );
  elemTriangularMesh.setTagName( QStringLiteral( "mesh-settings-triangular" ) );
  elem.appendChild( elemTriangularMesh );

  if ( mAveragingMethod )
  {
    QDomElement elemAveraging = doc.createElement( QStringLiteral( "averaging-3d" ) );
    elemAveraging.setAttribute( QStringLiteral( "method" ), QStringLiteral( "%1" ).arg( mAveragingMethod->method() ) ) ;
    QDomElement elemAveragingParams = mAveragingMethod->writeXml( doc );
    elemAveraging.appendChild( elemAveragingParams );
    elem.appendChild( elemAveraging );
  }

  return elem;
}

void QgsMeshRendererSettings::readXml( const QDomElement &elem )
{
  mRendererScalarSettings.clear();
  mRendererVectorSettings.clear();
  mAveragingMethod.reset();

  QDomElement elemActiveDataset = elem.firstChildElement( QStringLiteral( "active-dataset-group" ) );
  if ( elemActiveDataset.hasAttribute( QStringLiteral( "scalar" ) ) )
    mActiveScalarDatasetGroup = elemActiveDataset.attribute( QStringLiteral( "scalar" ) ).toInt();

  if ( elemActiveDataset.hasAttribute( QStringLiteral( "vector" ) ) )
    mActiveVectorDatasetGroup = elemActiveDataset.attribute( QStringLiteral( "vector" ) ).toInt();

  QDomElement elemScalar = elem.firstChildElement( QStringLiteral( "scalar-settings" ) );
  while ( !elemScalar.isNull() )
  {
    int groupIndex = elemScalar.attribute( QStringLiteral( "group" ) ).toInt();
    QgsMeshRendererScalarSettings scalarSettings;
    scalarSettings.readXml( elemScalar );
    mRendererScalarSettings.insert( groupIndex, scalarSettings );

    elemScalar = elemScalar.nextSiblingElement( QStringLiteral( "scalar-settings" ) );
  }

  QDomElement elemVector = elem.firstChildElement( QStringLiteral( "vector-settings" ) );
  while ( !elemVector.isNull() )
  {
    int groupIndex = elemVector.attribute( QStringLiteral( "group" ) ).toInt();
    QgsMeshRendererVectorSettings vectorSettings;
    vectorSettings.readXml( elemVector );
    mRendererVectorSettings.insert( groupIndex, vectorSettings );

    elemVector = elemVector.nextSiblingElement( QStringLiteral( "vector-settings" ) );
  }

  QDomElement elemNativeMesh = elem.firstChildElement( QStringLiteral( "mesh-settings-native" ) );
  mRendererNativeMeshSettings.readXml( elemNativeMesh );

  QDomElement elemEdgeMesh = elem.firstChildElement( QStringLiteral( "mesh-settings-edge" ) );
  mRendererEdgeMeshSettings.readXml( elemEdgeMesh );

  QDomElement elemTriangularMesh = elem.firstChildElement( QStringLiteral( "mesh-settings-triangular" ) );
  mRendererTriangularMeshSettings.readXml( elemTriangularMesh );

  QDomElement elemAveraging = elem.firstChildElement( QStringLiteral( "averaging-3d" ) );
  if ( !elemAveraging.isNull() )
  {
    mAveragingMethod.reset( QgsMesh3dAveragingMethod::createFromXml( elemAveraging ) );
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
  QDomElement elem = doc.createElement( QStringLiteral( "vector-streamline-settings" ) );

  elem.setAttribute( QStringLiteral( "seeding-method" ), mSeedingMethod );
  elem.setAttribute( QStringLiteral( "seeding-density" ), mSeedingDensity );

  return elem;
}

void QgsMeshRendererVectorStreamlineSettings::readXml( const QDomElement &elem )
{
  mSeedingMethod =
    static_cast<QgsMeshRendererVectorStreamlineSettings::SeedingStartPointsMethod>(
      elem.attribute( QStringLiteral( "seeding-method" ) ).toInt() );
  mSeedingDensity = elem.attribute( QStringLiteral( "seeding-density" ) ).toDouble();
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

QDomElement QgsMeshRendererVectorSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "vector-settings" ) );
  elem.setAttribute( QStringLiteral( "symbology" ), mDisplayingMethod );

  elem.setAttribute( QStringLiteral( "line-width" ), mLineWidth );
  elem.setAttribute( QStringLiteral( "coloring-method" ), coloringMethod() );
  elem.setAttribute( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( mColor ) );
  QDomElement elemShader = mColorRampShader.writeXml( doc );
  elem.appendChild( elemShader );
  elem.setAttribute( QStringLiteral( "filter-min" ), mFilterMin );
  elem.setAttribute( QStringLiteral( "filter-max" ), mFilterMax );

  elem.setAttribute( QStringLiteral( "user-grid-enabled" ), mOnUserDefinedGrid ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elem.setAttribute( QStringLiteral( "user-grid-width" ), mUserGridCellWidth );
  elem.setAttribute( QStringLiteral( "user-grid-height" ), mUserGridCellHeight );

  elem.appendChild( mArrowsSettings.writeXml( doc ) );
  elem.appendChild( mStreamLinesSettings.writeXml( doc ) );
  elem.appendChild( mTracesSettings.writeXml( doc ) );

  return elem;
}

void QgsMeshRendererVectorSettings::readXml( const QDomElement &elem )
{
  mDisplayingMethod = static_cast<QgsMeshRendererVectorSettings::Symbology>(
                        elem.attribute( QStringLiteral( "symbology" ) ).toInt() );

  mLineWidth = elem.attribute( QStringLiteral( "line-width" ) ).toDouble();
  mColoringMethod = static_cast<QgsInterpolatedLineColor::ColoringMethod>(
                      elem.attribute( QStringLiteral( "coloring-method" ) ).toInt() );
  mColor = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "color" ) ) );
  mColorRampShader.readXml( elem.firstChildElement( "colorrampshader" ) );
  mFilterMin = elem.attribute( QStringLiteral( "filter-min" ) ).toDouble();
  mFilterMax = elem.attribute( QStringLiteral( "filter-max" ) ).toDouble();

  mOnUserDefinedGrid = elem.attribute( QStringLiteral( "user-grid-enabled" ) ).toInt(); //bool
  mUserGridCellWidth = elem.attribute( QStringLiteral( "user-grid-width" ) ).toInt();
  mUserGridCellHeight = elem.attribute( QStringLiteral( "user-grid-height" ) ).toInt();

  QDomElement elemVector = elem.firstChildElement( QStringLiteral( "vector-arrow-settings" ) );
  if ( ! elemVector.isNull() )
    mArrowsSettings.readXml( elemVector );

  QDomElement elemStreamLine = elem.firstChildElement( QStringLiteral( "vector-streamline-settings" ) );
  if ( ! elemStreamLine.isNull() )
    mStreamLinesSettings.readXml( elemStreamLine );

  QDomElement elemTraces = elem.firstChildElement( QStringLiteral( "vector-traces-settings" ) );
  if ( ! elemTraces.isNull() )
    mTracesSettings.readXml( elemTraces );
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
  mMaximumTailLength = elem.attribute( QStringLiteral( "maximum-tail-length" ) ).toInt();
  mMaximumTailLengthUnit = static_cast<QgsUnitTypes::RenderUnit>(
                             elem.attribute( QStringLiteral( "maximum-tail-length-unit" ) ).toInt() );
  mParticlesCount = elem.attribute( QStringLiteral( "particles-count" ) ).toInt();
}

QDomElement QgsMeshRendererVectorTracesSettings::writeXml( QDomDocument &doc ) const
{
  QDomElement elem = doc.createElement( QStringLiteral( "vector-traces-settings" ) );
  elem.setAttribute( QStringLiteral( "maximum-tail-length" ), mMaximumTailLength );
  elem.setAttribute( QStringLiteral( "maximum-tail-length-unit" ), mMaximumTailLengthUnit );
  elem.setAttribute( QStringLiteral( "particles-count" ), mParticlesCount );

  return elem;
}

QgsUnitTypes::RenderUnit QgsMeshRendererVectorTracesSettings::maximumTailLengthUnit() const
{
  return mMaximumTailLengthUnit;
}

void QgsMeshRendererVectorTracesSettings::setMaximumTailLengthUnit( const QgsUnitTypes::RenderUnit &maximumTailLengthUnit )
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

