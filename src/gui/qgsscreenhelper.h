/***************************************************************************
     qgsscreenhelper.h
     ---------------
    Date                 : August 2022
    Copyright            : (C) 2022 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSSCREENHELPER_H
#define QGSSCREENHELPER_H

#include "qgis_gui.h"
#include "qgis_sip.h"

#include <QObject>
#include <QRect>

class QWidget;
class QScreen;
class QWindow;

/**
 * \ingroup gui
 * \class QgsScreenHelper
 * \brief A utility class for dynamic handling of changes to screen properties.
 *
 * \since QGIS 3.28
 */
class GUI_EXPORT QgsScreenHelper : public QObject
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsScreenHelper for the specified parent \a widget.
     */
    QgsScreenHelper( QWidget *parent SIP_TRANSFERTHIS );

    /**
     * Returns the screen that the parent widget appears on, or NULLPTR.
     */
    QScreen *screen();

    /**
     * Returns the window handle for the window the parent widget is associated with, or NULLPTR.
     */
    QWindow *windowHandle();

    /**
     * Returns the current screen DPI for the screen that the parent widget appears on.
     *
     * \see screenDpiChanged()
     */
    double screenDpi() const { return mScreenDpi; }

    /**
     * Returns the current screen available geometry in pixels.
     *
     * The available geometry is the geometry excluding window manager reserved areas such as task bars and system menus.
     *
     * \see availableGeometryChanged()
     */
    QRect availableGeometry() const { return mAvailableGeometry; }

  signals:

    /**
     * Emitted whenever the screen \a dpi associated with the widget is changed.
     *
     * \see screenDpi()
     */
    void screenDpiChanged( double dpi );

    /**
     * Emitted whenever the available geometry of the screen associated with the widget is changed.
     *
     * \see availableGeometry()
     */
    void availableGeometryChanged( const QRect &geometry );

  protected:
    bool eventFilter( QObject *watched, QEvent *event ) override;

  private slots:

    void updateDevicePixelFromScreen();
    void updateAvailableGeometryFromScreen();

  private:

    QWidget *mWidget = nullptr;

    double mScreenDpi = 96.0;
    QMetaObject::Connection mScreenDpiChangedConnection;

    QRect mAvailableGeometry;
    QMetaObject::Connection mAvailableGeometryChangedConnection;

};


#endif // QGSSCREENHELPER_H
