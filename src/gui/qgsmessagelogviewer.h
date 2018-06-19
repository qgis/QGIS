/***************************************************************************
                          qgsmessagelogviewer.h  -  description
                             -------------------
    begin                : October 2011
    copyright            : (C) 2011 by Juergen E. Fischer
    email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMESSAGELOGVIEWER_H
#define QGSMESSAGELOGVIEWER_H

#include "ui_qgsmessagelogviewer.h"
#include "qgsguiutils.h"
#include "qgsmessagelog.h"

#include <QString>
#include "qgis_gui.h"
#include "qgis.h"

class QStatusBar;
class QCloseEvent;

/**
 * \ingroup gui
 * A generic dialog widget for displaying QGIS log messages.
 */
class GUI_EXPORT QgsMessageLogViewer: public QDialog, private Ui::QgsMessageLogViewer
{
    Q_OBJECT
  public:

    /**
     * Create a new message log viewer. The viewer will automatically connect to the system's
     * QgsApplication::messageLog() instance.
     */
    QgsMessageLogViewer( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

  public slots:

    /**
     * Logs a \a message to the viewer.
     */
    void logMessage( const QString &message, const QString &tag, Qgis::MessageLevel level );

  protected:
    void closeEvent( QCloseEvent *e ) override;
    void reject() override;

  private slots:
    void closeTab( int index );
};

#endif
