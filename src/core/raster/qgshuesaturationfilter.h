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
    QgsHueSaturationFilter( QgsRasterInterface *input = 0 );
    ~QgsHueSaturationFilter();

    QgsRasterInterface * clone() const;

    int bandCount() const;

    QGis::DataType dataType( int bandNo ) const;

    bool setInput( QgsRasterInterface* input );

    QgsRasterBlock *block( int bandNo, const QgsRectangle &extent, int width, int height );

    void setSaturation( int saturation ) { mSaturation = qBound( -100, saturation, 100 ); }
    int saturation() const { return mSaturation; }

    void writeXML( QDomDocument& doc, QDomElement& parentElem );

    /**Sets base class members from xml. Usually called from create() methods of subclasses*/
    void readXML( const QDomElement& filterElem );

  private:
    /**Current saturation value. Range: -100 (desaturated) ... 0 (no change) ... 100 (increased)*/
    int mSaturation;

};

#endif // QGSHUESATURATIONFILTER_H
