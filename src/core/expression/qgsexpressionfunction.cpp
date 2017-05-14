#include "qgsexpressionfunction.h"
#include "qgsexpression.h"

const QString QgsExpressionFunction::helpText() const
{
  return mHelpText.isEmpty() ? QgsExpression::helpText( mName ) : mHelpText;
}
