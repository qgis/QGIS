/***************************************************************************
 qgssymbollayerwidget.h - symbol layer widgets

 ---------------------
 begin                : November 2009
 copyright            : (C) 2009 by Martin Dobias
 email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSYMBOLLAYERWIDGET_H
#define QGSSYMBOLLAYERWIDGET_H

#include "qgspropertyoverridebutton.h"
#include "qgis.h"
#include "qgssymbolwidgetcontext.h"
#include "qgssymbollayer.h"
#include <QWidget>
#include <QStandardItemModel>

class QgsVectorLayer;
class QgsMapCanvas;

/**
 * \ingroup gui
 * \class QgsSymbolLayerWidget
 */
class GUI_EXPORT QgsSymbolLayerWidget : public QWidget, protected QgsExpressionContextGenerator
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsSymbolLayerWidget( QWidget *parent SIP_TRANSFERTHIS, QgsVectorLayer *vl = nullptr )
      : QWidget( parent )
      , mVectorLayer( vl )
    {}

    virtual void setSymbolLayer( QgsSymbolLayer *layer ) = 0;
    virtual QgsSymbolLayer *symbolLayer() = 0;

    /**
     * Sets the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \param context symbol widget context
     * \see context()
     * \since QGIS 3.0
     */
    virtual void setContext( const QgsSymbolWidgetContext &context );

    /**
     * Returns the context in which the symbol widget is shown, e.g., the associated map canvas and expression contexts.
     * \see setContext()
     * \since QGIS 3.0
     */
    QgsSymbolWidgetContext context() const;

    /**
     * Returns the vector layer associated with the widget.
     * \since QGIS 2.12
     */
    const QgsVectorLayer *vectorLayer() const { return mVectorLayer; }

  protected:

    /**
     * Registers a data defined override button. Handles setting up connections
     * for the button and initializing the button to show the correct descriptions
     * and help text for the associated property.
     * \since QGIS 3.0
     */
    void registerDataDefinedButton( QgsPropertyOverrideButton *button, QgsSymbolLayer::Property key );

    QgsExpressionContext createExpressionContext() const override;

  private:
    QgsVectorLayer *mVectorLayer = nullptr;

    QgsMapCanvas *mMapCanvas = nullptr;

  signals:

    /**
     * Should be emitted whenever configuration changes happened on this symbol layer configuration.
     * If the subsymbol is changed, symbolChanged() should be emitted instead.
     */
    void changed();

    /**
     * Should be emitted whenever the sub symbol changed on this symbol layer configuration.
     * Normally changed() should be preferred.
     *
     * \see changed()
     */
    void symbolChanged();

  protected slots:
    void updateDataDefinedProperty();

  private slots:
    void createAuxiliaryField();

  private:
    QgsSymbolWidgetContext mContext;
};

///////////

#include "ui_widget_simpleline.h"

class QgsSimpleLineSymbolLayer;

/**
 * \ingroup gui
 * \class QgsSimpleLineSymbolLayerWidget
 */
class GUI_EXPORT QgsSimpleLineSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetSimpleLine
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSimpleLineSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsSimpleLineSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsSimpleLineSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsSimpleLineSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  protected:
    QgsSimpleLineSymbolLayer *mLayer = nullptr;

    //creates a new icon for the 'change pattern' button
    void updatePatternIcon();

  private slots:

    void updateAssistantSymbol();
    void penWidthChanged();
    void colorChanged( const QColor &color );
    void penStyleChanged();
    void offsetChanged();
    void mCustomCheckBox_stateChanged( int state );
    void mChangePatternButton_clicked();
    void mPenWidthUnitWidget_changed();
    void mOffsetUnitWidget_changed();
    void mDashPatternUnitWidget_changed();
    void mDrawInsideCheckBox_stateChanged( int state );

  private:

    std::shared_ptr< QgsLineSymbol > mAssistantPreviewSymbol;

};

///////////

#include "ui_widget_simplemarker.h"

