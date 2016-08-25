/***************************************************************************
    qgscolorrampcombobox.h
    ---------------------
    begin                : October 2010
    copyright            : (C) 2010 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSCOLORRAMPCOMBOBOX_H
#define QGSCOLORRAMPCOMBOBOX_H

#include <QComboBox>

class QgsStyle;
class QgsColorRamp;

/** \ingroup gui
 * \class QgsColorRampComboBox
 */
class GUI_EXPORT QgsColorRampComboBox : public QComboBox
{
    Q_OBJECT
  public:
    explicit QgsColorRampComboBox( QWidget *parent = nullptr );

    ~QgsColorRampComboBox();

    //! initialize the combo box with color ramps from the style
    void populate( QgsStyle* style );

    /** Adds or selects the current color ramp to show in the combo box. The ramp appears
     * in the combo box as the "source" ramp.
     * @param sourceRamp color ramp, ownership is transferred.
     * @see currentColorRamp()
     */
    void setSourceColorRamp( QgsColorRamp* sourceRamp );

    /** Returns a new instance of the current color ramp or NULL if there is no active color ramp.
     * The caller takes responsibility for deleting the returned value.
     * @see setSourceColorRamp()
     */
    QgsColorRamp* currentColorRamp() const;

    /** Returns true if the current selection in the combo box is the option for creating
     * a new color ramp
     * @note added in QGIS 2.7
     */
    bool createNewColorRampSelected() const;

    //! @note not available in python bindings
    static QSize rampIconSize;

    //! @note added in 2.2
    void setShowGradientOnly( bool gradientOnly ) { mShowGradientOnly = gradientOnly; }
    //! @note added in 2.2
    bool showGradientOnly() const { return mShowGradientOnly; }

  public slots:
    void colorRampChanged( int index );

    /** Triggers a dialog which allows users to edit the current source
     * ramp for the combo box.
     * @see sourceRampEdited
     * @note added in QGIS 2.12
     */
    void editSourceRamp();

  signals:

    /** Emitted when the user has edited the current source ramp.
     * @see editSourceRamp
     * @note added in QGIS 2.12
     */
    void sourceRampEdited();

  protected:
    QgsStyle* mStyle;
    QgsColorRamp* mSourceColorRamp; // owns the copy

  private slots:

    void rampWidgetUpdated();

  private:
    bool mShowGradientOnly;

};

#endif // QGSCOLORRAMPCOMBOBOX_H
