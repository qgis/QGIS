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
#include "qgis_sip.h"
#include "qgis_gui.h"

class QgsPaintEffect;
class QgsShadowEffect;
class QgsDrawSourceEffect;
class QgsBlurEffect;
class QgsGlowEffect;
class QgsTransformEffect;
class QgsColorEffect;


/**
 * \ingroup gui
 * \class QgsPaintEffectWidget
 * \brief Base class for effect properties widgets.
 *
 */

class GUI_EXPORT QgsPaintEffectWidget : public QWidget
{
    Q_OBJECT

  public:
    QgsPaintEffectWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr )
      : QWidget( parent ) {}

    /**
     * Sets the paint effect to modify with the widget
     * \param effect paint effect
     */
    virtual void setPaintEffect( QgsPaintEffect *effect ) = 0;

  signals:

    /**
     * Emitted when properties of the effect are changed through the widget
     */
    void changed();
};

//individual effect widgets

#include "ui_widget_drawsource.h"

/**
 * \ingroup gui
 * \class QgsDrawSourceWidget
 */
class GUI_EXPORT QgsDrawSourceWidget : public QgsPaintEffectWidget, private Ui::WidgetDrawSource
{
    Q_OBJECT

  public:
    QgsDrawSourceWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    static QgsPaintEffectWidget *create() SIP_FACTORY { return new QgsDrawSourceWidget(); }

    void setPaintEffect( QgsPaintEffect *effect ) override;

  private:
    QgsDrawSourceEffect *mEffect = nullptr;

    void initGui();
    void blockSignals( bool block );

  private slots:

    void opacityChanged( double value );
    void mDrawModeComboBox_currentIndexChanged( int index );
    void mBlendCmbBx_currentIndexChanged( int index );
};


#include "ui_widget_blur.h"

/**
 * \ingroup gui
 * \class QgsBlurWidget
 */
class GUI_EXPORT QgsBlurWidget : public QgsPaintEffectWidget, private Ui::WidgetBlur
{
    Q_OBJECT

  public:
    QgsBlurWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    static QgsPaintEffectWidget *create() SIP_FACTORY { return new QgsBlurWidget(); }

    void setPaintEffect( QgsPaintEffect *effect ) override;

  private:
    QgsBlurEffect *mEffect = nullptr;

    void initGui();
    void blockSignals( bool block );

  private slots:

    void mBlurTypeCombo_currentIndexChanged( int index );
    void mBlurStrengthSpnBx_valueChanged( double value );
    void mBlurUnitWidget_changed();
    void opacityChanged( double value );
    void mDrawModeComboBox_currentIndexChanged( int index );
    void mBlendCmbBx_currentIndexChanged( int index );
};


#include "ui_widget_shadoweffect.h"

/**
 * \ingroup gui
 * \class QgsShadowEffectWidget
 */
class GUI_EXPORT QgsShadowEffectWidget : public QgsPaintEffectWidget, private Ui::WidgetShadowEffect
{
    Q_OBJECT

  public:
    QgsShadowEffectWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    static QgsPaintEffectWidget *create() SIP_FACTORY { return new QgsShadowEffectWidget(); }

    void setPaintEffect( QgsPaintEffect *effect ) override;

  private:
    QgsShadowEffect *mEffect = nullptr;

    void initGui();
    void blockSignals( bool block );

  private slots:
    void mShadowOffsetAngleSpnBx_valueChanged( int value );
    void mShadowOffsetAngleDial_valueChanged( int value );
    void mShadowOffsetSpnBx_valueChanged( double value );
    void mOffsetUnitWidget_changed();
    void opacityChanged( double value );
    void mShadowColorBtn_colorChanged( const QColor &color );
    void mDrawModeComboBox_currentIndexChanged( int index );
    void mShadowBlendCmbBx_currentIndexChanged( int index );
    void mShadowRadiuSpnBx_valueChanged( double value );
    void mBlurUnitWidget_changed();
};


#include "ui_widget_glow.h"

/**
 * \ingroup gui
 * \class QgsGlowWidget
 */
class GUI_EXPORT QgsGlowWidget : public QgsPaintEffectWidget, private Ui::WidgetGlow
{
    Q_OBJECT

  public:
    QgsGlowWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    static QgsPaintEffectWidget *create() SIP_FACTORY { return new QgsGlowWidget(); }

    void setPaintEffect( QgsPaintEffect *effect ) override;

  private:
    QgsGlowEffect *mEffect = nullptr;

    void initGui();
    void blockSignals( bool block );

  private slots:
    void colorModeChanged();
    void mSpreadSpnBx_valueChanged( double value );
    void mSpreadUnitWidget_changed();
    void opacityChanged( double value );
    void mColorBtn_colorChanged( const QColor &color );
    void mBlendCmbBx_currentIndexChanged( int index );
    void mDrawModeComboBox_currentIndexChanged( int index );
    void mBlurRadiusSpnBx_valueChanged( double value );
    void mBlurUnitWidget_changed();
    void applyColorRamp();
};

#include "ui_widget_transform.h"

/**
 * \ingroup gui
 * \class QgsTransformWidget
 */
class GUI_EXPORT QgsTransformWidget : public QgsPaintEffectWidget, private Ui::WidgetTransform
{
    Q_OBJECT

  public:
    QgsTransformWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    static QgsPaintEffectWidget *create() SIP_FACTORY { return new QgsTransformWidget(); }

    void setPaintEffect( QgsPaintEffect *effect ) override;

  private:
    QgsTransformEffect *mEffect = nullptr;

    void initGui();
    void blockSignals( bool block );

  private slots:

    void mDrawModeComboBox_currentIndexChanged( int index );
    void mSpinTranslateX_valueChanged( double value );
    void mSpinTranslateY_valueChanged( double value );
    void mTranslateUnitWidget_changed();
    void mReflectXCheckBox_stateChanged( int state );
    void mReflectYCheckBox_stateChanged( int state );
    void mSpinShearX_valueChanged( double value );
    void mSpinShearY_valueChanged( double value );
    void mSpinScaleX_valueChanged( double value );
    void mSpinScaleY_valueChanged( double value );
    void mRotationSpinBox_valueChanged( double value );
};


#include "ui_widget_coloreffects.h"

/**
 * \ingroup gui
 * \class QgsColorEffectWidget
 */
class GUI_EXPORT QgsColorEffectWidget : public QgsPaintEffectWidget, private Ui::WidgetColorEffect
{
    Q_OBJECT

  public:
    QgsColorEffectWidget( QWidget *parent = nullptr );

    static QgsPaintEffectWidget *create() SIP_FACTORY { return new QgsColorEffectWidget(); }

    void setPaintEffect( QgsPaintEffect *effect ) override;

  private:
    QgsColorEffect *mEffect = nullptr;

    void initGui();
    void blockSignals( bool block );
    void enableColorizeControls( bool enable );

  private slots:

    void opacityChanged( double value );
    void mBlendCmbBx_currentIndexChanged( int index );
    void mDrawModeComboBox_currentIndexChanged( int index );
    void mBrightnessSpinBox_valueChanged( int value );
    void mContrastSpinBox_valueChanged( int value );
    void mSaturationSpinBox_valueChanged( int value );
    void mColorizeStrengthSpinBox_valueChanged( int value );
    void mColorizeCheck_stateChanged( int state );
    void mColorizeColorButton_colorChanged( const QColor &color );
    void mGrayscaleCombo_currentIndexChanged( int index );
};


#endif //QGSPAINTEFFECTWIDGET_H