class QgsSimpleMarkerSymbolLayer;

/**
 * \ingroup gui
 * \class QgsSimpleMarkerSymbolLayerWidget
 */
class GUI_EXPORT QgsSimpleMarkerSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetSimpleMarker
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSimpleMarkerSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsSimpleMarkerSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsSimpleMarkerSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsSimpleMarkerSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  public slots:

    void setColorStroke( const QColor &color );
    void setColorFill( const QColor &color );

  protected:
    QgsSimpleMarkerSymbolLayer *mLayer = nullptr;

  private slots:
    void setSize();
    void setAngle();
    void setOffset();
    void mSizeUnitWidget_changed();
    void mOffsetUnitWidget_changed();
    void mStrokeWidthUnitWidget_changed();
    void mStrokeStyleComboBox_currentIndexChanged( int index );
    void mStrokeWidthSpinBox_valueChanged( double d );
    void mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void mVerticalAnchorComboBox_currentIndexChanged( int index );
    void setShape();
    void updateAssistantSymbol();
    void penJoinStyleChanged();

  private:

    std::shared_ptr< QgsMarkerSymbol > mAssistantPreviewSymbol;
};

///////////

#include "ui_widget_simplefill.h"

class QgsSimpleFillSymbolLayer;

/**
 * \ingroup gui
 * \class QgsSimpleFillSymbolLayerWidget
 */
class GUI_EXPORT QgsSimpleFillSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetSimpleFill
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSimpleFillSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsSimpleFillSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsSimpleFillSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsSimpleFillSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  public slots:
    void setColor( const QColor &color );
    void setStrokeColor( const QColor &color );

  protected:
    QgsSimpleFillSymbolLayer *mLayer = nullptr;

  private slots:
    void setBrushStyle();
    void strokeWidthChanged();
    void strokeStyleChanged();
    void offsetChanged();
    void mStrokeWidthUnitWidget_changed();
    void mOffsetUnitWidget_changed();

};


///////////

#include "ui_widget_filledmarker.h"

class QgsFilledMarkerSymbolLayer;

/**
 * \ingroup gui
 * \class QgsFilledMarkerSymbolLayerWidget
 * \brief Widget for configuring QgsFilledMarkerSymbolLayer symbol layers.
 * \since QGIS 2.16
 */
class GUI_EXPORT QgsFilledMarkerSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetFilledMarker
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsFilledMarkerSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsFilledMarkerSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsFilledMarkerSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsFilledMarkerSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  protected:
    QgsFilledMarkerSymbolLayer *mLayer = nullptr;

  private slots:

    void updateAssistantSymbol();
    void setShape();
    void setSize();
    void setAngle();
    void setOffset();
    void mSizeUnitWidget_changed();
    void mOffsetUnitWidget_changed();
    void mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void mVerticalAnchorComboBox_currentIndexChanged( int index );

  private:

    std::shared_ptr< QgsMarkerSymbol > mAssistantPreviewSymbol;
};

///////////

#include "ui_widget_gradientfill.h"

class QgsGradientFillSymbolLayer;

/**
 * \ingroup gui
 * \class QgsGradientFillSymbolLayerWidget
 */
class GUI_EXPORT QgsGradientFillSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetGradientFill
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsGradientFillSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsGradientFillSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsGradientFillSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsGradientFillSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  public slots:
    void setColor( const QColor &color );
    void setColor2( const QColor &color );

    /**
     * Applies the color ramp passed on by the color ramp button
     */
    void applyColorRamp();
    void setGradientType( int index );
    void setCoordinateMode( int index );
    void setGradientSpread( int index );

  protected:
    QgsGradientFillSymbolLayer *mLayer = nullptr;

  private slots:
    void offsetChanged();
    void referencePointChanged();
    void mOffsetUnitWidget_changed();
    void colorModeChanged();
    void mSpinAngle_valueChanged( double value );

};

///////////

#include "ui_widget_shapeburstfill.h"

