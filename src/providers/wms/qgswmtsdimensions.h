/***************************************************************************
    qgswmtsdimensions.h  -  selector for WMTS dimensions
                             -------------------
    begin                : 2. May 2012
    copyright            : (C) 2012 by Jürgen Fischer, norBIT GmbH
    email:               : jef (at) norbit (dot) de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSWMTSDIMENSIONS_H
#define QGSWMTSDIMENSIONS_H
#include "ui_qgswmtsdimensionsbase.h"
#include "qgsguiutils.h"
#include "qgswmsprovider.h"

#include <QHash>

/*!
 * \brief   Dialog to select dimension values for WMTS layers
 *
 * This dialog allows the user to select dimensions values the
 * WMTS offers.
 *
 */
class QgsWmtsDimensions : public QDialog, private Ui::QgsWmtsDimensionsBase
{
    Q_OBJECT

  public:
    //! Constructor
    QgsWmtsDimensions( const QgsWmtsTileLayer &layer, QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::WindowFlags() );

    QHash<QString, QString> selectedDimensions() const;

};


#endif // QGSWMTSDIMENSIONS_H
