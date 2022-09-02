/***************************************************************************
    qgsprojectdisplaysettings.h
    ---------------------------
    begin                : January 2020
    copyright            : (C) 2020 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROJECTDISPLAYSETTINGS_H
#define QGSPROJECTDISPLAYSETTINGS_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgis.h"
#include "qgsunittypes.h"
#include "qgscoordinatereferencesystem.h"

#include <QObject>
#include <QVector>
#include <memory>

class QDomElement;
class QgsReadWriteContext;
class QDomDocument;
class QgsBearingNumericFormat;
class QgsGeographicCoordinateNumericFormat;

/**
 * \brief Contains settings and properties relating to how a QgsProject should display
 * values such as map coordinates and bearings.
 *
 * \ingroup core
 * \since QGIS 3.12
 */
class CORE_EXPORT QgsProjectDisplaySettings : public QObject
{
    Q_OBJECT

  public:

    Q_PROPERTY( Qgis::CoordinateDisplayType coordinateType READ coordinateType WRITE setCoordinateType NOTIFY coordinateTypeChanged )

    /**
     * Constructor for QgsProjectDisplaySettings with the specified \a parent object.
     */
    QgsProjectDisplaySettings( QObject *parent = nullptr );

    ~QgsProjectDisplaySettings() override;

    /**
     * Resets the settings to a default state.
     */
    void reset();

    /**
     * Sets the project bearing \a format, which controls how bearings associated with the project are displayed.
     *
     * Ownership of \a format is transferred to the settings.
     *
     * \see bearingFormat()
     * \see bearingFormatChanged()
     */
    void setBearingFormat( QgsBearingNumericFormat *format SIP_TRANSFER );

    /**
     * Returns the project bearing's format, which controls how bearings associated with the project are displayed.
     *
     * \see setBearingFormat()
     * \see bearingFormatChanged()
     */
    const QgsBearingNumericFormat *bearingFormat() const;

    /**
     * Sets the project geographic coordinate \a format, which controls how geographic coordinates associated with the project are displayed.
     *
     * Ownership of \a format is transferred to the settings.
     *
     * \see geographicCoordinateFormat()
     * \see geographicCoordinateFormatChanged()
     *
     * \since QGIS 3.26
     */
    void setGeographicCoordinateFormat( QgsGeographicCoordinateNumericFormat *format SIP_TRANSFER );

    /**
     * Returns the project's geographic coordinate format, which controls how geographic coordinates associated with the project are displayed.
     *
     * \see setGeographicCoordinateFormat()
     * \see geographicCoordinateFormatChanged()
     */
    const QgsGeographicCoordinateNumericFormat *geographicCoordinateFormat() const;

    /**
     * Returns default coordinate type for the project.
     * \see setCoordinateType()
     * \since QGIS 3.28
     */
    Qgis::CoordinateDisplayType coordinateType() const { return mCoordinateType; }

    /**
     * Sets the default coordinate \a type for the project.
     * \see coordinateType()
     * \since QGIS 3.28
     */
    void setCoordinateType( Qgis::CoordinateDisplayType type );

    QgsCoordinateReferenceSystem coordinateCustomCrs() const { return mCoordinateCustomCrs; }

    void setCoordinateCustomCrs( const QgsCoordinateReferenceSystem &crs );

    /**
     * Reads the settings's state from a DOM element.
     * \see writeXml()
     */
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context );

    /**
     * Returns a DOM element representing the settings.
     * \see readXml()
     */
    QDomElement writeXml( QDomDocument &doc, const QgsReadWriteContext &context ) const;

  signals:

    /**
     * Emitted when the bearing format changes.
     *
     * \see setBearingFormat()
     * \see bearingFormat()
     */
    void bearingFormatChanged();

    /**
     * Emitted when the geographic coordinate format changes.
     *
     * \see setGeographicCoordinateFormat()
     * \see geographicCoordinateFormat()
     */
    void geographicCoordinateFormatChanged();

    /**
     * Emitted when the default coordinate format changes.
     *
     * \see setCoordinateType()
     * \see coordinateType()
     * \since QGIS 3.28
     */
    void coordinateTypeChanged();

    void coordinateCustomCrsChanged();

  private:
    std::unique_ptr< QgsBearingNumericFormat > mBearingFormat;
    std::unique_ptr< QgsGeographicCoordinateNumericFormat > mGeographicCoordinateFormat;

    Qgis::CoordinateDisplayType mCoordinateType = Qgis::CoordinateDisplayType::MapCrs;
    QgsCoordinateReferenceSystem mCoordinateCustomCrs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );

};

#endif // QGSPROJECTDISPLAYSETTINGS_H
