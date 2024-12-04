/***************************************************************************
                         qgsnewauxiliaryfielddialog.h  -  description
                             -------------------
    begin                : Sept 05, 2017
    copyright            : (C) 2017 by Paul Blottiere
    email                : paul.blottiere@oslandia.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNEWAUXILIARYFIELDDIALOG_H
#define QGSNEWAUXILIARYFIELDDIALOG_H

#include "ui_qgsnewauxiliaryfielddialogbase.h"
#include "qgsguiutils.h"
#include "qgis_gui.h"
#include "qgsproperty.h"

/**
 * \ingroup gui
 *
 * \brief A dialog to create a new auxiliary field
 *
 */
class GUI_EXPORT QgsNewAuxiliaryFieldDialog : public QDialog, private Ui::QgsNewAuxiliaryFieldDialogBase
{
    Q_OBJECT

  public:
    /**
     * Constructor.
     *
     * \param definition The property definition to use to create the auxiliary field
     * \param layer The vector layer for which the auxiliary layer has to be created
     * \param nameOnly TRUE to indicate that only the name widget is enabled
     * \param parent Parent window
     */
    QgsNewAuxiliaryFieldDialog( const QgsPropertyDefinition &definition, QgsVectorLayer *layer, bool nameOnly = true, QWidget *parent = nullptr );

    /**
     * Returns the underlying property definition.
     */
    QgsPropertyDefinition propertyDefinition() const;

  protected:
    void accept() override;

    QgsVectorLayer *mLayer = nullptr;
    bool mNameOnly = true;
    QgsPropertyDefinition mPropertyDefinition;
};

#endif
