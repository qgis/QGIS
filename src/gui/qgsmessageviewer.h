/***************************************************************************
                          qgsmessageviewer.h  -  description
                             -------------------
    begin                : Wed Jun 4 2003
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMESSAGEVIEWER_H
#define QGSMESSAGEVIEWER_H

#include "ui_qgsmessageviewer.h"
#include "qgsguiutils.h"
#include "qgsmessageoutput.h"
#include "qgis_gui.h"

#include <QString>


/**
 * \ingroup gui
 * A generic message view for displaying QGIS messages.
 */
class GUI_EXPORT QgsMessageViewer: public QDialog, public QgsMessageOutput, private Ui::QgsMessageViewer
{
    Q_OBJECT
  public:
    QgsMessageViewer( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, bool deleteOnClose = true );

    void setMessage( const QString &message, MessageType msgType ) override;

    void appendMessage( const QString &message ) override;

    void showMessage( bool blocking = true ) override;

    void setTitle( const QString &title ) override;

    // Call one of the setMessage...() functions first.
    // Subsequent calls to appendMessage use the format as determined
    // by the call to setMessage...()

    // Treats the given text as html.
    void setMessageAsHtml( const QString &msg );
    // Treats the given text as plain text
    void setMessageAsPlainText( const QString &msg );
    // A checkbox that can be used for something like
    // "don't show this message again"
    void setCheckBoxText( const QString &text );
    // Make the checkbox visible/invisible
    void setCheckBoxVisible( bool visible );
    // Sets the check state
    void setCheckBoxState( Qt::CheckState state );
    // Get checkbox state
    Qt::CheckState checkBoxState();
    // Specifies a QgsSettings tag to store/retrieve the checkbox
    // state to/from. Use an empty QString to disable this feature.
    void setCheckBoxQgsSettingsLabel( const QString &label );

  private slots:
    void checkBox_toggled( bool );


  private:
    QString mCheckBoxQgsSettingsLabel;
};

#endif
