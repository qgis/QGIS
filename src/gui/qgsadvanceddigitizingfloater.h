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

class QgsMapCanvas;
class QgsAdvancedDigitizingDockWidget;

/**
* \ingroup gui
* \brief The QgsAdvancedDigitizingFloater class is widget that floats
* next to the mouse pointer, and allow interaction with the AdvancedDigitizing
* feature. It proxies display and actions to QgsMapToolAdvancedDigitizingDockWidget.
*/
class GUI_EXPORT QgsAdvancedDigitizingFloater : public QWidget, private Ui::QgsAdvancedDigitizingFloaterBase
{
    Q_OBJECT

  public:

    explicit QgsAdvancedDigitizingFloater( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget );

    /**
    * Whether the floater is active or not.
    * Note that the floater may be active but not visible (e.g. if the mouse is not over the canvas).
    * \since QGIS 3.8
    */
    bool active();

  public slots:

    /**
    * Set whether the floater should be active or not.
    * Note that the floater may be active but not visible (e.g. if the mouse is not over the canvas).
    * \since QGIS 3.8
    *
    * \param active
    */
    void setActive( bool active );

  private slots:

    void changeX( const QString &text );
    void changeY( const QString &text );
    void changeDistance( const QString &text );
    void changeAngle( const QString &text );
    void changeLockX( bool locked );
    void changeLockY( bool locked );
    void changeLockDistance( bool locked );
    void changeLockAngle( bool locked );
    void changeRelativeX( bool relative );
    void changeRelativeY( bool relative );
    // void changeRelativeDistance( bool relative );  // doesn't happen
    void changeRelativeAngle( bool relative );
    void focusOnX();
    void focusOnY();
    void focusOnAngle();
    void focusOnDistance();
    void enabledChangedX( bool enabled );
    void enabledChangedY( bool enabled );
    void enabledChangedAngle( bool enabled );
    void enabledChangedDistance( bool enabled );

  private:

    //! pointer to map canvas
    QgsMapCanvas *mMapCanvas = nullptr;

    //! pointer to map cad docker widget
    QgsAdvancedDigitizingDockWidget *mCadDockWidget = nullptr;

    /**
    * event filter to track mouse position
    * \note defined as private in Python bindings
    */
    bool eventFilter( QObject *obj, QEvent *event ) override SIP_SKIP;

    /**
    * Move the widget to a new cursor position. A hard-coded offet will be added.
    * \param pos position of the cursor
    */
    void updatePos( const QPoint &pos );

    /**
    * Hides the widget if either the floater or the cadDockWidget is disabled.
    */
    void hideIfDisabled();

    //! Whether the floater is enabled.
    bool mActive = false;

};

#endif // QGSADVANCEDDIGITIZINGFLOATER_H
