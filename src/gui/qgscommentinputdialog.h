/***************************************************************************
    qgscommentinputdialog.h
    ---------------------
    begin                : September 2025
    copyright            : (C) 2025 by Jan Caha
    email                : jan.caha at outlook dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMMENTINPUTDIALOG_H
#define QGSCOMMENTINPUTDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QPlainTextEdit>

#include "qgis_gui.h"
/**
 * \ingroup gui
 * \brief Dialog which displays comment for a specific item.
 *
 * \since QGIS 4.0
 */
class GUI_EXPORT QgsCommentInputDialog : public QDialog
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsCommentInputDialog.
     */
    explicit QgsCommentInputDialog( const QString &title, const QString &comment = QString(), QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the comment for the current item.
     */
    QString comment() const;

  private:
    QDialogButtonBox *mButtonBox = nullptr;
    QPlainTextEdit *mCommentEdit = nullptr;
};
#endif // QGSCOMMENTINPUTDIALOG_H
