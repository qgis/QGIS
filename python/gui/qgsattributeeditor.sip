
// \brief create attribute widget for editing
class QgsAttributeEditor : QObject
{
%TypeHeaderCode
#include <qgsattributeeditor.h>
%End

  public:
    QgsAttributeEditor( QObject *parent );

    /**
     * Creates or prepares a attribute editor widget
     * @param parent The parent object
     * @param editor The widget to prepare. Set to null if it should be generated
     * @param vl The vector layer to use as data source
     * @param idx The field index this widget refers to
     * @param value the value to initiate this widget with
     *
     */
    static QWidget* createAttributeEditor( QWidget* parent, QWidget* editor, QgsVectorLayer* vl, int idx, const QVariant& value ) /Factory/;
    /**
     * Creates or prepares a attribute editor widget
     * @param parent The parent object
     * @param editor The widget to prepare. Set to null if it should be generated
     * @param vl The vector layer to use as data source
     * @param idx The field index this widget refers to
     * @param value the value to initiate this widget with
     * @param context the context used for the created attribute editor
     *
     */
    static QWidget* createAttributeEditor( QWidget* parent, QWidget* editor, QgsVectorLayer* vl, int idx, const QVariant& value, QgsAttributeEditorContext& context ) /Factory/;

    /**
     * Creates a widget form a QgsAttributeEditorElement definition. Will recursively generate containers and widgets.
     * @param widgetDef The definition for the widget
     * @param parent The parent object
     * @param vl The vector layer to use as data source
     * @param feat The feature to create the widget for
     * @param context the context used for the created attribute editor
     * @param [out] labelText An optional label text will be written into the referenced QString. It will be set to
     *        a QString::null value if no label should be shown
     * @param [out] labelOnTop Will be set to true if the label should be placed on top of the field.
     *        If set to false, the label should be shown left or right of the field
     *
     */
    static QWidget *createWidgetFromDef( const QgsAttributeEditorElement* widgetDef, QWidget* parent, QgsVectorLayer* vl, const QgsFeature &feat, QgsAttributeEditorContext& context, QString& labelText, bool& labelOnTop ) /Factory/;

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
