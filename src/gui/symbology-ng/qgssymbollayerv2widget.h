/***************************************************************************
 qgssymbollayerv2widget.h - symbol layer widgets

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

#ifndef QGSSYMBOLLAYERV2WIDGET_H
#define QGSSYMBOLLAYERV2WIDGET_H

#include <qgsdatadefinedbutton.h>

#include <QWidget>

class QgsSymbolLayerV2;
class QgsVectorLayer;

class GUI_EXPORT QgsSymbolLayerV2Widget : public QWidget
{
    Q_OBJECT

  public:
    QgsSymbolLayerV2Widget( QWidget* parent, const QgsVectorLayer* vl = 0 ) : QWidget( parent ), mVectorLayer( vl ) {}
    virtual ~QgsSymbolLayerV2Widget() {}

    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) = 0;
    virtual QgsSymbolLayerV2* symbolLayer() = 0;

  protected:
    const QgsVectorLayer* mVectorLayer;

    void registerDataDefinedButton( QgsDataDefinedButton * button, const QString & propertyName, QgsDataDefinedButton::DataType type, const QString & description );

    /** Get label for data defined entry.
     * Implemented only for 'size' of marker symbols
     * @note added in 2.1
     * @deprecated no longer used
     */
    Q_DECL_DEPRECATED virtual QString dataDefinedPropertyLabel( const QString &entryName );

  signals:
    void changed();

  protected slots:
    void updateDataDefinedProperty();
};

///////////

#include "ui_widget_simpleline.h"

class QgsSimpleLineSymbolLayerV2;

class GUI_EXPORT QgsSimpleLineSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetSimpleLine
{
    Q_OBJECT

  public:
    QgsSimpleLineSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsSimpleLineSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  public slots:
    void penWidthChanged();
    void colorChanged( const QColor& color );
    void penStyleChanged();
    void offsetChanged();
    void on_mCustomCheckBox_stateChanged( int state );
    void on_mChangePatternButton_clicked();
    void on_mPenWidthUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mDashPatternUnitWidget_changed();
    void on_mDrawInsideCheckBox_stateChanged( int state );

  protected:
    QgsSimpleLineSymbolLayerV2* mLayer;

    //creates a new icon for the 'change pattern' button
    void updatePatternIcon();

};

///////////

#include "ui_widget_simplemarker.h"

class QgsSimpleMarkerSymbolLayerV2;

class GUI_EXPORT QgsSimpleMarkerSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetSimpleMarker
{
    Q_OBJECT

  public:
    QgsSimpleMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = NULL );
    ~QgsSimpleMarkerSymbolLayerV2Widget();

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsSimpleMarkerSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  public slots:
    void setName();
    void setColorBorder( const QColor& color );
    void setColorFill( const QColor& color );
    void setSize();
    void setAngle();
    void setOffset();
    void on_mSizeUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mOutlineWidthUnitWidget_changed();
    void on_mOutlineStyleComboBox_currentIndexChanged( int index );
    void on_mOutlineWidthSpinBox_valueChanged( double d );
    void on_mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void on_mVerticalAnchorComboBox_currentIndexChanged( int index );

  protected:
    QgsSimpleMarkerSymbolLayerV2* mLayer;

  private slots:

    void updateAssistantSymbol();

  private:

    QgsMarkerSymbolV2* mAssistantPreviewSymbol;
};

///////////

#include "ui_widget_simplefill.h"

class QgsSimpleFillSymbolLayerV2;

class GUI_EXPORT QgsSimpleFillSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetSimpleFill
{
    Q_OBJECT

  public:
    QgsSimpleFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsSimpleFillSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  public slots:
    void setColor( const QColor& color );
    void setBorderColor( const QColor& color );
    void setBrushStyle();
    void borderWidthChanged();
    void borderStyleChanged();
    void offsetChanged();
    void on_mBorderWidthUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();

  protected:
    QgsSimpleFillSymbolLayerV2* mLayer;
};


///////////

#include "ui_widget_gradientfill.h"

class QgsGradientFillSymbolLayerV2;

class GUI_EXPORT QgsGradientFillSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetGradientFill
{
    Q_OBJECT

  public:
    QgsGradientFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsGradientFillSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  public slots:
    void setColor( const QColor& color );
    void setColor2( const QColor& color );
    void applyColorRamp();
    void on_mButtonEditRamp_clicked();
    void setGradientType( int index );
    void setCoordinateMode( int index );
    void setGradientSpread( int index );
    void offsetChanged();
    void referencePointChanged();
    void on_mOffsetUnitWidget_changed();
    void colorModeChanged();
    void on_mSpinAngle_valueChanged( double value );

