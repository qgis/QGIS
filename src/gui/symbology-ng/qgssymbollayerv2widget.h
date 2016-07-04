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
#include <QStandardItemModel>

class QgsSymbolLayerV2;
class QgsVectorLayer;
class QgsMapCanvas;

/** \ingroup gui
 * \class QgsSymbolLayerV2Widget
 */
class GUI_EXPORT QgsSymbolLayerV2Widget : public QWidget
{
    Q_OBJECT

  public:
    QgsSymbolLayerV2Widget( QWidget* parent, const QgsVectorLayer* vl = nullptr )
        : QWidget( parent )
        , mVectorLayer( vl )
        , mPresetExpressionContext( nullptr )
        , mMapCanvas( nullptr )
    {}
    virtual ~QgsSymbolLayerV2Widget() {}

    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) = 0;
    virtual QgsSymbolLayerV2* symbolLayer() = 0;

    /** Returns the expression context used for the widget, if set. This expression context is used for
     * evaluating data defined symbol properties and for populating based expression widgets in
     * the layer widget.
     * @note added in QGIS 2.12
     * @see setExpressionContext()
     */
    QgsExpressionContext* expressionContext() const { return mPresetExpressionContext; }

    /** Sets the map canvas associated with the widget. This allows the widget to retrieve the current
     * map scale and other properties from the canvas.
     * @param canvas map canvas
     * @see mapCanvas()
     * @note added in QGIS 2.12
     */
    virtual void setMapCanvas( QgsMapCanvas* canvas );

    /** Returns the map canvas associated with the widget.
     * @see setMapCanvas
     * @note added in QGIS 2.12
     */
    const QgsMapCanvas* mapCanvas() const;

    /** Returns the vector layer associated with the widget.
     * @note added in QGIS 2.12
     */
    const QgsVectorLayer* vectorLayer() const { return mVectorLayer; }

  public slots:

    /** Sets the optional expression context used for the widget. This expression context is used for
     * evaluating data defined symbol properties and for populating based expression widgets in
     * the layer widget.
     * @param context expression context pointer. Ownership is not transferred and the object must
     * be kept alive for the lifetime of the layer widget.
     * @note added in QGIS 2.12
     * @see expressionContext()
     */
    void setExpressionContext( QgsExpressionContext* context ) { mPresetExpressionContext = context; }

  protected:
    const QgsVectorLayer* mVectorLayer;

    //! Optional preset expression context
    QgsExpressionContext* mPresetExpressionContext;

    QgsMapCanvas* mMapCanvas;

    void registerDataDefinedButton( QgsDataDefinedButton * button, const QString & propertyName, QgsDataDefinedButton::DataType type, const QString & description );

    /** Get label for data defined entry.
     * Implemented only for 'size' of marker symbols
     * @note added in 2.1
     * @deprecated no longer used
     */
    Q_DECL_DEPRECATED virtual QString dataDefinedPropertyLabel( const QString &entryName );

  signals:
    /**
     * Should be emitted whenever configuration changes happened on this symbol layer configuration.
     * If the subsymbol is changed, {@link symbolChanged()} should be emitted instead.
     */
    void changed();
    /**
     * Should be emitted whenever the sub symbol changed on this symbol layer configuration.
     * Normally {@link changed()} should be preferred.
     *
     * @see {@link changed()}
     */
    void symbolChanged();

  protected slots:
    void updateDataDefinedProperty();
};

///////////

#include "ui_widget_simpleline.h"

class QgsSimpleLineSymbolLayerV2;

/** \ingroup gui
 * \class QgsSimpleLineSymbolLayerV2Widget
 */
class GUI_EXPORT QgsSimpleLineSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetSimpleLine
{
    Q_OBJECT

  public:
    QgsSimpleLineSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    ~QgsSimpleLineSymbolLayerV2Widget();

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

  private slots:

    void updateAssistantSymbol();

  private:

    QgsLineSymbolV2* mAssistantPreviewSymbol;

};

///////////

#include "ui_widget_simplemarker.h"

class QgsSimpleMarkerSymbolLayerV2;

/** \ingroup gui
 * \class QgsSimpleMarkerSymbolLayerV2Widget
 */
