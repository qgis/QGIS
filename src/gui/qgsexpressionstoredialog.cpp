#include "qgsexpressionstoredialog.h"
#include <QPushButton>
#include <QStyle>

QgsExpressionStoreDialog::QgsExpressionStoreDialog( const QString &label, const QString &expression, const QString &helpText, const QStringList &existingLabels, QWidget *parent )
  : QDialog( parent )
  , mExistingLabels( existingLabels )
{
  setupUi( this );
  mExpression->setText( expression );
  mHelpText->setText( helpText );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsExpressionStoreDialog::accept );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsExpressionStoreDialog::reject );
  mValidationError->hide();
  mValidationError->setStyleSheet( QStringLiteral( "QLabel { color : red; }" ) );
  QPushButton *saveBtn { buttonBox->button( QDialogButtonBox::StandardButton::Save ) };
  saveBtn->setEnabled( false );
  connect( mLabel, &QLineEdit::textChanged, this, [ = ]( const QString & text )
  {
    if ( mExistingLabels.contains( text ) )
    {
      mValidationError->show();
      saveBtn->setEnabled( false );
    }
    else
    {
      mValidationError->hide();
      saveBtn->setEnabled( true );
    }
  } );
  mLabel->setText( label );
}

