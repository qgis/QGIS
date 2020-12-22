/***************************************************************************
    qgsrelationaddpolymorphicdlg.h
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
#ifndef QGSRELATIONADDPOLYMORPHICDLG_H
#define QGSRELATIONADDPOLYMORPHICDLG_H

#include <QDialog>
#include "qgis_app.h"
#include "ui_qgsrelationmanageraddpolymorphicdialogbase.h"
#include "qgsrelation.h"

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
 * QgsRelationAddDlg allows configuring a new relation.
 * Multiple field pairs can be set.
 */
class APP_EXPORT QgsRelationAddPolymorphicDlg : public QDialog, private Ui::QgsRelationManagerAddPolymorphicDialogBase
{
    Q_OBJECT

  public:
    explicit QgsRelationAddPolymorphicDlg( QWidget *parent = nullptr );

    QString referencingLayerId();
    QString referencedLayerField();
    QString referencedLayerExpression();
    QList< QPair< QString, QString > > references();
    QString relationId();
    QString relationName();
    QStringList referencedLayerIds();
    QgsRelation::RelationStrength relationStrength();

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

  private:
    bool isDefinitionValid();
    void updateFieldsMapping();
//    QList<QgsFieldPairWidget *> mFieldPairWidgets;

//    QDialogButtonBox *mButtonBox = nullptr;
//    QVBoxLayout *mFieldPairsLayout = nullptr;
//    QLineEdit *mNameLineEdit = nullptr;
//    QComboBox *mTypeComboBox = nullptr;
//    QLineEdit *mIdLineEdit = nullptr;
//    QLineEdit *mReferencedLayerExpressionLineEdit = nullptr;
//    QComboBox *mStrengthCombobox = nullptr;

};

#endif // QGSRELATIONADDPOLYMORPHICDLG_H
