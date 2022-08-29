/***************************************************************************
    qgisexpressionbuilderdialog.h - A generic expression string builder dialog.
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
#include "qgshelp.h"
#include "qgis_gui.h"


/**
 * \ingroup gui
 * \brief A generic dialog for building expression strings
  * @remarks This class also shows an example on how to use QgsExpressionBuilderWidget
  */
class GUI_EXPORT QgsExpressionBuilderDialog : public QDialog, private Ui::QgsExpressionBuilderDialogBase
{
    Q_OBJECT

    Q_PROPERTY( bool allowEvalErrors READ allowEvalErrors WRITE setAllowEvalErrors NOTIFY allowEvalErrorsChanged )

  public:
    QgsExpressionBuilderDialog( QgsVectorLayer *layer,
                                const QString &startText = QString(),
                                QWidget *parent SIP_TRANSFERTHIS = nullptr,
                                const QString &key = "generic",
                                const QgsExpressionContext &context = QgsExpressionContext() );

    //! The builder widget that is used by the dialog
    QgsExpressionBuilderWidget *expressionBuilder();

    void setExpressionText( const QString &text );

    QString expressionText();

    /**
     * Returns the expected format string, which is shown in the dialog.
     * This is purely a text format and no expression validation
     * is done against it.
     * \see setExpectedOutputFormat()
     */
    QString expectedOutputFormat();

    /**
     * Set the \a expected format string, which is shown in the dialog.
     * This is purely a text format and no expression validation is done against it.
     * \see expectedOutputFormat()
     */
    void setExpectedOutputFormat( const QString &expected );

    /**
     * Returns the expression context for the dialog. The context is used for the expression
     * preview result and for populating the list of available functions and variables.
     * \see setExpressionContext
     * \since QGIS 2.12
     */
    QgsExpressionContext expressionContext() const;

    /**
     * Sets the expression context for the dialog. The context is used for the expression
     * preview result and for populating the list of available functions and variables.
     * \param context expression context
     * \see expressionContext
     * \since QGIS 2.12
     */
    void setExpressionContext( const QgsExpressionContext &context );

    //! Sets geometry calculator used in distance/area calculations.
    void setGeomCalculator( const QgsDistanceArea &da );

    /**
     * Allow accepting invalid expressions. This can be useful when we are not able to
     * provide an expression context of which we are sure it's completely populated.
     *
     * \since QGIS 3.0
     */
    bool allowEvalErrors() const;

    /**
     * Allow accepting expressions with evaluation errors. This can be useful when we are not able to
     * provide an expression context of which we are sure it's completely populated.
     *
     * \since QGIS 3.0
     */
    void setAllowEvalErrors( bool allowEvalErrors );

  signals:

    /**
     * Allow accepting expressions with evaluation errors. This can be useful when we are not able to
     * provide an expression context of which we are sure it's completely populated.
     *
     * \since QGIS 3.0
     */
    void allowEvalErrorsChanged();

  protected:

    /**
     * Is called when the dialog get accepted or rejected
     * Used to save geometry
     *
     * \param r result value (unused)
     */
    void done( int r ) override;

    void accept() override;
    void reject() override;

  private:
    const QString mInitialText;
    QString mRecentKey;
    bool mAllowEvalErrors = false;

  private slots:
    void showHelp();
    void syncOkButtonEnabledState();

};

// clazy:excludeall=qstring-allocations

#endif
