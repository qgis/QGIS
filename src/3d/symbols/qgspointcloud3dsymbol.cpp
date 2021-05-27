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
#include "qgssymbollayerutils.h"

#include <Qt3DRender/QMaterial>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTexture>

// QgsPointCloud3DSymbol


QgsPointCloud3DSymbol::QgsPointCloud3DSymbol()
  : QgsAbstract3DSymbol()
{
}

QgsPointCloud3DSymbol::~QgsPointCloud3DSymbol() {  }

void QgsPointCloud3DSymbol::setPointSize( float size )
{
  mPointSize = size;
}

// QgsSingleColorPointCloud3DSymbol

QgsSingleColorPointCloud3DSymbol::QgsSingleColorPointCloud3DSymbol()
  : QgsPointCloud3DSymbol()
{

}

QString QgsSingleColorPointCloud3DSymbol::symbolType() const
{
  return QStringLiteral( "single-color" );
}

QgsAbstract3DSymbol *QgsSingleColorPointCloud3DSymbol::clone() const
{
  QgsSingleColorPointCloud3DSymbol *result = new QgsSingleColorPointCloud3DSymbol;
  result->mPointSize = mPointSize;
  result->mSingleColor = mSingleColor;
  copyBaseSettings( result );
  return result;
}

void QgsSingleColorPointCloud3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  elem.setAttribute( QStringLiteral( "point-size" ), mPointSize );
  elem.setAttribute( QStringLiteral( "single-color" ), QgsSymbolLayerUtils::encodeColor( mSingleColor ) );
}

void QgsSingleColorPointCloud3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  mPointSize = elem.attribute( QStringLiteral( "point-size" ), QStringLiteral( "2.0" ) ).toFloat();
  mSingleColor = QgsSymbolLayerUtils::decodeColor( elem.attribute( QStringLiteral( "single-color" ), QStringLiteral( "0,0,255" ) ) );
}

void QgsSingleColorPointCloud3DSymbol::setSingleColor( QColor color )
{
  mSingleColor = color;
}

void QgsSingleColorPointCloud3DSymbol::fillMaterial( Qt3DRender::QMaterial *mat )
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

QgsAbstract3DSymbol *QgsColorRampPointCloud3DSymbol::clone() const
{
  QgsColorRampPointCloud3DSymbol *result = new QgsColorRampPointCloud3DSymbol;
  result->mPointSize = mPointSize;
  result->mRenderingParameter = mRenderingParameter;
  result->mColorRampShader = mColorRampShader;
  result->mColorRampShaderMin = mColorRampShaderMin;
  result->mColorRampShaderMax = mColorRampShaderMax;
  copyBaseSettings( result );
  return result;
}

QString QgsColorRampPointCloud3DSymbol::symbolType() const
{
  return QStringLiteral( "color-ramp" );
}

void QgsColorRampPointCloud3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )

  elem.setAttribute( QStringLiteral( "point-size" ), mPointSize );
  elem.setAttribute( QStringLiteral( "rendering-parameter" ), mRenderingParameter );
  elem.setAttribute( QStringLiteral( "color-ramp-shader-min" ), mColorRampShaderMin );
  elem.setAttribute( QStringLiteral( "color-ramp-shader-max" ), mColorRampShaderMax );
  QDomDocument doc = elem.ownerDocument();
  QDomElement elemColorRampShader = mColorRampShader.writeXml( doc );
  elem.appendChild( elemColorRampShader );
}

void QgsColorRampPointCloud3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  mPointSize = elem.attribute( "point-size", QStringLiteral( "2.0" ) ).toFloat();
  mRenderingParameter = elem.attribute( "rendering-parameter", QString() );
  mColorRampShaderMin = elem.attribute( QStringLiteral( "color-ramp-shader-min" ), QStringLiteral( "0.0" ) ).toDouble();
  mColorRampShaderMax = elem.attribute( QStringLiteral( "color-ramp-shader-max" ), QStringLiteral( "1.0" ) ).toDouble();
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