  protected:
    QgsGradientFillSymbolLayerV2* mLayer;
};

///////////

#include "ui_widget_shapeburstfill.h"

class QgsShapeburstFillSymbolLayerV2;

class GUI_EXPORT QgsShapeburstFillSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetShapeburstFill
{
    Q_OBJECT

  public:
    QgsShapeburstFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsShapeburstFillSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  public slots:
    void setColor( const QColor& color );
    void setColor2( const QColor& color );
    void colorModeChanged();
    void on_mSpinBlurRadius_valueChanged( int value );
    void on_mSpinMaxDistance_valueChanged( double value );
    void on_mDistanceUnitWidget_changed();
    void on_mRadioUseWholeShape_toggled( bool value );
    void applyColorRamp();
    void on_mButtonEditRamp_clicked();
    void offsetChanged();
    void on_mOffsetUnitWidget_changed();
    void on_mIgnoreRingsCheckBox_stateChanged( int state );

  protected:
    QgsShapeburstFillSymbolLayerV2* mLayer;
};

///////////

#include "ui_widget_markerline.h"

class QgsMarkerLineSymbolLayerV2;

class GUI_EXPORT QgsMarkerLineSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetMarkerLine
{
    Q_OBJECT

  public:
    QgsMarkerLineSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsMarkerLineSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  public slots:

    void setInterval( double val );
    void setOffsetAlongLine( double val );
    void setRotate();
    void setOffset();
    void setPlacement();
    void on_mIntervalUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mOffsetAlongLineUnitWidget_changed();

  protected:

    QgsMarkerLineSymbolLayerV2* mLayer;
};


///////////

#include "ui_widget_svgmarker.h"

class QgsSvgMarkerSymbolLayerV2;

class GUI_EXPORT QgsSvgMarkerSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetSvgMarker
{
    Q_OBJECT

  public:
    QgsSvgMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = NULL );
    ~QgsSvgMarkerSymbolLayerV2Widget();

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsSvgMarkerSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  public slots:
    void setName( const QModelIndex& idx );
    void populateIcons( const QModelIndex& idx );
    void setSize();
    void setAngle();
    void setOffset();
    void on_mFileToolButton_clicked();
    void on_mFileLineEdit_textEdited( const QString& text );
    void on_mFileLineEdit_editingFinished();
    void on_mChangeColorButton_colorChanged( const QColor& color );
    void on_mChangeBorderColorButton_colorChanged( const QColor& color );
    void on_mBorderWidthSpinBox_valueChanged( double d );
    void on_mSizeUnitWidget_changed();
    void on_mBorderWidthUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void on_mVerticalAnchorComboBox_currentIndexChanged( int index );


  protected:

    void populateList();
    //update gui for svg file (insert new path, update activation of gui elements for svg params)
    void setGuiForSvg( const QgsSvgMarkerSymbolLayerV2* layer );

    QgsSvgMarkerSymbolLayerV2* mLayer;

  private slots:

    void updateAssistantSymbol();

  private:

    QgsMarkerSymbolV2* mAssistantPreviewSymbol;

};

///////////

#include "ui_widget_rasterfill.h"

class QgsRasterFillSymbolLayer;

class GUI_EXPORT QgsRasterFillSymbolLayerWidget : public QgsSymbolLayerV2Widget, private Ui::WidgetRasterFill
{
    Q_OBJECT

  public:
    QgsRasterFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsRasterFillSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  protected:
    QgsRasterFillSymbolLayer* mLayer;

  private slots:
    void on_mBrowseToolButton_clicked();
    void on_mImageLineEdit_editingFinished();
    void setCoordinateMode( int index );
    void on_mSpinTransparency_valueChanged( int value );
    void offsetChanged();
    void on_mOffsetUnitWidget_changed();
    void on_mRotationSpinBox_valueChanged( double d );
    void on_mWidthUnitWidget_changed();
    void on_mWidthSpinBox_valueChanged( double d );

  private:
    void updatePreviewImage();
};

///////////

#include "ui_widget_svgfill.h"

class QgsSVGFillSymbolLayer;

class GUI_EXPORT QgsSVGFillSymbolLayerWidget : public QgsSymbolLayerV2Widget, private Ui::WidgetSVGFill
{
    Q_OBJECT

