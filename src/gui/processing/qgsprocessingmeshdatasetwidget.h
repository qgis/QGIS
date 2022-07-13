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

#include <QAction>

#include "qgsprocessingwidgetwrapperimpl.h"
#include "qgsprocessingparametermeshdataset.h"
#include "qgsmeshlayer.h"

#include "ui_qgsprocessingmeshdatasettimewidget.h"

#define SIP_NO_FILE

/// @cond PRIVATE

class GUI_EXPORT QgsProcessingMeshDatasetGroupsWidget : public QWidget
{
    Q_OBJECT
  public:
    QgsProcessingMeshDatasetGroupsWidget( QWidget *parent = nullptr, const QgsProcessingParameterMeshDatasetGroups *param = nullptr );

    /**
     * Set the mesh layer for populate the dataset group names,
     * if \a layerFromProject is false, the layer will not stay referenced in the instance of this object but
     * all that is needed is stored in the instance
     */
    void setMeshLayer( QgsMeshLayer *layer, bool layerFromProject = false );
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
    QPointer<QAction> mActionCurrentActiveDatasetGroups = nullptr;
    QPointer<QAction> mActionAvailableDatasetGroups = nullptr;
    QgsMeshLayer *mMeshLayer = nullptr;
    QMap<int, QString> mDatasetGroupsNames; //used to store the dataet groups name if layer is not referenced

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
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;
    void postInitialize( const QList<QgsAbstractProcessingParameterWidgetWrapper *> &wrappers ) override;

    //! Sets the layer parameter widget wrapper
    void setMeshLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *wrapper );

  protected:
    QStringList compatibleParameterTypes() const override;
    QStringList compatibleOutputTypes() const override;
    QWidget *createWidget() override;
    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

  private:
    QgsProcessingMeshDatasetGroupsWidget *mWidget = nullptr;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingMeshDatasetGroupsParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT

  public:
    QgsProcessingMeshDatasetGroupsParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const;

  private:
    QComboBox *mParentLayerComboBox = nullptr;
};

class GUI_EXPORT QgsProcessingMeshDatasetTimeWidget : public QWidget, private Ui::QgsProcessingMeshDatasetTimeWidgetBase
{
    Q_OBJECT

  public:
    QgsProcessingMeshDatasetTimeWidget( QWidget *parent = nullptr,
                                        const QgsProcessingParameterMeshDatasetTime *param = nullptr,
                                        const QgsProcessingParameterWidgetContext &context =  QgsProcessingParameterWidgetContext() );

    /**
     * Set the mesh layer for populate the time steps,
     * if \a layerFromProject is false, the layer will not stay referenced in the instance of this object but
     * all that is needed is stored in the instance
     */
    void setMeshLayer( QgsMeshLayer *layer, bool layerFromProject = false );
    void setDatasetGroupIndexes( const QList<int> datasetGroupIndexes );

    void setValue( const QVariant &value );
    QVariant value() const;

  public slots:
    void updateValue();

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
    QDateTime mReferenceTime;

    void populateTimeSteps();
    bool hasTemporalDataset() const;
    //! Populates diretly the time steps combo box with the referenced layer, used if layer comes from project
    void populateTimeStepsFromLayer();
    //! Stores the dataset time steps to use them later depending of chosen dataset groups (setDatasetGroupIndexes()), used if layer does not come from project
    void storeTimeStepsFromLayer( QgsMeshLayer *layer );
    QMap<int, QList<qint64>> mDatasetTimeSteps; //used if layer does not come from project

    void updateWidget();
    void buildValue();

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
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    //! Sets the layer parameter widget wrapper
    void setMeshLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *wrapper );

    //! Sets the dataset group indexes widget wrapper
    void setDatasetGroupIndexesWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *wrapper );

  protected:
    QStringList compatibleParameterTypes() const override;
    QStringList compatibleOutputTypes() const override;
    QWidget *createWidget() override;
    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

  private:
    QgsProcessingMeshDatasetTimeWidget *mWidget = nullptr;
    std::unique_ptr<QgsMeshLayer> mTemporarytMeshLayer;
    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingMeshDatasetTimeParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT

  public:
    QgsProcessingMeshDatasetTimeParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const;

  private:
    QComboBox *mParentDatasetComboBox = nullptr;
    QString mMeshLayerParameterName;
};


///@endcond

#endif // QGSPROCESSINGMESHDATASETWIDGET_H
