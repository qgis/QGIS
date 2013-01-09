/****************************************************************************
**
** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtSql module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QSQL_OCISPATIAL_H
#define QSQL_OCISPATIAL_H

#include <QtSql/qsqlresult.h>
#include <QtSql/qsqldriver.h>
#include "qsqlcachedresult_p.h"

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_OCISPATIAL
#else
#define Q_EXPORT_SQLDRIVER_OCISPATIAL Q_SQL_EXPORT
#endif

QT_BEGIN_HEADER

typedef struct OCIEnv OCIEnv;
typedef struct OCISvcCtx OCISvcCtx;

QT_BEGIN_NAMESPACE

class QOCISpatialDriver;
class QOCISpatialCols;
struct QOCISpatialDriverPrivate;
struct QOCISpatialResultPrivate;

class Q_EXPORT_SQLDRIVER_OCISPATIAL QOCISpatialResult : public QSqlCachedResult
{
    friend class QOCISpatialDriver;
    friend struct QOCISpatialResultPrivate;
    friend class QOCISpatialCols;
  public:
    QOCISpatialResult( const QOCISpatialDriver * db, const QOCISpatialDriverPrivate* p );
    ~QOCISpatialResult();
    bool prepare( const QString& query );
    bool exec();
    QVariant handle() const;

  protected:
    bool gotoNext( ValueCache &values, int index );
    bool reset( const QString& query );
    int size();
    int numRowsAffected();
    QSqlRecord record() const;
    QVariant lastInsertId() const;
    void virtual_hook( int id, void *data );

  private:
    QOCISpatialResultPrivate *d;
};

class Q_EXPORT_SQLDRIVER_OCISPATIAL QOCISpatialDriver : public QSqlDriver
{
    Q_OBJECT
    friend struct QOCISpatialResultPrivate;
    friend class QOCISpatialPrivate;
  public:
    explicit QOCISpatialDriver( QObject* parent = 0 );
    QOCISpatialDriver( OCIEnv* env, OCISvcCtx* ctx, QObject* parent = 0 );
    ~QOCISpatialDriver();
    bool hasFeature( DriverFeature f ) const;
    bool open( const QString & db,
               const QString & user,
               const QString & password,
               const QString & host,
               int port,
               const QString& connOpts );
    void close();
    QSqlResult *createResult() const;
    QStringList tables( QSql::TableType ) const;
    QSqlRecord record( const QString& tablename ) const;
    QSqlIndex primaryIndex( const QString& tablename ) const;
    QString formatValue( const QSqlField &field,
                         bool trimStrings ) const;
    QVariant handle() const;
    QString escapeIdentifier( const QString &identifier, IdentifierType ) const;

  protected:
    bool                beginTransaction();
    bool                commitTransaction();
    bool                rollbackTransaction();
  private:
    QOCISpatialDriverPrivate *d;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QSQL_OCISPATIAL_H
