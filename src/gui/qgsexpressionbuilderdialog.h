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
