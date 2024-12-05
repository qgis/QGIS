/***************************************************************************
                         qgsanimationexportdialog.h
                         -------------------------------------
    begin                : May 2020
    copyright            : (C) 2020 by Nyall Dawson
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

#ifndef QGSANIMATIONEXPORTDIALOG_H
#define QGSANIMATIONEXPORTDIALOG_H

#include "ui_qgsanimationexportdialogbase.h"

#include "qgisapp.h"
#include "qgsrectangle.h"
#include "qgshelp.h"

#include <QDialog>
#include <QSize>

class QgsMapCanvas;


/**
 * \ingroup app
 * \brief A dialog for specifying map animation export settings.
 * \since QGIS 3.14
*/
class APP_EXPORT QgsAnimationExportDialog : public QDialog, private Ui::QgsAnimationExportDialogBase
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsAnimationExportDialog
     */
    QgsAnimationExportDialog( QWidget *parent = nullptr, QgsMapCanvas *mapCanvas = nullptr, const QList<QgsMapDecoration *> &decorations = QList<QgsMapDecoration *>() );

    //! Returns extent rectangle
    QgsRectangle extent() const;

    //! Returns the output size
    QSize size() const;

    //! Returns output directory for frames
    QString outputDirectory() const;

    //! Returns filename template for frames
    QString fileNameExpression() const;

    //! Returns the overall animation range
    QgsDateTimeRange animationRange() const;

    //! Returns the duration of each individual frame
    QgsInterval frameInterval() const;

    //! configure a map settings object
    void applyMapSettings( QgsMapSettings &mapSettings );

    //! returns whether the draw decorations element is checked
    bool drawDecorations() const;

  signals:

    void startExport();

  private slots:

    void setToProjectTime();

  private:
    void lockChanged( bool locked );

    void updateOutputWidth( int width );
    void updateOutputHeight( int height );
    void updateExtent( const QgsRectangle &extent );
    void updateOutputSize();

    QgsMapCanvas *mMapCanvas = nullptr;

    QgsRectangle mExtent;
    QSize mSize;

    QString mInfoDetails;
};

#endif // QGSANIMATIONEXPORTDIALOG_H
