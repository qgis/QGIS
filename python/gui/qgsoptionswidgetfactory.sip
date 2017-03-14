class QgsOptionsPageWidget : QWidget
{
%TypeHeaderCode
#include <qgsoptionswidgetfactory.h>
%End

  public:

    QgsOptionsPageWidget( QWidget *parent /TransferThis/ = 0 );

  public slots:

    virtual void apply() = 0;

};

class QgsOptionsWidgetFactory
{
%TypeHeaderCode
#include <qgsoptionswidgetfactory.h>
%End

  public:

    QgsOptionsWidgetFactory();

    QgsOptionsWidgetFactory( const QString &title, const QIcon &icon );
    virtual ~QgsOptionsWidgetFactory();

    virtual QIcon icon() const;
    void setIcon( const QIcon &icon );
    virtual QString title() const;
    void setTitle( const QString &title );

    virtual QgsOptionsPageWidget *createWidget( QWidget *parent = 0 ) const = 0 /Factory/;


};

