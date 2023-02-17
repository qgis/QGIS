/***************************************************************************
  qgssnapindicator.h
  --------------------------------------
  Date                 : October 2017
  Copyright            : (C) 2017 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSNAPINDICATOR_H
#define QGSSNAPINDICATOR_H

#include "qgspointlocator.h"
#include "qobjectuniqueptr.h"
#include "qgis_gui.h"

class QgsMapCanvas;
class QgsVertexMarker;


/**
 * \ingroup gui
 * \brief Class that shows snapping marker on map canvas for the current snapping match.
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsSnapIndicator
{
  public:
    //! Constructs an indicator for the given map canvas
    QgsSnapIndicator( QgsMapCanvas *canvas );
    ~QgsSnapIndicator();

    //! Sets snapping match that should be displayed in map canvas. Invalid match hides the indicator
    void setMatch( const QgsPointLocator::Match &match );
    //! Returns currently displayed snapping match
    QgsPointLocator::Match match() const { return mMatch; }

    //! Sets whether the snapping indicator is visible
    void setVisible( bool visible = true );
    //! Returns whether the snapping indicator is visible
    bool isVisible() const;

  private:
    Q_DISABLE_COPY( QgsSnapIndicator )

#ifdef SIP_RUN
    QgsSnapIndicator( const QgsSnapIndicator &rh );
    QgsSnapIndicator &operator=( const QgsSnapIndicator & );
#endif

    QgsMapCanvas *mCanvas = nullptr;
    QgsPointLocator::Match mMatch;
    QObjectParentUniquePtr< QgsVertexMarker > mSnappingMarker;
};

#endif // QGSSNAPINDICATOR_H
