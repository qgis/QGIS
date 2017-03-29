class QgsProcessingAlgorithm
{
%TypeHeaderCode
#include <qgsprocessingalgorithm.h>
%End

  public:

    QgsProcessingAlgorithm();
    virtual ~QgsProcessingAlgorithm();
    virtual QStringList tags() const;
    virtual QIcon icon() const;
    virtual QString svgIconPath() const;
};

