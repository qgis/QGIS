/***************************************************************************
                             qgslayoutruler.h
                             ----------------
    Date                 : July 2017
    Copyright            : (C) 2017 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSLAYOUTRULER_H
#define QGSLAYOUTRULER_H

#include "qgis_gui.h"
#include "qgis_sip.h"
#include <QWidget>
#include <QPointer>
#include <QMenu>
#include <memory>

class QgsLayout;
class QGraphicsLineItem;
class QgsLayoutView;
class QgsLayoutGuide;

/**
 * \ingroup gui
 * \brief A custom ruler widget for use with QgsLayoutView, displaying the
 * current zoom and position of the visible layout and for interacting
 * with guides in a layout.
 *
 * \since QGIS 3.0
 */
class GUI_EXPORT QgsLayoutRuler: public QWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLayoutRuler, with the specified \a parent widget and \a orientation.
     */
    explicit QgsLayoutRuler( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::Orientation orientation = Qt::Horizontal );

    QSize minimumSizeHint() const override;

    /**
     * Sets the current scene \a transform. This is usually the transform set for a view
     * showing the associated scene, in order to synchronize the view's display of
     * the scene with the rulers.
     */
    void setSceneTransform( const QTransform &transform );

    /**
     * Returns the current layout view associated with the ruler.
     * \see setLayoutView()
     */
    QgsLayoutView *layoutView() { return mView; }

    /**
     * Sets the current layout \a view to synchronize the ruler with.
     * \see layoutView()
     */
    void setLayoutView( QgsLayoutView *view );

    /**
     * Returns the ruler size (either the height of a horizontal ruler or the
     * width of a vertical rule).
     */
    int rulerSize() const { return mRulerMinSize; }

    /**
     * Sets a context \a menu to show when right clicking occurs on the ruler.
     * Ownership of \a menu is unchanged.
     */
    void setContextMenu( QMenu *menu );

  public slots:

    /**
     * Updates the \a position of the marker showing the current mouse position within
     * the view.
     * \a position is in layout coordinates.
     */
    void setCursorPosition( QPointF position );

  signals:
    //! Emitted when mouse cursor coordinates change
    void cursorPosChanged( QPointF );

  protected:
    void paintEvent( QPaintEvent *event ) override;
    void mouseMoveEvent( QMouseEvent *event ) override;
    void mousePressEvent( QMouseEvent *event ) override;
    void mouseReleaseEvent( QMouseEvent *event ) override;

  private:
    static const int VALID_SCALE_MULTIPLES[];
    static const int VALID_SCALE_MAGNITUDES[];

    Qt::Orientation mOrientation = Qt::Horizontal;
    QgsLayoutView *mView = nullptr;

    QTransform mTransform;
    QPoint mMarkerPos;

    QFont mRulerFont;
    std::unique_ptr< QFontMetrics > mRulerFontMetrics;

    double mScaleMinPixelsWidth = 0.0;
    int mRulerMinSize;
    int mMinPixelsPerDivision;
    int mPixelsBetweenLineAndText;
    int mTextBaseline;
    int mMinSpacingVerticalLabels;

    int mDragGuideTolerance = 0;
    QgsLayoutGuide *mDraggingGuide = nullptr;
    double mDraggingGuideOldPosition = 0.0;
    QgsLayoutGuide *mHoverGuide = nullptr;

    bool mCreatingGuide = false;
    QGraphicsLineItem *mGuideItem = nullptr;

    //! Polygon for drawing guide markers
    QPolygonF mGuideMarker;

    QPointer< QMenu > mMenu;

    //! Calculates the optimum labeled units for ruler so that labels are a good distance apart
    int optimumScale( double minPixelDiff, int &magnitude, int &multiple );

    /**
     * Calculate the number of small divisions for each ruler unit, ensuring that they
     * are sufficiently spaced.
     */
    int optimumNumberDivisions( double rulerScale, int scaleMultiple );

    //! Draws vertical text on a painter
    void drawRotatedText( QPainter *painter, QPointF pos, const QString &text );

    /**
     * Draws small ruler divisions.
     * Starting at startPos in mm, for numDivisions divisions, with major division spacing of rulerScale (in mm)
     * Stop drawing if position exceeds maxPos
     */
    void drawSmallDivisions( QPainter *painter, double startPos, int numDivisions, double rulerScale, double maxPos = 0 );

    //! Draw current marker pos on ruler
    void drawMarkerPos( QPainter *painter );

    void drawGuideMarkers( QPainter *painter, QgsLayout *layout );

    //! Draw a guide marker on the ruler
    void drawGuideAtPos( QPainter *painter, QPoint pos );

    void createTemporaryGuideItem();

    QPointF convertLocalPointToLayout( QPoint localPoint ) const;

    QPoint convertLayoutPointToLocal( QPointF layoutPoint ) const;

    /**
     * Returns the closest guide to a local ruler point, or NULLPTR if no guides
     * are within the acceptable tolerance of the point.
     */
    QgsLayoutGuide *guideAtPoint( QPoint localPoint ) const;

};

#endif // QGSLAYOUTRULER_H
