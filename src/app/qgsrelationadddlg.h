/***************************************************************************
    qgsrelationadddlg.h
    ---------------------
    begin                : December 2015
    copyright            : (C) 2015 by Matthias Kuhn
    email                : matthias at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRELATIONADDDLG_H
#define QGSRELATIONADDDLG_H

#include "ui_qgsrelationmanageradddialogbase.h"

#include "qgis.h"
#include "qgis_app.h"

#include <QDialog>

class QDialogButtonBox;
class QComboBox;
class QLineEdit;
class QSpacerItem;
class QToolButton;
class QVBoxLayout;
class QHBoxLayout;

class QgsVectorLayer;
class QgsFieldComboBox;
class QgsMapLayerComboBox;

/**
 * QgsCreateRelationDialog allows configuring a new relation.
 * Multiple field pairs can be set.
 */
class APP_EXPORT QgsCreateRelationDialog : public QDialog, private Ui::QgsRelationManagerAddDialogBase
{
    Q_OBJECT

  public:
    explicit QgsCreateRelationDialog( QWidget *parent = nullptr );

    [[nodiscard]] QString referencingLayerId() const;
    [[nodiscard]] QString referencedLayerId() const;
    [[nodiscard]] QList<QPair<QString, QString>> references() const;
    [[nodiscard]] QString relationId() const;
    [[nodiscard]] QString relationName() const;
    [[nodiscard]] Qgis::RelationshipStrength relationStrength() const;

  private slots:
    void addFieldsRow();
    void removeFieldsRow();
    void updateFieldsMappingButtons();
    void updateFieldsMappingHeaders();
    void updateDialogButtons();
    void updateChildRelationsComboBox();
    void updateReferencedFieldsComboBoxes();
    void updateReferencingFieldsComboBoxes();

  private:
    bool isDefinitionValid();
    void updateFieldsMapping();

    QgsMapLayerComboBox *mReferencedLayerCombobox = nullptr;
    QgsMapLayerComboBox *mReferencingLayerCombobox = nullptr;
};

#endif // QGSRELATIONADDDLG_H
