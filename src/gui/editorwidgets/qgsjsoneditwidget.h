/***************************************************************************
    qgsjsoneditwidget.h
     --------------------------------------
    Date                 : 3.5.2021
    Copyright            : (C) 2021 Damiano Lombardi
    Email                : damiano at opengis dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSJSONEDITWIDGET_H
#define QGSJSONEDITWIDGET_H

#include "ui_qgsjsoneditwidget.h"

#include "qgseditorconfigwidget.h"
#include "qgis_sip.h"
#include "qgis_gui.h"

/**
 * \ingroup gui
 * \class QgsJsonEditWidget
 * \brief The QgsJsonEditWidget is a widget to display JSON data in a code highlighted text or tree form.
 * \since QGIS 3.20
 */
class GUI_EXPORT QgsJsonEditWidget : public QWidget, private Ui::QgsJsonEditWidget
{
    Q_OBJECT

  public:

    //! View mode, text or tree.
    enum class View : int
    {
      Text = 0, //!< JSON data displayed as text.
      Tree = 1 //!< JSON data displayed as tree. Tree view is disabled for invalid JSON data.
    };

    //! Format mode in the text view
    enum class FormatJson : int
    {
      Indented = 0, //!< JSON data formatted with regular indentation
      Compact = 1, //!< JSON data formatted as a compact one line string
      Disabled = 2 //!< JSON data is not formatted
    };

    /**
     * Constructor for QgsJsonEditWidget.
     * \param parent parent widget
     */
    explicit QgsJsonEditWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Returns a reference to the JSON code editor used in the widget.
     *
     * \since QGIS 3.36
     */
    QgsCodeEditorJson *jsonEditor();

    /**
     * \brief Set the JSON text in the widget to \a jsonText.
     */
    void setJsonText( const QString &jsonText );

    /**
     * \brief Returns the JSON text.
     */
    QString jsonText() const;

    /**
     * \brief Set the \a view mode.
     * \see View
     */
    void setView( View view ) const;

    /**
     * \brief Set the \a formatJson mode.
     * \see FormatJson
     */
    void setFormatJsonMode( FormatJson formatJson );

    /**
     * \brief Set the visibility of controls to \a visible.
     */
    void setControlsVisible( bool visible );

  private slots:

    void textToolButtonClicked( bool checked );
    void treeToolButtonClicked( bool checked );

    void copyValueActionTriggered();
    void copyKeyActionTriggered();

    void codeEditorJsonTextChanged();
    void codeEditorJsonIndicatorClicked( int line, int index, Qt::KeyboardModifiers state );
    void codeEditorJsonDwellStart( int position, int x, int y );
    void codeEditorJsonDwellEnd( int position, int x, int y );

  private:

    enum class TreeWidgetColumn : int
    {
      Key = 0,
      Value = 1
    };

    const int SCINTILLA_UNDERLINE_INDICATOR_INDEX = 15;

    void refreshTreeView( const QJsonDocument &jsonDocument );
    void refreshTreeViewItem( QTreeWidgetItem *treeWidgetItemParent, const QJsonValue &jsonValue );
    void refreshTreeViewItemValue( QTreeWidgetItem *treeWidgetItem, const QString &jsonValueString, const QColor &textColor );

    QFont monospaceFont() const;

    QString mJsonText;

    FormatJson mFormatJsonMode = FormatJson::Indented;

    QStringList mClickableLinkList;

    QAction *mCopyValueAction;
    QAction *mCopyKeyAction;

    bool mEnableUrlHighlighting = true;
};

#endif // QGSJSONEDITWIDGET_H
