/***************************************************************************
    qgsadvanceddigitizingfloater.cpp  -  floater for CAD tools
    ----------------------
    begin                : May 2019
    copyright            : (C) Olivier Dalang
    email                : olivier.dalang@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSADVANCEDDIGITIZINGFLOATER
#define QGSADVANCEDDIGITIZINGFLOATER

#include <QWidget>
#include <QString>

#include "ui_qgsadvanceddigitizingfloaterbase.h"
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QPointer>

class QgsMapCanvas;
class QgsAdvancedDigitizingDockWidget;

/**
* \ingroup gui
* \brief The QgsAdvancedDigitizingFloater class is widget that floats
* next to the mouse pointer, and allow interaction with the AdvancedDigitizing
* feature. It proxies display and actions to QgsMapToolAdvancedDigitizingDockWidget.
* \note This class is a technology preview and unstable API.
* \since QGIS 3.8
*/
class GUI_EXPORT QgsAdvancedDigitizingFloater : public QWidget, private Ui::QgsAdvancedDigitizingFloaterBase
{
    Q_OBJECT

  public:
    //! Available floater items
    enum class FloaterItem : int SIP_ENUM_BASETYPE( IntFlag )
    {
      XCoordinate = 1 << 1,
      YCoordinate = 1 << 2,
      MCoordinate = 1 << 3,
      ZCoordinate = 1 << 4,
      Angle = 1 << 5,
      CommonAngleSnapping = 1 << 6,
      Distance = 1 << 7,
      Bearing = 1 << 8,
    };
    Q_DECLARE_FLAGS( FloaterItems, FloaterItem )
    Q_FLAG( FloaterItem )


    /**
     * Create an advanced digitizing floater widget
     * \param canvas The map canvas on which the widget operates
     * \param cadDockWidget The cadDockWidget to which the floater belongs
     * \since QGIS 3.8
     */
    explicit QgsAdvancedDigitizingFloater( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

    /**
    * Whether the floater is active or not.
    * Note that the floater may be active but not visible (e.g. if the mouse is not over the canvas).
    * \since QGIS 3.8
    */
    bool active();

    /**
    * Returns TRUE if the floater \a item visibility setting is enabled.
    *
    * \param item floater item
    * \since QGIS 3.32
    */
    bool itemVisibility( const QgsAdvancedDigitizingFloater::FloaterItem &item ) const;

  public slots:

    /**
    * Set whether the floater should be active or not.
    * Note that the floater may be active but not visible (e.g. if the mouse is not over the canvas).
    *
    * \param active
    * \since QGIS 3.8
    */
    void setActive( bool active );

    /**
    * Set whether the floater \a item should be visible or not.
    *
    * \param item floater item
    * \param visible
    * \since QGIS 3.32
    */
    void setItemVisibility( const QgsAdvancedDigitizingFloater::FloaterItem &item, bool visible );

  private slots:

    void changeX( const QString &text );
    void changeY( const QString &text );
    void changeZ( const QString &text );
    void changeM( const QString &text );
    void changeCommonAngleSnapping( double angle );
    void changeDistance( const QString &text );
    void changeAngle( const QString &text );
    void changeBearing( const QString &text );
    void changeLockX( bool locked );
    void changeLockY( bool locked );
    void changeLockZ( bool locked );
    void changeLockM( bool locked );
    void changeLockDistance( bool locked );
    void changeLockAngle( bool locked );
    void changeRelativeX( bool relative );
    void changeRelativeY( bool relative );
    void changeRelativeZ( bool relative );
    void changeRelativeM( bool relative );
    // void changeRelativeDistance( bool relative );  // doesn't happen
    void changeRelativeAngle( bool relative );
    void focusOnX();
    void focusOnY();
    void focusOnZ();
    void focusOnM();
    void focusOnAngle();
    void focusOnDistance();
    void enabledChangedX( bool enabled );
    void enabledChangedY( bool enabled );
    void enabledChangedZ( bool enabled );
    void enabledChangedM( bool enabled );
    void enabledChangedAngle( bool enabled );
    void enabledChangedDistance( bool enabled );
    void enabledCommonAngleSnapping( bool enabled );
    void enabledChangedBearing( bool enabled );

  private:
    //! pointer to map canvas
    QgsMapCanvas *mMapCanvas = nullptr;

    //! pointer to map cad docker widget
    QPointer<QgsAdvancedDigitizingDockWidget> mCadDockWidget;

    /**
    * event filter to track mouse position
    * \note defined as private in Python bindings
    */
    bool eventFilter( QObject *obj, QEvent *event ) override SIP_FORCE;

    /**
    * Move the widget to a new cursor position. A hard-coded offset will be added.
    * \param pos position of the cursor
    */
    void updatePos( const QPoint &pos );

    /**
    * Hides the widget if either the floater or the cadDockWidget is disabled.
    */
    void hideIfDisabled();

    //! Whether the floater is enabled.
    bool mActive = false;

    FloaterItems mItemsVisibility;
};

#endif // QGSADVANCEDDIGITIZINGFLOATER_H
