/***************************************************************************
                         qgshuesaturationfilter.h
                         -------------------
    begin                : February 2013
    copyright            : (C) 2013 by Alexander Bruy, Nyall Dawson
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

#ifndef QGSHUESATURATIONFILTER_H
#define QGSHUESATURATIONFILTER_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsrasterinterface.h"

class QDomElement;

/**
 * \ingroup core
  * Color and saturation filter pipe for rasters.
  */
class CORE_EXPORT QgsHueSaturationFilter : public QgsRasterInterface
{
  public:

    // Available modes for converting a raster to grayscale
    enum GrayscaleMode
    {
      GrayscaleOff,
      GrayscaleLightness,
      GrayscaleLuminosity,
      GrayscaleAverage
    };

    QgsHueSaturationFilter( QgsRasterInterface *input = nullptr );

    QgsHueSaturationFilter *clone() const override SIP_FACTORY;

    int bandCount() const override;

    Qgis::DataType dataType( int bandNo ) const override;

    bool setInput( QgsRasterInterface *input ) override;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height, QgsRasterBlockFeedback *feedback = nullptr ) override SIP_FACTORY;

    void setSaturation( int saturation );
    int saturation() const { return mSaturation; }

    void setGrayscaleMode( QgsHueSaturationFilter::GrayscaleMode grayscaleMode ) { mGrayscaleMode = grayscaleMode; }
    QgsHueSaturationFilter::GrayscaleMode grayscaleMode() const { return mGrayscaleMode; }

    void setColorizeOn( bool colorizeOn ) { mColorizeOn = colorizeOn; }
    bool colorizeOn() const { return mColorizeOn; }
    void setColorizeColor( const QColor &colorizeColor );
    QColor colorizeColor() const { return mColorizeColor; }
    void setColorizeStrength( int colorizeStrength ) { mColorizeStrength = colorizeStrength; }
    int colorizeStrength() const { return mColorizeStrength; }

    void writeXml( QDomDocument &doc, QDomElement &parentElem ) const override;

    //! Sets base class members from xml. Usually called from create() methods of subclasses
    void readXml( const QDomElement &filterElem ) override;

  private:
    //! Process a change in saturation and update resultant HSL & RGB values
    void processSaturation( int &r, int &g, int &b, int &h, int &s, int &l );
    //! Process a colorization and update resultant HSL & RGB values
    void processColorization( int &r, int &g, int &b, int &h, int &s, int &l );

    //! Current saturation value. Range: -100 (desaturated) ... 0 (no change) ... 100 (increased)
    int mSaturation = 0;
    double mSaturationScale = 1;

    //! Current grayscale mode
    QgsHueSaturationFilter::GrayscaleMode mGrayscaleMode = QgsHueSaturationFilter::GrayscaleOff;

    //! Colorize settings
    bool mColorizeOn = false;
    QColor mColorizeColor;
    int mColorizeH = 0, mColorizeS = 50;
    int mColorizeStrength = 100;

};

#endif // QGSHUESATURATIONFILTER_H
