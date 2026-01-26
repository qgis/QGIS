/***************************************************************************
  qgsmesh3dsymbol.cpp
  -------------------
  Date                 : January 2019
  Copyright            : (C) 2019 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmesh3dsymbol.h"

#include "qgs3dtypes.h"
#include "qgs3dutils.h"
#include "qgscolorutils.h"
#include "qgsphongmaterialsettings.h"

QgsMesh3DSymbol::QgsMesh3DSymbol()
  : mMaterialSettings( std::make_unique<QgsPhongMaterialSettings>() )
{
}

QgsMesh3DSymbol::~QgsMesh3DSymbol() = default;

QgsMesh3DSymbol *QgsMesh3DSymbol::clone() const
{
  auto result = std::make_unique<QgsMesh3DSymbol>();

  result->mAltClamping = mAltClamping;
  result->mHeight = mHeight;
  result->mMaterialSettings.reset( mMaterialSettings->clone() );
  result->mAddBackFaces = mAddBackFaces;
  result->mCullingMode = mCullingMode;
  result->mEnabled = mEnabled;
  result->mSmoothedTriangles = mSmoothedTriangles;
  result->mWireframeEnabled = mWireframeEnabled;
  result->mWireframeLineWidth = mWireframeLineWidth;
  result->mWireframeLineColor = mWireframeLineColor;
  result->mLevelOfDetailIndex = mLevelOfDetailIndex;
  result->mVerticalScale = mVerticalScale;
  result->mVerticalDatasetGroupIndex = mVerticalDatasetGroupIndex;
  result->mIsVerticalMagnitudeRelative = mIsVerticalMagnitudeRelative;
  result->mRenderingStyle = mRenderingStyle;
  result->mColorRampShader = mColorRampShader;
  result->mSingleColor = mSingleColor;
  result->mArrowsEnabled = mArrowsEnabled;
  result->mArrowsSpacing = mArrowsSpacing;
  result->mArrowsFixedSize = mArrowsFixedSize;
  result->mArrowsColor = mArrowsColor;
  result->mMaximumTextureSize = mMaximumTextureSize;
  copyBaseSettings( result.get() );
  return result.release();
}

void QgsMesh3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  QDomDocument doc = elem.ownerDocument();

  //Simple symbol
  QDomElement elemDataProperties = doc.createElement( u"data"_s );
  elemDataProperties.setAttribute( u"alt-clamping"_s, Qgs3DUtils::altClampingToString( mAltClamping ) );
  elemDataProperties.setAttribute( u"height"_s, mHeight );
  elemDataProperties.setAttribute( u"add-back-faces"_s, mAddBackFaces ? u"1"_s : u"0"_s );
  elem.appendChild( elemDataProperties );

  QDomElement elemMaterial = doc.createElement( u"material"_s );
  mMaterialSettings->writeXml( elemMaterial, context );
  elem.appendChild( elemMaterial );

  //Advanced symbol
  QDomElement elemAdvancedSettings = doc.createElement( u"advanced-settings"_s );
  elemAdvancedSettings.setAttribute( u"renderer-3d-enabled"_s, mEnabled ? u"1"_s : u"0"_s );
  elemAdvancedSettings.setAttribute( u"culling-mode"_s, Qgs3DUtils::cullingModeToString( mCullingMode ) );
  elemAdvancedSettings.setAttribute( u"smoothed-triangle"_s, mSmoothedTriangles ? u"1"_s : u"0"_s );
  elemAdvancedSettings.setAttribute( u"wireframe-enabled"_s, mWireframeEnabled ? u"1"_s : u"0"_s );
  elemAdvancedSettings.setAttribute( u"wireframe-line-width"_s, mWireframeLineWidth );
  elemAdvancedSettings.setAttribute( u"wireframe-line-color"_s, QgsColorUtils::colorToString( mWireframeLineColor ) );
  elemAdvancedSettings.setAttribute( u"level-of-detail"_s, mLevelOfDetailIndex );
  elemAdvancedSettings.setAttribute( u"vertical-scale"_s, mVerticalScale );
  elemAdvancedSettings.setAttribute( u"vertical-group-index"_s, mVerticalDatasetGroupIndex );
  elemAdvancedSettings.setAttribute( u"vertical-relative"_s, mIsVerticalMagnitudeRelative ? u"1"_s : u"0"_s );
  elemAdvancedSettings.setAttribute( u"texture-type"_s, static_cast<int>( mRenderingStyle ) );
  elemAdvancedSettings.appendChild( mColorRampShader.writeXml( doc, context ) );
  elemAdvancedSettings.setAttribute( u"min-color-ramp-shader"_s, mColorRampShader.minimumValue() );
  elemAdvancedSettings.setAttribute( u"max-color-ramp-shader"_s, mColorRampShader.maximumValue() );
  elemAdvancedSettings.setAttribute( u"texture-single-color"_s, QgsColorUtils::colorToString( mSingleColor ) );
  elemAdvancedSettings.setAttribute( u"arrows-enabled"_s, mArrowsEnabled ? u"1"_s : u"0"_s );
  elemAdvancedSettings.setAttribute( u"arrows-spacing"_s, mArrowsSpacing );
  elemAdvancedSettings.setAttribute( u"arrows-fixed-size"_s, mArrowsFixedSize ? u"1"_s : u"0"_s );
  elem.appendChild( elemAdvancedSettings );

  QDomElement elemDDP = doc.createElement( u"data-defined-properties"_s );
  mDataDefinedProperties.writeXml( elemDDP, propertyDefinitions() );
  elem.appendChild( elemDDP );
}

void QgsMesh3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  //Simple symbol
  const QDomElement elemDataProperties = elem.firstChildElement( u"data"_s );
  mAltClamping = Qgs3DUtils::altClampingFromString( elemDataProperties.attribute( u"alt-clamping"_s ) );
  mHeight = elemDataProperties.attribute( u"height"_s ).toFloat();
  mAddBackFaces = elemDataProperties.attribute( u"add-back-faces"_s ).toInt();

  const QDomElement elemMaterial = elem.firstChildElement( u"material"_s );
  mMaterialSettings->readXml( elemMaterial, context );

  //Advanced symbol
  const QDomElement elemAdvancedSettings = elem.firstChildElement( u"advanced-settings"_s );
  mEnabled = elemAdvancedSettings.attribute( u"renderer-3d-enabled"_s ).toInt();
  mCullingMode = Qgs3DUtils::cullingModeFromString( elemAdvancedSettings.attribute( u"culling-mode"_s, u"back"_s ) );
  mSmoothedTriangles = elemAdvancedSettings.attribute( u"smoothed-triangle"_s ).toInt();
  mWireframeEnabled = elemAdvancedSettings.attribute( u"wireframe-enabled"_s ).toInt();
  mWireframeLineWidth = elemAdvancedSettings.attribute( u"wireframe-line-width"_s ).toDouble();
  mWireframeLineColor = QgsColorUtils::colorFromString( elemAdvancedSettings.attribute( u"wireframe-line-color"_s ) );
  mLevelOfDetailIndex = elemAdvancedSettings.attribute( u"level-of-detail"_s ).toInt();
  mVerticalScale = elemAdvancedSettings.attribute( "vertical-scale" ).toDouble();
  mVerticalDatasetGroupIndex = elemAdvancedSettings.attribute( "vertical-group-index" ).toInt();
  mIsVerticalMagnitudeRelative = elemAdvancedSettings.attribute( "vertical-relative" ).toInt();
  mRenderingStyle = static_cast<QgsMesh3DSymbol::RenderingStyle>( elemAdvancedSettings.attribute( u"texture-type"_s ).toInt() );
  mColorRampShader.readXml( elemAdvancedSettings.firstChildElement( "colorrampshader" ), context );
  mColorRampShader.setMinimumValue( elemAdvancedSettings.attribute( u"min-color-ramp-shader"_s ).toDouble() );
  mColorRampShader.setMaximumValue( elemAdvancedSettings.attribute( u"max-color-ramp-shader"_s ).toDouble() );
  mSingleColor = QgsColorUtils::colorFromString( elemAdvancedSettings.attribute( u"texture-single-color"_s ) );
  mArrowsEnabled = elemAdvancedSettings.attribute( u"arrows-enabled"_s ).toInt();
  if ( elemAdvancedSettings.hasAttribute( u"arrows-spacing"_s ) )
    mArrowsSpacing = elemAdvancedSettings.attribute( u"arrows-spacing"_s ).toDouble();
  mArrowsFixedSize = elemAdvancedSettings.attribute( u"arrows-fixed-size"_s ).toInt();
  const QDomElement elemDDP = elem.firstChildElement( u"data-defined-properties"_s );
  if ( !elemDDP.isNull() )
    mDataDefinedProperties.readXml( elemDDP, propertyDefinitions() );
}

bool QgsMesh3DSymbol::smoothedTriangles() const
{
  return mSmoothedTriangles;
}

void QgsMesh3DSymbol::setSmoothedTriangles( bool smoothTriangles )
{
  mSmoothedTriangles = smoothTriangles;
}

bool QgsMesh3DSymbol::wireframeEnabled() const
{
  return mWireframeEnabled;
}

void QgsMesh3DSymbol::setWireframeEnabled( bool wireframeEnabled )
{
  mWireframeEnabled = wireframeEnabled;
}

double QgsMesh3DSymbol::wireframeLineWidth() const
{
  return mWireframeLineWidth;
}

void QgsMesh3DSymbol::setWireframeLineWidth( double wireframeLineWidth )
{
  mWireframeLineWidth = wireframeLineWidth;
}

QColor QgsMesh3DSymbol::wireframeLineColor() const
{
  return mWireframeLineColor;
}

void QgsMesh3DSymbol::setWireframeLineColor( const QColor &wireframeLineColor )
{
  mWireframeLineColor = wireframeLineColor;
}

double QgsMesh3DSymbol::verticalScale() const
{
  return mVerticalScale;
}

void QgsMesh3DSymbol::setVerticalScale( double verticalScale )
{
  mVerticalScale = verticalScale;
}

QgsColorRampShader QgsMesh3DSymbol::colorRampShader() const
{
  return mColorRampShader;
}

void QgsMesh3DSymbol::setColorRampShader( const QgsColorRampShader &colorRampShader )
{
  mColorRampShader = colorRampShader;
}

QColor QgsMesh3DSymbol::singleMeshColor() const
{
  return mSingleColor;
}

void QgsMesh3DSymbol::setSingleMeshColor( const QColor &color )
{
  mSingleColor = color;
}

QgsMesh3DSymbol::RenderingStyle QgsMesh3DSymbol::renderingStyle() const
{
  return mRenderingStyle;
}

void QgsMesh3DSymbol::setRenderingStyle( const QgsMesh3DSymbol::RenderingStyle &coloringType )
{
  mRenderingStyle = coloringType;
}

int QgsMesh3DSymbol::verticalDatasetGroupIndex() const
{
  return mVerticalDatasetGroupIndex;
}

void QgsMesh3DSymbol::setVerticalDatasetGroupIndex( int verticalDatasetGroupIndex )
{
  mVerticalDatasetGroupIndex = verticalDatasetGroupIndex;
}

bool QgsMesh3DSymbol::isVerticalMagnitudeRelative() const
{
  return mIsVerticalMagnitudeRelative;
}

void QgsMesh3DSymbol::setIsVerticalMagnitudeRelative( bool isVerticalScaleIsRelative )
{
  mIsVerticalMagnitudeRelative = isVerticalScaleIsRelative;
}

bool QgsMesh3DSymbol::arrowsEnabled() const
{
  return mArrowsEnabled;
}

void QgsMesh3DSymbol::setArrowsEnabled( bool vectorEnabled )
{
  mArrowsEnabled = vectorEnabled;
}

double QgsMesh3DSymbol::arrowsSpacing() const
{
  return mArrowsSpacing;
}

void QgsMesh3DSymbol::setArrowsSpacing( double arrowsSpacing )
{
  mArrowsSpacing = arrowsSpacing;
}

int QgsMesh3DSymbol::maximumTextureSize() const
{
  return mMaximumTextureSize;
}

void QgsMesh3DSymbol::setMaximumTextureSize( int maximumTextureSize )
{
  mMaximumTextureSize = maximumTextureSize;
}

bool QgsMesh3DSymbol::arrowsFixedSize() const
{
  return mArrowsFixedSize;
}

void QgsMesh3DSymbol::setArrowsFixedSize( bool arrowsFixeSize )
{
  mArrowsFixedSize = arrowsFixeSize;
}

int QgsMesh3DSymbol::levelOfDetailIndex() const
{
  return mLevelOfDetailIndex;
}

void QgsMesh3DSymbol::setLevelOfDetailIndex( int lod )
{
  mLevelOfDetailIndex = lod;
}

bool QgsMesh3DSymbol::operator==( const QgsMesh3DSymbol &other ) const
{
  if ( mAltClamping != other.mAltClamping
       || mHeight != other.mHeight
       || mAddBackFaces != other.mAddBackFaces
       || mEnabled != other.mEnabled
       || mCullingMode != other.mCullingMode
       || mSmoothedTriangles != other.mSmoothedTriangles
       || mWireframeEnabled != other.mWireframeEnabled
       || !qgsDoubleNear( mWireframeLineWidth, other.mWireframeLineWidth )
       || mWireframeLineColor != other.mWireframeLineColor
       || mLevelOfDetailIndex != other.mLevelOfDetailIndex
       || !qgsDoubleNear( mVerticalScale, other.mVerticalScale )
       || mVerticalDatasetGroupIndex != other.mVerticalDatasetGroupIndex
       || mIsVerticalMagnitudeRelative != other.mIsVerticalMagnitudeRelative
       || mRenderingStyle != other.mRenderingStyle
       || mColorRampShader != other.mColorRampShader
       || mSingleColor != other.mSingleColor
       || mArrowsEnabled != other.mArrowsEnabled
       || !qgsDoubleNear( mArrowsSpacing, other.mArrowsSpacing )
       || mArrowsFixedSize != other.mArrowsFixedSize
       || mArrowsColor != other.mArrowsColor
       || mMaximumTextureSize != other.mMaximumTextureSize )
    return false;

  if ( !mMaterialSettings->equals( other.materialSettings() ) )
    return false;

  // base class properties
  if ( mDataDefinedProperties != other.mDataDefinedProperties )
    return false;

  return true;
}

bool QgsMesh3DSymbol::operator!=( const QgsMesh3DSymbol &other ) const
{
  return !( *this == other );
}

bool QgsMesh3DSymbol::isEnabled() const
{
  return mEnabled;
}

void QgsMesh3DSymbol::setEnabled( bool enabled )
{
  mEnabled = enabled;
}

Qgs3DTypes::CullingMode QgsMesh3DSymbol::cullingMode() const
{
  return mCullingMode;
}

void QgsMesh3DSymbol::setCullingMode( const Qgs3DTypes::CullingMode &mode )
{
  mCullingMode = mode;
}

QgsAbstractMaterialSettings *QgsMesh3DSymbol::materialSettings() const
{
  return mMaterialSettings.get();
}

void QgsMesh3DSymbol::setMaterialSettings( QgsAbstractMaterialSettings *materialSettings )
{
  if ( materialSettings == mMaterialSettings.get() )
    return;

  mMaterialSettings.reset( materialSettings );
}
