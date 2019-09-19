/***************************************************************************
                         qgsbrightnesscontrastfilter.h
                         -------------------
    begin                : February 2013
    copyright            : (C) 2013 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBRIGHTNESSCONTRASTFILTER_H
#define QGSBRIGHTNESSCONTRASTFILTER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrasterinterface.h"

class QDomElement;

/**
 * \ingroup core
  * Brightness/contrast filter pipe for rasters.
  */
class CORE_EXPORT QgsBrightnessContrastFilter : public QgsRasterInterface
{
  public:
    QgsBrightnessContrastFilter( QgsRasterInterface *input = nullptr );

    QgsBrightnessContrastFilter *clone() const override SIP_FACTORY;

    int bandCount() const override;

    Qgis::DataType dataType( int bandNo ) const override;

    bool setInput( QgsRasterInterface *input ) override;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    void setBrightness( int brightness ) { mBrightness = qBound( -255, brightness, 255 ); }
    int brightness() const { return mBrightness; }

    void setContrast( int contrast ) { mContrast = qBound( -100, contrast, 100 ); }
    int contrast() const { return mContrast; }

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;

    //! Sets base class members from xml. Usually called from create() methods of subclasses
    void readXml( const QDomElement &filterElem ) override;

  private:
    //! Adjusts a color component by the specified brightness and contrast factor
    int  adjustColorComponent( int colorComponent, int alpha, int brightness, double contrastFactor ) const;

    //! Current brightness coefficient value. Default: 0. Range: -255...255
    int mBrightness = 0;

    //! Current contrast coefficient value. Default: 0. Range: -100...100
    double mContrast = 0;
};

#endif // QGSBRIGHTNESSCONTRASTFILTER_H
