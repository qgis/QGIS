/***************************************************************************
  qgsdecorationimage.h
  --------------------------------------
  Date                 : August 2019
  Copyright            : (C) 2019 by Mathieu Pellerin
  Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSDECORATIONIMAGE_H
#define QGSDECORATIONIMAGE_H

#include "qgsdecorationitem.h"

#include <QStringList>
#include <QColor>
#include "qgis_app.h"

class QAction;
class QToolBar;
class QPainter;

class APP_EXPORT QgsDecorationImage: public QgsDecorationItem
{
    Q_OBJECT

  public:

    /**
     * Format of source image
     */
    enum Format
    {
      FormatSVG, //!< SVG image
      FormatRaster, //!< Raster image
      FormatUnknown, //!< Invalid or unknown image type
    };

    /**
     * Constructor for QgsDecorationImage, with the specified \a parent object.
     */
    QgsDecorationImage( QObject *parent = nullptr );

  public slots:
    //! Sets values on the GUI when a project is read or the GUI first loaded
    void projectRead() override;
    //! Save values to the project
    void saveToProject() override;

    //! Show the dialog box
    void run() override;
    //! Draw some arbitrary text to the screen
    void render( const QgsMapSettings &mapSettings, QgsRenderContext &context ) override;

    //! Sets the image path
    void setImagePath( const QString &imagePath );

    //! Returns the image path
    QString imagePath();

  private:

    //! The image fill color used with parameter-enabled SVG files
    QColor mColor;
    //! The image outline color used with parameter-enabled SVG files
    QColor mOutlineColor;
    //! The image size in millimeter
    double mSize = 16.0;
    //! The image path
    QString mImagePath;
    QgsDecorationImage::Format mImageFormat = FormatUnknown;

    //! margin values
    int mMarginHorizontal = 0;
    int mMarginVertical = 0;

    friend class QgsDecorationImageDialog;
};

#endif
