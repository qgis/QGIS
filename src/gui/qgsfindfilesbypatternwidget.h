/***************************************************************************
    qgsfindfilesbypatternwidget.h
    -----------------------------
    begin                : April 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFINDFILESBYPATTERNWIDGET_H
#define QGSFINDFILESBYPATTERNWIDGET_H

#include "ui_qgsfindfilesbypatternwidget.h"
#include "qgis_gui.h"

#include <QDialog>

class QDialogButtonBox;

/**
 * \class QgsFindFilesByPatternWidget
 * \ingroup gui
 * \brief A reusable widget for finding files (recursively) by file pattern.
 * \since QGIS 3.8
 */
class GUI_EXPORT QgsFindFilesByPatternWidget : public QWidget, private Ui::QgsFindFilesByPatternWidgetBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsFindFilesByPatternWidget, with the specified \a parent widget.
     */
    QgsFindFilesByPatternWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the list of files found by the dialog. This may be empty if
     * no matching files were found.
     * \see findComplete()
     */
    QStringList files() const { return mFiles; }

  signals:

    /**
     * Emitted after files are found in the dialog.
     *
     * The \a files argument contains a list of all matching files found. This may be empty if
     * no matching files were found.
     *
     * \see files()
     */
    void findComplete( const QStringList &files );

  private slots:

    void find();
    void cancel();

  private:
    bool mCanceled = false;
    QStringList mFiles;
};

/**
 * \class QgsFindFilesByPatternDialog
 * \ingroup gui
 * \brief A dialog for finding files (recursively) by file pattern.
 * \since QGIS 3.8
 */
class GUI_EXPORT QgsFindFilesByPatternDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsFindFilesByPatternDialog, with the specified \a parent widget.
     */
    QgsFindFilesByPatternDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns the list of files found by the dialog. This may be empty if
     * no matching files were found.
     */
    QStringList files() const;

  private:
    QgsFindFilesByPatternWidget *mWidget = nullptr;
    QDialogButtonBox *mButtonBox = nullptr;
};

#endif // QGSFINDFILESBYPATTERNWIDGET_H
