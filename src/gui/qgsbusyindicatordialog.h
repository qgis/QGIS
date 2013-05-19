/***************************************************************************
                          qgsbusyindicatordialog.h
                          ------------------------
    begin                : Mar 27, 2013
    copyright            : (C) 2013 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSBUSYINDICATORDIALOG_H
#define QGSBUSYINDICATORDIALOG_H

#include "qgisgui.h"

#include <QDialog>
#include <QLabel>


/** \ingroup gui
 * \class QgsBusyIndicatorDialog
 * A simple dialog to show an indeterminate busy progress indicator.
 */
class GUI_EXPORT QgsBusyIndicatorDialog : public QDialog
{
    Q_OBJECT
  public:
    /** Constructor
     * Modal busy indicator dialog with no buttons.
     * @param message Text to show above busy progress indicator.
     * @param parent parent object (owner)
     * @param fl widget flags
     * @note added in 1.9
    */
    QgsBusyIndicatorDialog( const QString& message = "", QWidget *parent = 0, Qt::WFlags fl = QgisGui::ModalDialogFlags );
    ~QgsBusyIndicatorDialog();

    QString message() const { return mMessage; }
    void setMessage( const QString& message );

  private:
    QString mMessage;
    QLabel* mMsgLabel;
};

#endif // QGSBUSYINDICATORDIALOG_H
