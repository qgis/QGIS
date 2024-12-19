/***************************************************************************
  qgsprocessingdxflayerswidgetwrapper.h
  ---------------------
  Date                 : September 2020
  Copyright            : (C) 2020 by Alexander bruy
  Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGDXFLAYERSWIDGETWRAPPER_H
#define QGSPROCESSINGDXFLAYERSWIDGETWRAPPER_H

#define SIP_NO_FILE

#include "qgsprocessingcontext.h"
#include "qgsprocessingwidgetwrapper.h"
#include "qgsprocessingmultipleselectiondialog.h"
#include "qgsdxfexport.h"

#include "ui_qgsprocessingdxflayerdetailswidgetbase.h"

class QLineEdit;
class QToolButton;

/// @cond private

class GUI_EXPORT QgsProcessingDxfLayerDetailsWidget : public QgsPanelWidget, private Ui::QgsProcessingDxfLayerDetailsWidget
{
    Q_OBJECT
  public:
    QgsProcessingDxfLayerDetailsWidget( const QVariant &value, QgsProject *project );

    QVariant value() const;

    QDialogButtonBox *buttonBox() { return mButtonBox; }

  private:
    QgsVectorLayer *mLayer = nullptr;
    QgsProcessingContext mContext;
};


class GUI_EXPORT QgsProcessingDxfLayersPanelWidget : public QgsProcessingMultipleSelectionPanelWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingDxfLayersPanelWidget.
     */
    QgsProcessingDxfLayersPanelWidget(
      const QVariant &value,
      QgsProject *project,
      QWidget *parent SIP_TRANSFERTHIS = nullptr );

  private slots:

    void configureLayer();

  private:
    void setItemValue( QStandardItem *item, const QVariant &value );
    QString titleForLayer( const QgsDxfExport::DxfLayer &layer );

    QgsProject *mProject = nullptr;
    QgsProcessingContext mContext;
};


class GUI_EXPORT QgsProcessingDxfLayersWidget : public QWidget
{
    Q_OBJECT

  public:

    QgsProcessingDxfLayersWidget( QWidget *parent = nullptr );

    QVariant value() const { return mValue; }
    void setValue( const QVariant &value );

    void setProject( QgsProject *project );

  signals:

    void changed();

  private slots:

    void showDialog();

  private:

    void updateSummaryText();

    QLineEdit *mLineEdit = nullptr;
    QToolButton *mToolButton = nullptr;

    QVariantList mValue;

    QgsProject *mProject = nullptr;

    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingDxfLayersWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingDxfLayersWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                         QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    void setWidgetContext( const QgsProcessingParameterWidgetContext &context ) override;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;
    QStringList compatibleOutputTypes() const override;

  private:

    QgsProcessingDxfLayersWidget *mPanel = nullptr;

    friend class TestProcessingGui;
};

/// @endcond

#endif // QGSPROCESSINGDXFLAYERSWIDGETWRAPPER_H
