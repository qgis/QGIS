/***************************************************************************
                          qgsmaptoolpinlabels.h
                          --------------------
    begin                : 2012-07-12
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

#ifndef QGSMAPTOOLPINLABELS_H
#define QGSMAPTOOLPINLABELS_H

#include "qgsmaptoollabel.h"
#include "qgsrectangle.h"
#include "qgscoordinatetransform.h"

class QgsRubberBand;
class QgsLabelPosition;

/** A map tool for pinning (writing to attribute table) and unpinning labelpositions and rotation*/
class APP_EXPORT QgsMapToolPinLabels: public QgsMapToolLabel
{
    Q_OBJECT

  public:
    QgsMapToolPinLabels( QgsMapCanvas *canvas );
    ~QgsMapToolPinLabels();

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse press event
    virtual void canvasPressEvent( QgsMapMouseEvent* e ) override;

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QgsMapMouseEvent* e ) override;

    bool isShowingPinned() const { return mShowPinned; }
    void setShowingPinned( bool showing ) { mShowPinned = showing; }

    //! Called when Show Pinned Labels tool is toggled, via its qgisapp.cpp slot
    void showPinnedLabels( bool show );

    //! Remove rectangles from around pinned labels
    void removePinnedHighlights();

  public slots:

    //! Update pinned label highlights on layer edit mode change
    void updatePinnedLabels();

    //! Render highlight rectangles around pinned labels
    void highlightPinnedLabels();

  protected:

    //! Mapping of feature ids of layers that have been highlighted
    QMap<QString, QgsRubberBand*> mHighlights;

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;
    //! Flag to indicate whether to draw the highlight for pinned labels
    bool mShowPinned;

    //! Stores actual select rect
    QRect mSelectRect;

    //! Stores selection marquee
    QgsRubberBand* mRubberBand;

  private:

    //! Highlights a given label relative to whether its pinned and editable
    void highlightLabel( const QgsLabelPosition& labelpos,
                         const QString& id,
                         const QColor& color );

    //! Select valid labels to pin or unpin
    void pinUnpinLabels( const QgsRectangle& ext, QMouseEvent * e );

    //! Pin or unpin current label relative to whether its editable
    bool pinUnpinCurrentLabel( bool pin );

    //! Pin or unpin diagram relative to whether its editable
    bool pinUnpinCurrentDiagram( bool pin );

    //! Pin or unpin current feature (diagram or label)
    bool pinUnpinCurrentFeature( bool pin );
};

#endif // QGSMAPTOOLPINLABELS_H
