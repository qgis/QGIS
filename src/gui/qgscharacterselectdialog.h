/***************************************************************************
    qgscharacterselectdialog.h - single font character selector dialog

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

#ifndef QGSCHARACTERSELECTDIALOG_H
#define QGSCHARACTERSELECTDIALOG_H

#include <QDialog>
#include <QChar>
#include "qgisgui.h"
#include "ui_qgscharacterselectdialogbase.h"

class CharacterWidget;

/** \ingroup gui
 * A dialog for selecting a single character from a single font
  */

class GUI_EXPORT QgsCharacterSelectorDialog : public QDialog, private Ui::QgsCharacterSelectorBase
{
    Q_OBJECT

  public:
    QgsCharacterSelectorDialog( QWidget* parent = nullptr, const Qt::WindowFlags& fl = QgisGui::ModalDialogFlags );
    ~QgsCharacterSelectorDialog();

  public slots:
    const QChar& selectCharacter( bool* gotChar, const QFont& font, const QString& style );

  private slots:
    void setCharacter( QChar chr );

  protected:
    QChar mChar;
    CharacterWidget* mCharWidget;
};

#endif