void QgsColorRampPointCloud3DSymbol::fillMaterial( Qt3DRender::QMaterial *mat )
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
  int colorRampType = mColorRampShader.colorRampType();
  Qt3DRender::QParameter *colorRampTypeParameter = new Qt3DRender::QParameter( "u_colorRampType", colorRampType );
  mat->addParameter( colorRampTypeParameter );
}

// QgsRgbPointCloud3DSymbol

QgsRgbPointCloud3DSymbol::QgsRgbPointCloud3DSymbol()
  : QgsPointCloud3DSymbol()
{

}

QString QgsRgbPointCloud3DSymbol::symbolType() const
{
  return QStringLiteral( "rgb" );
}

QgsAbstract3DSymbol *QgsRgbPointCloud3DSymbol::clone() const
{
  QgsRgbPointCloud3DSymbol *result = new QgsRgbPointCloud3DSymbol;
  result->mPointSize = mPointSize;
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
  elem.setAttribute( QStringLiteral( "point-size" ), mPointSize );

  elem.setAttribute( QStringLiteral( "red" ), mRedAttribute );
  elem.setAttribute( QStringLiteral( "green" ), mGreenAttribute );
  elem.setAttribute( QStringLiteral( "blue" ), mBlueAttribute );

  QDomDocument doc = elem.ownerDocument();

  //contrast enhancement
  if ( mRedContrastEnhancement )
  {
    QDomElement redContrastElem = doc.createElement( QStringLiteral( "redContrastEnhancement" ) );
    mRedContrastEnhancement->writeXml( doc, redContrastElem );
    elem.appendChild( redContrastElem );
  }
  if ( mGreenContrastEnhancement )
  {
    QDomElement greenContrastElem = doc.createElement( QStringLiteral( "greenContrastEnhancement" ) );
    mGreenContrastEnhancement->writeXml( doc, greenContrastElem );
    elem.appendChild( greenContrastElem );
  }
  if ( mBlueContrastEnhancement )
  {
    QDomElement blueContrastElem = doc.createElement( QStringLiteral( "blueContrastEnhancement" ) );
    mBlueContrastEnhancement->writeXml( doc, blueContrastElem );
    elem.appendChild( blueContrastElem );
  }
}

void QgsRgbPointCloud3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )
  mPointSize = elem.attribute( "point-size", QStringLiteral( "2.0" ) ).toFloat();

  setRedAttribute( elem.attribute( QStringLiteral( "red" ), QStringLiteral( "Red" ) ) );
  setGreenAttribute( elem.attribute( QStringLiteral( "green" ), QStringLiteral( "Green" ) ) );
  setBlueAttribute( elem.attribute( QStringLiteral( "blue" ), QStringLiteral( "Blue" ) ) );

  //contrast enhancements
  QgsContrastEnhancement *redContrastEnhancement = nullptr;
  QDomElement redContrastElem = elem.firstChildElement( QStringLiteral( "redContrastEnhancement" ) );
  if ( !redContrastElem.isNull() )
  {
    redContrastEnhancement = new QgsContrastEnhancement( Qgis::DataType::UnknownDataType );
    redContrastEnhancement->readXml( redContrastElem );
    setRedContrastEnhancement( redContrastEnhancement );
  }

  QgsContrastEnhancement *greenContrastEnhancement = nullptr;
  QDomElement greenContrastElem = elem.firstChildElement( QStringLiteral( "greenContrastEnhancement" ) );
  if ( !greenContrastElem.isNull() )
  {
    greenContrastEnhancement = new QgsContrastEnhancement( Qgis::DataType::UnknownDataType );
    greenContrastEnhancement->readXml( greenContrastElem );
    setGreenContrastEnhancement( greenContrastEnhancement );
  }

  QgsContrastEnhancement *blueContrastEnhancement = nullptr;
  QDomElement blueContrastElem = elem.firstChildElement( QStringLiteral( "blueContrastEnhancement" ) );
  if ( !blueContrastElem.isNull() )
  {
    blueContrastEnhancement = new QgsContrastEnhancement( Qgis::DataType::UnknownDataType );
    blueContrastEnhancement->readXml( blueContrastElem );
    setBlueContrastEnhancement( blueContrastEnhancement );
  }
}

void QgsRgbPointCloud3DSymbol::fillMaterial( Qt3DRender::QMaterial *mat )
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

