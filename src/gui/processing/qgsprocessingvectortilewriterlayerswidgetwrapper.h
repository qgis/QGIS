/***************************************************************************
  qgsprocessingvectortilewriterlayerswidgetwrapper.h
  ---------------------
  Date                 : April 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGVECTORTILEWRITERLAYERSWIDGETWRAPPER_H
#define QGSPROCESSINGVECTORTILEWRITERLAYERSWIDGETWRAPPER_H

#define SIP_NO_FILE

#include "qgsprocessingcontext.h"
#include "qgsprocessingwidgetwrapper.h"
#include "qgsprocessingmultipleselectiondialog.h"
#include "qgsvectortilewriter.h"

#include "ui_qgsprocessingvectortilewriterlayerdetailswidgetbase.h"

class QLineEdit;
class QToolButton;

/// @cond private


class QgsProcessingVectorTileWriteLayerDetailsWidget : public QgsPanelWidget, private Ui::QgsProcessingVectorTileWriterLayerDetailsWidget
{
    Q_OBJECT
  public:
    QgsProcessingVectorTileWriteLayerDetailsWidget( const QVariant &value, QgsProject *project );

    QVariant value() const;

    QDialogButtonBox *buttonBox() { return mButtonBox; }

  private:
    QgsVectorLayer *mLayer = nullptr;
    QgsProcessingContext mContext;
};


class QgsProcessingVectorTileWriterLayersPanelWidget : public QgsProcessingMultipleSelectionPanelWidget
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingVectorTileWriterLayersPanelWidget.
     */
    QgsProcessingVectorTileWriterLayersPanelWidget(
      const QVariant &value,
      QgsProject *project,
      QWidget *parent SIP_TRANSFERTHIS = nullptr );

  private slots:

    void configureLayer();
    void copyLayer();

  private:
    void setItemValue( QStandardItem *item, const QVariant &value );
    QString titleForLayer( const QgsVectorTileWriter::Layer &layer );

    QgsProject *mProject = nullptr;
    QgsProcessingContext mContext;
};



class QgsProcessingVectorTileWriterLayersWidget : public QWidget
{
    Q_OBJECT

  public:

    QgsProcessingVectorTileWriterLayersWidget( QWidget *parent = nullptr );

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


class QgsProcessingVectorTileWriterLayersWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingVectorTileWriterLayersWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
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

    QgsProcessingVectorTileWriterLayersWidget *mPanel = nullptr;

    friend class TestProcessingGui;
};


/// @endcond

#endif // QGSPROCESSINGVECTORTILEWRITERLAYERSWIDGETWRAPPER_H
