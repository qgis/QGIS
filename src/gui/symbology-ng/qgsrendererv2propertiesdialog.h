/***************************************************************************
    qgsrendererv2propertiesdialog.h

    ---------------------
    begin                : December 2009
    copyright            : (C) 2009 by Martin Dobias
    email                : wonder.sk at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSRENDERERV2PROPERTIESDIALOG_H
#define QGSRENDERERV2PROPERTIESDIALOG_H

#include <QDialog>

#include "ui_qgsrendererv2propsdialogbase.h"

class QKeyEvent;

class QgsVectorLayer;
class QgsStyleV2;
class QgsSymbolV2;

class QgsRendererV2Widget;

class GUI_EXPORT QgsRendererV2PropertiesDialog : public QDialog, private Ui::QgsRendererV2PropsDialogBase
{
    Q_OBJECT

  public:
    QgsRendererV2PropertiesDialog( QgsVectorLayer* layer, QgsStyleV2* style, bool embedded = false );

  public slots:
    //! called when user changes renderer type
    void rendererChanged();

    void apply();
    void onOK();

    void useOldSymbology();

  signals:
    void useNewSymbology( bool );

  protected:

    //! Reimplements dialog keyPress event so we can ignore it
    void keyPressEvent( QKeyEvent * event );


    QgsVectorLayer* mLayer;
    QgsStyleV2* mStyle;

    QgsRendererV2Widget* mActiveWidget;
};


#endif
