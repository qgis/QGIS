/***************************************************************************
    qgspainteffectwidget.h
    ----------------------
    begin                : January 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPAINTEFFECTWIDGET_H
#define QGSPAINTEFFECTWIDGET_H

#include <QWidget>

class QgsPaintEffect;
class QgsShadowEffect;
class QgsDrawSourceEffect;
class QgsBlurEffect;
class QgsGlowEffect;
class QgsTransformEffect;
class QgsColorEffect;


/** \ingroup gui
 * \class QgsPaintEffectWidget
 * \brief Base class for effect properties widgets.
 *
 * \note Added in version 2.9
 */

class GUI_EXPORT QgsPaintEffectWidget : public QWidget
{
    Q_OBJECT

  public:
    QgsPaintEffectWidget( QWidget* parent = nullptr ) : QWidget( parent ) {}
    virtual ~QgsPaintEffectWidget() {}

    /**
     * Sets the paint effect to modify with the widget
     * @param effect paint effect
     */
    virtual void setPaintEffect( QgsPaintEffect* effect ) = 0;

  signals:

    /**
     * Emitted when properties of the effect are changed through the widget
     */
    void changed();

};

//individual effect widgets

#include "ui_widget_drawsource.h"

/** \ingroup gui
 * \class QgsDrawSourceWidget
 */
class GUI_EXPORT QgsDrawSourceWidget : public QgsPaintEffectWidget, private Ui::WidgetDrawSource
{
    Q_OBJECT

  public:
    QgsDrawSourceWidget( QWidget* parent = nullptr );

    static QgsPaintEffectWidget* create() { return new QgsDrawSourceWidget(); }

    virtual void setPaintEffect( QgsPaintEffect* effect ) override;

  private:
    QgsDrawSourceEffect* mEffect;

    void initGui();
    void blockSignals( const bool block );

  private slots:

    void on_mTransparencySpnBx_valueChanged( double value );
    void on_mDrawModeComboBox_currentIndexChanged( int index );
    void on_mBlendCmbBx_currentIndexChanged( int index );
    void on_mTransparencySlider_valueChanged( int value );

};



#include "ui_widget_blur.h"

/** \ingroup gui
 * \class QgsBlurWidget
 */
class GUI_EXPORT QgsBlurWidget : public QgsPaintEffectWidget, private Ui::WidgetBlur
{
    Q_OBJECT

  public:
    QgsBlurWidget( QWidget* parent = nullptr );

    static QgsPaintEffectWidget* create() { return new QgsBlurWidget(); }

    virtual void setPaintEffect( QgsPaintEffect* effect ) override;

  private:
    QgsBlurEffect* mEffect;

    void initGui();
    void blockSignals( const bool block );

  private slots:

    void on_mBlurTypeCombo_currentIndexChanged( int index );
    void on_mBlurStrengthSpnBx_valueChanged( int value );
    void on_mTransparencySpnBx_valueChanged( double value );
    void on_mDrawModeComboBox_currentIndexChanged( int index );
    void on_mBlendCmbBx_currentIndexChanged( int index );
    void on_mTransparencySlider_valueChanged( int value );

};



#include "ui_widget_shadoweffect.h"

/** \ingroup gui
 * \class QgsShadowEffectWidget
 */
class GUI_EXPORT QgsShadowEffectWidget : public QgsPaintEffectWidget, private Ui::WidgetShadowEffect
{
    Q_OBJECT

  public:
    QgsShadowEffectWidget( QWidget* parent = nullptr );

    static QgsPaintEffectWidget* create() { return new QgsShadowEffectWidget(); }

    virtual void setPaintEffect( QgsPaintEffect* effect ) override;

  private:
    QgsShadowEffect* mEffect;

    void initGui();
    void blockSignals( const bool block );

  private slots:
    void on_mShadowOffsetAngleSpnBx_valueChanged( int value );
    void on_mShadowOffsetAngleDial_valueChanged( int value );
    void on_mShadowOffsetSpnBx_valueChanged( double value );
    void on_mOffsetUnitWidget_changed();
    void on_mShadowTranspSpnBx_valueChanged( double value );
    void on_mShadowColorBtn_colorChanged( const QColor& color );
    void on_mDrawModeComboBox_currentIndexChanged( int index );
    void on_mShadowBlendCmbBx_currentIndexChanged( int index );
    void on_mShadowRadiuSpnBx_valueChanged( int value );
    void on_mShadowTranspSlider_valueChanged( int value );
};


