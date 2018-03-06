#include "qgsprocessingparametertype.h"

bool QgsProcessingParameterType::exposeToModeler() const
{
  return true;
}

QVariantMap QgsProcessingParameterType::metadata() const
{
  return QVariantMap();
}
