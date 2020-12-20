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

#ifndef QGSCOLORRAMPTEXTURE_H
#define QGSCOLORRAMPTEXTURE_H

/// @cond PRIVATE

#include <Qt3DRender/QEffect>
#include <Qt3DRender/QGraphicsApiFilter>
#include <Qt3DRender/QParameter>
#include <Qt3DRender/QTexture>

#include <QUrl>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <Qt3DRender/QBuffer>
#include <QByteArray>

#include "qgsmeshlayer.h"
#include "qgsmeshlayerutils.h"
#include "qgstriangularmesh.h"

#define SIP_NO_FILE

class QgsColorRampTextureGenerator: public Qt3DRender::QTextureImageDataGenerator
{

  public:
    QgsColorRampTextureGenerator( const QgsColorRampShader &colorRampShader, double verticalScale = 1 );

  public:
    Qt3DRender::QTextureImageDataPtr operator()() override;

    bool operator ==( const Qt3DRender::QTextureImageDataGenerator &other ) const override;

    QT3D_FUNCTOR( QgsColorRampTextureGenerator )

  private:
    QgsColorRampShader mColorRampShader;
    double mVerticalScale = 1;
};


class QgsColorRampTexture: public Qt3DRender::QAbstractTextureImage
{
  public:
    QgsColorRampTexture( const QgsColorRampShader &colorRampShader, double verticalScale = 1, Qt3DCore::QNode *parent = nullptr );
    // QAbstractTextureImage interface
  protected:
    Qt3DRender::QTextureImageDataGeneratorPtr dataGenerator() const override;

  private:
    QgsColorRampShader mColorRampShader;
    double mVerticalScale = 1;
};

/// @endcond

#endif // QGSCOLORRAMPTEXTURE_H
