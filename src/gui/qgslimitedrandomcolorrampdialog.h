/***************************************************************************
    qgslimitedrandomcolorrampdialog.h
    ---------------------
    begin                : December 2009
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

#ifndef QGsLIMITEDRANDOMCOLORRAMPDIALOG_H
#define QGsLIMITEDRANDOMCOLORRAMPDIALOG_H

#include <QDialog>
#include "qgscolorramp.h"
#include "ui_qgslimitedrandomcolorrampdialogbase.h"

class QgsLimitedRandomColorRamp;

/** \ingroup gui
 * \class QgsLimitedRandomColorRampDialog
 */
class GUI_EXPORT QgsLimitedRandomColorRampDialog : public QDialog, private Ui::QgsLimitedRandomColorRampDialogBase
{
    Q_OBJECT

  public:
    QgsLimitedRandomColorRampDialog( const QgsLimitedRandomColorRamp& ramp, QWidget* parent = nullptr );

    QgsLimitedRandomColorRamp ramp() const { return mRamp; }

  public slots:
    void setCount( int val );
    void setHue1( int val );
    void setHue2( int val );
    void setSat1( int val );
    void setSat2( int val );
    void setVal1( int val );
    void setVal2( int val );

  protected:

    void updatePreview();

    QgsLimitedRandomColorRamp mRamp;
};

#endif
