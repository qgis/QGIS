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

  signals:
    void changed();
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
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  public slots:
    void penWidthChanged();
    void colorChanged( const QColor& color );
    void penStyleChanged();
    void offsetChanged();
    void on_mCustomCheckBox_stateChanged( int state );
    void on_mChangePatternButton_clicked();
    void on_mPenWidthUnitComboBox_currentIndexChanged( int index );
    void on_mOffsetUnitComboBox_currentIndexChanged( int index );
    void on_mDashPatternUnitComboBox_currentIndexChanged( int index );
    void on_mDataDefinedPropertiesButton_clicked();

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

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsSimpleMarkerSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  public slots:
    void setName();
    void setColorBorder( const QColor& color );
    void setColorFill( const QColor& color );
    void setSize();
    void setAngle();
    void setOffset();
    void on_mSizeUnitComboBox_currentIndexChanged( int index );
    void on_mOffsetUnitComboBox_currentIndexChanged( int index );
    void on_mOutlineWidthUnitComboBox_currentIndexChanged( int index );
    void on_mDataDefinedPropertiesButton_clicked();
    void on_mOutlineWidthSpinBox_valueChanged( double d );

  protected:
    QgsSimpleMarkerSymbolLayerV2* mLayer;
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
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  public slots:
    void setColor( const QColor& color );
    void setBorderColor( const QColor& color );
    void setBrushStyle();
    void borderWidthChanged();
    void borderStyleChanged();
    void offsetChanged();
    void on_mBorderWidthUnitComboBox_currentIndexChanged( int index );
    void on_mOffsetUnitComboBox_currentIndexChanged( int index );
    void on_mDataDefinedPropertiesButton_clicked();

  protected:
    QgsSimpleFillSymbolLayerV2* mLayer;
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
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  public slots:

    void setInterval( double val );
    void setRotate();
    void setOffset();
    void setPlacement();
    void on_mIntervalUnitComboBox_currentIndexChanged( int index );
    void on_mOffsetUnitComboBox_currentIndexChanged( int index );
    void on_mDataDefinedPropertiesButton_clicked();

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

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsSvgMarkerSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

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
    void on_mSizeUnitComboBox_currentIndexChanged( int index );
    void on_mBorderWidthUnitComboBox_currentIndexChanged( int index );
    void on_mOffsetUnitComboBox_currentIndexChanged( int index );
    void on_mDataDefinedPropertiesButton_clicked();

  protected:

    void populateList();
    //update gui for svg file (insert new path, update activation of gui elements for svg params)
    void setGuiForSvg( const QgsSvgMarkerSymbolLayerV2* layer );

    QgsSvgMarkerSymbolLayerV2* mLayer;
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
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  protected:
    QgsSVGFillSymbolLayer* mLayer;
    void insertIcons();
    void updateParamGui();

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
    void on_mTextureWidthUnitComboBox_currentIndexChanged( int index );
    void on_mSvgOutlineWidthUnitComboBox_currentIndexChanged( int index );
    void on_mDataDefinedPropertiesButton_clicked();
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

    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  protected:
    QgsLinePatternFillSymbolLayer* mLayer;

  private slots:
    void on_mAngleSpinBox_valueChanged( double d );
    void on_mDistanceSpinBox_valueChanged( double d );
    void on_mLineWidthSpinBox_valueChanged( double d );
    void on_mOffsetSpinBox_valueChanged( double d );
    void on_mColorPushButton_colorChanged( const QColor& color );
    void on_mDistanceUnitComboBox_currentIndexChanged( int index );
    void on_mLineWidthUnitComboBox_currentIndexChanged( int index );
    void on_mOffsetUnitComboBox_currentIndexChanged( int index );
    void on_mDataDefinedPropertiesButton_clicked();
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

    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  protected:
    QgsPointPatternFillSymbolLayer* mLayer;

  private slots:
    void on_mHorizontalDistanceSpinBox_valueChanged( double d );
    void on_mVerticalDistanceSpinBox_valueChanged( double d );
    void on_mHorizontalDisplacementSpinBox_valueChanged( double d );
    void on_mVerticalDisplacementSpinBox_valueChanged( double d );
    void on_mHorizontalDistanceUnitComboBox_currentIndexChanged( int index );
    void on_mVerticalDistanceUnitComboBox_currentIndexChanged( int index );
    void on_mHorizontalDisplacementUnitComboBox_currentIndexChanged( int index );
    void on_mVerticalDisplacementUnitComboBox_currentIndexChanged( int index );
    void on_mDataDefinedPropertiesButton_clicked();
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

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) { return new QgsFontMarkerSymbolLayerV2Widget( vl ); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  public slots:
    void setFontFamily( const QFont& font );
    void setColor( const QColor& color );
    void setSize( double size );
    void setAngle( double angle );
    void setCharacter( const QChar& chr );
    void setOffset();
    void on_mSizeUnitComboBox_currentIndexChanged( int index );
    void on_mOffsetUnitComboBox_currentIndexChanged( int index );

  protected:
    QgsFontMarkerSymbolLayerV2* mLayer;
    CharacterWidget* widgetChar;
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
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  protected:
    QgsCentroidFillSymbolLayerV2* mLayer;
};


#endif