QgsAbstract3DSymbol *QgsClassificationPointCloud3DSymbol::clone() const
{
  QgsClassificationPointCloud3DSymbol *result = new QgsClassificationPointCloud3DSymbol;
  result->mPointSize = mPointSize;
  result->mRenderingParameter = mRenderingParameter;
  result->mCategoriesList = mCategoriesList;
  copyBaseSettings( result );
  return result;
}

QString QgsClassificationPointCloud3DSymbol::symbolType() const
{
  return QStringLiteral( "classification" );
}

void QgsClassificationPointCloud3DSymbol::writeXml( QDomElement &elem, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( context )
  QDomDocument doc = elem.ownerDocument();

  elem.setAttribute( QStringLiteral( "point-size" ), mPointSize );
  elem.setAttribute( QStringLiteral( "rendering-parameter" ), mRenderingParameter );

  // categories
  QDomElement catsElem = doc.createElement( QStringLiteral( "categories" ) );
  for ( const QgsPointCloudCategory &category : mCategoriesList )
  {
    QDomElement catElem = doc.createElement( QStringLiteral( "category" ) );
    catElem.setAttribute( QStringLiteral( "value" ), QString::number( category.value() ) );
    catElem.setAttribute( QStringLiteral( "label" ), category.label() );
    catElem.setAttribute( QStringLiteral( "color" ), QgsSymbolLayerUtils::encodeColor( category.color() ) );
    catElem.setAttribute( QStringLiteral( "render" ), category.renderState() ? "true" : "false" );
    catsElem.appendChild( catElem );
  }
  elem.appendChild( catsElem );
}

void QgsClassificationPointCloud3DSymbol::readXml( const QDomElement &elem, const QgsReadWriteContext &context )
{
  Q_UNUSED( context )

  mPointSize = elem.attribute( "point-size", QStringLiteral( "2.0" ) ).toFloat();
  mRenderingParameter = elem.attribute( "rendering-parameter", QString() );

  const QDomElement catsElem = elem.firstChildElement( QStringLiteral( "categories" ) );
  if ( !catsElem.isNull() )
  {
    mCategoriesList.clear();
    QDomElement catElem = catsElem.firstChildElement();
    while ( !catElem.isNull() )
    {
      if ( catElem.tagName() == QLatin1String( "category" ) )
      {
        const int value = catElem.attribute( QStringLiteral( "value" ) ).toInt();
        const QString label = catElem.attribute( QStringLiteral( "label" ) );
        const bool render = catElem.attribute( QStringLiteral( "render" ) ) != QLatin1String( "false" );
        const QColor color = QgsSymbolLayerUtils::decodeColor( catElem.attribute( QStringLiteral( "color" ) ) );
        mCategoriesList.append( QgsPointCloudCategory( value, color, label, render ) );
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
  for ( QgsPointCloudCategory category : mCategoriesList )
  {
    if ( !category.renderState() )
      filteredOut.push_back( category );
  }
  return filteredOut;
}

QgsColorRampShader QgsClassificationPointCloud3DSymbol::colorRampShader() const
{
  QgsColorRampShader colorRampShader;
  colorRampShader.setColorRampType( QgsColorRampShader::Type::Exact );
  colorRampShader.setClassificationMode( QgsColorRampShader::ClassificationMode::Continuous );
  QList<QgsColorRampShader::ColorRampItem> colorRampItemList;
  for ( const QgsPointCloudCategory &category : mCategoriesList )
  {
    QColor color = category.color();
    QgsColorRampShader::ColorRampItem item( category.value(), color, category.label() );
    colorRampItemList.push_back( item );
  }
  colorRampShader.setColorRampItemList( colorRampItemList );
  return colorRampShader;
}


void QgsClassificationPointCloud3DSymbol::fillMaterial( Qt3DRender::QMaterial *mat )
{
  QgsColorRampShader mColorRampShader = colorRampShader();
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
  int colorRampType = mColorRampShader.colorRampType();
  Qt3DRender::QParameter *colorRampTypeParameter = new Qt3DRender::QParameter( "u_colorRampType", colorRampType );
  mat->addParameter( colorRampTypeParameter );
}

