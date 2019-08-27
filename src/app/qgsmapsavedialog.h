/***************************************************************************
                         qgsmapsavedialog.h
                         -------------------------------------
    begin                : April 2017
    copyright            : (C) 2017 by Mathieu Pellerin
    email                : nirvn dot asia at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSMAPSAVEDIALOG_H
#define QGSMAPSAVEDIALOG_H

#include "ui_qgsmapsavedialog.h"

#include "qgisapp.h"
#include "qgsmapdecoration.h"
#include "qgsrectangle.h"
#include "qgshelp.h"

#include <QDialog>
#include <QSize>

class QgsMapCanvas;


/**
 * \ingroup app
 * \brief a dialog for saving a map to an image.
 * \since QGIS 3.0
*/
class APP_EXPORT QgsMapSaveDialog: public QDialog, private Ui::QgsMapSaveDialog
{
    Q_OBJECT

  public:

    enum DialogType
    {
      Image = 1, // Image-specific dialog
      Pdf        // PDF-specific dialog
    };

    /**
     * Constructor for QgsMapSaveDialog
     */
    QgsMapSaveDialog( QWidget *parent = nullptr, QgsMapCanvas *mapCanvas = nullptr,
                      const QList< QgsDecorationItem * > &decorations = QList< QgsDecorationItem * >(),
                      const QList<QgsAnnotation *> &annotations = QList< QgsAnnotation * >(),
                      DialogType type = Image );

    //! returns extent rectangle
    QgsRectangle extent() const;

    //! returns the numerical value of the dpi spin box
    int dpi() const;

    //! returns the output size
    QSize size() const;

    //! returns whether the draw annotations element is checked
    bool drawAnnotations() const;

    //! returns whether the draw decorations element is checked
    bool drawDecorations() const;

    //! returns whether the resulting image will be georeferenced (embedded or via world file)
    bool saveWorldFile() const;

    //! returns whether metadata such as title and subject will be exported whenever possible
    bool exportMetadata() const;

    //! returns whether the map will be rasterized
    bool saveAsRaster() const;

    //! configure a map settings object
    void applyMapSettings( QgsMapSettings &mapSettings );

  private slots:
    void onAccepted();

  private:

    void lockChanged( bool locked );
    void copyToClipboard();

    void updateDpi( int dpi );
    void updateOutputWidth( int width );
    void updateOutputHeight( int height );
    void updateExtent( const QgsRectangle &extent );
    void updateScale( double scale );
    void updateOutputSize();

    DialogType mDialogType;
    QgsMapCanvas *mMapCanvas = nullptr;
    QList< QgsMapDecoration * > mDecorations;
    QList< QgsAnnotation *> mAnnotations;

    QgsRectangle mExtent;
    int mDpi;
    QSize mSize;

    QString mInfoDetails;

  private slots:

    void showHelp();
};

#endif // QGSMAPSAVEDIALOG_H
