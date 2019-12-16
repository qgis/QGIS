#ifndef QGSEXPRESSIONSTOREDIALOG_H
#define QGSEXPRESSIONSTOREDIALOG_H

#include "qgis_gui.h"
#include <QDialog>
#include "ui_qgsexpressionstoredialogbase.h"



/**
 * \ingroup gui
 * A generic dialog for editing expression text, label and help text.
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsExpressionStoreDialog : public QDialog, private Ui::QgsExpressionStoreDialogBase
{
  public:

    /**
     * Creates a QgsExpressionStoreDialog with given \a label, \a expression and \a helpText.
     * \param existingLabels list of existing labels for unique label validation
     * \param parent optional parent widget
     */
    QgsExpressionStoreDialog( const QString &label,
                              const QString &expression,
                              const QString &helpText,
                              const QStringList &existingLabels,
                              QWidget *parent = nullptr );

    /**
     * Returns the expression text
     */
    QString expression( ) { return mExpression->text( ); }

    /**
     * Returns the label text
     */
    QString label() { return  mLabel->text(); }

    /**
     * Returns the help text
     */
    QString helpText() { return  mHelpText->toHtml(); }

  private:

    QStringList mExistingLabels;

};

#endif // QGSEXPRESSIONSTOREDIALOG_H
