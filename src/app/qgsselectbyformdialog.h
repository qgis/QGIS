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

class QgsAttributeForm;
class QgsMessageBar;
class QgsVectorLayer;

/** \ingroup app
 * \class QgsSelectByFormDialog
 * A dialog for selecting features from a layer, using a form based off the layer's editor widgets.
 * @note introduced in QGIS 2.16
 */

class APP_EXPORT QgsSelectByFormDialog : public QDialog
{
    Q_OBJECT

  public:

    /** Constructor for QgsSelectByFormDialog
     * @param layer vector layer to select from
     * @param context editor context
     * @param parent parent widget
     * @param fl window flags
     */
    QgsSelectByFormDialog( QgsVectorLayer* layer,
                           const QgsAttributeEditorContext& context = QgsAttributeEditorContext(),
                           QWidget* parent = nullptr, Qt::WindowFlags fl = nullptr );

    ~QgsSelectByFormDialog();

    /** Sets the message bar to display feedback from the form in. This is used in the search/filter
     * mode to display the count of selected features.
     * @param messageBar target message bar
     * @note added in QGIS 2.16
     */
    void setMessageBar( QgsMessageBar* messageBar );

  private:

    QgsAttributeForm* mForm;

};


#endif // QGSSELECTBYFORMDIALOG_H
