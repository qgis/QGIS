class QgsSymbolLayerV2Widget : QWidget
{
%TypeHeaderCode
#include <qgssymbollayerv2widget.h>
%End

  public:
    QgsSymbolLayerV2Widget( QWidget* parent /TransferThis/, const QgsVectorLayer* vl = 0 );
    virtual ~QgsSymbolLayerV2Widget();

    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) = 0;
    virtual QgsSymbolLayerV2* symbolLayer() = 0;

  protected:

    void registerDataDefinedButton( QgsDataDefinedButton * button, const QString & propertyName, QgsDataDefinedButton::DataType type, const QString & description );

    /** Get label for data defined entry.
     * Implemented only for 'size' of marker symbols
     * @note added in 2.1
     */
    virtual QString dataDefinedPropertyLabel( const QString &entryName );

  signals:
    void changed();

  protected slots:
    void updateDataDefinedProperty();
};

///////////

class QgsSimpleLineSymbolLayerV2Widget : QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgssymbollayerv2widget.h>
%End

  public:
    QgsSimpleLineSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) /Factory/;

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
    void on_mPenWidthUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mDashPatternUnitWidget_changed();
    void on_mDrawInsideCheckBox_stateChanged( int state );

  protected:
    //creates a new icon for the 'change pattern' button
    void updatePatternIcon();
};

///////////

class QgsSimpleMarkerSymbolLayerV2Widget : QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgssymbollayerv2widget.h>
%End

  public:
    QgsSimpleMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) /Factory/;

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
    void on_mSizeUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mOutlineWidthUnitWidget_changed();
    void on_mOutlineStyleComboBox_currentIndexChanged( int index );
    void on_mOutlineWidthSpinBox_valueChanged( double d );
    void on_mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void on_mVerticalAnchorComboBox_currentIndexChanged( int index );
};

///////////

class QgsSimpleFillSymbolLayerV2Widget : QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgssymbollayerv2widget.h>
%End

  public:
    QgsSimpleFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) /Factory/;

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
    void on_mBorderWidthUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
};


///////////

class QgsGradientFillSymbolLayerV2Widget : QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgssymbollayerv2widget.h>
%End
  public:
    QgsGradientFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) /Factory/;

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

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
};

///////////

class QgsShapeburstFillSymbolLayerV2Widget : QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgssymbollayerv2widget.h>
%End
  public:
    QgsShapeburstFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) /Factory/;

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

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
};

///////////

class QgsMarkerLineSymbolLayerV2Widget : QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgssymbollayerv2widget.h>
%End
  public:
    QgsMarkerLineSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) /Factory/;

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  public slots:

    void setInterval( double val );
    void setOffsetAlongLine( double val );
    void setRotate();
    void setOffset();
    void setPlacement();
    void on_mIntervalUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mOffsetAlongLineUnitWidget_changed();
};


///////////

class QgsSvgMarkerSymbolLayerV2Widget : QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgssymbollayerv2widget.h>
%End

  public:
    QgsSvgMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) /Factory/;

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
    void on_mSizeUnitWidget_changed();
    void on_mBorderWidthUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void on_mVerticalAnchorComboBox_currentIndexChanged( int index );

  protected:

    void populateList();
    //update gui for svg file (insert new path, update activation of gui elements for svg params)
    void setGuiForSvg( const QgsSvgMarkerSymbolLayerV2* layer );
};

///////////

class QgsRasterFillSymbolLayerWidget : QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgssymbollayerv2widget.h>
%End

  public:
    QgsRasterFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) /Factory/;

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

};

///////////

class QgsSVGFillSymbolLayerWidget : QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgssymbollayerv2widget.h>
%End

  public:
    QgsSVGFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) /Factory/;

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  protected:
    void insertIcons();
    /**Enables or disables svg fill color, border color and border width based on whether the
     * svg file supports custom parameters.
     * @param resetValues set to true to overwrite existing layer fill color, border color and border width
     * with default values from svg file
    */
    void updateParamGui( bool resetValues = true );
};

//////////

class QgsLinePatternFillSymbolLayerWidget : QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgssymbollayerv2widget.h>
%End
  public:

    QgsLinePatternFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = NULL );
    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) /Factory/;

    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();
};

//////////

class QgsPointPatternFillSymbolLayerWidget: QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgssymbollayerv2widget.h>
%End

  public:
    QgsPointPatternFillSymbolLayerWidget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = NULL );
    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) /Factory/;

    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();
};

/////////

class QgsFontMarkerSymbolLayerV2Widget : QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgssymbollayerv2widget.h>
%End

  public:
    QgsFontMarkerSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) /Factory/;

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
    void on_mSizeUnitWidget_changed();
    void on_mOffsetUnitWidget_changed();
    void on_mHorizontalAnchorComboBox_currentIndexChanged( int index );
    void on_mVerticalAnchorComboBox_currentIndexChanged( int index );
};

//////////

class QgsCentroidFillSymbolLayerV2Widget : QgsSymbolLayerV2Widget
{
%TypeHeaderCode
#include <qgssymbollayerv2widget.h>
%End

  public:
    QgsCentroidFillSymbolLayerV2Widget( const QgsVectorLayer* vl, QWidget* parent /TransferThis/ = NULL );

    static QgsSymbolLayerV2Widget* create( const QgsVectorLayer* vl ) /Factory/;

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();
};
