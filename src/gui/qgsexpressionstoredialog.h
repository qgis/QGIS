/***************************************************************************
    qgsexpressionstoredialog.h
    ---------------------
    begin                : December 2019
    copyright            : (C) 2019 by Alessandro Pasotti
    email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

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
     * \a existingLabels is an optional list of existing labels for unique label validation,
     * \a parent is the optional parent widget.
     */
    QgsExpressionStoreDialog( const QString &label,
                              const QString &expression,
                              const QString &helpText,
                              const QStringList &existingLabels = QStringList(),
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
    QString helpText() const;

  private:

    QStringList mExistingLabels;

};

#endif // QGSEXPRESSIONSTOREDIALOG_H
