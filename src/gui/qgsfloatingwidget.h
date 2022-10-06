/***************************************************************************
    qgsfloatingwidget.h
    -------------------
    begin                : April 2016
    copyright            : (C) Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSFLOATINGWIDGET_H
#define QGSFLOATINGWIDGET_H

#include <QWidget>
#include <QPointer>
#include "qgis_sip.h"
#include "qgis_gui.h"

class QgsFloatingWidgetEventFilter;

/**
 * \ingroup gui
 * \class QgsFloatingWidget
 * \brief A QWidget subclass for creating widgets which float outside of the normal Qt layout
 * system. Floating widgets use an "anchor widget" to determine how they are anchored
 * within their parent widget.
 * \since QGIS 3.0
 */

class GUI_EXPORT QgsFloatingWidget: public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QWidget *anchorWidget READ anchorWidget WRITE setAnchorWidget NOTIFY anchorWidgetChanged )
    Q_PROPERTY( AnchorPoint anchorPoint READ anchorPoint WRITE setAnchorPoint NOTIFY anchorPointChanged )
    Q_PROPERTY( AnchorPoint anchorWidgetPoint READ anchorWidgetPoint WRITE setAnchorWidgetPoint NOTIFY anchorWidgetPointChanged )

  public:

    //! Reference points for anchoring widget position
    enum AnchorPoint
    {
      TopLeft, //!< Top-left of widget
      TopMiddle, //!< Top center of widget
      TopRight, //!< Top-right of widget
      MiddleLeft, //!< Middle left of widget
      Middle, //!< Middle of widget
      MiddleRight, //!< Middle right of widget
      BottomLeft, //!< Bottom-left of widget
      BottomMiddle, //!< Bottom center of widget
      BottomRight, //!< Bottom-right of widget
    };
    Q_ENUM( AnchorPoint )

    /**
     * Constructor for QgsFloatingWidget.
     * \param parent parent widget
     */
    QgsFloatingWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Sets the widget to "anchor" the floating widget to. The floating widget will be repositioned whenever the
     * anchor widget moves or is resized so that it maintains the same relative position to the anchor widget.
     * \param widget anchor widget. Both the floating widget and the anchor widget must share some common parent.
     * \see anchorWidget()
     */
    void setAnchorWidget( QWidget *widget );

    /**
     * Returns the widget that the floating widget is "anchored" tto. The floating widget will be repositioned whenever the
     * anchor widget moves or is resized so that it maintains the same relative position to the anchor widget.
     * \see setAnchorWidget()
     */
    QWidget *anchorWidget();

    /**
     * Returns the floating widget's anchor point, which corresponds to the point on the widget which should remain
     * fixed in the same relative position whenever the widget's parent is resized or moved.
     * \see setAnchorPoint()
     */
    AnchorPoint anchorPoint() const { return mFloatAnchorPoint; }

    /**
     * Sets the floating widget's anchor point, which corresponds to the point on the widget which should remain
     * fixed in the same relative position whenever the widget's parent is resized or moved.
     * \param point anchor point
     * \see anchorPoint()
     */
    void setAnchorPoint( AnchorPoint point );

    /**
     * Returns the anchor widget's anchor point, which corresponds to the point on the anchor widget which
     * the floating widget should "attach" to. The floating widget should remain fixed in the same relative position
     * to this anchor widget whenever the widget's parent is resized or moved.
     * \see setAnchorWidgetPoint()
     */
    AnchorPoint anchorWidgetPoint() const { return mAnchorWidgetAnchorPoint; }

    /**
     * Returns the anchor widget's anchor point, which corresponds to the point on the anchor widget which
     * the floating widget should "attach" to. The floating widget should remain fixed in the same relative position
     * to this anchor widget whenever the widget's parent is resized or moved.
     * \see setAnchorWidgetPoint()
     */
    void setAnchorWidgetPoint( AnchorPoint point );

  signals:

    //! Emitted when the anchor widget changes
    void anchorWidgetChanged( QWidget *widget );

    //! Emitted when the anchor point changes
    void anchorPointChanged( QgsFloatingWidget::AnchorPoint point );

    //! Emitted when the anchor widget point changes
    void anchorWidgetPointChanged( QgsFloatingWidget::AnchorPoint point );

  protected:
    void showEvent( QShowEvent *e ) override;
    void paintEvent( QPaintEvent *e ) override;
    void resizeEvent( QResizeEvent *e ) override;

  private slots:

    //! Repositions the floating widget to a changed anchor point
    void onAnchorPointChanged();

  private:

    QPointer< QWidget > mAnchorWidget;
    QgsFloatingWidgetEventFilter *mParentEventFilter = nullptr;
    QgsFloatingWidgetEventFilter *mAnchorEventFilter = nullptr;
    AnchorPoint mFloatAnchorPoint = BottomMiddle;
    AnchorPoint mAnchorWidgetAnchorPoint = TopMiddle;

};


#ifndef SIP_RUN

/// @cond PRIVATE

class QgsFloatingWidgetEventFilter: public QObject
{
    Q_OBJECT

  public:

    QgsFloatingWidgetEventFilter( QWidget *parent = nullptr );

    bool eventFilter( QObject *object, QEvent *event ) override;

  signals:

    //! Emitted when the filter's parent is moved or resized
    void anchorPointChanged();

};

/// @endcond

#endif

#endif // QGSFLOATINGWIDGET_H
