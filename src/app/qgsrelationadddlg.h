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

#include <QDialog>
#include "qgis_app.h"
#include "qgsrelation.h"

class QDialogButtonBox;
class QComboBox;
class QLineEdit;
class QSpacerItem;
class QToolButton;
class QVBoxLayout;
class QHBoxLayout;

class QgsVectorLayer;
class QgsMapLayerComboBox;
class QgsFieldComboBox;

/**
 * QgsFieldPairWidget is horizontal widget with a field pair and buttons to enable/disable it
 */
class APP_EXPORT QgsFieldPairWidget : public QWidget
{
    Q_OBJECT
  public:
    explicit QgsFieldPairWidget( int index, QWidget *parent = nullptr );
    QString referencingField() const;
    QString referencedField() const;
    bool isPairEnabled() const;

  signals:
    void configChanged();
    void pairDisabled( int index );
    void pairEnabled();

  public slots:
    void setReferencingLayer( QgsMapLayer *layer );
    void setReferencedLayer( QgsMapLayer *layer );

  private:
    void updateWidgetVisibility();
    void changeEnable();

    int mIndex;
    bool mEnabled;
    QToolButton *mAddButton;
    QToolButton *mRemoveButton;
    QgsFieldComboBox *mReferencingFieldCombobox;
    QgsFieldComboBox *mReferencedFieldCombobox;
    QSpacerItem *mSpacerItem;
    QHBoxLayout *mLayout;

};


/**
 * QgsRelationAddDlg allows configuring a new relation.
 * Multiple field pairs can be set.
 */
class APP_EXPORT QgsRelationAddDlg : public QDialog
{
    Q_OBJECT

  public:
    explicit QgsRelationAddDlg( QWidget *parent = nullptr );

    QString referencingLayerId();
    QString referencedLayerId();
    QList< QPair< QString, QString > > references();
    QString relationId();
    QString relationName();
    QgsRelation::RelationStrength relationStrength();

  private slots:
    void checkDefinitionValid();
    void fieldPairRemoved( QgsFieldPairWidget *fieldPairWidget );
    void addFieldPairWidget();

  private:
    QList<QgsFieldPairWidget *> mFieldPairWidgets;

    QDialogButtonBox *mButtonBox = nullptr;
    QVBoxLayout *mFieldPairsLayout = nullptr;
    QLineEdit *mNameLineEdit = nullptr;
    QLineEdit *mIdLineEdit = nullptr;
    QComboBox *mStrengthCombobox = nullptr;
    QgsMapLayerComboBox *mReferencedLayerCombobox = nullptr;
    QgsMapLayerComboBox *mReferencingLayerCombobox = nullptr;

};

#endif // QGSRELATIONADDDLG_H
