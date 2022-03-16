/***************************************************************************
                             qgsprocessinghelpeditorwidget.h
                             ------------------------
    Date                 : February 2022
    Copyright            : (C) 2022 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSPROCESSINGHELPEDITORWIDGET_H
#define QGSPROCESSINGHELPEDITORWIDGET_H

#include "qgis.h"
#include "qgis_gui.h"
#include "ui_qgsprocessinghelpeditorwidgetbase.h"

#include <QDialog>

class QgsProcessingAlgorithm;



///@cond NOT_STABLE

#define SIP_NO_FILE


/**
 * \ingroup gui
 * \brief A widget for editing help for a Processing algorithm.
 * \warning Not stable API
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsProcessingHelpEditorWidget : public QWidget, public Ui::QgsProcessingHelpEditorWidgetBase
{
    Q_OBJECT
  public:

    /**
     * Constructor for QgsProcessingHelpEditorWidget, with the specified \a parent widget.
     */
    QgsProcessingHelpEditorWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );
    ~QgsProcessingHelpEditorWidget() override;

    /**
     * Sets the \a algorithm associated with the widget.
     */
    void setAlgorithm( const QgsProcessingAlgorithm *algorithm );

    /**
     * Returns the current help content defined by the widget.
     */
    QVariantMap helpContent();

  private slots:

    void updateHtmlView();

    void changeItem( QTreeWidgetItem *current, QTreeWidgetItem *previous );

  private:

    QString formattedHelp() const;
    QString helpComponent( const QString &name ) const;

    QVariantMap mHelpContent;

    QString mCurrentName;

    std::unique_ptr< QgsProcessingAlgorithm > mAlgorithm;

    static const QString ALGORITHM_DESCRIPTION;
    static const QString ALGORITHM_CREATOR;
    static const QString ALGORITHM_HELP_CREATOR;
    static const QString ALGORITHM_VERSION;
    static const QString ALGORITHM_SHORT_DESCRIPTION;
    static const QString ALGORITHM_HELP_URL;
    static const QString ALGORITHM_EXAMPLES;

};

/**
 * \ingroup gui
 * \brief A dialog for editing help for a Processing algorithm.
 * \warning Not stable API
 * \since QGIS 3.26
 */
class GUI_EXPORT QgsProcessingHelpEditorDialog : public QDialog
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsProcessingHelpEditorDialog, with the specified \a parent and \a flags.
     */
    QgsProcessingHelpEditorDialog( QWidget *parent SIP_TRANSFERTHIS = nullptr, Qt::WindowFlags flags = Qt::WindowFlags() );

    /**
     * Sets the \a algorithm associated with the dialog.
     */
    void setAlgorithm( const QgsProcessingAlgorithm *algorithm );

    /**
     * Returns the current help content defined by the dialog.
     */
    QVariantMap helpContent();

  private:

    QgsProcessingHelpEditorWidget *mWidget = nullptr;
};


///@endcond

#endif // QGSPROCESSINGHELPEDITORWIDGET_H
