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

    /**
      * Create crs from epsg code in QML
      */
    Q_INVOKABLE QgsCoordinateReferenceSystem coordinateReferenceSystemFromEpsgId( long epsg ) const;

    /**
      * Create QgsPointXY in QML
      */
    Q_INVOKABLE QgsPointXY pointXYFactory( double x, double y ) const;

    /**
      * Create QgsPoint in QML
      */
    Q_INVOKABLE QgsPoint pointFactory( double x, double y ) const;

    /**
      * Convert QGeoCoordinate to QgsPoint
      */
    Q_INVOKABLE QgsPoint coordinateToPoint( const QGeoCoordinate &coor ) const;

    /**
      * Transform point between different crs from QML
      */
    Q_INVOKABLE QgsPointXY transformPoint( const QgsCoordinateReferenceSystem &srcCrs,
                                           const QgsCoordinateReferenceSystem &destCrs,
                                           const QgsCoordinateTransformContext &context,
                                           const QgsPointXY &srcPoint ) const;

    /**
      * Calculate the distance in meter representing baseLengthPixels pixels on the screen based on the current map settings.
      */
    Q_INVOKABLE double screenUnitsToMeters( QgsQuickMapSettings *mapSettings, int baseLengthPixels ) const;

    /**
      * Has QgsFeature a geometry that can be added to the layer (non-empty, same geometry type)?
      */
    Q_INVOKABLE bool hasValidGeometry( QgsVectorLayer *layer, const QgsFeature &feat );

    //! Check if file on path exists
    Q_INVOKABLE bool fileExists( QString path );

    //! Copy file from sourcePath to targetPath
    Q_INVOKABLE void copyFile( QString sourcePath, QString targetPath );

    //! Delete file on path from disk
    Q_INVOKABLE void remove( QString path );

    //! Extract filename from path
    Q_INVOKABLE QString getFileName( QString path );

    //! Log message in QgsMessageLog
    Q_INVOKABLE void logMessage( const QString &message,
                                 const QString &tag = QString( "QgsQuick" ),
                                 Qgis::MessageLevel level = Qgis::Warning );

    /**
      * Get icon from custom theme dir or default if not found in the theme dir
      */
    Q_INVOKABLE QUrl getThemeIcon( const QString &name );

    /**
      * point to string, e.g. -2.234521, 34.4444421 -> -2.234, 34.444
      */
    Q_INVOKABLE QString qgsPointToString( const QgsPoint &point, int decimals = 3 );

    /**
      * distance in meters to human readable length e.g. 1222.234 m -> 1.2 km
      */
    Q_INVOKABLE QString distanceToString( qreal dist, int decimals = 1 );

    //! Returns a string with information about screen size and resolution - useful for debugging
    QString dumpScreenInfo() const;

  signals:

  private:
    explicit QgsQuickUtils( QObject *parent = 0 );
    ~QgsQuickUtils();

    static QgsQuickUtils *sInstance;

    qreal mScreenDensity;

};

#endif // QGSQUICKUTILS_H
