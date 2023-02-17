/***************************************************************************
    qgsrelationaddpolymorphicdialog.h
    ---------------------
    begin                : December 2020
    copyright            : (C) 2020 by Ivan Ivanov
    email                : ivan at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSRELATIONADDPOLYMORPHICDIALOG_H
#define QGSRELATIONADDPOLYMORPHICDIALOG_H

#include <QDialog>
#include "qgis_app.h"
#include "ui_qgsrelationmanageraddpolymorphicdialogbase.h"
#include "qgis.h"

class QgsPolymorphicRelation;

/**
 * QgsRelationAddDlg allows configuring a new relation.
 * Multiple field pairs can be set.
 */
class APP_EXPORT QgsRelationAddPolymorphicDialog : public QDialog, private Ui::QgsRelationManagerAddPolymorphicDialogBase
{
    Q_OBJECT

  public:
    explicit QgsRelationAddPolymorphicDialog( bool isEditDialog, QWidget *parent = nullptr );

    /**
     * Returns the id of the referencing layer
     */
    QString referencingLayerId() const;

    /**
     * Returns the field in the referencing layer that stores the referenced layer representation
     */
    QString referencedLayerField() const;

    /**
     * Returns the expression used to generate the referenced layer representation
     */
    QString referencedLayerExpression() const;

    /**
     * Returns field pairs
     */
    QList< QPair< QString, QString > > fieldPairs() const;

    /**
     * Returns the polymorphic relation id
     */
    QString relationId() const;

    /**
     * Returns the polymorphic relation name
     */
    QString relationName() const;

    /**
     * Returns a list of layer ids used as referenced layers and stored in the referencing layers
     */
    QStringList referencedLayerIds() const;

    /**
      * Return the relation strength for the generated normal relations
      */
    Qgis::RelationshipStrength relationStrength() const;

    /**
     * Sets the values of form fields in the dialog with the values of the passed \a polyRel
     */
    void setPolymorphicRelation( const QgsPolymorphicRelation &polyRel );

  private slots:
    void addFieldsRow();
    void removeFieldsRow();
    void updateTypeConfigWidget();
    void updateFieldsMappingButtons();
    void updateFieldsMappingHeaders();
    void updateDialogButtons();
    void updateChildRelationsComboBox();
    void updateReferencingFieldsComboBoxes();
    void updateReferencedLayerFieldComboBox();
    void referencedLayersChanged();

  private:
    bool isDefinitionValid();
    void updateFieldsMapping();

    bool mIsEditDialog = false;

};

#endif // QGSRELATIONADDPOLYMORPHICDIALOG_H
