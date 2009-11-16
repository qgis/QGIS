
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

  protected:
    QgsSimpleLineSymbolLayerV2* mLayer;
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

  protected:

    void populateList();

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

  protected:
    QgsLineDecorationSymbolLayerV2* mLayer;
};


#endif
