/***************************************************************************
  QgsFilterTableFieldsDialog.h - dialog for attribute table
  -------------------
         date                 : Feb 2016
         copyright            : Stéphane Brunner
         email                : stephane.brunner@gmail.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSFILTERTABLEFIELDSDIALOG_H_
#define QGSFILTERTABLEFIELDSDIALOG_H_

#include <QDialog>
#include <QModelIndex>
#include <QItemSelectionModel>

#include <time.h>

#include "ui_qgsfiltertablefieldsdialog.h"
#include "qgscontexthelp.h"

#include "qgsattributedialog.h"
#include "qgsvectorlayer.h" //QgsFeatureIds
#include "qgsfieldmodel.h"
#include "qgssearchwidgetwrapper.h"
#include <QDockWidget>

class QDialogButtonBox;
class QPushButton;
class QLineEdit;
class QComboBox;
class QMenu;
class QSignalMapper;
class QgsAttributeTableModel;
class QgsAttributeTableFilterModel;
class QgsRubberBand;

class GUI_EXPORT QgsFilterTableFieldsDialog : public QDialog, private Ui::QgsFilterTableFieldsDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * @param vl The concerned vector layer
     * @param visible the current list of visible fields name
     * @param parent parent object
     * @param flags window flags
     */
    QgsFilterTableFieldsDialog( const QgsVectorLayer* vl, QStringList visible, QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::Window );
    /**
     * Destructor
     */
    ~QgsFilterTableFieldsDialog();

    /**
     * Get the selected fields name
     * @return The selected fields name
     */
    QStringList selectedFields();
};

#endif
