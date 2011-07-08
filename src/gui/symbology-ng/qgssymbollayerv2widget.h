
#ifndef QGSSYMBOLLAYERV2WIDGET_H
#define QGSSYMBOLLAYERV2WIDGET_H

#include <QWidget>

class QgsSymbolLayerV2;


class GUI_EXPORT QgsSymbolLayerV2Widget : public QWidget
{
    Q_OBJECT

  public:
    QgsSymbolLayerV2Widget( QWidget* parent ) : QWidget( parent ) {}
    virtual ~QgsSymbolLayerV2Widget() {}

    virtual void setSymbolLayer( QgsSymbolLayerV2* layer ) = 0;
    virtual QgsSymbolLayerV2* symbolLayer() = 0;

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
    QgsSimpleLineSymbolLayerV2Widget( QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create() { return new QgsSimpleLineSymbolLayerV2Widget(); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  public slots:
    void penWidthChanged();
    void colorChanged();
    void penStyleChanged();
    void offsetChanged();
    void on_mCustomCheckBox_stateChanged( int state );
    void on_mChangePatternButton_clicked();


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
    QgsSimpleMarkerSymbolLayerV2Widget( QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create() { return new QgsSimpleMarkerSymbolLayerV2Widget(); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  public slots:
    void setName();
    void setColorBorder();
    void setColorFill();
    void setSize();
    void setAngle();
    void setOffset();

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
    QgsSimpleFillSymbolLayerV2Widget( QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create() { return new QgsSimpleFillSymbolLayerV2Widget(); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  public slots:
    void setColor();
    void setBorderColor();
    void setBrushStyle();
    void borderWidthChanged();
    void borderStyleChanged();
    void offsetChanged();

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
    QgsMarkerLineSymbolLayerV2Widget( QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create() { return new QgsMarkerLineSymbolLayerV2Widget(); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  public slots:

    void setInterval( double val );
    void setMarker();
    void setRotate();
    void setOffset();
    void setPlacement();

  protected:

    void updateMarker();

    QgsMarkerLineSymbolLayerV2* mLayer;
};


///////////

#include "ui_widget_svgmarker.h"

class QgsSvgMarkerSymbolLayerV2;

class GUI_EXPORT QgsSvgMarkerSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetSvgMarker
{
    Q_OBJECT

  public:
    QgsSvgMarkerSymbolLayerV2Widget( QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create() { return new QgsSvgMarkerSymbolLayerV2Widget(); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  public slots:
    void setName( const QModelIndex& idx );
    void setSize();
    void setAngle();
    void setOffset();
    void on_mFileToolButton_clicked();
    void on_mFileLineEdit_textEdited( const QString& text );
    void on_mChangeColorButton_clicked();
    void on_mChangeBorderColorButton_clicked();
    void on_mBorderWidthSpinBox_valueChanged( double d );

  protected:

    void populateList();
    //update gui for svg file (insert new path, update activation of gui elements for svg params)
    void setGuiForSvg( const QgsSvgMarkerSymbolLayerV2* layer );

    QgsSvgMarkerSymbolLayerV2* mLayer;
};


///////////

#include "ui_widget_linedecoration.h"

class QgsLineDecorationSymbolLayerV2;

class GUI_EXPORT QgsLineDecorationSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetLineDecoration
{
    Q_OBJECT

  public:
    QgsLineDecorationSymbolLayerV2Widget( QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create() { return new QgsLineDecorationSymbolLayerV2Widget(); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  public slots:
    void colorChanged();
    void penWidthChanged();

  protected:
    QgsLineDecorationSymbolLayerV2* mLayer;
};

//////////

#include "ui_widget_svgfill.h"

class QgsSVGFillSymbolLayer;

class GUI_EXPORT QgsSVGFillSymbolLayerWidget : public QgsSymbolLayerV2Widget, private Ui::WidgetSVGFill
{
    Q_OBJECT

  public:
    QgsSVGFillSymbolLayerWidget( QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create() { return new QgsSVGFillSymbolLayerWidget(); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  protected:
    QgsSVGFillSymbolLayer* mLayer;
    //sets new output unit. Is called on combo box or spin box change
    void setOutputUnit();
    void insertIcons();
    void updateOutlineIcon();

  private slots:
    void on_mBrowseToolButton_clicked();
    void on_mTextureWidthSpinBox_valueChanged( double d );
    void on_mSVGLineEdit_textChanged( const QString & text );
    void setFile( const QModelIndex& item );
    void on_mChangeOutlinePushButton_clicked();
    void on_mRotationSpinBox_valueChanged( double d );
};

//////////

#include "ui_widget_linepatternfill.h"

class QgsLinePatternFillSymbolLayer;

class GUI_EXPORT QgsLinePatternFillSymbolLayerWidget : public QgsSymbolLayerV2Widget, private Ui::WidgetLinePatternFill
{
  Q_OBJECT

  public:

    QgsLinePatternFillSymbolLayerWidget( QWidget* parent = NULL );
    static QgsSymbolLayerV2Widget* create() { return new QgsLinePatternFillSymbolLayerWidget(); }

    virtual void setSymbolLayer( QgsSymbolLayerV2* layer);
    virtual QgsSymbolLayerV2* symbolLayer();

  protected:
    QgsLinePatternFillSymbolLayer* mLayer;

  private slots:
    void on_mAngleSpinBox_valueChanged( double d );
    void on_mDistanceSpinBox_valueChanged( double d );
    void on_mLineWidthSpinBox_valueChanged( double d );
    void on_mColorPushButton_clicked();
    void on_mOutlinePushButton_clicked();
};

//////////


#include "ui_widget_fontmarker.h"

class QgsFontMarkerSymbolLayerV2;
class CharacterWidget;

class GUI_EXPORT QgsFontMarkerSymbolLayerV2Widget : public QgsSymbolLayerV2Widget, private Ui::WidgetFontMarker
{
    Q_OBJECT

  public:
    QgsFontMarkerSymbolLayerV2Widget( QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create() { return new QgsFontMarkerSymbolLayerV2Widget(); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  public slots:
    void setFontFamily( const QFont& font );
    void setColor();
    void setSize( double size );
    void setAngle( double angle );
    void setCharacter( const QChar& chr );
    void setOffset();

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
    QgsCentroidFillSymbolLayerV2Widget( QWidget* parent = NULL );

    static QgsSymbolLayerV2Widget* create() { return new QgsCentroidFillSymbolLayerV2Widget(); }

    // from base class
    virtual void setSymbolLayer( QgsSymbolLayerV2* layer );
    virtual QgsSymbolLayerV2* symbolLayer();

  public slots:
    void setMarker();

  protected:
    void updateMarker();

    QgsCentroidFillSymbolLayerV2* mLayer;
};


#endif
