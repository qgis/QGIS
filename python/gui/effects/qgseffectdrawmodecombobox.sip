
/** \ingroup gui
 * \class QgsEffectDrawModeComboBox
 * \brief A combo box allowing selection of paint effect draw modes
 *
 * \note Added in version 2.9
 */

class QgsEffectDrawModeComboBox : QComboBox
{
%TypeHeaderCode
#include <qgseffectdrawmodecombobox.h>
%End

  public:

    QgsEffectDrawModeComboBox( QWidget* parent /TransferThis/ = NULL );

    /** Returns the currently selected draw mode for the combo box
     * @returns current draw mode
     */
    QgsPaintEffect::DrawMode drawMode() const;

    /** Sets the currently selected draw mode for the combo box
     * @param drawMode selected draw mode
     */
    void setDrawMode( QgsPaintEffect::DrawMode drawMode );

};

