/***************************************************************************
  qgspointcloud3dsymbol.h
  ------------------------------
  Date                 : November 2020
  Copyright            : (C) 2020 by Nedjima Belgacem
  Email                : belgacem dot nedjima at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloud3dsymbol.h"

#include "qgscolorramptexture.h"
#include "qgscolorutils.h"
#include "qgsmaterial.h"

#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTexture>

// QgsPointCloud3DSymbol


QgsPointCloud3DSymbol::QgsPointCloud3DSymbol()
  : QgsAbstract3DSymbol()
{
}

QgsPointCloud3DSymbol::~QgsPointCloud3DSymbol() {}

void QgsPointCloud3DSymbol::setPointSize( float size )
{
  mPointSize = size;
}

bool QgsPointCloud3DSymbol::renderAsTriangles() const
{
  return mRenderAsTriangles;
}

void QgsPointCloud3DSymbol::setRenderAsTriangles( bool asTriangles )
{
  mRenderAsTriangles = asTriangles;
}

bool QgsPointCloud3DSymbol::horizontalTriangleFilter() const
{
  return mHorizontalTriangleFilter;
}

void QgsPointCloud3DSymbol::setHorizontalTriangleFilter( bool horizontalTriangleFilter )
{
  mHorizontalTriangleFilter = horizontalTriangleFilter;
}

float QgsPointCloud3DSymbol::horizontalFilterThreshold() const
{
  return mHorizontalFilterThreshold;
}

void QgsPointCloud3DSymbol::setHorizontalFilterThreshold( float horizontalFilterThreshold )
{
  mHorizontalFilterThreshold = horizontalFilterThreshold;
}

bool QgsPointCloud3DSymbol::verticalTriangleFilter() const
{
  return mVerticalTriangleFilter;
}

void QgsPointCloud3DSymbol::setVerticalTriangleFilter( bool verticalTriangleFilter )
{
  mVerticalTriangleFilter = verticalTriangleFilter;
}

float QgsPointCloud3DSymbol::verticalFilterThreshold() const
{
  return mVerticalFilterThreshold;
}

void QgsPointCloud3DSymbol::setVerticalFilterThreshold( float verticalFilterThreshold )
{
  mVerticalFilterThreshold = verticalFilterThreshold;
}

void QgsPointCloud3DSymbol::writeBaseXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  elem.setAttribute( u"point-size"_s, mPointSize );
  elem.setAttribute( u"render-as-triangles"_s, mRenderAsTriangles ? 1 : 0 );
  elem.setAttribute( u"horizontal-triangle-filter"_s, mHorizontalTriangleFilter ? 1 : 0 );
  elem.setAttribute( u"horizontal-filter-threshold"_s, mHorizontalFilterThreshold );
  elem.setAttribute( u"vertical-triangle-filter"_s, mVerticalTriangleFilter ? 1 : 0 );
  elem.setAttribute( u"vertical-filter-threshold"_s, mVerticalFilterThreshold );
}

void QgsPointCloud3DSymbol::readBaseXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  mPointSize = elem.attribute( u"point-size"_s, u"3.0"_s ).toFloat();
  mRenderAsTriangles = elem.attribute( u"render-as-triangles"_s, u"0"_s ).toInt() == 1;
  mHorizontalTriangleFilter = elem.attribute( u"horizontal-triangle-filter"_s, u"0"_s ).toInt() == 1;
  mHorizontalFilterThreshold = elem.attribute( u"horizontal-filter-threshold"_s, u"10.0"_s ).toFloat();
  mVerticalTriangleFilter = elem.attribute( u"vertical-triangle-filter"_s, u"0"_s ).toInt() == 1;
  mVerticalFilterThreshold = elem.attribute( u"vertical-filter-threshold"_s, u"10.0"_s ).toFloat();
}

void QgsPointCloud3DSymbol::copyBaseSettings( QgsAbstract3DSymbol *destination ) const
{
  QgsAbstract3DSymbol::copyBaseSettings( destination );
  QgsPointCloud3DSymbol *pcDestination = static_cast<QgsPointCloud3DSymbol *>( destination );
  pcDestination->mPointSize = mPointSize;
  pcDestination->mRenderAsTriangles = mRenderAsTriangles;
  pcDestination->mHorizontalFilterThreshold = mHorizontalFilterThreshold;
  pcDestination->mHorizontalTriangleFilter = mHorizontalTriangleFilter;
  pcDestination->mVerticalFilterThreshold = mVerticalFilterThreshold;
  pcDestination->mVerticalTriangleFilter = mVerticalTriangleFilter;
}

// QgsSingleColorPointCloud3DSymbol

QgsSingleColorPointCloud3DSymbol::QgsSingleColorPointCloud3DSymbol()
  : QgsPointCloud3DSymbol()
{
}

QString QgsSingleColorPointCloud3DSymbol::symbolType() const
{
  return u"single-color"_s;
}

QgsSingleColorPointCloud3DSymbol *QgsSingleColorPointCloud3DSymbol::clone() const
{
  QgsSingleColorPointCloud3DSymbol *result = new QgsSingleColorPointCloud3DSymbol;
  result->mSingleColor = mSingleColor;
  copyBaseSettings( result );
  return result;
}

void QgsSingleColorPointCloud3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  writeBaseXml( elem, context );
  elem.setAttribute( u"single-color"_s, QgsColorUtils::colorToString( mSingleColor ) );
}

void QgsSingleColorPointCloud3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  readBaseXml( elem, context );
  mSingleColor = QgsColorUtils::colorFromString( elem.attribute( u"single-color"_s, u"0,0,255"_s ) );
}

void QgsSingleColorPointCloud3DSymbol::setSingleColor( QColor color )
{
  mSingleColor = color;
}

void QgsSingleColorPointCloud3DSymbol::fillMaterial( QgsMaterial *mat )
{
  Qt3DRender::QParameter *renderingStyle = new Qt3DRender::QParameter( "u_renderingStyle", QgsPointCloud3DSymbol::SingleColor );
  mat->addParameter( renderingStyle );
  Qt3DRender::QParameter *pointSizeParameter = new Qt3DRender::QParameter( "u_pointSize", QVariant::fromValue( mPointSize ) );
  mat->addParameter( pointSizeParameter );
  Qt3DRender::QParameter *singleColorParameter = new Qt3DRender::QParameter( "u_singleColor", QVector3D( mSingleColor.redF(), mSingleColor.greenF(), mSingleColor.blueF() ) );
  mat->addParameter( singleColorParameter );
}

// QgsColorRampPointCloud3DSymbol

QgsColorRampPointCloud3DSymbol::QgsColorRampPointCloud3DSymbol()
  : QgsPointCloud3DSymbol()
{
}

QgsColorRampPointCloud3DSymbol *QgsColorRampPointCloud3DSymbol::clone() const
{
  QgsColorRampPointCloud3DSymbol *result = new QgsColorRampPointCloud3DSymbol;
  result->mRenderingParameter = mRenderingParameter;
  result->mColorRampShader = mColorRampShader;
  result->mColorRampShaderMin = mColorRampShaderMin;
  result->mColorRampShaderMax = mColorRampShaderMax;
  copyBaseSettings( result );
  return result;
}

QString QgsColorRampPointCloud3DSymbol::symbolType() const
{
  return u"color-ramp"_s;
}

void QgsColorRampPointCloud3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  writeBaseXml( elem, context );
  elem.setAttribute( u"rendering-parameter"_s, mRenderingParameter );
  elem.setAttribute( u"color-ramp-shader-min"_s, mColorRampShaderMin );
  elem.setAttribute( u"color-ramp-shader-max"_s, mColorRampShaderMax );
  QDomDocument doc = elem.ownerDocument();
  const QDomElement elemColorRampShader = mColorRampShader.writeXml( doc );
  elem.appendChild( elemColorRampShader );
}

void QgsColorRampPointCloud3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  readBaseXml( elem, context );
  mRenderingParameter = elem.attribute( "rendering-parameter", QString() );
  mColorRampShaderMin = elem.attribute( u"color-ramp-shader-min"_s, u"0.0"_s ).toDouble();
  mColorRampShaderMax = elem.attribute( u"color-ramp-shader-max"_s, u"1.0"_s ).toDouble();
  mColorRampShader.readXml( elem );
}

QString QgsColorRampPointCloud3DSymbol::attribute() const
{
  return mRenderingParameter;
}

void QgsColorRampPointCloud3DSymbol::setAttribute( const QString &parameter )
{
  mRenderingParameter = parameter;
}

QgsColorRampShader QgsColorRampPointCloud3DSymbol::colorRampShader() const
{
  return mColorRampShader;
}

void QgsColorRampPointCloud3DSymbol::setColorRampShader( const QgsColorRampShader &colorRampShader )
{
  mColorRampShader = colorRampShader;
}

void QgsColorRampPointCloud3DSymbol::setColorRampShaderMinMax( double min, double max )
{
  mColorRampShaderMin = min;
  mColorRampShaderMax = max;
}

void QgsColorRampPointCloud3DSymbol::fillMaterial( QgsMaterial *mat )
{
  Qt3DRender::QParameter *renderingStyle = new Qt3DRender::QParameter( "u_renderingStyle", QgsPointCloud3DSymbol::ColorRamp );
  mat->addParameter( renderingStyle );
  Qt3DRender::QParameter *pointSizeParameter = new Qt3DRender::QParameter( "u_pointSize", QVariant::fromValue( mPointSize ) );
  mat->addParameter( pointSizeParameter );
  // Create the texture to pass the color ramp
  Qt3DRender::QTexture1D *colorRampTexture = nullptr;
  if ( mColorRampShader.colorRampItemList().count() > 0 )
  {
    colorRampTexture = new Qt3DRender::QTexture1D( mat );
    colorRampTexture->addTextureImage( new QgsColorRampTexture( mColorRampShader, 1 ) );
    colorRampTexture->setMinificationFilter( Qt3DRender::QTexture1D::Linear );
    colorRampTexture->setMagnificationFilter( Qt3DRender::QTexture1D::Linear );
  }

  // Parameters
  Qt3DRender::QParameter *colorRampTextureParameter = new Qt3DRender::QParameter( "u_colorRampTexture", colorRampTexture );
  mat->addParameter( colorRampTextureParameter );
  Qt3DRender::QParameter *colorRampCountParameter = new Qt3DRender::QParameter( "u_colorRampCount", mColorRampShader.colorRampItemList().count() );
  mat->addParameter( colorRampCountParameter );
  const Qgis::ShaderInterpolationMethod colorRampType = mColorRampShader.colorRampType();
  Qt3DRender::QParameter *colorRampTypeParameter = new Qt3DRender::QParameter( "u_colorRampType", static_cast<int>( colorRampType ) );
  mat->addParameter( colorRampTypeParameter );
}

// QgsRgbPointCloud3DSymbol

QgsRgbPointCloud3DSymbol::QgsRgbPointCloud3DSymbol()
  : QgsPointCloud3DSymbol()
{
}

QString QgsRgbPointCloud3DSymbol::symbolType() const
{
  return u"rgb"_s;
}

QgsRgbPointCloud3DSymbol *QgsRgbPointCloud3DSymbol::clone() const
{
  QgsRgbPointCloud3DSymbol *result = new QgsRgbPointCloud3DSymbol;
  result->mRedAttribute = mRedAttribute;
  result->mGreenAttribute = mGreenAttribute;
  result->mBlueAttribute = mBlueAttribute;

  if ( mRedContrastEnhancement )
  {
    result->setRedContrastEnhancement( new QgsContrastEnhancement( *mRedContrastEnhancement ) );
  }
  if ( mGreenContrastEnhancement )
  {
    result->setGreenContrastEnhancement( new QgsContrastEnhancement( *mGreenContrastEnhancement ) );
  }
  if ( mBlueContrastEnhancement )
  {
    result->setBlueContrastEnhancement( new QgsContrastEnhancement( *mBlueContrastEnhancement ) );
  }
  copyBaseSettings( result );
  return result;
}

void QgsRgbPointCloud3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  writeBaseXml( elem, context );

  elem.setAttribute( u"red"_s, mRedAttribute );
  elem.setAttribute( u"green"_s, mGreenAttribute );
  elem.setAttribute( u"blue"_s, mBlueAttribute );

  QDomDocument doc = elem.ownerDocument();

  //contrast enhancement
  if ( mRedContrastEnhancement )
  {
    QDomElement redContrastElem = doc.createElement( u"redContrastEnhancement"_s );
    mRedContrastEnhancement->writeXml( doc, redContrastElem );
    elem.appendChild( redContrastElem );
  }
  if ( mGreenContrastEnhancement )
  {
    QDomElement greenContrastElem = doc.createElement( u"greenContrastEnhancement"_s );
    mGreenContrastEnhancement->writeXml( doc, greenContrastElem );
    elem.appendChild( greenContrastElem );
  }
  if ( mBlueContrastEnhancement )
  {
    QDomElement blueContrastElem = doc.createElement( u"blueContrastEnhancement"_s );
    mBlueContrastEnhancement->writeXml( doc, blueContrastElem );
    elem.appendChild( blueContrastElem );
  }
}

void QgsRgbPointCloud3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  readBaseXml( elem, context );

  setRedAttribute( elem.attribute( u"red"_s, u"Red"_s ) );
  setGreenAttribute( elem.attribute( u"green"_s, u"Green"_s ) );
  setBlueAttribute( elem.attribute( u"blue"_s, u"Blue"_s ) );

  //contrast enhancements
  QgsContrastEnhancement *redContrastEnhancement = nullptr;
  const QDomElement redContrastElem = elem.firstChildElement( u"redContrastEnhancement"_s );
  if ( !redContrastElem.isNull() )
  {
    redContrastEnhancement = new QgsContrastEnhancement( Qgis::DataType::UnknownDataType );
    redContrastEnhancement->readXml( redContrastElem );
    setRedContrastEnhancement( redContrastEnhancement );
  }

  QgsContrastEnhancement *greenContrastEnhancement = nullptr;
  const QDomElement greenContrastElem = elem.firstChildElement( u"greenContrastEnhancement"_s );
  if ( !greenContrastElem.isNull() )
  {
    greenContrastEnhancement = new QgsContrastEnhancement( Qgis::DataType::UnknownDataType );
    greenContrastEnhancement->readXml( greenContrastElem );
    setGreenContrastEnhancement( greenContrastEnhancement );
  }

  QgsContrastEnhancement *blueContrastEnhancement = nullptr;
  const QDomElement blueContrastElem = elem.firstChildElement( u"blueContrastEnhancement"_s );
  if ( !blueContrastElem.isNull() )
  {
    blueContrastEnhancement = new QgsContrastEnhancement( Qgis::DataType::UnknownDataType );
    blueContrastEnhancement->readXml( blueContrastElem );
    setBlueContrastEnhancement( blueContrastEnhancement );
  }
}

void QgsRgbPointCloud3DSymbol::fillMaterial( QgsMaterial *mat )
{
  Qt3DRender::QParameter *renderingStyle = new Qt3DRender::QParameter( "u_renderingStyle", QgsPointCloud3DSymbol::RgbRendering );
  mat->addParameter( renderingStyle );
  Qt3DRender::QParameter *pointSizeParameter = new Qt3DRender::QParameter( "u_pointSize", QVariant::fromValue( mPointSize ) );
  mat->addParameter( pointSizeParameter );
}


QString QgsRgbPointCloud3DSymbol::redAttribute() const
{
  return mRedAttribute;
}

void QgsRgbPointCloud3DSymbol::setRedAttribute( const QString &redAttribute )
{
  mRedAttribute = redAttribute;
}

QString QgsRgbPointCloud3DSymbol::greenAttribute() const
{
  return mGreenAttribute;
}

void QgsRgbPointCloud3DSymbol::setGreenAttribute( const QString &greenAttribute )
{
  mGreenAttribute = greenAttribute;
}

QString QgsRgbPointCloud3DSymbol::blueAttribute() const
{
  return mBlueAttribute;
}

void QgsRgbPointCloud3DSymbol::setBlueAttribute( const QString &blueAttribute )
{
  mBlueAttribute = blueAttribute;
}

QgsContrastEnhancement *QgsRgbPointCloud3DSymbol::redContrastEnhancement()
{
  return mRedContrastEnhancement.get();
}

void QgsRgbPointCloud3DSymbol::setRedContrastEnhancement( QgsContrastEnhancement *enhancement )
{
  mRedContrastEnhancement.reset( enhancement );
}

QgsContrastEnhancement *QgsRgbPointCloud3DSymbol::greenContrastEnhancement()
{
  return mGreenContrastEnhancement.get();
}

void QgsRgbPointCloud3DSymbol::setGreenContrastEnhancement( QgsContrastEnhancement *enhancement )
{
  mGreenContrastEnhancement.reset( enhancement );
}

QgsContrastEnhancement *QgsRgbPointCloud3DSymbol::blueContrastEnhancement()
{
  return mBlueContrastEnhancement.get();
}

void QgsRgbPointCloud3DSymbol::setBlueContrastEnhancement( QgsContrastEnhancement *enhancement )
{
  mBlueContrastEnhancement.reset( enhancement );
}

// QgsClassificationPointCloud3DSymbol


QgsClassificationPointCloud3DSymbol::QgsClassificationPointCloud3DSymbol()
  : QgsPointCloud3DSymbol()
{
}

QgsClassificationPointCloud3DSymbol *QgsClassificationPointCloud3DSymbol::clone() const
{
  QgsClassificationPointCloud3DSymbol *result = new QgsClassificationPointCloud3DSymbol;
  result->mRenderingParameter = mRenderingParameter;
  result->mCategoriesList = mCategoriesList;
  copyBaseSettings( result );
  return result;
}

QString QgsClassificationPointCloud3DSymbol::symbolType() const
{
  return u"classification"_s;
}

void QgsClassificationPointCloud3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )
  QDomDocument doc = elem.ownerDocument();

  writeBaseXml( elem, context );

  elem.setAttribute( u"rendering-parameter"_s, mRenderingParameter );

  // categories
  QDomElement catsElem = doc.createElement( u"categories"_s );
  for ( const QgsPointCloudCategory &category : mCategoriesList )
  {
    QDomElement catElem = doc.createElement( u"category"_s );
    catElem.setAttribute( u"value"_s, QString::number( category.value() ) );
    catElem.setAttribute( u"pointSize"_s, category.pointSize() );
    catElem.setAttribute( u"label"_s, category.label() );
    catElem.setAttribute( u"color"_s, QgsColorUtils::colorToString( category.color() ) );
    catElem.setAttribute( u"render"_s, category.renderState() ? "true" : "false" );
    catsElem.appendChild( catElem );
  }
  elem.appendChild( catsElem );
}

void QgsClassificationPointCloud3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  readBaseXml( elem, context );
  mRenderingParameter = elem.attribute( "rendering-parameter", QString() );

  const QDomElement catsElem = elem.firstChildElement( u"categories"_s );
  if ( !catsElem.isNull() )
  {
    mCategoriesList.clear();
    QDomElement catElem = catsElem.firstChildElement();
    while ( !catElem.isNull() )
    {
      if ( catElem.tagName() == "category"_L1 )
      {
        const int value = catElem.attribute( u"value"_s ).toInt();
        const double size = catElem.attribute( u"pointSize"_s, u"0"_s ).toDouble();
        const QString label = catElem.attribute( u"label"_s );
        const bool render = catElem.attribute( u"render"_s ) != "false"_L1;
        const QColor color = QgsColorUtils::colorFromString( catElem.attribute( u"color"_s ) );
        mCategoriesList.append( QgsPointCloudCategory( value, color, label, render, size ) );
      }
      catElem = catElem.nextSiblingElement();
    }
  }
}

QString QgsClassificationPointCloud3DSymbol::attribute() const
{
  return mRenderingParameter;
}

void QgsClassificationPointCloud3DSymbol::setAttribute( const QString &attribute )
{
  mRenderingParameter = attribute;
}

void QgsClassificationPointCloud3DSymbol::setCategoriesList( const QgsPointCloudCategoryList &categories )
{
  mCategoriesList = categories;
}

QgsPointCloudCategoryList QgsClassificationPointCloud3DSymbol::getFilteredOutCategories() const
{
  QgsPointCloudCategoryList filteredOut;
  for ( const QgsPointCloudCategory &category : mCategoriesList )
  {
    if ( !category.renderState() )
      filteredOut.push_back( category );
  }
  return filteredOut;
}

QgsColorRampShader QgsClassificationPointCloud3DSymbol::colorRampShader() const
{
  QgsColorRampShader colorRampShader;
  colorRampShader.setColorRampType( Qgis::ShaderInterpolationMethod::Exact );
  colorRampShader.setClassificationMode( Qgis::ShaderClassificationMethod::Continuous );
  QList<QgsColorRampShader::ColorRampItem> colorRampItemList;
  for ( const QgsPointCloudCategory &category : mCategoriesList )
  {
    const QColor color = category.color();
    const QgsColorRampShader::ColorRampItem item( category.value(), color, category.label() );
    colorRampItemList.push_back( item );
  }
  colorRampShader.setColorRampItemList( colorRampItemList );
  return colorRampShader;
}


void QgsClassificationPointCloud3DSymbol::fillMaterial( QgsMaterial *mat )
{
  const QgsColorRampShader mColorRampShader = colorRampShader();
  Qt3DRender::QParameter *renderingStyle = new Qt3DRender::QParameter( "u_renderingStyle", QgsPointCloud3DSymbol::Classification );
  mat->addParameter( renderingStyle );
  Qt3DRender::QParameter *pointSizeParameter = new Qt3DRender::QParameter( "u_pointSize", QVariant::fromValue( mPointSize ) );
  mat->addParameter( pointSizeParameter );
  // Create the texture to pass the color ramp
  Qt3DRender::QTexture1D *colorRampTexture = nullptr;
  if ( mColorRampShader.colorRampItemList().count() > 0 )
  {
    colorRampTexture = new Qt3DRender::QTexture1D( mat );
    colorRampTexture->addTextureImage( new QgsColorRampTexture( mColorRampShader, 1 ) );
    colorRampTexture->setMinificationFilter( Qt3DRender::QTexture1D::Linear );
    colorRampTexture->setMagnificationFilter( Qt3DRender::QTexture1D::Linear );
  }

  // Parameters
  Qt3DRender::QParameter *colorRampTextureParameter = new Qt3DRender::QParameter( "u_colorRampTexture", colorRampTexture );
  mat->addParameter( colorRampTextureParameter );
  Qt3DRender::QParameter *colorRampCountParameter = new Qt3DRender::QParameter( "u_colorRampCount", mColorRampShader.colorRampItemList().count() );
  mat->addParameter( colorRampCountParameter );
  const Qgis::ShaderInterpolationMethod colorRampType = mColorRampShader.colorRampType();
  Qt3DRender::QParameter *colorRampTypeParameter = new Qt3DRender::QParameter( "u_colorRampType", static_cast<int>( colorRampType ) );
  mat->addParameter( colorRampTypeParameter );
}