class GUI_EXPORT QgsSimpleMarkerSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetSimpleMarker
{
    Q_OBJECT

  public:
    QgsSimpleMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = nullptr );
    ~QgsSimpleMarkerSymbolLayerV2Widget();

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsSimpleMarkerSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  public slots:
    //TODO QGIS 3.0 - rename to setShape
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
    void penJoinStyleChanged();

  private:

    QgsMarkerSymbolV2* mAssistantPreviewSymbol;
};

///////////

#include "ui_widget_simplefill.h"

class QgsSimpleFillSymbolLayerV2;

/** \ingroup gui
 * \class QgsSimpleFillSymbolLayerV2Widget
 */
class GUI_EXPORT QgsSimpleFillSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetSimpleFill
{
    Q_OBJECT

  public:
    QgsSimpleFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

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

#include "ui_widget_filledmarker.h"

class QgsFilledMarkerSymbolLayer;

/** \ingroup gui
 * \class QgsFilledMarkerSymbolLayerWidget
 * \brief Widget for configuring QgsFilledMarkerSymbolLayer symbol layers.
 * \note Added in version 2.16
 */
class GUI_EXPORT QgsFilledMarkerSymbolLayerWidget : public QgsSymbolLayerV2Widget, private Ui::WidgetFilledMarker
{
    Q_OBJECT

  public:

    /** Constructor for QgsFilledMarkerSymbolLayerWidget.
     * @param vl associated vector layer
     * @param parent parent widget
     */
    QgsFilledMarkerSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    ~QgsFilledMarkerSymbolLayerWidget();

    /** Creates a new QgsFilledMarkerSymbolLayerWidget.
     * @param vl associated vector layer
     */
    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsFilledMarkerSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  protected:
    QgsFilledMarkerSymbolLayer* mLayer;

  private slots:

    void updateAssistantSymbol();
    void setShape();
    void setSize();
    void setAngle();
    void setOffset();
    void on_mSizeUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void on_mVerticalAnchorComboBox_currentIndexChanged( int index );

  private:

    QgsMarkerSymbolV2* mAssistantPreviewSymbol;
};

///////////

#include "ui_widget_gradientfill.h"

class QgsGradientFillSymbolLayerV2;

/** \ingroup gui
 * \class QgsGradientFillSymbolLayerV2Widget
 */
class GUI_EXPORT QgsGradientFillSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetGradientFill
{
    Q_OBJECT

  public:
    QgsGradientFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsGradientFillSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  public slots:
    void setColor( const QColor& color );
    void setColor2( const QColor& color );
    void applyColorRamp();
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

/** \ingroup gui
 * \class QgsShapeburstFillSymbolLayerV2Widget
 */
class GUI_EXPORT QgsShapeburstFillSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetShapeburstFill
{
    Q_OBJECT

  public:
    QgsShapeburstFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

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
    void offsetChanged();
    void on_mOffsetUnitWidget_changed();
    void on_mIgnoreRingsCheckBox_stateChanged( int state );

  protected:
    QgsShapeburstFillSymbolLayerV2* mLayer;
};

///////////

#include "ui_widget_markerline.h"

class QgsMarkerLineSymbolLayerV2;

/** \ingroup gui
 * \class QgsMarkerLineSymbolLayerV2Widget
 */
class GUI_EXPORT QgsMarkerLineSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetMarkerLine
{
    Q_OBJECT

  public:
    QgsMarkerLineSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

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

/** \ingroup gui
 * \class QgsSvgMarkerSymbolLayerV2Widget
 */
class GUI_EXPORT QgsSvgMarkerSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetSvgMarker
{
    Q_OBJECT

  public:
    QgsSvgMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = nullptr );
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

/** \ingroup gui
 * \class QgsRasterFillSymbolLayerWidget
 */
class GUI_EXPORT QgsRasterFillSymbolLayerWidget : public QgsSymbolLayerV2Widget, private Ui::WidgetRasterFill
{
    Q_OBJECT

  public:
    QgsRasterFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

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

/** \ingroup gui
 * \class QgsSVGFillSymbolLayerWidget
 */
class GUI_EXPORT QgsSVGFillSymbolLayerWidget : public QgsSymbolLayerV2Widget, private Ui::WidgetSVGFill
{
    Q_OBJECT

