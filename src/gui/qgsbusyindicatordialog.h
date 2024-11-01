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

#include "qgsguiutils.h"

#include <QDialog>
#include <QLabel>
#include "qgis_gui.h"
#include "qgis_sip.h"


/**
 * \ingroup gui
 * \class QgsBusyIndicatorDialog
 * \brief A simple dialog to show an indeterminate busy progress indicator.
 */
class GUI_EXPORT QgsBusyIndicatorDialog : public QDialog
{
    Q_OBJECT
  public:
    /**
     * Constructor
     * Modal busy indicator dialog with no buttons.
     * \param message Text to show above busy progress indicator.
     * \param parent parent object (owner)
     * \param fl widget flags
     */
    QgsBusyIndicatorDialog( const QString &message = QString(), QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

    QString message() const { return mMessage; }
    void setMessage( const QString &message );

  private:
    QString mMessage;
    QLabel *mMsgLabel = nullptr;
};

// clazy:excludeall=qstring-allocations

#endif // QGSBUSYINDICATORDIALOG_H
