/***************************************************************************
                         qgsnativealgorithms.h
                         ---------------------
    begin                : April 2017
    copyright            : (C) 2020 by Wang Peng
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSNATIVEALGORITHMS_H
#define QGSNATIVEALGORITHMS_H

#include "qgis_analysis.h"
#include "qgis_sip.h"
#include "qgsprocessingprovider.h"

/**
 * \ingroup analysis
 * \class QgsNativeAlgorithms
 * \brief Native c++ processing algorithm provider.
 * \since QGIS 3.0
 */
class ANALYSIS_EXPORT QgsNativeAlgorithms: public QgsProcessingProvider
{
    Q_OBJECT

  public:

    /**
     * Constructor for QgsNativeAlgorithms.
     */
    QgsNativeAlgorithms( QObject *parent = nullptr );

    QIcon icon() const override;
    QString svgIconPath() const override;
    QString id() const override;
    QString helpId() const override;
    QString name() const override;
    bool supportsNonFileBasedOutput() const override;

  protected:

    void loadAlgorithms() override;

};

#endif // QGSNATIVEALGORITHMS_H


