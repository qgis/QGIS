/***************************************************************************
  qgswidgetstatehelper_p.h - QgsWidgetStateHelper

 ---------------------
 begin                : 3.12.2017
 copyright            : (C) 2017 by Nathan Woodrow
 Email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSWIDGETSTATEHELPER_P_H
#define QGSWIDGETSTATEHELPER_P_H

#include <QMap>
#include <QObject>

#define SIP_NO_FILE

/**
 * \ingroup gui
 * \brief QgsWidgetStateHelper is a helper class to save and restore the geometry of QWidgets in the application.
 * This removes the need for devs to remember to call saveGeometry() and restoreGeometry() when writing new widgets.
 *
 * This helper is internal and should only be called via QgsGui::enabledAutoGeometryRestore
 */
class QgsWidgetStateHelper : public QObject
{
    Q_OBJECT
  public:
    /**
     * QgsWidgetStateHelper
     * \param parent Parent object
     */
    explicit QgsWidgetStateHelper( QObject *parent = nullptr );

    /**
     * Event filter to catch events from registered widgets.
     * \param object Object getting the event.
     * \param event Event sent from Qt.
     * \return Always returns TRUE so that widget still gets event.
     */
    bool eventFilter( QObject *object, QEvent *event ) override;

    /**
     * Register a widget to have it geometry state automatically saved and restored.
     * \param widget The widget to save. Must have objectName() set.
     * \param key The override settings key name to use if objectName() isn't to be used.
     * objectName() is the default if not set.
     */
    void registerWidget( QWidget *widget, const QString &key = QString() );

  private:
    QMap<QString, QString> mKeys;

    /**
     * Returns a non null safe name for the widget.
     * \param widget The widget.
     * \return A non null safe name for the widget.
     */
    QString widgetSafeName( QWidget *widget );
};

#endif // QGSWIDGETSTATEHELPER_P_H
