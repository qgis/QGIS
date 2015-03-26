/***************************************************************************
    qgseffectstackpropertieswidget.h
    --------------------------------
    begin                : January 2015
    copyright            : (C) 2015 by Nyall Dawson
    email                : nyall dot dawson at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEFFECTSTACKPROPERTIESWIDGET_H
#define QGSEFFECTSTACKPROPERTIESWIDGET_H

#include "qgsdialog.h"
#include "effects/qgseffectstack.h"
#include <QWidget>
#include <QStandardItemModel>

#include "ui_qgseffectstackpropertieswidgetbase.h"

class EffectItem;
class QgsPaintEffect;
class QCheckBox;
class QToolButton;

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

class GUI_EXPORT QgsEffectStackPropertiesWidget : public QWidget, private Ui::QgsEffectStackPropertiesWidgetBase
{
    Q_OBJECT

  public:

    /** QgsEffectStackPropertiesWidget constructor
     * @param stack QgsEffectStack to modify in the widget
     * @param parent parent widget
     */
    QgsEffectStackPropertiesWidget( QgsEffectStack* stack, QWidget* parent = 0 );

    ~QgsEffectStackPropertiesWidget();

    /** Returns effect stack attached to the widget
     * @returns QgsEffectStack modified by the widget
     */
    QgsEffectStack* stack() { return mStack; }

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

    QgsEffectStack* mStack;
    QStandardItemModel* mModel;
    QWidget* mPresentWidget;
    QPicture* mPreviewPicture;

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
    EffectItem* currentEffectItem();

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

class GUI_EXPORT QgsEffectStackPropertiesDialog: public QgsDialog
{
    Q_OBJECT

  public:

    /** QgsEffectStackPropertiesDialog constructor
     * @param stack QgsEffectStack to modify in the dialog
     * @param parent parent widget
     * @param f window flags
     */
    QgsEffectStackPropertiesDialog( QgsEffectStack* stack, QWidget* parent = 0, Qt::WindowFlags f = 0 );
    ~QgsEffectStackPropertiesDialog();

    /** Returns effect stack attached to the dialog
     * @returns QgsEffectStack modified by the dialog
     */
    QgsEffectStack* stack();

    /** Sets the picture to use for effect previews for the dialog
     * @param picture preview picture
     */
    void setPreviewPicture( const QPicture& picture );

  protected:

    QgsEffectStackPropertiesWidget* mPropertiesWidget;

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

class GUI_EXPORT QgsEffectStackCompactWidget: public QWidget
{
    Q_OBJECT

  public:

    /** QgsEffectStackCompactWidget constructor
     * @param parent parent widget
     * @param effect QgsPaintEffect for modification by the widget. If the effect
     * is not a QgsEffectStack, it will be automatically converted to an effect
     * stack consisting of the original effect
     */
    QgsEffectStackCompactWidget( QWidget* parent = 0, QgsPaintEffect* effect = 0 );
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
    QgsPaintEffect* paintEffect() const { return mStack; }

    /** Sets the picture to use for effect previews for the dialog
     * @param picture preview picture
     */
    void setPreviewPicture( const QPicture &picture );

  signals:

    /** Emitted when the paint effect properties change
     */
    void changed();

  private slots:

    void showDialog();

    void enableToggled( bool checked );

  private:

    QgsEffectStack* mStack;
    QCheckBox* mEnabledCheckBox;
    QToolButton* mButton;
    QPicture* mPreviewPicture;

};

#endif //QGSEFFECTSTACKPROPERTIESWIDGET_H
