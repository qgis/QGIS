/***************************************************************************
 *  qgsgeometrycheckerfixsummarydialog.h                                   *
 *  -------------------                                                    *
 *  copyright            : (C) 2015 by Sandro Mani / Sourcepole AG         *
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

#ifndef QGS_GEOMETRY_CHECKER_FIX_SUMMARY_DIALOG_H
#define QGS_GEOMETRY_CHECKER_FIX_SUMMARY_DIALOG_H

#include <QDialog>
#include <QSet>
#include "ui_qgsgeometrycheckerfixsummarydialog.h"

class QgisInterface;
class QgsGeometryCheckError;
class QgsGeometryChecker;
class QgsFeaturePool;

class QgsGeometryCheckerFixSummaryDialog : public QDialog
{
    Q_OBJECT
  public:
    struct Statistics
    {
        QSet<QgsGeometryCheckError *> newErrors;
        QSet<QgsGeometryCheckError *> obsoleteErrors;
        QSet<QgsGeometryCheckError *> fixedErrors;
        QSet<QgsGeometryCheckError *> failedErrors;
        int itemCount() const
        {
          return newErrors.size() + obsoleteErrors.size() + fixedErrors.size() + failedErrors.size();
        }
    };

    QgsGeometryCheckerFixSummaryDialog( const Statistics &stats, QgsGeometryChecker *checker, QWidget *parent = nullptr );

  signals:
    void errorSelected( QgsGeometryCheckError *error );

  private:
    Ui::QgsGeometryCheckerFixSummaryDialog ui;
    QgsGeometryChecker *mChecker;

    void addError( QTableWidget *table, QgsGeometryCheckError *error );
    void setupTable( QTableWidget *table );

  private slots:
    void onTableSelectionChanged( const QItemSelection &newSel, const QItemSelection &oldSel );
};

#endif // QGS_GEOMETRY_CHECKER_FIX_SUMMARY_DIALOG_H
