/***************************************************************************
                         qgscomposertablebackgroundcolorsdialog.h
                         ----------------------------------------
    begin                : August 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOMPOSERTABLEBACKGROUNDCOLORSDIALOG_H
#define QGSCOMPOSERTABLEBACKGROUNDCOLORSDIALOG_H

#include <QDialog>
#include "ui_qgscomposertablebackgroundstyles.h"
#include "qgscomposertablev2.h"

class QCheckBox;
class QgsColorButtonV2;

/** A dialog for customisation of the cell background colors for a QgsComposerTableV2
 * /note added in QGIS 2.12
*/
class QgsComposerTableBackgroundColorsDialog: public QDialog, private Ui::QgsComposerTableBackgroundDialog
{
    Q_OBJECT
  public:

    /** Constructor for QgsComposerTableBackgroundColorsDialog
     * @param table associated composer table
     * @param parent parent widget
     * @param flags window flags
     */
    QgsComposerTableBackgroundColorsDialog( QgsComposerTableV2* table, QWidget* parent = nullptr, Qt::WindowFlags flags = nullptr );

    ~QgsComposerTableBackgroundColorsDialog();

  private slots:

    void apply();

    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

  private:

    QgsComposerTableV2* mComposerTable;
    QMap< QgsComposerTableV2::CellStyleGroup, QCheckBox* > mCheckBoxMap;
    QMap< QgsComposerTableV2::CellStyleGroup, QgsColorButtonV2* > mColorButtonMap;


    /** Sets the GUI elements to the values of the table*/
    void setGuiElementValues();


};

#endif // QGSCOMPOSERTABLEBACKGROUNDCOLORSDIALOG_H
