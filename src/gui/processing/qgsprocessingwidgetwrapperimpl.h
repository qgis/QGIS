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
class QgsExpressionLineEdit;
class QgsProcessingParameterEnum;
class QgsLayoutComboBox;
class QgsLayoutItemComboBox;
class QgsPrintLayout;
class QgsScaleWidget;
class QgsSnapIndicator;
class QgsFilterLineEdit;
class QgsColorButton;

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

    QList< int > compatibleDataTypes() const override;

  private:

    QCheckBox *mCheckBox = nullptr;
    QComboBox *mComboBox = nullptr;

    friend class TestProcessingGui;
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

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;
    QStringList compatibleOutputTypes() const override;
    QList< int > compatibleDataTypes() const override;
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

    QList< int > compatibleDataTypes() const override;

  private:

    QLineEdit *mLineEdit = nullptr;
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

    QList< int > compatibleDataTypes() const override;

  private:

    QgsAuthConfigSelect *mAuthConfigSelect = nullptr;

    friend class TestProcessingGui;
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

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

    QList< int > compatibleDataTypes() const override;

  protected:

    QgsSpinBox *mSpinBox = nullptr;
    QgsDoubleSpinBox *mDoubleSpinBox = nullptr;

  private:

    static double calculateStep( double minimum, double maximum );

    bool mAllowingNull = false;

    friend class TestProcessingGui;
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


class GUI_EXPORT QgsProcessingScaleWidgetWrapper : public QgsProcessingNumericWidgetWrapper
{
    Q_OBJECT

  public:

    QgsProcessingScaleWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                     QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;

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

class GUI_EXPORT QgsProcessingRangeWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingRangeWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
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
    QList< int > compatibleDataTypes() const override;
    QString modelerExpressionFormatString() const override;

  protected:

    QgsDoubleSpinBox *mMinSpinBox = nullptr;
    QgsDoubleSpinBox *mMaxSpinBox = nullptr;

  private:

    int mBlockChangedSignal = 0;

    friend class TestProcessingGui;
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

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;
    QStringList compatibleOutputTypes() const override;
    QList< int > compatibleDataTypes() const override;
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

    QList< int > compatibleDataTypes() const override;
    QString modelerExpressionFormatString() const override;

  private:

    QgsFileWidget *mFileWidget = nullptr;

    friend class TestProcessingGui;
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

    QList< int > compatibleDataTypes() const override;
    QString modelerExpressionFormatString() const override;
    const QgsVectorLayer *linkedVectorLayer() const override;
  private:

    QgsFieldExpressionWidget *mFieldExpWidget = nullptr;
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


class GUI_EXPORT QgsProcessingEnumWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingEnumWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
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

    QList< int > compatibleDataTypes() const override;
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

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

    QList< int > compatibleDataTypes() const override;
    QString modelerExpressionFormatString() const override;
  private:

    QgsLayoutComboBox *mComboBox = nullptr;
    QLineEdit *mLineEdit = nullptr;

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

    QList< int > compatibleDataTypes() const override;
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


class GUI_EXPORT QgsProcessingPointWidgetWrapper : public QgsAbstractProcessingParameterWidgetWrapper, public QgsProcessingParameterWidgetFactoryInterface
{
    Q_OBJECT

  public:

    QgsProcessingPointWidgetWrapper( const QgsProcessingParameterDefinition *parameter = nullptr,
                                     QgsProcessingGui::WidgetType type = QgsProcessingGui::Standard, QWidget *parent = nullptr );

    // QgsProcessingParameterWidgetFactoryInterface
    QString parameterType() const override;
    QgsAbstractProcessingParameterWidgetWrapper *createWidgetWrapper( const QgsProcessingParameterDefinition *parameter, QgsProcessingGui::WidgetType type ) override;

    // QgsProcessingParameterWidgetWrapper interface
    QWidget *createWidget() override SIP_FACTORY;
    void setWidgetContext( const QgsProcessingParameterWidgetContext &context ) override;
    void setDialog( QDialog *dialog ) override;

  protected:

    void setWidgetValue( const QVariant &value, QgsProcessingContext &context ) override;
    QVariant widgetValue() const override;

    QStringList compatibleParameterTypes() const override;

    QStringList compatibleOutputTypes() const override;

    QList< int > compatibleDataTypes() const override;
    QString modelerExpressionFormatString() const override;
  private:

    QgsProcessingPointPanel *mPanel = nullptr;
    QLineEdit *mLineEdit = nullptr;
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

    QList< int > compatibleDataTypes() const override;
    QString modelerExpressionFormatString() const override;
  private:

    QgsColorButton *mColorButton = nullptr;
    friend class TestProcessingGui;
};


///@endcond PRIVATE

#endif // QGSPROCESSINGWIDGETWRAPPERIMPL_H
