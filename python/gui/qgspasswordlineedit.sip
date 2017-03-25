/** \class QgsPasswordLineEdit
 * \ingroup gui
 * QLineEdit subclass with built in support for showing/hiding
 * entered password.
 * @note added in QGIS 3.0
 **/
class QgsPasswordLineEdit : QLineEdit
{
%TypeHeaderCode
#include <qgspasswordlineedit.h>
%End

  public:

    /** Constructor for QgsPasswordLineEdit.
     * @param parent parent widget
     */
    QgsPasswordLineEdit( QWidget *parent = nullptr );

    /** Define if a lock icon shall be shown on the left of the widget
     * @param visible set to false to hide the lock icon
     */
    void setShowLockIcon( bool visible );

    /** Returns if a lock icon shall be shown on the left of the widget
     */
    bool showLockIcon() const;

    /** Set state of the password's visibility
     */
    void setPasswordVisibility( bool visible );
};
