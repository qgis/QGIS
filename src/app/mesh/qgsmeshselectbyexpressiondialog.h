/***************************************************************************
  qgsmeshselectbyexpressiondialog.h - QgsMeshSelectByExpressionDialog

 ---------------------
 begin                : 23.8.2021
 copyright            : (C) 2021 by Vincent Cloarec
 email                : vcloarec at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef QGSMESHSELECTBYEXPRESSIONDIALOG_H
#define QGSMESHSELECTBYEXPRESSIONDIALOG_H

#include "ui_qgsmeshselectbyexpressiondialogbase.h"

#include "qgis_app.h"
#include "qgsmeshdataprovider.h"

/**
 * \brief A Dialog Widget that is used select mesh element by expression during mesh editing.
 *
 * The selected elements by expression are either faces or vertices, depending of the choice of the user.
 *
 * The instance emits signals when select actions are triggered.
 *
 * \since QGIS 3.22
 */
class APP_EXPORT QgsMeshSelectByExpressionDialog : public QDialog, private Ui::QgsMeshSelectByExpressionDialogBase
{
    Q_OBJECT
  public:

    //! Constructor
    QgsMeshSelectByExpressionDialog( QWidget *parent = nullptr );

    //! Returns the text expression defined in the dialog
    QString expression() const;

  signals:
    //! Emitted when one of the select tool button is clicked
    void select( const QString &expression, Qgis::SelectBehavior behavior, QgsMesh::ElementType elementType );

    //! Emittes when the zoom to selected element button is clicked
    void zoomToSelected();

  private slots:
    void showHelp() const;
    void saveRecent() const;
    void onElementTypeChanged() const;

  private:
    QAction *mActionSelect = nullptr;
    QAction *mActionAddToSelection = nullptr;
    QAction *mActionRemoveFromSelection = nullptr;

    QgsMesh::ElementType currentElementType() const;
};

#endif // QGSMESHSELECTBYEXPRESSIONDIALOG_H
