/***************************************************************************
                         qgsmodelgroupboxdefinitionwidget.h
                         ----------------------------------------
    begin                : March 2020
    copyright            : (C) 2020 by Nyall Dawson
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


#ifndef QGSPROCESSINGGROUPBBOXDEFINITIONWIDGET_H
#define QGSPROCESSINGGROUPBBOXDEFINITIONWIDGET_H

#include <QWidget>
#include <QDialog>
#include "qgsprocessingmodelgroupbox.h"
#include "qgis_gui.h"

#define SIP_NO_FILE

class QLineEdit;
class QCheckBox;
class QTabWidget;
class QTextEdit;
class QgsColorButton;
class QgsProcessingModelGroupBox;


/**
 * A widget which allow users to specify the properties of a model group box.
 *
 * \ingroup gui
 * \note Not available in Python bindings.
 * \since QGIS 3.14
 */
class GUI_EXPORT QgsModelGroupBoxDefinitionDialog : public QDialog
{
    Q_OBJECT
  public:
    /**
     * Constructor for QgsModelGroupBoxDefinitionWidget, for the specified group \a box.
     */
    QgsModelGroupBoxDefinitionDialog( const QgsProcessingModelGroupBox &box, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a new instance of the group box, using the current settings defined in the dialog.
     */
    QgsProcessingModelGroupBox groupBox() const;

  private:
    QTextEdit *mCommentEdit = nullptr;
    QgsColorButton *mCommentColorButton = nullptr;
    QgsProcessingModelGroupBox mBox;
};


#endif // QGSPROCESSINGGROUPBBOXDEFINITIONWIDGET_H
