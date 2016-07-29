/***************************************************************************
 *  qgsgeometrycheckerresulttab.h                                          *
 *  -------------------                                                    *
 *  copyright            : (C) 2014 by Sandro Mani / Sourcepole AG         *
 *  email                : smani@sourcepole.ch                             *
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGS_GEOMETRY_CHECKER_RESULT_TAB_H
#define QGS_GEOMETRY_CHECKER_RESULT_TAB_H

#include "ui_qgsgeometrycheckerresulttab.h"
#include "qgsgeometrycheckerfixsummarydialog.h"

class QDialog;
class QgisInterface;
class QgsFeaturePool;
class QgsGeometryChecker;
class QgsGeometryCheckError;
class QgsRubberBand;
class QgsVectorLayer;

class QgsGeometryCheckerResultTab : public QWidget
{
    Q_OBJECT
  public:
    QgsGeometryCheckerResultTab( QgisInterface* iface, QgsGeometryChecker* checker, QgsFeaturePool *featurePool, QTabWidget *tabWidget, QWidget* parent = nullptr );
    ~QgsGeometryCheckerResultTab();
    void finalize();
    bool isCloseable() const { return mCloseable; }

    static QString sSettingsGroup;

  private:
    QTabWidget* mTabWidget;
    Ui::QgsGeometryCheckerResultTab ui;
    QgisInterface* mIface;
    QgsGeometryChecker* mChecker;
    QgsFeaturePool* mFeaturePool;
    QList<QgsRubberBand*> mCurrentRubberBands;
    QMap<QgsGeometryCheckError*, QPersistentModelIndex> mErrorMap;
    QDialog* mAttribTableDialog;
    int mErrorCount;
    int mFixedCount;
    bool mCloseable;

    QgsGeometryCheckerFixSummaryDialog::Statistics mStatistics;

    bool exportErrorsDo( const QString &file );
    void fixErrors( bool prompt );
    void setRowStatus( int row, const QColor& color, const QString& message, bool selectable );

  private slots:
    void addError( QgsGeometryCheckError* error );
    void updateError( QgsGeometryCheckError* error , bool statusChanged );
    void exportErrors();
    void highlightError( QgsGeometryCheckError* error );
    void highlightErrors( bool current = false );
    void onSelectionChanged( const QItemSelection& newSel, const QItemSelection& /*oldSel*/ );
    void openAttributeTable();
    void fixErrorsWithDefault() { fixErrors( false ); }
    void fixErrorsWithPrompt() { fixErrors( true ); }
    void setDefaultResolutionMethods();
    void storeDefaultResolutionMethod( int ) const;
    void checkRemovedLayer( const QStringList &ids );
    void clearAttribTableDialog() { mAttribTableDialog = nullptr; }
};

#endif // QGS_GEOMETRY_CHECKER_RESULT_TAB_H
