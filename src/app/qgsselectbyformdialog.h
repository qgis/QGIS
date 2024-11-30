/***************************************************************************
    qgsselectbyformdialog.h
     ----------------------
    Date                 : June 2016
    Copyright            : (C) 2016 nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSSELECTBYFORMDIALOG_H
#define QGSSELECTBYFORMDIALOG_H

#include <QDialog>
#include "qgsattributeeditorcontext.h"
#include "qgis_app.h"

class QgsAttributeForm;
class QgsMessageBar;
class QgsVectorLayer;
class QgsMapCanvas;

/**
 * \ingroup app
 * \class QgsSelectByFormDialog
 * \brief A dialog for selecting features from a layer, using a form based off the layer's editor widgets.
 */

class APP_EXPORT QgsSelectByFormDialog : public QDialog
{
    Q_OBJECT

  public:
    /**
     * Constructor for QgsSelectByFormDialog
     * \param layer vector layer to select from
     * \param context editor context
     * \param parent parent widget
     * \param fl window flags
     */
    QgsSelectByFormDialog( QgsVectorLayer *layer, const QgsAttributeEditorContext &context = QgsAttributeEditorContext(), QWidget *parent = nullptr, Qt::WindowFlags fl = Qt::WindowFlags() );

    /**
     * Sets the message bar to display feedback from the form in. This is used in the search/filter
     * mode to display the count of selected features.
     * \param messageBar target message bar
     */
    void setMessageBar( QgsMessageBar *messageBar );

    /**
     * Sets a map canvas associated with the dialog.
     */
    void setMapCanvas( QgsMapCanvas *canvas );

  private slots:

    void zoomToFeatures( const QString &filter );
    void flashFeatures( const QString &filter );
    void openFeaturesAttributeTable( const QString &filter );

  signals:

    void showFilteredFeaturesAttributeTable( const QString &filter );

  private:
    QgsAttributeForm *mForm = nullptr;
    QgsVectorLayer *mLayer = nullptr;
    QgsMessageBar *mMessageBar = nullptr;
    QgsMapCanvas *mMapCanvas = nullptr;
};


#endif // QGSSELECTBYFORMDIALOG_H
