/***************************************************************************
    qgisexpressionbuilderdialog.h - A genric expression string builder dialog.
     --------------------------------------
    Date                 :  29-May-2011
    Copyright            : (C) 2011 by Nathan Woodrow
    Email                : woodrow.nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXPRESSIONBUILDERDIALOG_H
#define QGSEXPRESSIONBUILDERDIALOG_H

#include <QDialog>
#include "ui_qgsexpressionbuilderdialogbase.h"

class QgsExpressionBuilderDialog : public QDialog, private Ui::QgsExpressionBuilderDialogBase
{
    public:
        QgsExpressionBuilderDialog( QgsVectorLayer* layer, QString startText = "", QWidget* parent = NULL);

        /** The builder widget that is used by the dialog */
        QgsExpressionBuilderWidget* expressionBuilder();

        void setExpressionText( QString text );

    protected:
        /**
         * Handle closing of the window
         * @param event unused
         */
        void closeEvent( QCloseEvent * event );
};

#endif
