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

#include "qgsrasterdataprovider.h"
#include "qgsrasterinterface.h"

class QDomElement;

/** \ingroup core
  * Brightness/contrast filter pipe for rasters.
  */
class CORE_EXPORT QgsBrightnessContrastFilter : public QgsRasterInterface
{
  public:
    QgsBrightnessContrastFilter( QgsRasterInterface *input = 0 );
    ~QgsBrightnessContrastFilter();

    QgsRasterInterface * clone() const override;

    int bandCount() const override;

    QGis::DataType dataType( int bandNo ) const override;

    bool setInput( QgsRasterInterface* input ) override;

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height ) override;

    void setBrightness( int brightness ) { mBrightness = qBound( -255, brightness, 255 ); }
    int brightness() const { return mBrightness; }

    void setContrast( int contrast ) { mContrast = qBound( -100, contrast, 100 ); }
    int contrast() const { return mContrast; }

    void writeXML( QDomDocument& doc, QDomElement& parentElem ) const override;

    /** Sets base class members from xml. Usually called from create() methods of subclasses*/
    void readXML( const QDomElement& filterElem ) override;

  private:
    /** Adjusts a color component by the specified brightness and contrast factor*/
    int  adjustColorComponent( int colorComponent, int alpha, int brightness, double contrastFactor ) const;

    /** Current brightness coefficient value. Default: 0. Range: -255...255 */
    int mBrightness;

    /** Current contrast coefficient value. Default: 0. Range: -100...100 */
    double mContrast;
};

#endif // QGSBRIGHTNESSCONTRASTFILTER_H
