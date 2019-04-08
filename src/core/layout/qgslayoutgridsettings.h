/***************************************************************************
                             qgslayoutgridsettings.h
                             -----------------------
    begin                : July 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTGRIDSETTINGS_H
#define QGSLAYOUTGRIDSETTINGS_H

#include "qgis_core.h"
#include "qgslayoutmeasurement.h"
#include "qgslayoutpoint.h"
#include "qgslayoutundocommand.h"
#include "qgslayoutserializableobject.h"
#include <QPen>

class QgsLayout;
class QgsReadWriteContext;

/**
 * \ingroup core
 * \class QgsLayoutGridSettings
 * \brief Contains settings relating to the appearance, spacing and offset for layout grids.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutGridSettings : public QgsLayoutSerializableObject
{

  public:

    //! Style for drawing the page/snapping grid
    enum Style
    {
      StyleLines, //!< Solid lines
      StyleDots, //!< Dots
      StyleCrosses //!< Crosses
    };

    /**
     * Constructor for QgsLayoutGridSettings.
     */
    QgsLayoutGridSettings( QgsLayout *layout );

    QString stringType() const override { return QStringLiteral( "LayoutGrid" ); }
    QgsLayout *layout() override;

    /**
     * Sets the page/snap grid \a resolution.
     * \see resolution()
     * \see setOffset()
     */
    void setResolution( QgsLayoutMeasurement resolution );

    /**
     * Returns the page/snap grid resolution.
     * \see setResolution()
     * \see offset()
     */
    QgsLayoutMeasurement resolution() const { return mGridResolution;}

    /**
     * Sets the \a offset of the page/snap grid.
     * \see offset()
     * \see setResolution()
     */
    void setOffset( const QgsLayoutPoint &offset );

    /**
     * Returns the offset of the page/snap grid.
     * \see setOffset()
     * \see resolution()
     */
    QgsLayoutPoint offset() const { return mGridOffset; }

    /**
     * Sets the \a pen used for drawing page/snap grids.
     * \see pen()
     * \see setStyle()
     */
    void setPen( const QPen &pen ) { mGridPen = pen; }

    /**
     * Returns the pen used for drawing page/snap grids.
     * \see setPen()
     * \see style()
     */
    QPen pen() const { return mGridPen; }

    /**
     * Sets the \a style used for drawing the page/snap grids.
     * \see style()
     * \see setPen()
     */
    void setStyle( const Style style ) { mGridStyle = style; }

    /**
     * Returns the style used for drawing the page/snap grids.
     * \see setStyle()
     * \see pen()
     */
    Style style() const { return mGridStyle; }

    /**
     * Loads grid settings from the application layout settings.
     */
    void loadFromSettings();

    /**
     * Stores the grid's state in a DOM element. The \a parentElement should refer to the parent layout's DOM element.
     * \see readXml()
     */
    bool writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext &context ) const override;

    /**
     * Sets the grid's state from a DOM element. gridElement is the DOM node corresponding to the grid.
     * \see writeXml()
     */
    bool readXml( const QDomElement &gridElement, const QDomDocument &document, const QgsReadWriteContext &context ) override;

  private:

    // Used for 'collapsing' undo commands
    enum UndoCommand
    {
      UndoGridResolution = 1,
      UndoGridOffset,
    };

    QgsLayoutMeasurement mGridResolution;
    QgsLayoutPoint mGridOffset;
    QPen mGridPen;
    Style mGridStyle = StyleLines;
    QgsLayout *mLayout = nullptr;

};

#endif //QGSLAYOUTGRIDSETTINGS_H
