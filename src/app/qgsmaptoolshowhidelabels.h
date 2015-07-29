/***************************************************************************
                          qgsmaptoolshowhidelabels.h
                          --------------------
    begin                : 2012-08-12
    copyright            : (C) 2012 by Larry Shaffer
    email                : larrys at dakotacarto dot com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPTOOLSHOWHIDELABELS_H
#define QGSMAPTOOLSHOWHIDELABELS_H

#include "qgsmaptoollabel.h"
#include "qgsfeature.h"


/** A map tool for showing or hidding a feature's label*/
class APP_EXPORT QgsMapToolShowHideLabels : public QgsMapToolLabel
{
    Q_OBJECT

  public:
    QgsMapToolShowHideLabels( QgsMapCanvas *canvas );
    ~QgsMapToolShowHideLabels();

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e ) override;

    //! Overridden mouse press event
    virtual void canvasPressEvent( QMouseEvent * e ) override;

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QMouseEvent * e ) override;

  protected:

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;

    //! Stores actual select rect
    QRect mSelectRect;

    //! Stores selection marquee
    QgsRubberBand* mRubberBand;

  private:

    //! Select valid labels to pin or unpin
    void showHideLabels( QMouseEvent * e );

    //! Return features intersecting rubberband
    bool selectedFeatures( QgsVectorLayer* vlayer,
                           QgsFeatureIds& selectedFeatIds );

    //! Return label features intersecting rubberband
    bool selectedLabelFeatures( QgsVectorLayer* vlayer,
                                QgsFeatureIds& selectedFeatIds );

    //! Show or hide chosen label by setting data defined Show Label to 0
    bool showHideLabel( QgsVectorLayer* vlayer,
                        const QgsFeatureId &fid,
                        bool hide );
};

#endif // QGSMAPTOOLSHOWHIDELABELS_H
