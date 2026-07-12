/***************************************************************************
    qgsaipythonapprovaldialog.cpp
    ---------------------
    begin                : April 2026
    copyright            : (C) 2026 by Francesco Mazzi
    email                : francemazzi at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsaipythonapprovaldialog.h"

#include "qgscodeeditorpython.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QPushButton>
#include <QRegularExpression>
#include <QString>
#include <QVBoxLayout>

#include "moc_qgsaipythonapprovaldialog.cpp"

using namespace Qt::StringLiterals;

QStringList QgsAiPythonApprovalDialog::detectRiskMarkers( const QString &code )
{
  // Advisory heuristics: surface what the snippet COULD do, never block on it.
  struct Marker
  {
      const char *pattern;
      const char *label;
  };
  static const Marker markers[] = {
    { R"(\b(import|from)\s+(socket|requests|urllib|http\.client|ftplib|smtplib)\b)", QT_TR_NOOP( "network access" ) },
    { R"(\b(import|from)\s+(subprocess|multiprocessing)\b|\bos\.(system|popen|exec[a-z]*|spawn[a-z]*)\s*\(|\bQProcess\b)", QT_TR_NOOP( "process execution" ) },
    { R"(\b(import|from)\s+(shutil|tempfile|pathlib)\b|\bos\.(remove|unlink|rmdir|rename|makedirs|chmod)\s*\(|\bopen\s*\([^)]*['"](w|a|x|r\+))", QT_TR_NOOP( "file write/delete" ) },
    { R"(\b(eval|exec|compile|__import__)\s*\()", QT_TR_NOOP( "dynamic code execution" ) },
    { R"(\b(import|from)\s+ctypes\b)", QT_TR_NOOP( "native code (ctypes)" ) },
    { R"(\bos\.environ\b|\bgetenv\s*\()", QT_TR_NOOP( "environment variables" ) },
  };

  QStringList detected;
  for ( const Marker &marker : markers )
  {
    const QRegularExpression re( QString::fromUtf8( marker.pattern ) );
    if ( re.match( code ).hasMatch() )
      detected << tr( marker.label );
  }
  return detected;
}

QgsAiPythonApprovalDialog::QgsAiPythonApprovalDialog( const QString &description, const QString &code, QWidget *parent )
  : QgsAiPythonApprovalDialog( description, code, false, parent )
{}

QgsAiPythonApprovalDialog::QgsAiPythonApprovalDialog( const QString &description, const QString &code, bool canRememberSessionApproval, QWidget *parent )
  : QDialog( parent )
{
  setWindowTitle( tr( "AI: confirm Python execution" ) );
  resize( 900, 600 );
  setSizeGripEnabled( true );
  setModal( true );

  QVBoxLayout *layout = new QVBoxLayout( this );

  // Permanent warning banner: this is not a sandbox.
  QLabel *banner = new QLabel( this );
  banner->setObjectName( u"aiPythonWarningBanner"_s );
  banner->setTextFormat( Qt::PlainText );
  banner->setWordWrap( true );
  banner->setText( tr( "⚠ This code runs unsandboxed with your full user privileges: it can read or write any file, access the network, and control QGIS." ) );
  banner->setStyleSheet( u"QLabel { background: #fff3cd; color: #664d03; border: 1px solid #ffe69c; border-radius: 4px; padding: 6px; font-weight: bold; }"_s );
  layout->addWidget( banner );

  // Advisory risk chips from the heuristic scan.
  const QStringList riskMarkers = detectRiskMarkers( code );
  if ( !riskMarkers.isEmpty() )
  {
    QLabel *risks = new QLabel( this );
    risks->setObjectName( u"aiPythonRiskMarkersLabel"_s );
    risks->setTextFormat( Qt::PlainText );
    risks->setWordWrap( true );
    risks->setText( tr( "Detected: %1" ).arg( riskMarkers.join( ", "_L1 ) ) );
    risks->setStyleSheet( u"QLabel { color: #842029; font-weight: bold; }"_s );
    layout->addWidget( risks );
  }

  QLabel *intro = new QLabel( this );
  intro->setTextFormat( Qt::PlainText );
  intro->setWordWrap( true );
  const QString why = description.trimmed().isEmpty() ? tr( "(no description provided)" ) : description.trimmed();
  intro->setText( tr(
                    "The AI assistant wants to run the following PyQGIS code in this QGIS session.\n"
                    "It runs with the same privileges as the GUI: it can read/write files, modify the project, and call any QGIS API.\n\n"
                    "Reason: %1"
  )
                    .arg( why ) );
  layout->addWidget( intro );

  // Read-only Python editor with syntax highlighting via QScintilla.
  mEditor = new QgsCodeEditorPython( this );
  mEditor->setText( code );
  mEditor->setReadOnly( true );
  layout->addWidget( mEditor, /*stretch=*/1 );

  QLabel *footer = new QLabel( this );
  footer->setObjectName( u"aiPythonApprovalFooterLabel"_s );
  footer->setTextFormat( Qt::PlainText );
  footer->setWordWrap( true );
  footer->setText(
    canRememberSessionApproval
      ? tr( "Click Run only if you understand and trust this code. This approval will allow later low-risk Python snippets to run without asking again during this app session. High-risk code still asks." )
      : tr( "Click Run only if you understand and trust this code. Click Cancel to refuse." )
  );
  layout->addWidget( footer );

  QDialogButtonBox *buttons = new QDialogButtonBox( this );
  QPushButton *runBtn = buttons->addButton( tr( "Run" ), QDialogButtonBox::AcceptRole );
  QPushButton *cancelBtn = buttons->addButton( tr( "Cancel" ), QDialogButtonBox::RejectRole );

  // Cancel is the safe default.
  cancelBtn->setDefault( true );
  cancelBtn->setAutoDefault( true );
  runBtn->setDefault( false );

  connect( runBtn, &QPushButton::clicked, this, &QDialog::accept );
  connect( cancelBtn, &QPushButton::clicked, this, &QDialog::reject );
  layout->addWidget( buttons );
}
