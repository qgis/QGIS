/***************************************************************************
    qgscolordialog.h - color selection dialog

    ---------------------
    begin                : March 19, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOLORDIALOG_H
#define QGSCOLORDIALOG_H

#include <QColorDialog>
#include "qgisgui.h"

class QColor;

/** \ingroup gui
 * \class QgsColorDialog
 * A dialog for selecting a color
 */

class GUI_EXPORT QgsColorDialog : public QObject
{
    Q_OBJECT

  public:
    QgsColorDialog();
    ~QgsColorDialog();

    /** Return a color selection from a QColorDialog, with live updating of interim selections.
     * @param initialColor The initial color of the selection dialog.
     * @param updateObject The receiver object of the live updating.
     * @param updateSlot The receiver object's slot for live updating (e.g. SLOT( setValidColor( const QColor& ) ) ).
     * @param parent Parent widget. Usually 0 is best for native system color dialogs.
     * @param title The title of the QColorDialog.
     * @param options ColorDialogOptions passed to QColorDialog.
     * @return Selected color on accepted() or initialColor on rejected().
     */
    static QColor getLiveColor( const QColor& initialColor, QObject* updateObject, const char* updateSlot,
                                QWidget* parent = 0,
                                const QString& title = "",
                                QColorDialog::ColorDialogOptions options = 0 );
};

#endif // #ifndef QGSCOLORDIALOG_H