  public:
    QgsSVGFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsSVGFillSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  protected:
    QgsSVGFillSymbolLayer* mLayer;
    void insertIcons();
    /** Enables or disables svg fill color, border color and border width based on whether the
     * svg file supports custom parameters.
     * @param resetValues set to true to overwrite existing layer fill color, border color and border width
     * with default values from svg file
    */
    void updateParamGui( bool resetValues = true );

  private slots:
    void on_mBrowseToolButton_clicked();
    void on_mTextureWidthSpinBox_valueChanged( double d );
    void on_mSVGLineEdit_textEdited( const QString & text );
    void on_mSVGLineEdit_editingFinished();
    void setFile( const QModelIndex& item );
    void populateIcons( const QModelIndex& item );
    void on_mRotationSpinBox_valueChanged( double d );
    void on_mChangeColorButton_colorChanged( const QColor& color );
    void on_mChangeBorderColorButton_colorChanged( const QColor& color );
    void on_mBorderWidthSpinBox_valueChanged( double d );
    void on_mTextureWidthUnitWidget_changed();
    void on_mSvgOutlineWidthUnitWidget_changed();
};

//////////

#include "ui_widget_linepatternfill.h"

class QgsLinePatternFillSymbolLayer;

class GUI_EXPORT QgsLinePatternFillSymbolLayerWidget : public QgsSymbolLayerV2Widget, private Ui::WidgetLinePatternFill
{
    Q_OBJECT

  public:

    QgsLinePatternFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = NULL );
    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsLinePatternFillSymbolLayerWidget( vl ); }

    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  protected:
    QgsLinePatternFillSymbolLayer* mLayer;

  private slots:
    void on_mAngleSpinBox_valueChanged( double d );
    void on_mDistanceSpinBox_valueChanged( double d );
    void on_mOffsetSpinBox_valueChanged( double d );
    void on_mDistanceUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
};

//////////

#include "ui_widget_pointpatternfill.h"

class QgsPointPatternFillSymbolLayer;

class GUI_EXPORT QgsPointPatternFillSymbolLayerWidget: public QgsSymbolLayerV2Widget, private Ui::WidgetPointPatternFill
{
    Q_OBJECT

  public:
    QgsPointPatternFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = NULL );
    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsPointPatternFillSymbolLayerWidget( vl ); }

    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  protected:
    QgsPointPatternFillSymbolLayer* mLayer;

  private slots:
    void on_mHorizontalDistanceSpinBox_valueChanged( double d );
    void on_mVerticalDistanceSpinBox_valueChanged( double d );
    void on_mHorizontalDisplacementSpinBox_valueChanged( double d );
    void on_mVerticalDisplacementSpinBox_valueChanged( double d );
    void on_mHorizontalDistanceUnitWidget_changed();
    void on_mVerticalDistanceUnitWidget_changed();
    void on_mHorizontalDisplacementUnitWidget_changed();
    void on_mVerticalDisplacementUnitWidget_changed();
};

/////////

#include "ui_widget_fontmarker.h"

class QgsFontMarkerSymbolLayerV2;
class CharacterWidget;

class GUI_EXPORT QgsFontMarkerSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetFontMarker
{
    Q_OBJECT

  public:
    QgsFontMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = NULL );

    ~QgsFontMarkerSymbolLayerV2Widget();

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsFontMarkerSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  public slots:
    void setFontFamily( const QFont& font );
    void setColor( const QColor& color );
    void setSize( double size );
    void setAngle( double angle );
    void setCharacter( const QChar& chr );
    void setOffset();
    void on_mSizeUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void on_mVerticalAnchorComboBox_currentIndexChanged( int index );

  protected:
    QgsFontMarkerSymbolLayerV2* mLayer;
    CharacterWidget* widgetChar;

  private slots:

    void updateAssistantSymbol();

  private:

    QgsMarkerSymbolV2* mAssistantPreviewSymbol;

};

//////////


#include "ui_widget_centroidfill.h"

class QgsCentroidFillSymbolLayerV2;

class GUI_EXPORT QgsCentroidFillSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetCentroidFill
{
    Q_OBJECT

  public:
    QgsCentroidFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsCentroidFillSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  public slots:
    void on_mDrawInsideCheckBox_stateChanged( int state );

  protected:
    QgsCentroidFillSymbolLayerV2* mLayer;
};


#endif
