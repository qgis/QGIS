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
#include "qgis_gui.h"
#include "qgis_sip.h"

class QgsVectorLayer;

/**
 * \class QgsOrganizeTableColumnsDialog
 * \ingroup gui
 * \brief Dialog for organising (hiding and reordering) columns in the attributes table.
 */
class GUI_EXPORT QgsOrganizeTableColumnsDialog : public QDialog, private Ui::QgsOrganizeTableColumnsDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor
     * \param vl The concerned vector layer
     * \param parent parent object
     * \param config attribute table config to use.
     * \param flags window flags
     */
    QgsOrganizeTableColumnsDialog( const QgsVectorLayer *vl, const QgsAttributeTableConfig &config, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = Qt::Window );

    ///@cond PRIVATE

    /**
     * Constructor
     * \param vl The concerned vector layer
     * \param parent parent object
     * \param flags window flags
     */
    QgsOrganizeTableColumnsDialog( const QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = Qt::Window );
    ///@endcond

    /**
     * Gets the updated configuration
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

    /**
     * Toggle the check state of selected fields to hide or show them in the attribute table
     * \since QGIS 3.36
     */
    void toggleSelection();

  private:
    QgsAttributeTableConfig mConfig;
};

#endif
