/***************************************************************************
  qgsprocessingmeshdatasetwidget.h
  ---------------------
  Date                 : October 2020
  Copyright            : (C) 2020 by Vincent Cloarec
  Email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGMESHDATASETWIDGET_H
#define QGSPROCESSINGMESHDATASETWIDGET_H

#include "qgsprocessingwidgetwrapperimpl.h"
#include "qgsprocessingparametermeshdataset.h"

#include "ui_qgsprocessingmeshdatasettimewidget.h"

#define SIP_NO_FILE

/// @cond PRIVATE

class GUI_EXPORT QgsProcessingMeshDatasetGroupsWidget : public QWidget
{
    Q_OBJECT
  public:
    QgsProcessingMeshDatasetGroupsWidget( QWidget *parent = nullptr, const QgsProcessingParameterMeshDatasetGroups *param = nullptr );

    void setMeshLayer( QgsMeshLayer *layer );
    void setValue( const QVariant &value );
    QVariant value() const;

  signals:
    void changed();

  private slots:
    void showDialog();
    void selectCurrentActiveDatasetGroup();

  private:
    const QgsProcessingParameterMeshDatasetGroups *mParam = nullptr;
    QVariantList mValue;

    QPointer<QLineEdit> mLineEdit = nullptr;
    QPointer<QToolButton> mToolButton = nullptr;
    QgsMeshLayer *mMeshLayer = nullptr;

    QStringList datasetGroupsNames();
    void updateSummaryText();

    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingMeshDatasetGroupsWidgetWrapper  : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:
    QgsProcessingMeshDatasetGroupsWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    void postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers ) override;

    //! Sets the layer parameter widget wrapper
    void setMeshLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *wrapper );

  protected:
    QStringList compatibleParameterTypes() const override {return QStringList();}
    QStringList compatibleOutputTypes() const override {return QStringList();}
    QWidget *createWidget() override;
    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

  private:
    QgsProcessingMeshDatasetGroupsWidget *mWidget = nullptr;
    std::unique_ptr<QgsMeshLayer> mTemporarytMeshLayer;

    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingMeshDatasetTimeWidget : public QWidget, private Ui::QgsProcessingMeshDatasetTimeWidgetBase
{
    Q_OBJECT
  public:
    QgsProcessingMeshDatasetTimeWidget( QWidget *parent = nullptr,
                                        const QgsProcessingParameterMeshDatasetTime *param = nullptr,
                                        const QgsProcessingParameterWidgetContext &context =  QgsProcessingParameterWidgetContext() );

    void setMeshLayer( QgsMeshLayer *layer );
    void setDatasetGroupIndexes( const QList<int> datasetGroupIndexes );

    void setValue( const QVariant &value );
    QVariant value() const;

  public slots:
    void buildValue();

  signals:
    void changed();

  private:
    const QgsProcessingParameterMeshDatasetTime *mParam = nullptr;
    QVariantMap mValue;
    QgsMapCanvas *mCanvas;

    QLineEdit *mLineEdit = nullptr;
    QToolButton *mToolButton = nullptr;
    QgsMeshLayer *mMeshLayer = nullptr;
    QList<int> mDatasetGroupIndexes;

    bool hasTemporalDataset() const;
    void populateTimeStep();

    void updateWidget();

    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingMeshDatasetTimeWidgetWrapper  : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:
    QgsProcessingMeshDatasetTimeWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    void postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers ) override;

    //! Sets the layer parameter widget wrapper
    void setMeshLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *wrapper );

    //! Sets the dataset group indexes widget wrapper
    void setDatasetGroupIndexesWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *wrapper );

  protected:
    QStringList compatibleParameterTypes() const override {return QStringList();}
    QStringList compatibleOutputTypes() const override {return QStringList();}
    QWidget *createWidget() override;
    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

  private:
    QgsProcessingMeshDatasetTimeWidget *mWidget = nullptr;
    std::unique_ptr<QgsMeshLayer> mTemporarytMeshLayer;
    friend class TestProcessingGui;
};

///@endcond

#endif // QGSPROCESSINGMESHDATASETWIDGET_H
