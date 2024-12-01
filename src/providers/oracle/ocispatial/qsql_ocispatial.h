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

#include <QtSql/qsqldriver.h>

#ifdef QT_PLUGIN
#define Q_EXPORT_SQLDRIVER_OCISPATIAL
#else
#define Q_EXPORT_SQLDRIVER_OCISPATIAL Q_SQL_EXPORT
#endif

typedef struct OCIEnv OCIEnv;
typedef struct OCISvcCtx OCISvcCtx;

QT_BEGIN_NAMESPACE

class QSqlResult;
class QOCISpatialDriverPrivate;

class Q_EXPORT_SQLDRIVER_OCISPATIAL QOCISpatialDriver : public QSqlDriver
{
    Q_DECLARE_PRIVATE( QOCISpatialDriver )
    Q_OBJECT
    friend class QOCISpatialCols;
    friend class QOCISpatialResultPrivate;

  public:
    explicit QOCISpatialDriver( QObject *parent = nullptr );
    QOCISpatialDriver( OCIEnv *env, OCISvcCtx *ctx, QObject *parent = nullptr );
    ~QOCISpatialDriver() override;
    bool hasFeature( DriverFeature f ) const override;
    bool open( const QString &db, const QString &user, const QString &password, const QString &host, int port, const QString &connOpts ) override;
    void close() override;
    QSqlResult *createResult() const override;
    QStringList tables( QSql::TableType ) const override;
    QSqlRecord record( const QString &tablename ) const override;
    QSqlIndex primaryIndex( const QString &tablename ) const override;
    QString formatValue( const QSqlField &field, bool trimStrings ) const override;
    QVariant handle() const override;
    QString escapeIdentifier( const QString &identifier, IdentifierType ) const override;

  protected:
    bool beginTransaction() override;
    bool commitTransaction() override;
    bool rollbackTransaction() override;
};

QT_END_NAMESPACE

#endif // QSQL_OCISPATIAL_H
