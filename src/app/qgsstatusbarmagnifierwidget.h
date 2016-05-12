/***************************************************************************
                         qgsstatusbarmagnifierwidget.h
    begin                : April 2016
    copyright            : (C) 2016 Paul Blottiere, Oslandia
    email                : paul dot blottiere at oslandia dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTATUSBARMAGNIFIERWIDGET_H
#define QGSSTATUSBARMAGNIFIERWIDGET_H

class QLabel;
class QFont;
class QHBoxLayout;
class QgsMapCanvas;
class QgsDoubleSpinBox;

#include <QWidget>

/**
  * A widget which lets the user select the current level of magnification to
  * apply to the canvas.
  * @note added in 2.16
  */
class APP_EXPORT QgsStatusBarMagnifierWidget : public QWidget
{
    Q_OBJECT

  public:

    /** Constructor
      * @param parent is the parent widget
      * @param canvas the map canvas
      */
    QgsStatusBarMagnifierWidget( QWidget* parent, QgsMapCanvas *canvas );

    /** Destructor */
    virtual ~QgsStatusBarMagnifierWidget();

    /** Set the font of the text
      * @param font the font to use
      */
    void setFont( const QFont& font );

    /** Returns the current magnification level
      * @return magnification level
      */
    double magnificationLevel();

    /** Set the magnification level
      * @param level the magnification level
      * @return true if the level is valid, false otherwise
      */
    bool setMagnificationLevel( int level );

  private slots:

    void updateMagnifier();

  private:
    QgsMapCanvas *mCanvas;
    QHBoxLayout *mLayout;
    QLabel *mLabel;
    QgsDoubleSpinBox *mSpinBox;
    int mMagnifier;
    int mMagnifierMin;
    int mMagnifierMax;
};

#endif