class QgsShapeburstFillSymbolLayer;

/**
 * \ingroup gui
 * \class QgsShapeburstFillSymbolLayerWidget
 */
class GUI_EXPORT QgsShapeburstFillSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetShapeburstFill
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsShapeburstFillSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsShapeburstFillSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsShapeburstFillSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsShapeburstFillSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  public slots:
    void setColor( const QColor &color );
    void setColor2( const QColor &color );

  protected:
    QgsShapeburstFillSymbolLayer *mLayer = nullptr;

  private slots:
    void colorModeChanged();
    void mSpinBlurRadius_valueChanged( int value );
    void mSpinMaxDistance_valueChanged( double value );
    void mDistanceUnitWidget_changed();
    void mRadioUseWholeShape_toggled( bool value );
    void applyColorRamp();
    void offsetChanged();
    void mOffsetUnitWidget_changed();
    void mIgnoreRingsCheckBox_stateChanged( int state );
};

///////////

#include "ui_widget_markerline.h"

class QgsMarkerLineSymbolLayer;

/**
 * \ingroup gui
 * \class QgsMarkerLineSymbolLayerWidget
 */
class GUI_EXPORT QgsMarkerLineSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetMarkerLine
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsMarkerLineSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsMarkerLineSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsMarkerLineSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsMarkerLineSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  public slots:

    void setInterval( double val );
    void setOffsetAlongLine( double val );

  protected:

    QgsMarkerLineSymbolLayer *mLayer = nullptr;

  private slots:
    void setRotate();
    void setOffset();
    void setPlacement();
    void mIntervalUnitWidget_changed();
    void mOffsetUnitWidget_changed();
    void mOffsetAlongLineUnitWidget_changed();

};


///////////

#include "ui_widget_svgmarker.h"

class QgsSvgMarkerSymbolLayer;

/**
 * \ingroup gui
 * \class QgsSvgMarkerSymbolLayerWidget
 */
class GUI_EXPORT QgsSvgMarkerSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetSvgMarker
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSvgMarkerSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsSvgMarkerSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsSvgMarkerSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsSvgMarkerSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

    void setContext( const QgsSymbolWidgetContext &context ) override;

  protected:

    void populateList();
    //update gui for svg file (insert new path, update activation of gui elements for svg params)
    void setGuiForSvg( const QgsSvgMarkerSymbolLayer *layer );

    QgsSvgMarkerSymbolLayer *mLayer = nullptr;

  private slots:
    void setName( const QModelIndex &idx );
    void populateIcons( const QModelIndex &idx );
    void svgSourceChanged( const QString &text );
    void mChangeColorButton_colorChanged( const QColor &color );
    void mChangeStrokeColorButton_colorChanged( const QColor &color );
    void mStrokeWidthSpinBox_valueChanged( double d );
    void mSizeUnitWidget_changed();
    void mStrokeWidthUnitWidget_changed();
    void mOffsetUnitWidget_changed();
    void mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void mVerticalAnchorComboBox_currentIndexChanged( int index );
    void setWidth();
    void setHeight();
    void lockAspectRatioChanged( bool locked );
    void setAngle();
    void setOffset();
    void updateAssistantSymbol();

  private:

    std::shared_ptr< QgsMarkerSymbol > mAssistantPreviewSymbol;
    int mIconSize = 30;

};

///////////

#include "ui_widget_rastermarker.h"

class QgsRasterMarkerSymbolLayer;

/**
 * \ingroup gui
 * \class QgsRasterMarkerSymbolLayerWidget
 * \brief Widget for configuring QgsRasterMarkerSymbolLayer symbol layers.
 * \since QGIS 3.6
 */
