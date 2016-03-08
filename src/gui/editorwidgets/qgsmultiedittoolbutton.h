/***************************************************************************
     qgsmultiedittoolbutton.h
     ------------------------
    Date                 : March 2016
    Copyright            : (C) 2016 Nyall Dawson
    Email                : nyall dot dawson at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMULTIEDITTOOLBUTTON_H
#define QGSMULTIEDITTOOLBUTTON_H

#include "qgsfield.h"
#include <QToolButton>

/** \ingroup gui
 * \class QgsMultiEditToolButton
 * A tool button widget which is displayed next to editor widgets in attribute forms, and
 * allows for controlling how the widget behaves and interacts with the form while in multi
 * edit mode.
 * \note Added in version 2.16
 */
class GUI_EXPORT QgsMultiEditToolButton : public QToolButton
{
    Q_OBJECT

  public:

    //! Button states
    enum State
    {
      Default, /*!< Default state, all features have same value for widget */
      MixedValues, /*!< Mixed state, some features have different values for the widget */
      Changed, /*!< Value for widget has changed but changes have not yet been committed */
    };

    /** Constructor for QgsMultiEditToolButton.
     * @param parent parent object
     */
    explicit QgsMultiEditToolButton( QWidget *parent = nullptr );

    /** Returns the current displayed state of the button.
     */
    State state() const { return mState; }

    /** Sets the field associated with this button. This is used to customise the widget menu
     * and tooltips to match the field properties.
     * @param field associated field
     */
    void setField( const QgsField& field ) { mField = field; }

  public slots:

    /** Sets whether the associated field contains mixed values.
     * @param mixed whether field values are mixed
     * @see isMixed()
     * @see setIsChanged()
     * @see resetChanges()
     */
    void setIsMixed( bool mixed ) { mIsMixedValues = mixed; updateState(); }

    /** Sets whether the associated field has changed.
     * @param changed whether field has changed
     * @see isChanged()
     * @see setIsMixed()
     * @see resetChanges()
     */
    void setIsChanged( bool changed ) { mIsChanged = changed; updateState(); }

    /** Resets the changed state for the field.
     * @see setIsMixed()
     * @see setIsChanged()
     * @see changesCommitted()
     */
    void resetChanges() { mIsChanged = false; updateState(); }

    /** Called when field values have been changed and field now contains all the same values.
     * @see resetChanges()
     */
    void changesCommitted() { mIsMixedValues = false; mIsChanged = false; updateState(); }

  signals:

    //! Emitted when the "set field value for all features" option is selected.
    void setFieldValueTriggered();

    //! Emitted when the "reset to original values" option is selected.
    void resetFieldValueTriggered();

  private slots:

    void aboutToShowMenu();
    void setFieldTriggered();
    void resetFieldTriggered();

  private:

    bool mIsMixedValues;
    bool mIsChanged;
    State mState;
    QgsField mField;

    QMenu* mMenu;

    void updateState();

};

#endif // QGSMULTIEDITTOOLBUTTON_H
