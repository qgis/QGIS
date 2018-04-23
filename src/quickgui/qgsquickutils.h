/***************************************************************************
  qgsquickutils.h
  --------------------------------------
  Date                 : Nov 2017
  Copyright            : (C) 2017 by Peter Petrik
  Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSQUICKUTILS_H
#define QGSQUICKUTILS_H


#include <QObject>
#include <QString>
#include <QUrl>
#include <QtPositioning/QGeoCoordinate>

#include "qgis.h"
#include "qgsfeature.h"
#include "qgsmessagelog.h"
#include "qgspoint.h"
#include "qgspointxy.h"

#include "qgsquickmapsettings.h"
#include "qgis_quick.h"


class QgsFeature;
class QgsVectorLayer;
class QgsCoordinateReferenceSystem;

/**
 * \ingroup quick
 *
 * Singleton encapsulating the common utilies for QgsQuick library.
 *
 *
 * \since QGIS 3.2
 */
class QUICK_EXPORT QgsQuickUtils: public QObject
{
    Q_OBJECT

    /**
      * "dp" is useful for building building components that work well with different screen densities.
      * It stands for density-independent pixels. A width of 10dp is going to be the same physical size
      * on all screens regardless their density. In QML code, all values are specified in screen pixels,
      * so in order to set a width of 10dp, one would use the following code: "width: 10 * QgsQuick.Utils.dp"
      *
      * 1dp is approximately 0.16mm. When screen has 160 DPI (baseline), the value of "dp" is 1.
      * On high DPI screen the value will be greater, e.g. 1.5.
      */
    Q_PROPERTY( qreal dp READ screenDensity CONSTANT )

  public:
    //! return instance of the QgsQuickUtils singleton
    static QgsQuickUtils *instance();

    //! Calculated density of the screen - see "dp" property for more details
    qreal screenDensity() const;

    //! Returns a string with information about screen size and resolution - useful for debugging
    QString dumpScreenInfo() const;

  signals:

  private:
    explicit QgsQuickUtils( QObject *parent = nullptr );
    ~QgsQuickUtils() = default;

    static QgsQuickUtils *sInstance;

    qreal mScreenDensity;

};

#endif // QGSQUICKUTILS_H
