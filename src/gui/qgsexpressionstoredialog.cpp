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
    QString errorMessage;
    if ( mExistingLabels.contains( text ) )
    {
      errorMessage = tr( "A stored expression with this name already exists" );
    }
    else if ( text.contains( '/' ) || text.contains( '\\' ) )
    {
      errorMessage = tr( "Labels cannot contain slashes (/ or \\)" );
    }
    if ( ! errorMessage.isEmpty() )
    {
      mValidationError->show();
      mValidationError->setText( errorMessage );
      saveBtn->setEnabled( false );
    }
    else
    {
      mValidationError->hide();
      saveBtn->setEnabled( true );
    }
  } );
  // No slashes in labels!
  QString labelFixed { label };
  labelFixed.remove( '/' ).remove( '\\' );
  mLabel->setText( labelFixed );
}