  public:
    QgsSVGFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

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

/** \ingroup gui
 * \class QgsLinePatternFillSymbolLayerWidget
 */
class GUI_EXPORT QgsLinePatternFillSymbolLayerWidget : public QgsSymbolLayerV2Widget, private Ui::WidgetLinePatternFill
{
    Q_OBJECT

  public:

    QgsLinePatternFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );
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

/** \ingroup gui
 * \class QgsPointPatternFillSymbolLayerWidget
 */
class GUI_EXPORT QgsPointPatternFillSymbolLayerWidget: public QgsSymbolLayerV2Widget, private Ui::WidgetPointPatternFill
{
    Q_OBJECT

  public:
    QgsPointPatternFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );
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

/** \ingroup gui
 * \class QgsFontMarkerSymbolLayerV2Widget
 */
class GUI_EXPORT QgsFontMarkerSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetFontMarker
{
    Q_OBJECT

  public:
    QgsFontMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    ~QgsFontMarkerSymbolLayerV2Widget();

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsFontMarkerSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  public slots:
    void setFontFamily( const QFont& font );
    void setColor( const QColor& color );

    /** Set outline color.
     * @note added in 2.16 */
    void setColorBorder( const QColor& color );
    void setSize( double size );
    void setAngle( double angle );
    void setCharacter( QChar chr );
    void setOffset();
    void on_mSizeUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mBorderWidthUnitWidget_changed();
    void on_mBorderWidthSpinBox_valueChanged( double d );
    void on_mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void on_mVerticalAnchorComboBox_currentIndexChanged( int index );

  protected:
    QgsFontMarkerSymbolLayerV2* mLayer;
    CharacterWidget* widgetChar;

  private slots:

    void penJoinStyleChanged();
    void updateAssistantSymbol();

  private:

    QgsMarkerSymbolV2* mAssistantPreviewSymbol;

};

//////////


#include "ui_widget_centroidfill.h"

class QgsCentroidFillSymbolLayerV2;

/** \ingroup gui
 * \class QgsCentroidFillSymbolLayerV2Widget
 */
class GUI_EXPORT QgsCentroidFillSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetCentroidFill
{
    Q_OBJECT

  public:
    QgsCentroidFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsCentroidFillSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  protected:
    QgsCentroidFillSymbolLayerV2* mLayer;

  private slots:
    void on_mDrawInsideCheckBox_stateChanged( int state );
    void on_mDrawAllPartsCheckBox_stateChanged( int state );

};


///@cond PRIVATE

class QgsSvgListModel : public QAbstractListModel
{
    Q_OBJECT

  public:
    explicit QgsSvgListModel( QObject* parent );

    // Constructor to create model for icons in a specific path
    QgsSvgListModel( QObject* parent, const QString& path );

    int rowCount( const QModelIndex & parent = QModelIndex() ) const override;

    QVariant data( const QModelIndex & index, int role = Qt::DisplayRole ) const override;

  protected:
    QStringList mSvgFiles;
};

class QgsSvgGroupsModel : public QStandardItemModel
{
    Q_OBJECT

  public:
    explicit QgsSvgGroupsModel( QObject* parent );

  private:
    void createTree( QStandardItem* &parentGroup );
};

///@endcond

#include "ui_qgsgeometrygeneratorwidgetbase.h"

class QgsGeometryGeneratorSymbolLayerV2;

/** \ingroup gui
 * \class QgsGeometryGeneratorSymbolLayerWidget
 */
class GUI_EXPORT QgsGeometryGeneratorSymbolLayerWidget : public QgsSymbolLayerV2Widget, private Ui::GeometryGeneratorWidgetBase
{
    Q_OBJECT

  public:
    QgsGeometryGeneratorSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent = nullptr );

    /**
     * Will be registered as factory
     */
    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsGeometryGeneratorSymbolLayerWidget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) override;
    virtual QgsSymbolLayerV2* symbolLayer() override;

  private:
    QgsGeometryGeneratorSymbolLayerV2* mLayer;

  private slots:
    void updateExpression();
    void updateSymbolType();
};

#endif
