/***************************************************************************
                          qgsdecorationlayoutextent.h
                              -------------------
    begin                : May 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSDECORATIONLAYOUTEXTENT_H
#define QGSDECORATIONLAYOUTEXTENT_H

#include "qgsdecorationitem.h"

#include <QColor>
#include <QFont>
#include <QObject>
#include "qgis_app.h"
#include "qgssymbol.h"
#include <memory>

class QgsDecorationLayoutExtentDialog;

class APP_EXPORT QgsDecorationLayoutExtent : public QgsDecorationItem
{
    Q_OBJECT
  public:

    //! Constructor
    QgsDecorationLayoutExtent( QObject *parent = nullptr );

    QgsFillSymbol *symbol() const;
    void setSymbol( QgsFillSymbol *symbol SIP_TRANSFER );

  public slots:
    //! set values on the gui when a project is read or the gui first loaded
    void projectRead() override;
    //! save values to the project
    void saveToProject() override;

    //! Show the dialog box
    void run() override;
    //! render the copyright label
    void render( const QgsMapSettings &mapSettings, QgsRenderContext &context ) override;

  private:
    std::unique_ptr< QgsFillSymbol > mSymbol;

    friend class QgsDecorationLayoutExtentDialog;
};

#endif //QGSDECORATIONLAYOUTEXTENT_H
