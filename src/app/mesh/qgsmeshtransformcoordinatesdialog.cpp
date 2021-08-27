#include "qgsmeshtransformcoordinatesdialog.h"

#include "qgsgui.h"
#include "qgsexpressioncontextutils.h"
#include "qgsmesheditor.h"
#include "qgsmeshlayer.h"
#include "qgsmeshadvancedediting.h"
#include "qgsproject.h"
#include "qgsguiutils.h"
#include "qgshelp.h"

QgsMeshTransformCoordinatesDialog::QgsMeshTransformCoordinatesDialog( QWidget *parent ):
  QDialog( parent )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  setWindowTitle( tr( "Transform Mesh Vertices by Expression" ) );

  mExpressionLineEdits << mExpressionEditX << mExpressionEditY << mExpressionEditZ;
  mCheckBoxes << mCheckBoxX << mCheckBoxY << mCheckBoxZ;

  Q_ASSERT( mExpressionLineEdits.count() == mCheckBoxes.count() );

  for ( int i = 0; i < mExpressionLineEdits.count(); ++i )
  {
    mExpressionLineEdits.at( i )->registerExpressionContextGenerator( this );
    mExpressionLineEdits.at( i )->setEnabled( mCheckBoxes.at( i )->isChecked() );
    connect( mCheckBoxes.at( i ), &QCheckBox::toggled, mExpressionLineEdits.at( i ), &QWidget::setEnabled );

    connect( mExpressionLineEdits.at( i ), &QgsExpressionLineEdit::expressionChanged, this, &QgsMeshTransformCoordinatesDialog::updateButton );
    connect( mCheckBoxes.at( i ), &QCheckBox::toggled, this, &QgsMeshTransformCoordinatesDialog::updateButton );
  }

  connect( mButtonPreview, &QToolButton::clicked, this, &QgsMeshTransformCoordinatesDialog::calculate );
  connect( mButtonApply, &QPushButton::clicked, this, &QgsMeshTransformCoordinatesDialog::apply );
  connect( mButtonClose, &QPushButton::clicked, this, &QDialog::close );

  connect( mButtonHelp, &QDialogButtonBox::helpRequested, this, &QgsMeshTransformCoordinatesDialog::showHelp );

}

QgsExpressionContext QgsMeshTransformCoordinatesDialog::createExpressionContext() const
{
  return QgsExpressionContext( {QgsExpressionContextUtils::meshExpressionScope()} );
}

QgsMeshVertex QgsMeshTransformCoordinatesDialog::transformedVertex( int i )
{
  if ( ! mInputLayer || !mIsCalculated )
    return QgsMeshVertex();

  return mTransformVertices.transformedVertices( mInputLayer, i );
}

bool QgsMeshTransformCoordinatesDialog::isResultValid() const
{
  return mIsResultValid;
}

bool QgsMeshTransformCoordinatesDialog::isCalculated() const
{
  return mIsCalculated;
}

void QgsMeshTransformCoordinatesDialog::setInput( QgsMeshLayer *layer, const QList<int> &vertexIndexes )
{
  mInputLayer = layer;
  mInputVertices = vertexIndexes;
  mIsCalculated = false;
  mIsResultValid = false;
  updateButton();
  emit calculationUpdated();
}

void QgsMeshTransformCoordinatesDialog::calculate()
{
  if ( !mInputLayer || mInputVertices.isEmpty() )
    return;

  QgsTemporaryCursorOverride busyCursor( Qt::WaitCursor );
  mTransformVertices.clear();
  mTransformVertices.setInputVertices( mInputVertices );
  mTransformVertices.setExpressions( mCheckBoxX->isChecked() ? mExpressionEditX->expression() : QString(),
                                     mCheckBoxY->isChecked() ? mExpressionEditY->expression() : QString(),
                                     mCheckBoxZ->isChecked() ? mExpressionEditZ->expression() : QString() );
  QgsExpressionContext context;
  context.appendScope( QgsExpressionContextUtils::projectScope( QgsProject::instance() ) );

  mIsResultValid = mTransformVertices.calculate( mInputLayer );

  mIsCalculated = true;
  mButtonApply->setEnabled( mIsResultValid );

  emit calculationUpdated();
}

void QgsMeshTransformCoordinatesDialog::updateButton()
{
  mButtonApply->setEnabled( false );
  bool isCalculable = mInputLayer && !mInputVertices.isEmpty();
  if ( isCalculable )
  {
    for ( const QCheckBox *cb : std::as_const( mCheckBoxes ) )
      isCalculable |= cb->isChecked();

    if ( isCalculable )
    {
      for ( int i = 0; i < mCheckBoxes.count(); ++i )
      {
        bool checked = mCheckBoxes.at( i )->isChecked();
        isCalculable &= ( !checked ) || ( checked && mExpressionLineEdits.at( i )->isValidExpression() );
      }
    }
  }

  mButtonPreview->setEnabled( isCalculable );
}

void QgsMeshTransformCoordinatesDialog::apply()
{
  emit aboutToBeApplied();
  QgsTemporaryCursorOverride busyCursor( Qt::WaitCursor );
  if ( mIsResultValid && mInputLayer && mInputLayer->meshEditor() )
    mInputLayer->meshEditor()->advancedEdit( & mTransformVertices );
  emit applied();
}

void QgsMeshTransformCoordinatesDialog::showHelp() const
{
  QgsHelp::openHelp( QStringLiteral( "introduction/general_tools.html" ) );
}