#include "ui_widget_glow.h"

/** \ingroup gui
 * \class QgsGlowWidget
 */
class GUI_EXPORT QgsGlowWidget : public QgsPaintEffectWidget, private Ui::WidgetGlow
{
    Q_OBJECT

  public:
    QgsGlowWidget( QWidget* parent = nullptr );

    static QgsPaintEffectWidget* create() { return new QgsGlowWidget(); }

    virtual void setPaintEffect( QgsPaintEffect* effect ) override;

  private:
    QgsGlowEffect* mEffect;

    void initGui();
    void blockSignals( const bool block );

  private slots:
    void colorModeChanged();
    void on_mSpreadSpnBx_valueChanged( double value );
    void on_mSpreadUnitWidget_changed();
    void on_mTranspSpnBx_valueChanged( double value );
    void on_mColorBtn_colorChanged( const QColor& color );
    void on_mBlendCmbBx_currentIndexChanged( int index );
    void on_mDrawModeComboBox_currentIndexChanged( int index );
    void on_mBlurRadiusSpnBx_valueChanged( int value );
    void on_mTranspSlider_valueChanged( int value );
    void applyColorRamp();

};

#include "ui_widget_transform.h"

/** \ingroup gui
 * \class QgsTransformWidget
 */
class GUI_EXPORT QgsTransformWidget : public QgsPaintEffectWidget, private Ui::WidgetTransform
{
    Q_OBJECT

  public:
    QgsTransformWidget( QWidget* parent = nullptr );

    static QgsPaintEffectWidget* create() { return new QgsTransformWidget(); }

    virtual void setPaintEffect( QgsPaintEffect* effect ) override;

  private:
    QgsTransformEffect* mEffect;

    void initGui();
    void blockSignals( const bool block );

  private slots:

    void on_mDrawModeComboBox_currentIndexChanged( int index );
    void on_mSpinTranslateX_valueChanged( double value );
    void on_mSpinTranslateY_valueChanged( double value );
    void on_mTranslateUnitWidget_changed();
    void on_mReflectXCheckBox_stateChanged( int state );
    void on_mReflectYCheckBox_stateChanged( int state );
    void on_mSpinShearX_valueChanged( double value );
    void on_mSpinShearY_valueChanged( double value );
    void on_mSpinScaleX_valueChanged( double value );
    void on_mSpinScaleY_valueChanged( double value );
    void on_mRotationSpinBox_valueChanged( double value );

};


#include "ui_widget_coloreffects.h"

/** \ingroup gui
 * \class QgsColorEffectWidget
 */
class GUI_EXPORT QgsColorEffectWidget : public QgsPaintEffectWidget, private Ui::WidgetColorEffect
{
    Q_OBJECT

  public:
    QgsColorEffectWidget( QWidget* parent = nullptr );

    static QgsPaintEffectWidget* create() { return new QgsColorEffectWidget(); }

    virtual void setPaintEffect( QgsPaintEffect* effect ) override;

  private:
    QgsColorEffect* mEffect;

    void initGui();
    void blockSignals( const bool block );
    void enableColorizeControls( const bool enable );

  private slots:

    void on_mTranspSpnBx_valueChanged( double value );
    void on_mBlendCmbBx_currentIndexChanged( int index );
    void on_mDrawModeComboBox_currentIndexChanged( int index );
    void on_mTranspSlider_valueChanged( int value );
    void on_mBrightnessSpinBox_valueChanged( int value );
    void on_mContrastSpinBox_valueChanged( int value );
    void on_mSaturationSpinBox_valueChanged( int value );
    void on_mColorizeStrengthSpinBox_valueChanged( int value );
    void on_mColorizeCheck_stateChanged( int state );
    void on_mColorizeColorButton_colorChanged( const QColor& color );
    void on_mGrayscaleCombo_currentIndexChanged( int index );

};



#endif //QGSPAINTEFFECTWIDGET_H
