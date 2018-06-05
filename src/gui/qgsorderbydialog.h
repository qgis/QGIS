/***************************************************************************
 qgsorderbydialog.h

 ---------------------
 begin                : 20.12.2015
 copyright            : (C) 2015 by Matthias Kuhn
 email                : matthias@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSORDERBYDIALOG_H
#define QGSORDERBYDIALOG_H

#include <QDialog>
#include "qgis.h"

#include "ui_qgsorderbydialogbase.h"
#include "qgsfeaturerequest.h"
#include "qgshelp.h"
#include "qgis_gui.h"

class QgsVectorLayer;

/**
 * \ingroup gui
 * This is a dialog to build and manage a list of order by clauses.
 *
 * \since QGIS 2.14
 */

class GUI_EXPORT QgsOrderByDialog : public QDialog, private Ui::OrderByDialogBase
{
    Q_OBJECT

  public:

    /**
     * Create a new order by dialog. This helps building order by structures.
     *
     * \param layer  The vector layer for which the order by should be produced
     * \param parent The parent widget, optional
     */
    QgsOrderByDialog( QgsVectorLayer *layer, QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Set the order by to manage
     */
    void setOrderBy( const QgsFeatureRequest::OrderBy &orderBy );

    /**
     * Gets the order by defined in the dialog
     */
    QgsFeatureRequest::OrderBy orderBy();

  protected:

    bool eventFilter( QObject *obj, QEvent *e ) override;

  private slots:
    void onExpressionChanged( const QString &expression );
    void showHelp();

  private:

    /**
     * Initialize a row with the given information
     */
    void setRow( int row, const QgsFeatureRequest::OrderByClause &orderByClause );

    QgsVectorLayer *mLayer = nullptr;

};

#endif // QGSORDERBYDIALOG_H
