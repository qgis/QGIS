/***************************************************************************
    qgssourcefieldsproperties.h
    ---------------------
    begin                : July 2017
    copyright            : (C) 2017 by David Signer
    email                : david at opengis dot ch

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSOURCEFIELDSPROPERTIES_H
#define QGSSOURCEFIELDSPROPERTIES_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "ui_qgssourcefieldsproperties.h"

#include "qgis_gui.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsfieldcalculator.h"
#include "qgslogger.h"

#include <QDropEvent>
#include <QFileDialog>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QMimeData>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QWidget>

/**
 * \ingroup gui
 * \class QgsSourceFieldsProperties
 *
 * \brief A widget which displays information about vector layer fields, and allows some configuration of them.
 */
class GUI_EXPORT QgsSourceFieldsProperties : public QWidget, private Ui_QgsSourceFieldsProperties
{
    Q_OBJECT

  public:
    explicit QgsSourceFieldsProperties( QgsVectorLayer *layer, QWidget *parent = nullptr );

    void init();
    void apply();

    void loadRows();
    void setRow( int row, int idx, const QgsField &field );

    /**
     * Adds an attribute to the table (but does not commit it yet)
     * \param field the field to add
     * \returns FALSE in case of a name conflict, TRUE in case of success
    */
    bool addAttribute( const QgsField &field );

  protected:
    void updateButtons();

    QgsVectorLayer *mLayer = nullptr;

    // Holds all the first column items (header: id) of the table.
    // The index in the list is the fieldIdx, and therefore acts as a mapping
    // between fieldIdx and QTableWidgetItem->row()
    QList<QTableWidgetItem *> mIndexedWidgets;

    enum AttrColumns
    {
      AttrIdCol = 0,
      AttrNameCol,
      AttrAliasCol,
      AttrTypeCol,
      AttrTypeNameCol,
      AttrLengthCol,
      AttrPrecCol,
      AttrCommentCol,
      AttrConfigurationFlagsCol,
      AttrColCount,
    };

  private:
    void updateFieldRenamingStatus();

  signals:
    void toggleEditing();

  private slots:

    void updateExpression();

    //! Editing of layer was toggled
    void editingToggled();
    void addAttributeClicked();
    void deleteAttributeClicked();
    void calculateFieldClicked();
    void saveLayerEditsClicked();

    void attributeAdded( int idx );
    void attributeDeleted( int idx );

    void attributesListCellChanged( int row, int column );
    void attributesListCellPressed( int row, int column );

    void applyToLayer( QgsVectorLayer *layer );

    friend class QgsAttributesFormProperties;
};

#endif // QGSSOURCEFIELDSPROPERTIES_H
