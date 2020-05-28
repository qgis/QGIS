/***************************************************************************
    qgscharacterselectordialog.h - single font character selector dialog

    ---------------------
    begin                : November 2012
    copyright            : (C) 2012 by Larry Shaffer
    email                : larrys at dakcarto dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCHARACTERSELECTORDIALOG_H
#define QGSCHARACTERSELECTORDIALOG_H

#include <QDialog>
#include <QChar>
#include "qgsguiutils.h"
#include "ui_qgscharacterselectdialogbase.h"
#include "qgis_gui.h"
#include "qgis_sip.h"

class CharacterWidget;

/**
 * \ingroup gui
 * A dialog for selecting a single character from a single font
  */

class GUI_EXPORT QgsCharacterSelectorDialog : public QDialog, private Ui::QgsCharacterSelectorBase
{
    Q_OBJECT

  public:
    QgsCharacterSelectorDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags );

  public slots:

    /**
     * Opens the dialog modally and returns when the user has selected a character.
     *
     * If \a initialSelection is specified, then that character will be initially selected in the dialog.
     */
    QChar selectCharacter( bool *gotChar, const QFont &font, const QString &style, QChar initialSelection = QChar() );

  private slots:
    void setCharacter( QChar chr );

  protected:
    QChar mChar;
    CharacterWidget *mCharWidget = nullptr;
};

#endif
