/* \brief create attribute widget for editing */
class QgsAttributeEditor : public QObject
{
%TypeHeaderCode
#incude <qgsattributeeditor.h>
%End
  public:
    QgsAttributeEditor( QObject *parent );
    static QWidget *createAttributeEditor( QWidget *parent, QWidget *editor, QgsVectorLayer *vl, int idx, const QVariant &value );
    static bool retrieveValue( QWidget *widget, QgsVectorLayer *vl, int idx, QVariant &value );
    static bool setValue( QWidget *widget, QgsVectorLayer *vl, int idx, const QVariant &value );

  public slots:
    void selectFileName( void );
    void selectDate( void );
};
