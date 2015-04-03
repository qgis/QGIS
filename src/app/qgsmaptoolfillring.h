/***************************************************************************
    qgsmaptoolfillring.h  - map tool to cut rings in polygon and multipolygon
                            features and fill them with new feature
    ---------------------
    begin                : December 2013
    copyright            : (C) 2013 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolcapture.h"
#include "qgsmapmouseevent.h"

/** A tool to cut holes into polygon and multipolygon features and fill them
 *  with new feature. Attributes are copied from parent feature.
 * */
class APP_EXPORT QgsMapToolFillRing: public QgsMapToolCapture
{
    Q_OBJECT
  public:
    QgsMapToolFillRing( QgsMapCanvas* canvas );
    virtual ~QgsMapToolFillRing();
    void canvasMapReleaseEvent( QgsMapMouseEvent * e ) override;
};
