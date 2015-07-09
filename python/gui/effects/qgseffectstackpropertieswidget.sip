/** \ingroup gui
 * \class QgsEffectStackPropertiesWidget
 * \brief A widget for modifying the properties of a QgsEffectStack, including adding
 * and reordering effects within the stack.
 *
 * \note Added in version 2.9
 * \see QgsEffectStack
 * \see QgsEffectStackPropertiesDialog
 * \see QgsEffectStackCompactWidget
 */

class QgsEffectStackPropertiesWidget : QWidget
{
%TypeHeaderCode
#include <qgseffectstackpropertieswidget.h>
%End

  public:

    /** QgsEffectStackPropertiesWidget constructor
     * @param stack QgsEffectStack to modify in the widget
     * @param parent parent widget
     */
    QgsEffectStackPropertiesWidget( QgsEffectStack* stack, QWidget* parent /TransferThis/ = 0 );

    ~QgsEffectStackPropertiesWidget();

    /** Returns effect stack attached to the widget
     * @returns QgsEffectStack modified by the widget
     */
    QgsEffectStack* stack();

    /** Sets the picture to use for effect previews for the dialog
     * @param picture preview picture
     */
    void setPreviewPicture( const QPicture& picture );

  public slots:

    /** Moves the currently selected effect down in the stack.
     */
    void moveEffectDown();

    /** Moves the currently selected effect up in the stack.
     */
    void moveEffectUp();

    /** Adds a new effect to the stack.
     */
    void addEffect();

    /** Removes the currently selected effect from the stack.
     */
    void removeEffect();

    /** Updates the widget when the selected effect changes type.
     */
    void effectChanged();

    /** Updates the effect preview icon.
     */
    void updatePreview();

    /** Updates the effect stack when the currently selected effect changes properties.
     * @param newEffect new effect to replace existing effect at selected position within the stack.
     */
    void changeEffect( QgsPaintEffect* newEffect );

  protected:

    /** Refreshes the widget to reflect the current state of the stack.
     */
    void loadStack();

    /** Refreshes the widget to reflect the current state of a specified stack.
     * @param stack QgsEffectStack for widget
     */
    void loadStack( QgsEffectStack* stack );

    /** Enables or disables widgets depending on the selected effect within the stack.
     */
    void updateUi();

    /** Returns the currently selected effect within the stack.
     * @note not available in python bindings
     */
    //EffectItem* currentEffectItem();

    /** Moves the currently selected effect within the stack by a specified offset
     */
    void moveEffectByOffset( int offset );

    /** Sets the effect properties widget
     */
    void setWidget( QWidget* widget );

};


/** \ingroup gui
 * \class QgsEffectStackPropertiesDialog
 * \brief A dialog for modifying the properties of a QgsEffectStack, including adding
 * and reordering effects within the stack.
 *
 * \note Added in version 2.9
 * \see QgsEffectStack
 * \see QgsEffectStackPropertiesWidget
 * \see QgsEffectStackCompactWidget
 */

class QgsEffectStackPropertiesDialog : QgsDialog
{
%TypeHeaderCode
#include <qgseffectstackpropertieswidget.h>
%End

  public:

    /** QgsEffectStackPropertiesDialog constructor
     * @param stack QgsEffectStack to modify in the dialog
     * @param parent parent widget
     * @param f window flags
     */
    QgsEffectStackPropertiesDialog( QgsEffectStack* stack, QWidget* parent /TransferThis/ = 0, Qt::WindowFlags f = 0 );
    ~QgsEffectStackPropertiesDialog();

    /** Returns effect stack attached to the dialog
     * @returns QgsEffectStack modified by the dialog
     */
    QgsEffectStack* stack();

    /** Sets the picture to use for effect previews for the dialog
     * @param picture preview picture
     */
    void setPreviewPicture( const QPicture& picture );

};

/** \ingroup gui
 * \class QgsEffectStackCompactWidget
 * \brief A small widget consisting of a check box for enabling/disabling an effect stack
 * and a button for opening an effect stack customisation dialog.
 *
 * \note Added in version 2.9
 * \see QgsEffectStack
 * \see QgsEffectStackPropertiesWidget
 * \see QgsEffectStackPropertiesDialog
 */

class QgsEffectStackCompactWidget : QWidget
{
%TypeHeaderCode
#include <qgseffectstackpropertieswidget.h>
%End
  public:

    /** QgsEffectStackCompactWidget constructor
     * @param parent parent widget
     * @param effect QgsPaintEffect for modification by the widget. If the effect
     * is not a QgsEffectStack, it will be automatically converted to an effect
     * stack consisting of the original effect
     */
    QgsEffectStackCompactWidget( QWidget* parent /TransferThis/ = 0, QgsPaintEffect* effect = 0 );
    ~QgsEffectStackCompactWidget();

    /** Sets paint effect attached to the widget
     * @param effect QgsPaintEffect for modification by the widget. If the effect
     * is not a QgsEffectStack, it will be automatically converted to an effect
     * stack consisting of the original effect
     * @see paintEffect
     */
    void setPaintEffect( QgsPaintEffect* effect );

    /** Returns paint effect attached to the widget
     * @returns QgsPaintEffect modified by the widget
     * @see setPaintEffect
     */
    QgsPaintEffect* paintEffect() const;

    /** Sets the picture to use for effect previews for the dialog
     * @param picture preview picture
     */
    void setPreviewPicture( const QPicture& picture );

  signals:

    /** Emitted when the paint effect properties change
     */
    void changed();

};
