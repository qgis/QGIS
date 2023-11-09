/***************************************************************************
    qgsvectorlayerdigitizingproperties.h
    ------------------------------------
  copyright            : (C) 2018 by Matthias Kuhn
  email                : matthias@opengis.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSVECTORLAYERDIGITIZINGPROPERTIES_H
#define QGSVECTORLAYERDIGITIZINGPROPERTIES_H

#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerconfigwidgetfactory.h"
#include "ui_qgsvectorlayerdigitizingproperties.h"

class QgsCollapsibleGroupBox;
class QgsMapLayerComboBox;
class QgsDoubleSpinBox;


class QgsVectorLayerDigitizingPropertiesPage : public QgsMapLayerConfigWidget, private Ui::QgsVectorLayerDigitizingPropertiesPage
{
    Q_OBJECT

  public:
    explicit QgsVectorLayerDigitizingPropertiesPage( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent = nullptr );

  public slots:
    virtual void apply();

  private:
    bool mRemoveDuplicateNodesManuallyActivated = false;
    QHash<QCheckBox *, QString> mGeometryCheckFactoriesGroupBoxes;
    QgsCollapsibleGroupBox *mGapCheckAllowExceptionsActivatedCheckBox = nullptr;
    QgsMapLayerComboBox *mGapCheckAllowExceptionsLayerComboBox = nullptr;
    QgsDoubleSpinBox *mGapCheckAllowExceptionsBufferSpinBox = nullptr;
};


class QgsVectorLayerDigitizingPropertiesFactory : public QObject, public QgsMapLayerConfigWidgetFactory
{
    Q_OBJECT
  public:
    explicit QgsVectorLayerDigitizingPropertiesFactory( QObject *parent = nullptr );
    QgsMapLayerConfigWidget *createWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, bool dockWidget, QWidget *parent ) const override;

    bool supportLayerPropertiesDialog() const override { return true; }

    bool supportsLayer( QgsMapLayer *layer ) const override;
};

#endif // QGSVECTORLAYERDIGITIZINGPROPERTIES_H
