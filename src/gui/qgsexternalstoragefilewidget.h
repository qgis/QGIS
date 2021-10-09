/***************************************************************************
  qgsexternalstoragefilewidget.h
  --------------------------------------
  Date                 : August 2021
  Copyright            : (C) 2021 by Julien Cabieces
  Email                : julien dot cabieces at oslandia dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSEXTERNALSTORAGEFILEWIDGET_H
#define QGSEXTERNALSTORAGEFILEWIDGET_H

class QLabel;
class QToolButton;
class QVariant;
class QHBoxLayout;
class QProgressBar;
class QgsExternalStorage;
class QgsMessageBar;

#include <QWidget>
#include <QFileDialog>

#include "qgsfilewidget.h"
#include "qgsexpressioncontext.h"

/**
 * \ingroup gui
 * \brief The QgsExternalStorageFileWidget class creates a widget for selecting a file or a folder
 * and stores it to a given external storage backend if defined
 * \since QGIS 3.22
 */
class GUI_EXPORT QgsExternalStorageFileWidget : public QgsFileWidget
{

#ifdef SIP_RUN
    SIP_CONVERT_TO_SUBCLASS_CODE
    if ( qobject_cast<QgsExternalStorageFileWidget *>( sipCpp ) )
      sipType = sipType_QgsExternalStorageFileWidget;
    else
      sipType = NULL;
    SIP_END
#endif

    Q_OBJECT
    Q_PROPERTY( QString storageType READ storageType WRITE setStorageType )
    Q_PROPERTY( QString auth READ storageAuthConfigId WRITE setStorageAuthConfigId )
    Q_PROPERTY( QString storageUrlExpression READ storageUrlExpressionString WRITE setStorageUrlExpression )

  public:

    /**
     * \brief QgsExternalStorageFileWidget creates a widget for selecting a file or a folder.
     */
    explicit QgsExternalStorageFileWidget( QWidget *parent SIP_TRANSFERTHIS = nullptr );

    /**
     * Set \a storageType storage type unique identifier as defined in QgsExternalStorageRegistry or
     * null QString if there is no storage defined.
     * If no external storage has been defined, QgsExternalStorageFileWidget will only update file path according to
     * selected files.
     * \see storageType
     * \since QGIS 3.22
     */
    void setStorageType( const QString &storageType );

    /**
     * Returns storage type unique identifier as defined in QgsExternalStorageRegistry.
     * Returns null QString if there is no storage defined, only file selection.
     * \see setStorageType
     * \since QGIS 3.22
     */
    QString storageType() const;

    /**
     * Returns external storage used to store selected file names, nullptr if none have been defined.
     * If no external storage has been defined, QgsExternalStorageFileWidget will only update file path according to
     * selected files.
     * \see setStorageType
     * \since QGIS 3.22
     */
    QgsExternalStorage *externalStorage() const;

    /**
     * Sets the authentication configuration ID to be used for the current external storage (if
     * defined)
     * \since QGIS 3.22
     */
    void setStorageAuthConfigId( const QString &authCfg );

    /**
     * Returns the authentication configuration ID used for the current external storage (if defined)
     * \since QGIS 3.22
     */
    const QString &storageAuthConfigId() const;

    /**
     * Set \a urlExpression expression, which once evaluated, provide the URL used to store selected
     * documents. This is used only if an external storage has been defined
     * \see setStorageType(), externalStorage()
     * \since 3.22
     */
    void setStorageUrlExpression( const QString &urlExpression );

    /**
     * Returns the original, unmodified expression string, which once evaluated, provide the
     * URL used to store selected documents. This is used only if an external storage has been defined.
     * Returns null if no expression has been set.
     * \see setStorageUrlExpression()
     * \since 3.22
     */
    QString storageUrlExpressionString() const;

    /**
     * Returns expression, which once evaluated, provide the URL used to store selected
     * documents. This is used only if an external storage has been defined.
     * Returns null if no expression has been set.
     * \see setStorageUrlExpression()
     * \since 3.22
     */
    QgsExpression *storageUrlExpression() const;

    /**
     * Set expression context to be used when for storage URL expression evaluation
     * \see setStorageUrlExpression
     * \since 3.22
     */
    void setExpressionContext( const QgsExpressionContext &context );

    /**
     * Returns expression context used for storage url expression evaluation
     * \see storageUrlExpression
     * \since 3.22
     */
    const QgsExpressionContext &expressionContext() const;

    /**
     * Set \a messageBar to report messages
     * \since QGIS 3.22
     */
    void setMessageBar( QgsMessageBar *messageBar );

    /**
     * Returns message bar used to report messages
     * \since QGIS 3.22
     */
    QgsMessageBar *messageBar() const;

    /**
     * Creates and Returns an expression context scope specific to QgsExternalStorageFileWidget
     * It defines the variable containing the user selected file name
     * \since 3.22
     */
    static QgsExpressionContextScope *createFileWidgetScope();

    void setReadOnly( bool readOnly ) override;

  protected:

    void updateLayout() override;

    void setSelectedFileNames( QStringList fileNames ) override;

    /**
     *  Add file widget specific scope to expression context
     */
    void addFileWidgetScope();

    void dragEnterEvent( QDragEnterEvent *event ) override;

    void dropEvent( QDropEvent *event ) override;

  private:

    // stores \a fileNames files using current external storage.
    // This is a recursive method, \a storedUrls contains urls for previously stored
    // fileNames. When all files have been successfully stored, current mFilePath
    // is updated
    void storeExternalFiles( QStringList fileNames, QStringList storedUrls = QStringList() );

    //! Update whether the widget accept drop or not
    void updateAcceptDrops();

    bool mStoreInProgress = false;

    QgsExternalStorage *mExternalStorage = nullptr;
    QString mAuthCfg;
    std::unique_ptr< QgsExpression > mStorageUrlExpression;
    QgsExpressionContext mExpressionContext;
    QgsExpressionContextScope *mScope = nullptr;

    QLabel *mProgressLabel = nullptr;
    QProgressBar *mProgressBar = nullptr;
    QToolButton *mCancelButton = nullptr;
    QgsMessageBar *mMessageBar = nullptr;

    friend class TestQgsExternalResourceWidgetWrapper;
    friend class TestQgsExternalStorageFileWidget;
};

#endif // QGSEXTERNALSTORAGEFILEWIDGET_H
