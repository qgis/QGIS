/***************************************************************************
  qgsorganizetablecolumnsdialog.h - dialog for attribute table
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

#ifndef QGSORGANIZEFILTERTABLESDIALOG_H_
#define QGSORGANIZEFILTERTABLESDIALOG_H_

#include <QDialog>

#include "ui_qgsorganizetablecolumnsdialog.h"

#include "qgsattributetableconfig.h"

class QgsVectorLayer;

/** \class QgsOrganizeTableColumnsDialog
 * \ingroup gui
 * Dialog for organising (hiding and reordering) columns in the attributes table.
 * \note added in QGIS 2.16
 */
class GUI_EXPORT QgsOrganizeTableColumnsDialog : public QDialog, private Ui::QgsOrganizeTableColumnsDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * @param vl The concerned vector layer
     * @param parent parent object
     * @param flags window flags
     */
    QgsOrganizeTableColumnsDialog( const QgsVectorLayer* vl, QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::Window );

    /**
     * Destructor
     */
    ~QgsOrganizeTableColumnsDialog();

    /**
     * Get the updated configuration
     */
    QgsAttributeTableConfig config() const;

  public slots:
    /**
     * showAll checks all the  fields to show them all in the attribute table
     */
    void showAll();

    /**
     * hideAll unchecks all the fields to hide them all in the attribute table
     */
    void hideAll();

  private:
    QgsAttributeTableConfig mConfig;

};

#endif
