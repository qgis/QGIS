#include "qgsprocessingparametertype.h"

bool QgsProcessingParameterType::exposeToModeller() const
{
  return true;
}

QVariantMap QgsProcessingParameterType::metadata() const
{
  return QVariantMap();
}
