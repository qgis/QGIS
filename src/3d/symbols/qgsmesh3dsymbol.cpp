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
  std::unique_ptr<QgsMesh3DSymbol> result = std::make_unique<QgsMesh3DSymbol>();

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
  QDomElement elemDataProperties = doc.createElement( QStringLiteral( "data" ) );
  elemDataProperties.setAttribute( QStringLiteral( "alt-clamping" ), Qgs3DUtils::altClampingToString( mAltClamping ) );
  elemDataProperties.setAttribute( QStringLiteral( "height" ), mHeight );
  elemDataProperties.setAttribute( QStringLiteral( "add-back-faces" ), mAddBackFaces ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elem.appendChild( elemDataProperties );

  QDomElement elemMaterial = doc.createElement( QStringLiteral( "material" ) );
  mMaterialSettings->writeXml( elemMaterial, context );
  elem.appendChild( elemMaterial );

  //Advanced symbol
  QDomElement elemAdvancedSettings = doc.createElement( QStringLiteral( "advanced-settings" ) );
  elemAdvancedSettings.setAttribute( QStringLiteral( "renderer-3d-enabled" ), mEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elemAdvancedSettings.setAttribute( QStringLiteral( "culling-mode" ), Qgs3DUtils::cullingModeToString( mCullingMode ) );
  elemAdvancedSettings.setAttribute( QStringLiteral( "smoothed-triangle" ), mSmoothedTriangles ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elemAdvancedSettings.setAttribute( QStringLiteral( "wireframe-enabled" ), mWireframeEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elemAdvancedSettings.setAttribute( QStringLiteral( "wireframe-line-width" ), mWireframeLineWidth );
  elemAdvancedSettings.setAttribute( QStringLiteral( "wireframe-line-color" ), QgsColorUtils::colorToString( mWireframeLineColor ) );
  elemAdvancedSettings.setAttribute( QStringLiteral( "level-of-detail" ), mLevelOfDetailIndex );
  elemAdvancedSettings.setAttribute( QStringLiteral( "vertical-scale" ), mVerticalScale );
  elemAdvancedSettings.setAttribute( QStringLiteral( "vertical-group-index" ), mVerticalDatasetGroupIndex );
  elemAdvancedSettings.setAttribute( QStringLiteral( "vertical-relative" ), mIsVerticalMagnitudeRelative ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elemAdvancedSettings.setAttribute( QStringLiteral( "texture-type" ), static_cast<int>( mRenderingStyle ) );
  elemAdvancedSettings.appendChild( mColorRampShader.writeXml( doc, context ) );
  elemAdvancedSettings.setAttribute( QStringLiteral( "min-color-ramp-shader" ), mColorRampShader.minimumValue() );
  elemAdvancedSettings.setAttribute( QStringLiteral( "max-color-ramp-shader" ), mColorRampShader.maximumValue() );
  elemAdvancedSettings.setAttribute( QStringLiteral( "texture-single-color" ), QgsColorUtils::colorToString( mSingleColor ) );
  elemAdvancedSettings.setAttribute( QStringLiteral( "arrows-enabled" ), mArrowsEnabled ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elemAdvancedSettings.setAttribute( QStringLiteral( "arrows-spacing" ), mArrowsSpacing );
  elemAdvancedSettings.setAttribute( QStringLiteral( "arrows-fixed-size" ), mArrowsFixedSize ? QStringLiteral( "1" ) : QStringLiteral( "0" ) );
  elem.appendChild( elemAdvancedSettings );

  QDomElement elemDDP = doc.createElement( QStringLiteral( "data-defined-properties" ) );
  mDataDefinedProperties.writeXml( elemDDP, propertyDefinitions() );
  elem.appendChild( elemDDP );
}

void QgsMesh3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  //Simple symbol
  const QDomElement elemDataProperties = elem.firstChildElement( QStringLiteral( "data" ) );
  mAltClamping = Qgs3DUtils::altClampingFromString( elemDataProperties.attribute( QStringLiteral( "alt-clamping" ) ) );
  mHeight = elemDataProperties.attribute( QStringLiteral( "height" ) ).toFloat();
  mAddBackFaces = elemDataProperties.attribute( QStringLiteral( "add-back-faces" ) ).toInt();

  const QDomElement elemMaterial = elem.firstChildElement( QStringLiteral( "material" ) );
  mMaterialSettings->readXml( elemMaterial, context );

  //Advanced symbol
  const QDomElement elemAdvancedSettings = elem.firstChildElement( QStringLiteral( "advanced-settings" ) );
  mEnabled = elemAdvancedSettings.attribute( QStringLiteral( "renderer-3d-enabled" ) ).toInt();
  mCullingMode = Qgs3DUtils::cullingModeFromString( elemAdvancedSettings.attribute( QStringLiteral( "culling-mode" ), QStringLiteral( "back" ) ) );
  mSmoothedTriangles = elemAdvancedSettings.attribute( QStringLiteral( "smoothed-triangle" ) ).toInt();
  mWireframeEnabled = elemAdvancedSettings.attribute( QStringLiteral( "wireframe-enabled" ) ).toInt();
  mWireframeLineWidth = elemAdvancedSettings.attribute( QStringLiteral( "wireframe-line-width" ) ).toDouble();
  mWireframeLineColor = QgsColorUtils::colorFromString( elemAdvancedSettings.attribute( QStringLiteral( "wireframe-line-color" ) ) );
  mLevelOfDetailIndex = elemAdvancedSettings.attribute( QStringLiteral( "level-of-detail" ) ).toInt();
  mVerticalScale = elemAdvancedSettings.attribute( "vertical-scale" ).toDouble();
  mVerticalDatasetGroupIndex = elemAdvancedSettings.attribute( "vertical-group-index" ).toInt();
  mIsVerticalMagnitudeRelative = elemAdvancedSettings.attribute( "vertical-relative" ).toInt();
  mRenderingStyle = static_cast<QgsMesh3DSymbol::RenderingStyle>( elemAdvancedSettings.attribute( QStringLiteral( "texture-type" ) ).toInt() );
  mColorRampShader.readXml( elemAdvancedSettings.firstChildElement( "colorrampshader" ), context );
  mColorRampShader.setMinimumValue( elemAdvancedSettings.attribute( QStringLiteral( "min-color-ramp-shader" ) ).toDouble() );
  mColorRampShader.setMaximumValue( elemAdvancedSettings.attribute( QStringLiteral( "max-color-ramp-shader" ) ).toDouble() );
  mSingleColor = QgsColorUtils::colorFromString( elemAdvancedSettings.attribute( QStringLiteral( "texture-single-color" ) ) );
  mArrowsEnabled = elemAdvancedSettings.attribute( QStringLiteral( "arrows-enabled" ) ).toInt();
  if ( elemAdvancedSettings.hasAttribute( QStringLiteral( "arrows-spacing" ) ) )
    mArrowsSpacing = elemAdvancedSettings.attribute( QStringLiteral( "arrows-spacing" ) ).toDouble();
  mArrowsFixedSize = elemAdvancedSettings.attribute( QStringLiteral( "arrows-fixed-size" ) ).toInt();
  const QDomElement elemDDP = elem.firstChildElement( QStringLiteral( "data-defined-properties" ) );
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
