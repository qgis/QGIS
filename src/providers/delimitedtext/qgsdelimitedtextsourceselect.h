/***************************************************************************
 *   Copyright (C) 2003 by Tim Sutton                                      *
 *   Copyright (C) 2004 by Gary Sherman                                    *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This is a plugin generated from the QGIS plugin template              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#ifndef QGSDELIMITEDTEXTSOURCESELECT_H
#define QGSDELIMITEDTEXTSOURCESELECT_H

#include "ui_qgsdelimitedtextsourceselectbase.h"

#include <QTextStream>
#include <QThread>

#include "qgshelp.h"
#include "qgsguiutils.h"
#include "qgsfeedback.h"
#include "qgsfields.h"
#include "qgstaskmanager.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdatasourcewidget.h"
#include "qgsdelimitedtextfile.h"
#include "qgsdelimitedtextprovider.h"

class QButtonGroup;
class QgisInterface;


/**
 * \brief The QgsDelimitedTextFileScan class scans a CSV file to identify field types.
 */
class QgsDelimitedTextFileScanTask: public QgsTask
{

    Q_OBJECT

  public:

    QgsDelimitedTextFileScanTask( const QString &dataSource )
      : QgsTask( QStringLiteral( "delimited text scan %1" ).arg( dataSource ) )
      , mDataSource( dataSource )
    {
    };

    ~QgsDelimitedTextFileScanTask()
    {
    }

    // QThread interface
  protected:
    bool run() override;

  public slots:

    void cancel() override;

  signals:

    /**
     * \brief scanCompleted is always emitted, even if the \a fields could not be determined.
     */
    void scanCompleted( const QgsFields &field );

    /**
     * \brief scanStarted is emitted only on valid layers when the first row is scanned,
     *        this allows for quicker update of the GUI if the scan is performed on large files.
     */
    void scanStarted( const QgsFields &field );

    /**
     * \brief processedCountChanged is emitted when the file scan reports a change in the number of scanned rows.
     */
    void processedCountChanged( unsigned long long processedCount );

  private:

    QString mDataSource;
    QgsFeedback mFeedback;

};

/**
 * \class QgsDelimitedTextSourceSelect
 */
class QgsDelimitedTextSourceSelect : public QgsAbstractDataSourceWidget, private Ui::QgsDelimitedTextSourceSelectBase
{
    Q_OBJECT

  public:
    QgsDelimitedTextSourceSelect( QWidget *parent = nullptr, Qt::WindowFlags fl = QgsGuiUtils::ModalDialogFlags, QgsProviderRegistry::WidgetMode widgetMode = QgsProviderRegistry::WidgetMode::None );

  private:
    bool loadDelimitedFileDefinition();
    void updateFieldLists();
    QString selectedChars();
    void setSelectedChars( const QString &delimiters );
    void loadSettings( const QString &subkey = QString(), bool loadGeomSettings = true );
    void saveSettings( const QString &subkey = QString(), bool saveGeomSettings = true );
    void loadSettingsForFile( const QString &filename );
    void saveSettingsForFile( const QString &filename );
    bool trySetXYField( QStringList &fields, QList<bool> &isValidNumber, const QString &xname, const QString &yname );

  private:
    std::unique_ptr<QgsDelimitedTextFile> mFile;
    int mExampleRowCount = 20;
    int mBadRowCount = 0;
    QgsFields mFields; //!< Stores the fields as returned by the provider to determine if their types were overridden
    QSet<int> mOverriddenFields; //!< Stores user-overridden fields
    static constexpr int DEFAULT_MAX_FIELDS = 10000;
    int mMaxFields = DEFAULT_MAX_FIELDS; //!< To avoid Denial Of Service (at least in source select). Configurable through /max_fields settings sub-key.
    QString mSettingsKey;
    QString mLastFileType;
    QPointer<QgsDelimitedTextFileScanTask> mScanTask;
    QButtonGroup *bgFileFormat = nullptr;
    QButtonGroup *bgGeomType = nullptr;
    void showHelp();
    void updateCrsWidgetVisibility();
    QString url( bool skipOverriddenTypes = false );

  public slots:
    void addButtonClicked() override;
    void updateFileName();
    void updateFieldsAndEnable();
    void enableAccept();
    bool validate();
    void cancelScanTask();
    void updateFieldTypes( const QgsFields &fields );
};


#endif // QGSDELIMITEDTEXTSOURCESELECT_H
