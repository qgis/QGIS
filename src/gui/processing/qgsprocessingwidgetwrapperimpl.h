/***************************************************************************
                         qgsprocessingwidgetwrapperimpl.h
                         ---------------------
    begin                : August 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#ifndef QGSPROCESSINGWIDGETWRAPPERIMPL_H
#define QGSPROCESSINGWIDGETWRAPPERIMPL_H

#define SIP_NO_FILE
#include "qgsprocessingwidgetwrapper.h"
#include "qgsprocessingparameterdefinitionwidget.h"
#include "qgsmaptool.h"
#include "qgsprocessingcontext.h"
#include "processing/models/qgsprocessingmodelchildparametersource.h"

#include <QAbstractButton>

class QCheckBox;
class QComboBox;
class QLineEdit;
class QPlainTextEdit;
class QToolButton;
class QButtonGroup;
class QgsProjectionSelectionWidget;
class QgsSpinBox;
class QgsDoubleSpinBox;
class QgsAuthConfigSelect;
class QgsProcessingMatrixParameterPanel;
class QgsFileWidget;
class QgsFieldExpressionWidget;
class QgsExpressionBuilderWidget;
class QgsExpressionLineEdit;
class QgsProcessingParameterEnum;
class QgsLayoutComboBox;
class QgsLayoutItemComboBox;
class QgsPrintLayout;
class QgsScaleWidget;
class QgsSnapIndicator;
class QgsFilterLineEdit;
class QgsColorButton;
class QgsCoordinateOperationWidget;
class QgsFieldComboBox;
class QgsDateTimeEdit;
class QgsDateEdit;
class QgsTimeEdit;
class QgsProviderConnectionComboBox;
class QgsDatabaseSchemaComboBox;
class QgsDatabaseTableComboBox;
class QgsExtentWidget;
class QgsProcessingEnumModelerWidget;
class QgsProcessingMatrixModelerWidget;
class QgsProcessingMapLayerComboBox;
class QgsRasterBandComboBox;
class QgsProcessingLayerOutputDestinationWidget;
class QgsCheckableComboBox;
class QgsMapLayerComboBox;

///@cond PRIVATE

class GUI_EXPORT QgsProcessingBooleanParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingBooleanParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QCheckBox *mDefaultCheckBox = nullptr;

};

class GUI_EXPORT QgsProcessingBooleanWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingBooleanWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                       QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    QLabel *createLabel() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

  private:

    QCheckBox *mCheckBox = nullptr;
    QComboBox *mComboBox = nullptr;

    friend class TestProcessingGui;
};



class GUI_EXPORT QgsProcessingCrsParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingCrsParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QgsProjectionSelectionWidget *mCrsSelector = nullptr;
};

class GUI_EXPORT QgsProcessingCrsWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingCrsWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                   QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;
    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;

  private:

    QgsProjectionSelectionWidget *mProjectionSelectionWidget = nullptr;
    QCheckBox *mUseProjectCrsCheckBox = nullptr;

    friend class TestProcessingGui;
};



class GUI_EXPORT QgsProcessingStringParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingStringParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QLineEdit *mDefaultLineEdit = nullptr;
    QCheckBox *mMultiLineCheckBox = nullptr;

};

class GUI_EXPORT QgsProcessingStringWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingStringWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                      QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
  private:

    QLineEdit *mLineEdit = nullptr;
    QComboBox *mComboBox = nullptr;
    QPlainTextEdit *mPlainTextEdit = nullptr;

    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingAuthConfigWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingAuthConfigWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                          QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
  private:

    QgsAuthConfigSelect *mAuthConfigSelect = nullptr;

    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingNumberParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingNumberParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QComboBox *mTypeComboBox = nullptr;
    QLineEdit *mMinLineEdit = nullptr;
    QLineEdit *mMaxLineEdit = nullptr;
    QLineEdit *mDefaultLineEdit = nullptr;

};


class GUI_EXPORT QgsProcessingNumericWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingNumericWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                       QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

  protected:

    QgsSpinBox *mSpinBox = nullptr;
    QgsDoubleSpinBox *mDoubleSpinBox = nullptr;

  private:

    static double calculateStep( double minimum, double maximum );

    bool mAllowingNull = false;

    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingDistanceParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingDistanceParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QComboBox *mParentLayerComboBox = nullptr;
    QLineEdit *mMinLineEdit = nullptr;
    QLineEdit *mMaxLineEdit = nullptr;
    QLineEdit *mDefaultLineEdit = nullptr;

};

class GUI_EXPORT QgsProcessingDistanceWidgetWrapper : public QgsProcessingNumericWidgetWrapper
{
    Q_OBJECT

  public:

    QgsProcessingDistanceWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    void postInitialize( const QList< QgsAbstractProcessingParameterWidgetWrapper * > &wrappers ) override;

  public slots:
    void setUnitParameterValue( const QVariant &value );
    void setUnits( QgsUnitTypes::DistanceUnit unit );

  protected:

    QVariant widgetValue() const override;

  private:

    QgsUnitTypes::DistanceUnit mBaseUnit = QgsUnitTypes::DistanceUnknownUnit;
    QLabel *mLabel = nullptr;
    QWidget *mWarningLabel = nullptr;
    QComboBox *mUnitsCombo = nullptr;

    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingDurationParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingDurationParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QLineEdit *mMinLineEdit = nullptr;
    QLineEdit *mMaxLineEdit = nullptr;
    QLineEdit *mDefaultLineEdit = nullptr;
    QComboBox *mUnitsCombo = nullptr;

};

class GUI_EXPORT QgsProcessingDurationWidgetWrapper : public QgsProcessingNumericWidgetWrapper
{
    Q_OBJECT

  public:

    QgsProcessingDurationWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    QLabel *createLabel() override SIP_FACTORY;

  protected:

    QVariant widgetValue() const override;
    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;

  private:

    QgsUnitTypes::TemporalUnit mBaseUnit = QgsUnitTypes::TemporalMilliseconds;
    QComboBox *mUnitsCombo = nullptr;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingScaleParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingScaleParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QLineEdit *mDefaultLineEdit = nullptr;

};

class GUI_EXPORT QgsProcessingScaleWidgetWrapper : public QgsProcessingNumericWidgetWrapper
{
    Q_OBJECT

  public:

    QgsProcessingScaleWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                     QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    void setWidgetContext( const QgsProcessingParameterWidgetContext &context ) override;
  protected:

    QVariant widgetValue() const override;
    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;

  private:

    QgsScaleWidget *mScaleWidget = nullptr;

    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingRangeParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingRangeParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QComboBox *mTypeComboBox = nullptr;
    QLineEdit *mMinLineEdit = nullptr;
    QLineEdit *mMaxLineEdit = nullptr;

};

class GUI_EXPORT QgsProcessingRangeWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingRangeWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                     QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;
    QStringList compatibleParameterTypes() const override;
    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;

  protected:

    QgsDoubleSpinBox *mMinSpinBox = nullptr;
    QgsDoubleSpinBox *mMaxSpinBox = nullptr;

  private:

    int mBlockChangedSignal = 0;
    bool mAllowingNull = false;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingMatrixParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingMatrixParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QgsProcessingMatrixModelerWidget *mMatrixWidget = nullptr;

};

class GUI_EXPORT QgsProcessingMatrixWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingMatrixWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                      QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;
    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;

  private:

    QgsProcessingMatrixParameterPanel *mMatrixWidget = nullptr;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingFileParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingFileParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QComboBox *mTypeComboBox = nullptr;
    QComboBox *mFilterComboBox = nullptr;
    QgsFileWidget *mDefaultFileWidget = nullptr;

};

class GUI_EXPORT QgsProcessingFileWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingFileWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                    QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;

  private:

    QgsFileWidget *mFileWidget = nullptr;

    friend class TestProcessingGui;
};



class GUI_EXPORT QgsProcessingExpressionParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingExpressionParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QComboBox *mParentLayerComboBox = nullptr;
    QgsExpressionLineEdit *mDefaultLineEdit = nullptr;

};

class GUI_EXPORT QgsProcessingExpressionWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingExpressionWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                          QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    void postInitialize( const QList< QgsAbstractProcessingParameterWidgetWrapper * > &wrappers ) override;
    void registerProcessingContextGenerator( QgsProcessingContextGenerator *generator ) override;

  public slots:
    void setParentLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper );
  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

    QString modelerExpressionFormatString() const override;
    const QgsVectorLayer *linkedVectorLayer() const override;
  private:

    QgsFieldExpressionWidget *mFieldExpWidget = nullptr;
    QgsExpressionBuilderWidget *mExpBuilderWidget = nullptr;
    QgsExpressionLineEdit *mExpLineEdit = nullptr;
    std::unique_ptr< QgsVectorLayer > mParentLayer;

    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingEnumCheckboxPanelWidget : public QWidget
{
    Q_OBJECT

  public:

    QgsProcessingEnumCheckboxPanelWidget( QWidget *parent = nullptr, const QgsProcessingParameterEnum *param = nullptr, int columns = 2 );
    QVariant value() const;
    void setValue( const QVariant &value );

  signals:

    void changed();

  private slots:

    void showPopupMenu();
    void selectAll();
    void deselectAll();

  private:

    const QgsProcessingParameterEnum *mParam = nullptr;
    QMap< QVariant, QAbstractButton * > mButtons;
    QButtonGroup *mButtonGroup = nullptr;
    int mColumns = 2;
    bool mBlockChangedSignal = false;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingEnumPanelWidget : public QWidget
{
    Q_OBJECT

  public:

    QgsProcessingEnumPanelWidget( QWidget *parent = nullptr, const QgsProcessingParameterEnum *param = nullptr );
    QVariant value() const { return mValue; }
    void setValue( const QVariant &value );

  signals:

    void changed();

  private slots:

    void showDialog();

  private:

    void updateSummaryText();

    const QgsProcessingParameterEnum *mParam = nullptr;
    QLineEdit *mLineEdit = nullptr;
    QToolButton *mToolButton = nullptr;

    QVariantList mValue;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingEnumParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingEnumParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QgsProcessingEnumModelerWidget *mEnumWidget = nullptr;

};

class GUI_EXPORT QgsProcessingEnumWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingEnumWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                    QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

    QString modelerExpressionFormatString() const override;
  private:

    QComboBox *mComboBox = nullptr;
    QgsProcessingEnumPanelWidget *mPanel = nullptr;
    QgsProcessingEnumCheckboxPanelWidget *mCheckboxPanel = nullptr;

    friend class TestProcessingGui;
};



class GUI_EXPORT QgsProcessingLayoutWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingLayoutWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                      QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;
    void setWidgetContext( const QgsProcessingParameterWidgetContext &context ) override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

    QString modelerExpressionFormatString() const override;
  private:

    QgsLayoutComboBox *mComboBox = nullptr;
    QComboBox *mPlainComboBox = nullptr;

    friend class TestProcessingGui;
};



class GUI_EXPORT QgsProcessingLayoutItemParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingLayoutItemParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QComboBox *mParentLayoutComboBox = nullptr;

};

class GUI_EXPORT QgsProcessingLayoutItemWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingLayoutItemWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                          QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    void postInitialize( const QList< QgsAbstractProcessingParameterWidgetWrapper * > &wrappers ) override;


  public slots:
    void setLayoutParameterValue( const QVariant &value );
    void setLayout( QgsPrintLayout *layout );

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;
  private:

    QgsLayoutItemComboBox *mComboBox = nullptr;
    QLineEdit *mLineEdit = nullptr;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingPointMapTool : public QgsMapTool
{
    Q_OBJECT
  public:
    QgsProcessingPointMapTool( QgsMapCanvas *canvas );
    ~QgsProcessingPointMapTool() override;
    void deactivate() override;
    void canvasMoveEvent( QgsMapMouseEvent *e ) override;
    void canvasPressEvent( QgsMapMouseEvent *e ) override;
    void keyPressEvent( QKeyEvent *e ) override;

  signals:

    void clicked( const QgsPointXY &point );
    void complete();

  private:

    std::unique_ptr<QgsSnapIndicator> mSnapIndicator;
    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingPointPanel : public QWidget
{
    Q_OBJECT

  public:

    QgsProcessingPointPanel( QWidget *parent );
    void setMapCanvas( QgsMapCanvas *canvas );
    void setAllowNull( bool allowNull );

    QVariant value() const;
    void clear();
    void setValue( const QgsPointXY &point, const QgsCoordinateReferenceSystem &crs );

  signals:

    void toggleDialogVisibility( bool visible );
    void changed();

  private slots:

    void selectOnCanvas();
    void updatePoint( const QgsPointXY &point );
    void pointPicked();

  private:

    QgsFilterLineEdit *mLineEdit = nullptr;
    QToolButton *mButton = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QgsCoordinateReferenceSystem mCrs;
    QPointer< QgsMapTool > mPrevTool;
    std::unique_ptr< QgsProcessingPointMapTool > mTool;
    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingPointParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingPointParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QLineEdit *mDefaultLineEdit = nullptr;

};

class GUI_EXPORT QgsProcessingPointWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingPointWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                     QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    void setWidgetContext( const QgsProcessingParameterWidgetContext &context ) override;
    void setDialog( QDialog *dialog ) override;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;
  private:

    QgsProcessingPointPanel *mPanel = nullptr;
    QLineEdit *mLineEdit = nullptr;
    QDialog *mDialog = nullptr;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingGeometryParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingGeometryParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QLineEdit *mDefaultLineEdit = nullptr;

};

class GUI_EXPORT QgsProcessingGeometryWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingGeometryWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;
  private:

    QLineEdit *mLineEdit = nullptr;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingExtentParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingExtentParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QgsExtentWidget *mDefaultWidget = nullptr;

};

class GUI_EXPORT QgsProcessingExtentWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingExtentWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                      QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    void setWidgetContext( const QgsProcessingParameterWidgetContext &context ) override;
    void setDialog( QDialog *dialog ) override;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;
  private:

    QgsExtentWidget *mExtentWidget = nullptr;
    QDialog *mDialog = nullptr;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingColorParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingColorParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QgsColorButton *mDefaultColorButton = nullptr;
    QCheckBox *mAllowOpacity = nullptr;

};

class GUI_EXPORT QgsProcessingColorWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingColorWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                     QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;
  private:

    QgsColorButton *mColorButton = nullptr;
    friend class TestProcessingGui;
};



class GUI_EXPORT QgsProcessingCoordinateOperationParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingCoordinateOperationParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QLineEdit *mDefaultLineEdit = nullptr;

    QComboBox *mSourceParamComboBox = nullptr;
    QComboBox *mDestParamComboBox = nullptr;

    QgsProjectionSelectionWidget *mStaticSourceWidget = nullptr;
    QgsProjectionSelectionWidget *mStaticDestWidget = nullptr;


};

class GUI_EXPORT QgsProcessingCoordinateOperationWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingCoordinateOperationWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    void postInitialize( const QList< QgsAbstractProcessingParameterWidgetWrapper * > &wrappers ) override;
    void setWidgetContext( const QgsProcessingParameterWidgetContext &context ) override;
  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;
  private:

    void setSourceCrsParameterValue( const QVariant &value );
    void setDestinationCrsParameterValue( const QVariant &value );

    QgsCoordinateOperationWidget *mOperationWidget = nullptr;
    QgsMapCanvas *mCanvas = nullptr;
    QLineEdit *mLineEdit = nullptr;
    QgsCoordinateReferenceSystem mSourceCrs;
    QgsCoordinateReferenceSystem mDestCrs;
    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingFieldPanelWidget : public QWidget
{
    Q_OBJECT

  public:

    QgsProcessingFieldPanelWidget( QWidget *parent = nullptr, const QgsProcessingParameterField *param = nullptr );

    void setFields( const QgsFields &fields );

    QgsFields fields() const { return mFields; }

    QVariant value() const { return mValue; }
    void setValue( const QVariant &value );

  signals:

    void changed();

  private slots:

    void showDialog();

  private:

    void updateSummaryText();

    QgsFields mFields;

    const QgsProcessingParameterField *mParam = nullptr;
    QLineEdit *mLineEdit = nullptr;
    QToolButton *mToolButton = nullptr;

    QVariantList mValue;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingFieldParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingFieldParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QComboBox *mParentLayerComboBox = nullptr;
    QComboBox *mDataTypeComboBox = nullptr;
    QLineEdit *mDefaultLineEdit = nullptr;
    QCheckBox *mAllowMultipleCheckBox = nullptr;
    QCheckBox *mDefaultToAllCheckBox = nullptr;
};

class GUI_EXPORT QgsProcessingFieldWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingFieldWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                     QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    void postInitialize( const QList< QgsAbstractProcessingParameterWidgetWrapper * > &wrappers ) override;

  public slots:
    void setParentLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper );

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;
    const QgsVectorLayer *linkedVectorLayer() const override;

  private:

    QgsFieldComboBox *mComboBox = nullptr;
    QgsProcessingFieldPanelWidget *mPanel = nullptr;
    QLineEdit *mLineEdit = nullptr;

    std::unique_ptr< QgsVectorLayer > mParentLayer;

    QgsFields filterFields( const QgsFields &fields ) const;

    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingMapThemeParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingMapThemeParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QComboBox *mDefaultComboBox = nullptr;

};

class GUI_EXPORT QgsProcessingMapThemeWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingMapThemeWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;

  private:

    QComboBox *mComboBox = nullptr;

    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingDateTimeParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingDateTimeParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QComboBox *mTypeComboBox = nullptr;

};

class GUI_EXPORT QgsProcessingDateTimeWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingDateTimeWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;
    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;

  private:

    QgsDateTimeEdit *mDateTimeEdit = nullptr;
    QgsDateEdit *mDateEdit = nullptr;
    QgsTimeEdit *mTimeEdit = nullptr;

    friend class TestProcessingGui;
};



//
// QgsProcessingProviderConnectionWidgetWrapper
//

class GUI_EXPORT QgsProcessingProviderConnectionParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingProviderConnectionParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QComboBox *mProviderComboBox = nullptr;
    QLineEdit *mDefaultEdit = nullptr;

};

class GUI_EXPORT QgsProcessingProviderConnectionWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingProviderConnectionWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;


    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;

  private:

    QgsProviderConnectionComboBox *mProviderComboBox = nullptr;
    int mBlockSignals = 0;

    friend class TestProcessingGui;
};



class GUI_EXPORT QgsProcessingDatabaseSchemaParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingDatabaseSchemaParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QComboBox *mConnectionParamComboBox = nullptr;
    QLineEdit *mDefaultEdit = nullptr;

};

class GUI_EXPORT QgsProcessingDatabaseSchemaWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingDatabaseSchemaWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    void postInitialize( const QList< QgsAbstractProcessingParameterWidgetWrapper * > &wrappers ) override;


    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

  public slots:
    void setParentConnectionWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper );

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;

  private:

    QgsDatabaseSchemaComboBox *mSchemaComboBox = nullptr;
    int mBlockSignals = 0;

    friend class TestProcessingGui;
};




class GUI_EXPORT QgsProcessingDatabaseTableParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingDatabaseTableParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QComboBox *mConnectionParamComboBox = nullptr;
    QComboBox *mSchemaParamComboBox = nullptr;
    QLineEdit *mDefaultEdit = nullptr;

};

class GUI_EXPORT QgsProcessingDatabaseTableWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingDatabaseTableWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    void postInitialize( const QList< QgsAbstractProcessingParameterWidgetWrapper * > &wrappers ) override;


    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

  public slots:
    void setParentConnectionWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper );
    void setParentSchemaWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper );

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;

  private:

    QgsDatabaseTableComboBox *mTableComboBox = nullptr;
    int mBlockSignals = 0;
    QString mConnection;
    QString mProvider;
    QString mSchema;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingMapLayerParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingMapLayerParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QgsCheckableComboBox *mLayerTypeComboBox = nullptr;
};

class GUI_EXPORT QgsProcessingMapLayerWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingMapLayerWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    void setWidgetContext( const QgsProcessingParameterWidgetContext &context ) override;
    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;
    QgsProcessingModelChildParameterSource::Source defaultModelSource( const QgsProcessingParameterDefinition *parameter ) const override;

  private:

    QPointer< QgsProcessingMapLayerComboBox > mComboBox;
    int mBlockSignals = 0;

    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingRasterLayerWidgetWrapper : public QgsProcessingMapLayerWidgetWrapper
{
    Q_OBJECT

  public:

    QgsProcessingRasterLayerWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                           QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

  protected:
    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

    QString modelerExpressionFormatString() const override;

};


class GUI_EXPORT QgsProcessingVectorLayerParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingVectorLayerParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QgsCheckableComboBox *mGeometryTypeComboBox = nullptr;
};

class GUI_EXPORT QgsProcessingVectorLayerWidgetWrapper : public QgsProcessingMapLayerWidgetWrapper
{
    Q_OBJECT

  public:

    QgsProcessingVectorLayerWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                           QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

  protected:
    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

    QString modelerExpressionFormatString() const override;
    QList< int > compatibleDataTypes( const QgsProcessingParameterDefinition *parameter ) const override;
};


class GUI_EXPORT QgsProcessingFeatureSourceParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingFeatureSourceParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QgsCheckableComboBox *mGeometryTypeComboBox = nullptr;
};

class GUI_EXPORT QgsProcessingFeatureSourceWidgetWrapper : public QgsProcessingMapLayerWidgetWrapper
{
    Q_OBJECT

  public:

    QgsProcessingFeatureSourceWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

  protected:
    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

    QString modelerExpressionFormatString() const override;
    QList< int > compatibleDataTypes( const QgsProcessingParameterDefinition *parameter ) const override;
};


class GUI_EXPORT QgsProcessingMeshLayerWidgetWrapper : public QgsProcessingMapLayerWidgetWrapper
{
    Q_OBJECT

  public:

    QgsProcessingMeshLayerWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                         QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

  protected:
    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

    QString modelerExpressionFormatString() const override;

};


class GUI_EXPORT QgsProcessingRasterBandPanelWidget : public QWidget
{
    Q_OBJECT

  public:

    QgsProcessingRasterBandPanelWidget( QWidget *parent = nullptr, const QgsProcessingParameterBand *param = nullptr );

    void setBands( const QList< int > &bands );
    void setBandNames( const QHash<int, QString > &names );
    QList< int > bands() const { return mBands; }

    QVariant value() const { return mValue; }
    void setValue( const QVariant &value );

  signals:

    void changed();

  private slots:

    void showDialog();

  private:

    void updateSummaryText();

    QList< int > mBands;
    QHash<int, QString > mBandNames;

    const QgsProcessingParameterBand *mParam = nullptr;
    QLineEdit *mLineEdit = nullptr;
    QToolButton *mToolButton = nullptr;

    QVariantList mValue;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingBandParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingBandParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QComboBox *mParentLayerComboBox = nullptr;
    QLineEdit *mDefaultLineEdit = nullptr;
    QCheckBox *mAllowMultipleCheckBox = nullptr;
};

class GUI_EXPORT QgsProcessingBandWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingBandWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                    QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    void postInitialize( const QList< QgsAbstractProcessingParameterWidgetWrapper * > &wrappers ) override;

  public slots:
    void setParentLayerWrapperValue( const QgsAbstractProcessingParameterWidgetWrapper *parentWrapper );

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;

  private:

    QgsRasterBandComboBox *mComboBox = nullptr;
    QgsProcessingRasterBandPanelWidget *mPanel = nullptr;
    QLineEdit *mLineEdit = nullptr;

    std::unique_ptr< QgsRasterLayer > mParentLayer;

    friend class TestProcessingGui;
};



class GUI_EXPORT QgsProcessingMultipleLayerPanelWidget : public QWidget
{
    Q_OBJECT

  public:

    QgsProcessingMultipleLayerPanelWidget( QWidget *parent = nullptr, const QgsProcessingParameterMultipleLayers *param = nullptr );

    QVariant value() const { return mValue; }
    void setValue( const QVariant &value );

    void setProject( QgsProject *project );
    void setModel( QgsProcessingModelAlgorithm *model, const QString &modelChildAlgorithmID );

  signals:

    void changed();

  private slots:

    void showDialog();

  private:

    void updateSummaryText();

    const QgsProcessingParameterMultipleLayers *mParam = nullptr;
    QLineEdit *mLineEdit = nullptr;
    QToolButton *mToolButton = nullptr;

    QVariantList mValue;
    QList< QgsProcessingModelChildParameterSource > mModelSources;
    QgsProcessingModelAlgorithm *mModel = nullptr;

    QgsProject *mProject = nullptr;

    friend class TestProcessingGui;
};

class GUI_EXPORT QgsProcessingMultipleLayerParameterDefinitionWidget : public QgsProcessingAbstractParameterDefinitionWidget
{
    Q_OBJECT
  public:

    QgsProcessingMultipleLayerParameterDefinitionWidget( QgsProcessingContext &context,
        const QgsProcessingParameterWidgetContext &widgetContext,
        const QgsProcessingParameterDefinition *definition = nullptr,
        const QgsProcessingAlgorithm *algorithm = nullptr, QWidget *parent SIP_TRANSFERTHIS = nullptr );
    QgsProcessingParameterDefinition *createParameter( const QString &name, const QString &description, QgsProcessingParameterDefinition::Flags flags ) const override;

  private:

    QComboBox *mLayerTypeComboBox = nullptr;
};

class GUI_EXPORT QgsProcessingMultipleLayerWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingMultipleLayerWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    void setWidgetContext( const QgsProcessingParameterWidgetContext &context ) override;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;
    QString modelerExpressionFormatString() const override;

  private:

    QgsProcessingMultipleLayerPanelWidget *mPanel = nullptr;

    friend class TestProcessingGui;
};



class GUI_EXPORT QgsProcessingOutputWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingOutputWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                      QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;
    QVariantMap customProperties() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

  private:

    QgsProcessingLayerOutputDestinationWidget *mOutputWidget = nullptr;
    int mBlockSignals = 0;

    friend class TestProcessingGui;
};


class GUI_EXPORT QgsProcessingFeatureSinkWidgetWrapper : public QgsProcessingOutputWidgetWrapper
{
    Q_OBJECT

  public:

    QgsProcessingFeatureSinkWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                           QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;

  protected:
    QString modelerExpressionFormatString() const override;

  private:
    QgsProcessingContext mContext;

};

class GUI_EXPORT QgsProcessingVectorDestinationWidgetWrapper : public QgsProcessingOutputWidgetWrapper
{
    Q_OBJECT

  public:

    QgsProcessingVectorDestinationWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;

  protected:
    QString modelerExpressionFormatString() const override;

};

class GUI_EXPORT QgsProcessingRasterDestinationWidgetWrapper : public QgsProcessingOutputWidgetWrapper
{
    Q_OBJECT

  public:

    QgsProcessingRasterDestinationWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;

  protected:
    QString modelerExpressionFormatString() const override;

};

class GUI_EXPORT QgsProcessingPointCloudDestinationWidgetWrapper : public QgsProcessingOutputWidgetWrapper
{
    Q_OBJECT

  public:

    QgsProcessingPointCloudDestinationWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;

  protected:
    QString modelerExpressionFormatString() const override;

};

class GUI_EXPORT QgsProcessingFileDestinationWidgetWrapper : public QgsProcessingOutputWidgetWrapper
{
    Q_OBJECT

  public:

    QgsProcessingFileDestinationWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;

  protected:
    QString modelerExpressionFormatString() const override;

};

class GUI_EXPORT QgsProcessingFolderDestinationWidgetWrapper : public QgsProcessingOutputWidgetWrapper
{
    Q_OBJECT

  public:

    QgsProcessingFolderDestinationWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;

  protected:
    QString modelerExpressionFormatString() const override;

};

class GUI_EXPORT QgsProcessingPointCloudLayerWidgetWrapper : public QgsProcessingMapLayerWidgetWrapper
{
    Q_OBJECT

  public:

    QgsProcessingPointCloudLayerWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;

  protected:
    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

    QString modelerExpressionFormatString() const override;

};


class GUI_EXPORT QgsProcessingAnnotationLayerWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingAnnotationLayerWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
        QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;
    QgsProcessingAbstractParameterDefinitionWidget *createParameterDefinitionWidget(
      QgsProcessingContext &context,
      const QgsProcessingParameterWidgetContext &widgetContext,
      const QgsProcessingParameterDefinition *definition = nullptr,
      const QgsProcessingAlgorithm *algorithm = nullptr ) override;
    void setWidgetContext( const QgsProcessingParameterWidgetContext &context ) override;

    QWidget *createWidget() override SIP_FACTORY;
  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

    QString modelerExpressionFormatString() const override;

  private:

    QPointer< QgsMapLayerComboBox > mComboBox;
    int mBlockSignals = 0;

    friend class TestProcessingGui;
};


///@endcond PRIVATE

#endif // QGSPROCESSINGWIDGETWRAPPERIMPL_H
