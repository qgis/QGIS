/***************************************************************************
    qgsprojectstylesettings.h
    ---------------------------
    begin                : May 2022
    copyright            : (C) 2022 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSPROJECTSTYLESETTINGS_H
#define QGSPROJECTSTYLESETTINGS_H

#include "qgis_core.h"
#include "qgstextformat.h"
#include "qgswkbtypes.h"

#include <memory.h>

class QDomElement;
class QgsReadWriteContext;
class QDomDocument;
class QgsProject;
class QgsSymbol;
class QgsColorRamp;

/**
 * \brief Contains settings and properties relating to how a QgsProject should handle
 * styling.
 *
 * \ingroup core
 * \since QGIS 3.26
 */
class CORE_EXPORT QgsProjectStyleSettings : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProjectStyleSettings for the specified \a project.
     *
     * Ownership is transferred to the \a project.
     */
    QgsProjectStyleSettings( QgsProject *project = nullptr );

    /**
     * Returns the project default symbol for a given geometry type.
     * \note the symbol ownership is transferred to the caller
     * \param geomType the geometry type
     * \returns a symbol pointer or NULL if there is no default set
     */
    QgsSymbol *defaultSymbol( QgsWkbTypes::GeometryType geomType ) const;

    /**
     * Sets the project default symbol for a given geometry type.
     * \note the symbol ownership is not transferred
     * \param geomType the geometry type
     * \param symbol the symbol pointer, set to NULL to clear default
     */
    void setDefaultSymbol( QgsWkbTypes::GeometryType geomType, QgsSymbol *symbol );

    /**
     * Returns the project default color ramp.
     * \note the color ramp ownership is transferred to the caller
     * \returns a color ramp pointer or NULL if there is no default set
     */
    QgsColorRamp *defaultColorRamp() const;

    /**
     * Sets the project default color ramp.
     * \note the color ramp ownership is not transferred
     * \param colorRamp the color ramp, set to NULL to clear default
     */
    void setDefaultColorRamp( QgsColorRamp *colorRamp );

    /**
     * Returns the project default text format.
     * \note if no default is defined, the returned format will be invalid
     */
    QgsTextFormat defaultTextFormat() const;

    /**
     * Sets the project default text format.
     * \param textFormat the text format, an invalid format is interpreted as no default
     */
    void setDefaultTextFormat( QgsTextFormat textFormat );

    /**
     * Resets the settings to a default state.
     */
    void reset();

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

  private:

    QgsProject *mProject = nullptr;

    std::unique_ptr< QgsSymbol > mDefaultMarkerSymbol;
    std::unique_ptr< QgsSymbol > mDefaultLineSymbol;
    std::unique_ptr< QgsSymbol > mDefaultFillSymbol;
    std::unique_ptr< QgsColorRamp > mDefaultColorRamp;

    QgsTextFormat mDefaultTextFormat;

};

#endif // QGSPROJECTSTYLESETTINGS_H