class GUI_EXPORT QgsRasterMarkerSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetRasterMarker
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsRasterMarkerSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsRasterMarkerSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsRasterMarkerSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsRasterMarkerSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;
    void setContext( const QgsSymbolWidgetContext &context ) override;

  protected:

    QgsRasterMarkerSymbolLayer *mLayer = nullptr;

  private slots:
    void imageSourceChanged( const QString &text );
    void mSizeUnitWidget_changed();
    void mOffsetUnitWidget_changed();
    void mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void mVerticalAnchorComboBox_currentIndexChanged( int index );
    void setWidth();
    void setHeight();
    void setLockAspectRatio( bool locked );
    void setAngle();
    void setOffset();
    void setOpacity( double value );
    void updatePreviewImage();

};

///////////

#include "ui_widget_rasterfill.h"

class QgsRasterFillSymbolLayer;

/**
 * \ingroup gui
 * \class QgsRasterFillSymbolLayerWidget
 */
class GUI_EXPORT QgsRasterFillSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetRasterFill
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsRasterFillSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsRasterFillSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsRasterFillSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsRasterFillSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  protected:
    QgsRasterFillSymbolLayer *mLayer = nullptr;

  private slots:
    void imageSourceChanged( const QString &text );
    void setCoordinateMode( int index );
    void opacityChanged( double value );
    void offsetChanged();
    void mOffsetUnitWidget_changed();
    void mRotationSpinBox_valueChanged( double d );
    void mWidthUnitWidget_changed();
    void mWidthSpinBox_valueChanged( double d );

  private:
    void updatePreviewImage();
};

///////////

#include "ui_widget_svgfill.h"

class QgsSVGFillSymbolLayer;

/**
 * \ingroup gui
 * \class QgsSVGFillSymbolLayerWidget
 */
class GUI_EXPORT QgsSVGFillSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetSVGFill
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsSVGFillSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsSVGFillSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsSVGFillSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsSVGFillSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;
    void setContext( const QgsSymbolWidgetContext &context ) override;

  protected:
    QgsSVGFillSymbolLayer *mLayer = nullptr;
    void insertIcons();

    /**
     * Enables or disables svg fill color, stroke color and stroke width based on whether the
     * svg file supports custom parameters.
     * \param resetValues set to true to overwrite existing layer fill color, stroke color and stroke width
     * with default values from svg file
     */
    void updateParamGui( bool resetValues = true );

  private slots:
    void mTextureWidthSpinBox_valueChanged( double d );
    void svgSourceChanged( const QString &text );
    void setFile( const QModelIndex &item );
    void populateIcons( const QModelIndex &item );
    void mRotationSpinBox_valueChanged( double d );
    void mChangeColorButton_colorChanged( const QColor &color );
    void mChangeStrokeColorButton_colorChanged( const QColor &color );
    void mStrokeWidthSpinBox_valueChanged( double d );
    void mTextureWidthUnitWidget_changed();
    void mSvgStrokeWidthUnitWidget_changed();
};

//////////

#include "ui_widget_linepatternfill.h"

class QgsLinePatternFillSymbolLayer;

/**
 * \ingroup gui
 * \class QgsLinePatternFillSymbolLayerWidget
 */
class GUI_EXPORT QgsLinePatternFillSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetLinePatternFill
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsLinePatternFillSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsLinePatternFillSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsLinePatternFillSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsLinePatternFillSymbolLayerWidget( vl ); }

    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  protected:
    QgsLinePatternFillSymbolLayer *mLayer = nullptr;

  private slots:
    void mAngleSpinBox_valueChanged( double d );
    void mDistanceSpinBox_valueChanged( double d );
    void mOffsetSpinBox_valueChanged( double d );
    void mDistanceUnitWidget_changed();
    void mOffsetUnitWidget_changed();
};

//////////

#include "ui_widget_pointpatternfill.h"

class QgsPointPatternFillSymbolLayer;

/**
 * \ingroup gui
 * \class QgsPointPatternFillSymbolLayerWidget
 */
