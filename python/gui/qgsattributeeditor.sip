
// \brief create attribute widget for editing
class QgsAttributeEditor : QObject
{
%TypeHeaderCode
#include <qgsattributeeditor.h>
%End

  public:
    QgsAttributeEditor( QObject *parent /TransferThis/ );

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

    static bool retrieveValue( QWidget *widget, QgsVectorLayer *vl, int idx, QVariant &value );
    static bool setValue( QWidget *widget, QgsVectorLayer *vl, int idx, const QVariant &value );
};
