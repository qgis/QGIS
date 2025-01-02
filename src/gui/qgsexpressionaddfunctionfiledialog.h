/***************************************************************************
    qgsexpressionaddfunctionfiledialog.h
    ---------------------
    begin                : May 2024
    copyright            : (C) 2024 by Germán Carrillo
    email                : german at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSIONADDFUNCTIONFILEDIALOG_H
#define QGSEXPRESSIONADDFUNCTIONFILEDIALOG_H
#define SIP_NO_FILE

#include "qgis_gui.h"
#include <QDialog>
#include "ui_qgsexpressionaddfunctionfiledialogbase.h"

/**
 * \ingroup gui
 * \brief A dialog to select whether to create a function file or project functions.
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsExpressionAddFunctionFileDialog : public QDialog, private Ui::QgsExpressionAddFunctionFileDialogBase
{
    Q_OBJECT
  public:
    /**
     * Creates a QgsExpressionAddFunctionFileDialog to create function files or to set the current
     * project as a function container.
     * \a enableProjectFunctions defines whether the dialog will enable the 'Project function' option
     * or not, which is useful to avoid the creation of multiple 'Project Functions' items in the
     * Function Editor file list.
     * \a parent is the parent widget.
     */
    QgsExpressionAddFunctionFileDialog( bool enableProjectFunctions, QWidget *parent );

    /**
     * Returns whether user has selected to create project functions
     */
    bool createProjectFunctions() const;

    /**
     * Returns the new file name
     */
    QString fileName();

  private slots:
    void cboFileOptions_currentIndexChanged( int );

  private:
    void updateOkButtonStatus();
};

#endif // QGSEXPRESSIONADDFUNCTIONFILEDIALOG_H
