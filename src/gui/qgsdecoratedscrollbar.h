/***************************************************************************
    qgsdecoratedscrollbar.h
     --------------------------------------
    Date                 : May 2024
    Copyright            : (C) 2024 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDECORATEDSCROLLBAR_H
#define QGSDECORATEDSCROLLBAR_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QColor>
#include <QHash>
#include <QPointer>
#include <QWidget>
#include <QMap>

class QScrollBar;
class QAbstractScrollArea;
class QgsScrollBarHighlightOverlay;

// ported from QtCreator's HighlightScrollBarController implementation

/**
 * \ingroup gui
 * \brief Encapsulates the details of a highlight in a scrollbar, used alongside QgsScrollBarHighlightController.
 * \since QGIS 3.38
 */
class GUI_EXPORT QgsScrollBarHighlight
{
  public:
    /**
    * Priority, which dictates how overlapping highlights are rendered
    */
    enum class Priority : int
    {
      Invalid = -1,       //!< Invalid
      LowPriority = 0,    //!< Low priority, rendered below all other highlights
      NormalPriority = 1, //!< Normal priority
      HighPriority = 2,   //!< High priority
      HighestPriority = 3 //!< Highest priority, rendered above all other highlights
    };

    /**
    * Constructor for QgsScrollBarHighlight.
    */
    QgsScrollBarHighlight( int category, int position, const QColor &color, QgsScrollBarHighlight::Priority priority = QgsScrollBarHighlight::Priority::NormalPriority );

    QgsScrollBarHighlight() = default;

    //! Category ID
    int category = -1;

    //! Position in scroll bar
    int position = -1;

    //! Highlight color
    QColor color;

    //! Priority, which dictates how overlapping highlights are rendered
    QgsScrollBarHighlight::Priority priority = QgsScrollBarHighlight::Priority::Invalid;
};

/**
 * \ingroup gui
 * \brief Adds highlights (colored markers) to a scrollbar.
 * \since QGIS 3.38
 */
class GUI_EXPORT QgsScrollBarHighlightController
{
  public:
    QgsScrollBarHighlightController();
    ~QgsScrollBarHighlightController();

    /**
     * Returns the associated scroll bar.
     */
    QScrollBar *scrollBar() const;

    /**
     * Returns the associated scroll area.
     *
     * \see setScrollArea()
     */
    QAbstractScrollArea *scrollArea() const;

    /**
     * Sets the associated scroll bar.
     *
     * \see scrollArea()
     */
    void setScrollArea( QAbstractScrollArea *scrollArea );

    /**
     * Returns the line height for text associated with the scroll area.
     *
     * \see setLineHeight()
     */
    double lineHeight() const;

    /**
     * Sets the line \a height for text associated with the scroll area.
     *
     * \see lineHeight()
     */
    void setLineHeight( double height );

    /**
     * Returns the visible range of the scroll area (i.e. the viewport's height).
     *
     * \see setVisibleRange()
     */
    double visibleRange() const;

    /**
     * Sets the visible range of the scroll area (i.e. the viewport's height).
     *
     * \see visibleRange()
     */
    void setVisibleRange( double visibleRange );

    /**
     * Returns the document margins for the associated viewport.
     *
     * \see setMargin()
     */
    double margin() const;

    /**
     * Sets the document \a margin for the associated viewport.
     *
     * \see margin()
     */
    void setMargin( double margin );

    /**
     * Returns the hash of all highlights in the scrollbar, with highlight categories as hash keys.
     *
     * \note Not available in Python bindings
     */
    QHash<int, QVector<QgsScrollBarHighlight>> highlights() const SIP_SKIP;

    /**
     * Adds a \a highlight to the scrollbar.
     */
    void addHighlight( const QgsScrollBarHighlight &highlight );

    /**
     * Removes all highlights with matching \a category from the scrollbar.
     */
    void removeHighlights( int category );

    /**
     * Removes all highlights from the scroll bar.
     */
    void removeAllHighlights();

  private:
    QHash<int, QVector<QgsScrollBarHighlight>> mHighlights;
    double mLineHeight = 0.0;
    double mVisibleRange = 0.0; // in pixels
    double mMargin = 0.0;       // in pixels
    QAbstractScrollArea *mScrollArea = nullptr;
    QPointer<QgsScrollBarHighlightOverlay> mOverlay;
};

///@cond PRIVATE
#ifndef SIP_RUN
class QgsScrollBarHighlightOverlay : public QWidget
{
    Q_OBJECT

  public:
    QgsScrollBarHighlightOverlay( QgsScrollBarHighlightController *scrollBarController );

    void doResize();
    void doMove();
    void scheduleUpdate();

  protected:
    void paintEvent( QPaintEvent *paintEvent ) override;
    bool eventFilter( QObject *object, QEvent *event ) override;

  private:
    void drawHighlights( QPainter *painter, int docStart, int docSize, double docSizeToHandleSizeRatio, int handleOffset, const QRect &viewport );
    void updateCache();
    QRect overlayRect() const;
    QRect handleRect() const;

    // line start to line end
    QMap<QgsScrollBarHighlight::Priority, QMap<QRgb, QMap<int, int>>> mHighlightCache;

    inline QScrollBar *scrollBar() const { return mHighlightController->scrollBar(); }
    QgsScrollBarHighlightController *mHighlightController = nullptr;
    bool mIsCacheUpdateScheduled = true;
};
#endif
///@endcond PRIVATE

#endif // QGSDECORATEDSCROLLBAR_H
