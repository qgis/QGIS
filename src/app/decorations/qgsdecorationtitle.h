/***************************************************************************
  qgsdecorationtitle.h
  --------------------------------------
  Date                 : November 2018
  Copyright            : (C) 2018 by Mathieu Pellerin
  Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDECORATIONTITLE_H
#define QGSDECORATIONTITLE_H

#include "qgis_app.h"
#include "qgsdecorationitem.h"
#include "qgstextrenderer.h"

#include <QColor>
#include <QFont>
#include <QObject>

class QgsDecorationTitleDialog;

class QAction;
class QPainter;

class APP_EXPORT QgsDecorationTitle : public QgsDecorationItem
{
    Q_OBJECT
  public:

    //! Constructor
    QgsDecorationTitle( QObject *parent = nullptr );

  public slots:
    //! Sets values on the gui when a project is read or the gui first loaded
    void projectRead() override;
    //! save values to the project
    void saveToProject() override;

    //! Show the dialog box
    void run() override;
    //! render the title label
    void render( const QgsMapSettings &mapSettings, QgsRenderContext &context ) override;

    /**
     * Returns the text format for extent labels.
     * \see setTextFormat()
     * \see labelExtents()
     */
    QgsTextFormat textFormat() const { return mTextFormat; }

    /**
     * Sets the text \a format for extent labels.
     * \see textFormat()
     * \see setLabelExtents()
     */
    void setTextFormat( const QgsTextFormat &format ) { mTextFormat = format; }

  private:
    //! This is the string that will be used for the title label
    QString mLabelText;

    //! The background bar color
    QColor mBackgroundColor;

    //! enable or disable use of position percentage for placement
    int mMarginHorizontal = 0;
    int mMarginVertical = 0;

    QgsTextFormat mTextFormat;

    friend class QgsDecorationTitleDialog;
};

#endif