class GUI_EXPORT QgsPointPatternFillSymbolLayerWidget: public QgsSymbolLayerWidget, private Ui::WidgetPointPatternFill
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsPointPatternFillSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsPointPatternFillSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsPointPatternFillSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsPointPatternFillSymbolLayerWidget( vl ); }

    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  protected:
    QgsPointPatternFillSymbolLayer *mLayer = nullptr;

  private slots:
    void mHorizontalDistanceSpinBox_valueChanged( double d );
    void mVerticalDistanceSpinBox_valueChanged( double d );
    void mHorizontalDisplacementSpinBox_valueChanged( double d );
    void mVerticalDisplacementSpinBox_valueChanged( double d );
    void mHorizontalDistanceUnitWidget_changed();
    void mVerticalDistanceUnitWidget_changed();
    void mHorizontalDisplacementUnitWidget_changed();
    void mVerticalDisplacementUnitWidget_changed();
};

/////////

#include "ui_widget_fontmarker.h"

class QgsFontMarkerSymbolLayer;
class CharacterWidget;

/**
 * \ingroup gui
 * \class QgsFontMarkerSymbolLayerWidget
 */
class GUI_EXPORT QgsFontMarkerSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetFontMarker
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsFontMarkerSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsFontMarkerSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsFontMarkerSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsFontMarkerSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  public slots:
    void setFontFamily( const QFont &font );
    void setColor( const QColor &color );

    /**
     * Set stroke color.
     * \since QGIS 2.16 */
    void setColorStroke( const QColor &color );
    void setSize( double size );
    void setAngle( double angle );
    void setCharacter( QChar chr );

  protected:
    QgsFontMarkerSymbolLayer *mLayer = nullptr;
    CharacterWidget *widgetChar = nullptr;

  private slots:
    void setOffset();
    void mSizeUnitWidget_changed();
    void mOffsetUnitWidget_changed();
    void mStrokeWidthUnitWidget_changed();
    void mStrokeWidthSpinBox_valueChanged( double d );
    void mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void mVerticalAnchorComboBox_currentIndexChanged( int index );
    void penJoinStyleChanged();
    void updateAssistantSymbol();

  private:

    std::shared_ptr< QgsMarkerSymbol > mAssistantPreviewSymbol;

};

//////////


#include "ui_widget_centroidfill.h"

class QgsCentroidFillSymbolLayer;

/**
 * \ingroup gui
 * \class QgsCentroidFillSymbolLayerWidget
 */
class GUI_EXPORT QgsCentroidFillSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::WidgetCentroidFill
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsCentroidFillSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsCentroidFillSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Creates a new QgsCentroidFillSymbolLayerWidget.
     * \param vl associated vector layer
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsCentroidFillSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  protected:
    QgsCentroidFillSymbolLayer *mLayer = nullptr;

  private slots:
    void mDrawInsideCheckBox_stateChanged( int state );
    void mDrawAllPartsCheckBox_stateChanged( int state );

};


#include "ui_qgsgeometrygeneratorwidgetbase.h"
#include "qgis_gui.h"

class QgsGeometryGeneratorSymbolLayer;

/**
 * \ingroup gui
 * \class QgsGeometryGeneratorSymbolLayerWidget
 */
class GUI_EXPORT QgsGeometryGeneratorSymbolLayerWidget : public QgsSymbolLayerWidget, private Ui::GeometryGeneratorWidgetBase
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsGeometryGeneratorSymbolLayerWidget.
     * \param vl associated vector layer
     * \param parent parent widget
     */
    QgsGeometryGeneratorSymbolLayerWidget( QgsVectorLayer *vl, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Will be registered as factory
     */
    static QgsSymbolLayerWidget *create( QgsVectorLayer *vl ) SIP_FACTORY { return new QgsGeometryGeneratorSymbolLayerWidget( vl ); }

    // from base class
    void setSymbolLayer( QgsSymbolLayer *layer ) override;
    QgsSymbolLayer *symbolLayer() override;

  private:
    QgsGeometryGeneratorSymbolLayer *mLayer = nullptr;

  private slots:
    void updateExpression( const QString &string );
    void updateSymbolType();
};

#endif
