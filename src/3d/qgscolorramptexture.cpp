/***************************************************************************
                         qgscolorramptexture.h
                         -------------------------
    begin                : january 2020
    copyright            : (C) 2020 by Vincent Cloarec
    email                : vcloarec at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscolorramptexture.h"

/// @cond PRIVATE

// ColorRampTextureGenerator

QgsColorRampTextureGenerator::QgsColorRampTextureGenerator( const QgsColorRampShader &colorRampShader, double verticalScale )
  : mColorRampShader( colorRampShader )
  , mVerticalScale( verticalScale )
{
}

Qt3DRender::QTextureImageDataPtr QgsColorRampTextureGenerator::operator()()
{
  Qt3DRender::QTextureImageDataPtr dataPtr = Qt3DRender::QTextureImageDataPtr::create();
  dataPtr->setFormat( QOpenGLTexture::RGBA32F );
  dataPtr->setTarget( QOpenGLTexture::Target1D );
  dataPtr->setPixelFormat( QOpenGLTexture::RGBA );
  dataPtr->setPixelType( QOpenGLTexture::Float32 );

  QByteArray data;
  const QList<QgsColorRampShader::ColorRampItem> colorItemList = mColorRampShader.colorRampItemList();
  const int size = colorItemList.count() ;

  dataPtr->setWidth( size );
  dataPtr->setHeight( 1 );
  dataPtr->setDepth( 1 );
  dataPtr->setFaces( 1 );
  dataPtr->setLayers( 1 );
  dataPtr->setMipLevels( 1 );

  for ( int i = 0; i < colorItemList.count(); ++i )
  {
    float mag = float( colorItemList.at( i ).value * mVerticalScale );

    const QColor color = colorItemList.at( i ).color;
    if ( color.alphaF() == 0.0f )
      continue;
    float rf = float( color.redF() );
    float gf = float( color.greenF() );
    float bf = float( color.blueF() );

    data.append( reinterpret_cast<const char *>( &mag ), sizeof( float ) );
    data.append( reinterpret_cast<const char *>( &rf ), sizeof( float ) );
    data.append( reinterpret_cast<const char *>( &gf ), sizeof( float ) );
    data.append( reinterpret_cast<const char *>( &bf ), sizeof( float ) );
  }

  dataPtr->setData( data, sizeof( float ) ); //size is the size of the type, here float

  return dataPtr;
}

bool QgsColorRampTextureGenerator::operator==( const Qt3DRender::QTextureImageDataGenerator &other ) const
{
  const QgsColorRampTextureGenerator *otherFunctor = functor_cast<QgsColorRampTextureGenerator>( &other );
  if ( !otherFunctor )
    return false;

  return mColorRampShader == otherFunctor->mColorRampShader;
}

// ColorRampTexture

QgsColorRampTexture::QgsColorRampTexture( const QgsColorRampShader &colorRampShader, double verticalScale, Qt3DCore::QNode *parent )
  : Qt3DRender::QAbstractTextureImage( parent ),
    mColorRampShader( colorRampShader ),
    mVerticalScale( verticalScale )
{

}

Qt3DRender::QTextureImageDataGeneratorPtr QgsColorRampTexture::dataGenerator() const
{
  return Qt3DRender::QTextureImageDataGeneratorPtr( new QgsColorRampTextureGenerator( mColorRampShader, mVerticalScale ) );
}

/// @endcond
