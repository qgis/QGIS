/** \ingroup core
 * Holder for the widget type and its configuration for a field.
 */
class QgsEditorWidgetSetup
{
%TypeHeaderCode
#include <qgseditorwidgetsetup.h>
%End
  public:
    /**
     * Constructor
     */
    QgsEditorWidgetSetup( const QString& type, const QgsEditorWidgetConfig& config );
    QgsEditorWidgetSetup();

    /**
     * @return the widget type to use
     */
    QString type() const;

    /**
     * @return the widget configuration to used
     */
    QgsEditorWidgetConfig config() const;

    /**
     * @return true if there is no widget configured.
     */
    bool isNull() const;
};


