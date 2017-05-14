#include "qgsexpressionnodeimpl.h"

QString QgsExpressionNodeBinaryOperator::text() const
{
  return BINARY_OPERATOR_TEXT[mOp];
}
