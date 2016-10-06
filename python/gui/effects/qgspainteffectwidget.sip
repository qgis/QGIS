/** \ingroup gui
 * \class QgsPaintEffectWidget
 * \brief Base class for effect properties widgets.
 *
 * \note Added in version 2.9
 */

class QgsPaintEffectWidget : QWidget
{
%TypeHeaderCode
#include <qgspainteffectwidget.h>
%End

  public:
    QgsPaintEffectWidget( QWidget* parent /TransferThis/ = 0 );
    virtual ~QgsPaintEffectWidget();

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


class QgsDrawSourceWidget : QgsPaintEffectWidget
{
%TypeHeaderCode
#include <qgspainteffectwidget.h>
%End

  public:
    QgsDrawSourceWidget( QWidget* parent /TransferThis/ = NULL );

    static QgsPaintEffectWidget* create() /Factory/;

    virtual void setPaintEffect( QgsPaintEffect* effect );

};

class QgsBlurWidget : QgsPaintEffectWidget
{
%TypeHeaderCode
#include <qgspainteffectwidget.h>
%End

  public:
    QgsBlurWidget( QWidget* parent /TransferThis/ = NULL );

    static QgsPaintEffectWidget* create() /Factory/;

    virtual void setPaintEffect( QgsPaintEffect* effect );

};

class QgsShadowEffectWidget : QgsPaintEffectWidget
{
%TypeHeaderCode
#include <qgspainteffectwidget.h>
%End

  public:
    QgsShadowEffectWidget( QWidget* parent /TransferThis/ = NULL );

    static QgsPaintEffectWidget* create() /Factory/;

    virtual void setPaintEffect( QgsPaintEffect* effect );

};

class QgsGlowWidget : QgsPaintEffectWidget
{
%TypeHeaderCode
#include <qgspainteffectwidget.h>
%End

  public:
    QgsGlowWidget( QWidget* parent /TransferThis/ = NULL );

    static QgsPaintEffectWidget* create() /Factory/;

    virtual void setPaintEffect( QgsPaintEffect* effect );
};


class QgsTransformWidget : QgsPaintEffectWidget
{
%TypeHeaderCode
#include <qgspainteffectwidget.h>
%End

  public:
    QgsTransformWidget( QWidget* parent /TransferThis/ = NULL );

    static QgsPaintEffectWidget* create() /Factory/;

    virtual void setPaintEffect( QgsPaintEffect* effect );

};

class QgsColorEffectWidget : QgsPaintEffectWidget
{
%TypeHeaderCode
#include <qgspainteffectwidget.h>
%End

  public:
    QgsColorEffectWidget( QWidget* parent /TransferThis/ = NULL );

    static QgsPaintEffectWidget* create() /Factory/;

    virtual void setPaintEffect( QgsPaintEffect* effect );

};


