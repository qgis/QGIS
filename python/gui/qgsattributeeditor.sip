
// \brief create attribute widget for editing
class QgsAttributeEditor : QObject
{
%TypeHeaderCode
#include <qgsattributeeditor.h>
%End

  public:
    QgsAttributeEditor( QObject *parent );
    static QWidget *createAttributeEditor( QWidget *parent, QWidget *editor, QgsVectorLayer *vl, int idx, const QVariant &value ) /Factory/;
    static bool retrieveValue( QWidget *widget, QgsVectorLayer *vl, int idx, QVariant &value );
    static bool setValue( QWidget *widget, QgsVectorLayer *vl, int idx, const QVariant &value );

  public slots:
    void selectFileName();
    void selectDate();
    void loadUrl( const QString & );
    void loadPixmap( const QString & );
    void updateUrl();
    void openUrl();
    void updateColor();
};

class QgsStringRelay : QObject
{
%TypeHeaderCode
#include "qgsattributeeditor.h"
%End
  public:
    QgsStringRelay( QObject* parent = 0 );

    void appendProxy( QWidget* proxy );

  public slots:
    void changeText();
    void changeText( QString str );

  signals:
    void textChanged( QString );
};
