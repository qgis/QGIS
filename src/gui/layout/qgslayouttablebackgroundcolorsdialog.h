/***************************************************************************
                         qgslayouttablebackgroundcolorsdialog.h
                         ----------------------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#ifndef QGSLAYOUTTABLEBACKGROUNDCOLORSDIALOG_H
#define QGSLAYOUTTABLEBACKGROUNDCOLORSDIALOG_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include <QDialog>
#include "qgis_gui.h"
#include "ui_qgslayouttablebackgroundstyles.h"
#include "qgslayouttable.h"

class QCheckBox;
class QgsColorButton;

/**
 * \ingroup gui
 * A dialog for customization of the cell background colors for a QgsLayoutTable
 *
 * \note This class is not a part of public API
 * \since QGIS 3.0
*/
class GUI_EXPORT QgsLayoutTableBackgroundColorsDialog: public QDialog, private Ui::QgsLayoutTableBackgroundDialog
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsLayoutTableBackgroundColorsDialog
     * \param table associated layout table
     * \param parent parent widget
     * \param flags window flags
     */
    QgsLayoutTableBackgroundColorsDialog( QgsLayoutTable *table, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );

  private slots:

    void apply();

    void buttonBox_accepted();
    void buttonBox_rejected();
    void showHelp();

  private:

    QgsLayoutTable *mTable = nullptr;
    QMap< QgsLayoutTable::CellStyleGroup, QCheckBox * > mCheckBoxMap;
    QMap< QgsLayoutTable::CellStyleGroup, QgsColorButton * > mColorButtonMap;


    //! Sets the GUI elements to the values of the table
    void setGuiElementValues();


};

#endif // QGSLAYOUTTABLEBACKGROUNDCOLORSDIALOG_H
