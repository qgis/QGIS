/***************************************************************************
                          qgsmaptoolfreezelabels.h
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

#ifndef QGSMAPTOOLFREEZELABELS_H
#define QGSMAPTOOLFREEZELABELS_H

#include "qgsmaptoollabel.h"
#include "qgsrectangle.h"
#include "qgslegend.h"
#include "qgscoordinatetransform.h"

class QgsHighlight;
class QgsLabelPosition;

/**A map tool for freezing (writing to attribute table) and thawing label positions and rotation*/
class QgsMapToolFreezeLabels: public QgsMapToolLabel
{
    Q_OBJECT

  public:
    QgsMapToolFreezeLabels( QgsMapCanvas *canvas );
    ~QgsMapToolFreezeLabels();

    //! Overridden mouse move event
    virtual void canvasMoveEvent( QMouseEvent * e );

    //! Overridden mouse press event
    virtual void canvasPressEvent( QMouseEvent * e );

    //! Overridden mouse release event
    virtual void canvasReleaseEvent( QMouseEvent * e );

    bool isShowingFrozen() const { return mShowFrozen; }
    void setShowingFrozen( bool showing ) { mShowFrozen = showing; }

    //! Called when Show Frozen Labels tool is toggled, via its qgisapp.cpp slot
    void showFrozenLabels( bool show );

    //! Remove rectangles from around frozen labels
    void removeFrozenHighlights();

  public slots:

    //! Update frozen label highlights on layer edit mode change
    void updateFrozenLabels();

    //! Render highlight rectangles around frozen labels
    void highlightFrozenLabels();

  protected:

    //! Mapping of feature ids of layers that have been highlighted
    QMap<QString, QgsHighlight*> mHighlights;

    //! Flag to indicate a map canvas drag operation is taking place
    bool mDragging;
    //! Flag to indicate whether to draw the highlight for frozen labels
    bool mShowFrozen;

    //! Stores actual select rect
    QRect mSelectRect;

    //! Stores selection marquee
    QgsRubberBand* mRubberBand;

  private:

    //! Pointer to map renderer
    QgsMapRenderer* mRender;

    //! Highlights a given label relative to whether its frozen and editable
    void highlightLabel( QgsVectorLayer* vlayer,
                         const QgsLabelPosition& labelpos,
                         const QString& id,
                         const QColor& color );

    //! Select valid labels to freeze or thaw
    void freezeThawLabels( const QgsRectangle& ext, QMouseEvent * e  );

    //! Freeze or thaw label relative to whether its editable
    bool freezeThawLabel( QgsVectorLayer* vlayer,
                          const QgsLabelPosition& labelpos,
                          bool freeze );

    //! Hide chosen label by setting font size to 0
    bool hideLabel( QgsVectorLayer* vlayer, const QgsLabelPosition& labelpos );
};

#endif // QGSMAPTOOLFREEZELABELS_H
