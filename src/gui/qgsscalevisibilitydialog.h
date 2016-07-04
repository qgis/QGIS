/***************************************************************************
   qgsscalevisibilitydialog.cpp
    --------------------------------------
   Date                 : 20.05.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef QGSSCALEVISIBILITYDIALOG_H
#define QGSSCALEVISIBILITYDIALOG_H

#include <QDialog>
#include <QGroupBox>

#include "qgsscalerangewidget.h"

/** \ingroup gui
 * \class QgsScaleVisibilityDialog
 */
class GUI_EXPORT QgsScaleVisibilityDialog : public QDialog
{
    Q_OBJECT

  public:
    explicit QgsScaleVisibilityDialog( QWidget *parent = nullptr, const QString& title = QString(), QgsMapCanvas* mapCanvas = nullptr );

    //! return if scale visibilty is enabled
    bool hasScaleVisibility();

    //! return minimum scale (true scale, not scale denominator)
    double minimumScale();

    //! return maximum scale (true scale, not scale denominator)
    double maximumScale();


  public slots:
    //! set if scale visibility is enabled
    void setScaleVisiblity( bool hasScaleVisibility );

    //! set minimum scale (true scale, not scale denominator)
    void setMinimumScale( double minScale );

    //! set maximum scale (true scale, not scale denominator)
    void setMaximumScale( double maxScale );


  private:
    QGroupBox* mGroupBox;
    QgsScaleRangeWidget* mScaleWidget;

};

#endif // QGSSCALEVISIBILITYDIALOG_H
