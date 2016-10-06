/** \ingroup gui
 * \class QgsPaintEffectPropertiesWidget
 * \brief A widget which modifies the properties of a QgsPaintEffect
 *
 * \note Added in version 2.9
 */

class QgsPaintEffectPropertiesWidget : QWidget
{
%TypeHeaderCode
#include <qgspainteffectpropertieswidget.h>
%End

  public:

    /** QgsPaintEffectPropertiesWidget constructor
     * @param effect QgsPaintEffect to modify in the widget
     * @param parent parent widget
     */
    QgsPaintEffectPropertiesWidget( QgsPaintEffect* effect, QWidget* parent /TransferThis/ = NULL );

  public slots:

    /** Update widget when effect type changes
     */
    void effectTypeChanged();

    /** Emits the changed signal
     */
    void emitSignalChanged();

  signals:

    /** Emitted when paint effect properties changes
     */
    void changed();

    /** Emitted when paint effect type changes
     */
    void changeEffect( QgsPaintEffect* effect );

};

