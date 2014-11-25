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

class QgsStyleV2;
class QgsVectorColorRampV2;

class GUI_EXPORT QgsColorRampComboBox : public QComboBox
{
    Q_OBJECT
  public:
    explicit QgsColorRampComboBox( QWidget *parent = 0 );

    ~QgsColorRampComboBox();

    //! initialize the combo box with color ramps from the style
    void populate( QgsStyleV2* style );

    //! add/select color ramp which was used previously by the renderer
    void setSourceColorRamp( QgsVectorColorRampV2* sourceRamp );

    //! return new instance of the current color ramp or NULL if there is no active color ramp
    QgsVectorColorRampV2* currentColorRamp();

    /**Returns true if the current selection in the combo box is the option for creating
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

  protected:
    QgsStyleV2* mStyle;
    QgsVectorColorRampV2* mSourceColorRamp; // owns the copy

  private:
    bool mShowGradientOnly;

};

#endif // QGSCOLORRAMPCOMBOBOX_H
