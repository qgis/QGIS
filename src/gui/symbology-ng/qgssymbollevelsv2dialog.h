/***************************************************************************
    qgssymbollevelsv2dialog.h
    ---------------------
    begin                : November 2009
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
#ifndef QGSSYMBOLLEVELSV2DIALOG_H
#define QGSSYMBOLLEVELSV2DIALOG_H

#include <QDialog>
#include <QList>

#include "qgsrendererv2.h"

#include "ui_qgssymbollevelsv2dialogbase.h"


class GUI_EXPORT QgsSymbolLevelsV2Dialog : public QDialog, private Ui::QgsSymbolLevelsV2DialogBase
{
    Q_OBJECT
  public:
    QgsSymbolLevelsV2Dialog( QgsLegendSymbolList list, bool usingSymbolLevels, QWidget* parent = NULL );

    bool usingLevels() const;

    // used by rule-based renderer (to hide checkbox to enable/disable ordering)
    void setForceOrderingEnabled( bool enabled );

  public slots:
    void updateUi();

    void renderingPassChanged( int row, int column );

  protected:
    void populateTable();
    void setDefaultLevels();

  protected:
    //! maximal number of layers from all symbols
    int mMaxLayers;
    QgsLegendSymbolList mList;
    //! whether symbol layers always should be used (default false)
    bool mForceOrderingEnabled;
};

#endif // QGSSYMBOLLEVELSV2DIALOG_H
