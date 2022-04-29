/***************************************************************************
                         qgsmeshlayerelevationproperties.h
                         ---------------
    begin                : February 2022
    copyright            : (C) 2022 by Nyall Dawson
    email                : nyall dot dawson dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSMESHLAYERELEVATIONPROPERTIES_H
#define QGSMESHLAYERELEVATIONPROPERTIES_H

#include "qgis_core.h"
#include "qgis_sip.h"
#include "qgsmaplayerelevationproperties.h"
#include "qgis.h"

class QgsLineSymbol;
class QgsFillSymbol;

/**
 * \class QgsMeshLayerElevationProperties
 * \ingroup core
 * \brief Mesh layer specific subclass of QgsMapLayerElevationProperties.
 *
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsMeshLayerElevationProperties : public QgsMapLayerElevationProperties
{

    Q_OBJECT

  public:

    /**
     * Constructor for QgsMeshLayerElevationProperties, with the specified \a parent object.
     */
    QgsMeshLayerElevationProperties( QObject *parent SIP_TRANSFERTHIS );
    ~QgsMeshLayerElevationProperties() override;

    bool hasElevation() const override;
    QDomElement writeXml( QDomElement &element, QDomDocument &doc, const QgsReadWriteContext &context ) override;
    bool readXml( const QDomElement &element, const QgsReadWriteContext &context ) override;
    QString htmlSummary() const override;
    QgsMeshLayerElevationProperties *clone() const override SIP_FACTORY;
    bool isVisibleInZRange( const QgsDoubleRange &range ) const override;
    QgsDoubleRange calculateZRange( QgsMapLayer *layer ) const override;
    bool showByDefaultInElevationProfilePlots() const override;

    /**
     * Returns the line symbol used to render the mesh profile in elevation profile plots.
     *
     * \see setProfileLineSymbol()
     */
    QgsLineSymbol *profileLineSymbol() const;

    /**
     * Sets the line \a symbol used to render the mesh profile in elevation profile plots.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * \see profileLineSymbol()
     */
    void setProfileLineSymbol( QgsLineSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the fill symbol used to render the mesh profile in elevation profile plots.
     *
     * \see setProfileFillSymbol()
     */
    QgsFillSymbol *profileFillSymbol() const;

    /**
     * Sets the fill \a symbol used to render the mesh profile in elevation profile plots.
     *
     * Ownership of \a symbol is transferred to the plot.
     *
     * \see profileFillSymbol()
     */
    void setProfileFillSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

    /**
     * Returns the symbology option used to render the mesh profile in elevation profile plots.
     *
     * \see setProfileSymbology()
     */
    Qgis::ProfileSurfaceSymbology profileSymbology() const { return mSymbology; }

    /**
     * Sets the \a symbology option used to render the mesh profile in elevation profile plots.
     *
     * \see setProfileSymbology()
     */
    void setProfileSymbology( Qgis::ProfileSurfaceSymbology symbology );

  private:

    void setDefaultProfileLineSymbol( const QColor &color );
    void setDefaultProfileFillSymbol( const QColor &color );

    std::unique_ptr< QgsLineSymbol > mProfileLineSymbol;
    std::unique_ptr< QgsFillSymbol > mProfileFillSymbol;
    Qgis::ProfileSurfaceSymbology mSymbology = Qgis::ProfileSurfaceSymbology::Line;
};

#endif // QGSMESHLAYERELEVATIONPROPERTIES_H
