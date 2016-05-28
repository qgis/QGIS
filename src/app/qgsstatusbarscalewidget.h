/***************************************************************************
                         qgsstatusbarscalewidget.h
    begin                : May 2016
    copyright            : (C) 2016 Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSTATUSBARSCALEWIDGET_H
#define QGSSTATUSBARSCALEWIDGET_H

class QFont;
class QHBoxLayout;
class QLabel;
class QValidator;

class QgsMapCanvas;
class QgsScaleComboBox;


#include <QWidget>

/**
  * Widget to define scale of the map canvas.
  * @note added in 2.16
  */
class APP_EXPORT QgsStatusBarScaleWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QgsStatusBarScaleWidget(QgsMapCanvas* canvas, QWidget *parent = 0);

    /** Destructor */
    virtual ~QgsStatusBarScaleWidget();

    /**
     * @brief setScale set the selected scale from double
     * @param scale
     */
    void setScale( double scale );

    /** Set the font of the text
      * @param font the font to use
      */
    void setFont( const QFont& font );

public slots:
  void updateScales( const QStringList &scales = QStringList() );

private slots:
    void userScale();
private:
    QgsMapCanvas* mMapCanvas;
    QHBoxLayout *mLayout;

    //! Widget that will live on the statusbar to display "scale 1:"
    QLabel* mLabel;
    //! Widget that will live on the statusbar to display scale value
    QgsScaleComboBox* mScale;
    //! The validator for the mScaleEdit
    QValidator* mScaleEditValidator;
};

#endif // QGSSTATUSBARSCALEWIDGET_H
