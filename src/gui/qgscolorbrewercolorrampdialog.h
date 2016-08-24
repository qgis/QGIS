/***************************************************************************
    qgscolorbrewercolorrampdialog.h
    ---------------------
    begin                : November 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSCOLORBREWERCOLORRAMPDIALOG_H
#define QGSCOLORBREWERCOLORRAMPDIALOG_H

#include <QDialog>

#include "ui_qgscolorbrewercolorrampdialogbase.h"

class QgsColorBrewerColorRamp;

/** \ingroup gui
 * \class QgsColorBrewerColorRampDialog
 */
class GUI_EXPORT QgsColorBrewerColorRampDialog : public QDialog, private Ui::QgsColorBrewerColorRampDialogBase
{
    Q_OBJECT

  public:
    QgsColorBrewerColorRampDialog( QgsColorBrewerColorRamp* ramp, QWidget* parent = nullptr );

  public slots:
    void setSchemeName();
    void setColors();

    void populateVariants();

  protected:

    void updatePreview();

    QgsColorBrewerColorRamp* mRamp;
};

#endif
