/***************************************************************************
                              qgslayoutvaliditychecks.h
                              ---------------------------
    begin                : November 2018
    copyright            : (C) 2018 Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/
/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef QGSLAYOUTVALIDITYCHECKS_H
#define QGSLAYOUTVALIDITYCHECKS_H

// We don't want to expose this in the public API
#define SIP_NO_FILE

#include "qgis_gui.h"
#include "qgsabstractvaliditycheck.h"

/**
 * \ingroup gui
 * \brief Layout scalebar validity check
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutScaleBarValidityCheck : public QgsAbstractValidityCheck
{
  public:
    //! constructor
    QgsLayoutScaleBarValidityCheck *create() const override;
    QString id() const override;
    int checkType() const override;
    bool prepareCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) override;
    QList<QgsValidityCheckResult> runCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) override;

  private:
    QList<QgsValidityCheckResult> mResults;
};


/**
 * \ingroup gui
 * \brief Layout north arrow validity check
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutNorthArrowValidityCheck : public QgsAbstractValidityCheck
{
  public:
    //! constructor
    QgsLayoutNorthArrowValidityCheck *create() const override;
    QString id() const override;
    int checkType() const override;
    bool prepareCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) override;
    QList<QgsValidityCheckResult> runCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) override;

  private:
    QList<QgsValidityCheckResult> mResults;
};


/**
 * \ingroup gui
 * \brief Layout overview validity check
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutOverviewValidityCheck : public QgsAbstractValidityCheck
{
  public:
    //! constructor
    QgsLayoutOverviewValidityCheck *create() const override;
    QString id() const override;
    int checkType() const override;
    bool prepareCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) override;
    QList<QgsValidityCheckResult> runCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) override;

  private:
    QList<QgsValidityCheckResult> mResults;
};


/**
 * \ingroup gui
 * \brief Layout picture source validity check
 *
 * \note This class is not a part of public API
 * \since QGIS 3.12
 */
class GUI_EXPORT QgsLayoutPictureSourceValidityCheck : public QgsAbstractValidityCheck
{
  public:
    //! constructor
    QgsLayoutPictureSourceValidityCheck *create() const override;
    QString id() const override;
    int checkType() const override;
    bool prepareCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) override;
    QList<QgsValidityCheckResult> runCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) override;

  private:
    QList<QgsValidityCheckResult> mResults;
};

#ifndef WITH_QTWEBKIT
/**
 * \ingroup gui
 * \brief Layout HTML item validity check
 *
 * \note This class is not a part of public API
 * \since QGIS 3.40
 */
class GUI_EXPORT QgsLayoutHtmlItemValidityCheck : public QgsAbstractValidityCheck
{
  public:
    //! constructor
    QgsLayoutHtmlItemValidityCheck *create() const override;
    QString id() const override;
    int checkType() const override;
    bool prepareCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) override;
    QList<QgsValidityCheckResult> runCheck( const QgsValidityCheckContext *context, QgsFeedback *feedback ) override;

  private:
    QList<QgsValidityCheckResult> mResults;
};
#endif

#endif // QGSLAYOUTVALIDITYCHECKS_H
