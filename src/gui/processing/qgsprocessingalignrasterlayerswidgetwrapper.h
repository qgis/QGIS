/***************************************************************************
  qgsprocessingalingrasterlayerswidgetwrapper.h
  ---------------------
  Date                 : July 2023
  Copyright            : (C) 2023 by Alexander bruy
  Email                : alexander dot bruy at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGALIGNRASTERLAYERSWIDGETWRAPPER_H
#define QGSPROCESSINGALIGNRASTERLAYERSWIDGETWRAPPER_H

#define SIP_NO_FILE

#include "qgsprocessingcontext.h"
#include "qgsprocessingwidgetwrapper.h"
#include "qgsprocessingmultipleselectiondialog.h"
#include "qgsalignrasterdata.h"

#include "ui_qgsprocessingalignrasterlayerdetailswidgetbase.h"

class QLineEdit;
class QToolButton;

/// @cond private

class GUI_EXPORT QgsProcessingAlignRasterLayerDetailsWidget : public QgsPanelWidget, private Ui::QgsProcessingAlignRasterLayerDetailsWidget
{
    Q_OBJECT
  public:
    QgsProcessingAlignRasterLayerDetailsWidget( const QVariant &value, QgsProject *project );

    QVariant value() const;

    QDialogButtonBox *buttonBox() { return mButtonBox; }

  private:
    QgsProcessingContext mContext;
    QString mInputPath;
};


class GUI_EXPORT QgsProcessingAlignRasterLayersPanelWidget : public QgsProcessingMultipleSelectionPanelWidget
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsProcessingAlignRasterLayersPanelWidget.
     */
    QgsProcessingAlignRasterLayersPanelWidget(
      const QVariant &value,
      QgsProject *project,
      QWidget *parent SIP_TRANSFERTHIS = nullptr
    );

  private slots:

    void configureRaster();

  private:
    void setItemValue( QStandardItem *item, const QVariant &value );
    QString titleForItem( const QgsAlignRasterData::RasterItem &item );

    QgsProject *mProject = nullptr;
    QgsProcessingContext mContext;
};


class GUI_EXPORT QgsProcessingAlignRasterLayersWidget : public QWidget
{
    Q_OBJECT

  public:
    QgsProcessingAlignRasterLayersWidget( QWidget *parent = nullptr );

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


class GUI_EXPORT QgsProcessingAlignRasterLayersWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:
    QgsProcessingAlignRasterLayersWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr, QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

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
    QgsProcessingAlignRasterLayersWidget *mPanel = nullptr;

    friend class TestProcessingGui;
};

/// @endcond

#endif // QGSPROCESSINGALIGNRASTERLAYERSWIDGETWRAPPER_H
