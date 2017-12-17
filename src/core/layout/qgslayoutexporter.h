/***************************************************************************
                              qgslayoutexporter.h
                             -------------------
    begin                : October 2017
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
#ifndef QGSLAYOUTEXPORTER_H
#define QGSLAYOUTEXPORTER_H

#include "qgis_core.h"
#include <QPointer>

class QgsLayout;
class QPainter;

/**
 * \ingroup core
 * \class QgsLayoutExporter
 * \brief Handles rendering and exports of layouts to various formats.
 * \since QGIS 3.0
 */
class CORE_EXPORT QgsLayoutExporter
{

  public:

    /**
     * Constructor for QgsLayoutExporter, for the specified \a layout.
     */
    QgsLayoutExporter( QgsLayout *layout );

    /**
     * Renders a full page to a destination \a painter.
     *
     * The \a page argument specifies the page number to render. Page numbers
     * are 0 based, such that the first page in a layout is page 0.
     *
     * \see renderRect()
     */
    void renderPage( QPainter *painter, int page );

    /**
     * Renders a \a region from the layout to a \a painter. This method can be used
     * to render sections of pages rather than full pages.
     *
     * \see renderPage()
     */
    void renderRegion( QPainter *painter, const QRectF &region );

  private:

    QPointer< QgsLayout > mLayout;
};

#endif //QGSLAYOUTEXPORTER_H



