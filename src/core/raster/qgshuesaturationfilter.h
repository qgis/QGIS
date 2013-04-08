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

#include "qgsrasterdataprovider.h"
#include "qgsrasterinterface.h"

class QDomElement;

/** \ingroup core
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

    QgsHueSaturationFilter( QgsRasterInterface *input = 0 );
    ~QgsHueSaturationFilter();

    QgsRasterInterface * clone() const;

    int bandCount() const;

    QGis::DataType dataType( int bandNo ) const;

    bool setInput( QgsRasterInterface* input );

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height );

    void setSaturation( int saturation );
    int saturation() const { return mSaturation; }

    void setGrayscaleMode( QgsHueSaturationFilter::GrayscaleMode grayscaleMode ) { mGrayscaleMode = grayscaleMode; }
    QgsHueSaturationFilter::GrayscaleMode grayscaleMode() const { return mGrayscaleMode; }

    void setColorizeOn( bool colorizeOn ) { mColorizeOn = colorizeOn; }
    bool colorizeOn() const { return mColorizeOn; }
    void setColorizeColor( QColor colorizeColor );
    QColor colorizeColor() const { return mColorizeColor; }
    void setColorizeStrength( int colorizeStrength ) { mColorizeStrength = colorizeStrength; }
    int colorizeStrength() const { return mColorizeStrength; }

    void writeXML( QDomDocument& doc, QDomElement& parentElem );

    /**Sets base class members from xml. Usually called from create() methods of subclasses*/
    void readXML( const QDomElement& filterElem );

  private:
    /** Process a change in saturation and update resultant HSL & RGB values*/
    void processSaturation( int &r, int &g, int &b, int &h, int &s, int &l );
    /** Process a colorization and update resultant HSL & RGB values*/
    void processColorization( int &r, int &g, int &b, int &h, int &s, int &l ) ;

    /**Current saturation value. Range: -100 (desaturated) ... 0 (no change) ... 100 (increased)*/
    int mSaturation;
    double mSaturationScale;

    /**Current grayscale mode*/
    QgsHueSaturationFilter::GrayscaleMode mGrayscaleMode;

    /**Colorize settings*/
    bool mColorizeOn;
    QColor mColorizeColor;
    int mColorizeH, mColorizeS;
    int mColorizeStrength;

};

#endif // QGSHUESATURATIONFILTER_H
