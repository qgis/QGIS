/******************************************************************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#include "qwt_curve_fitter.h"

/*!
   Constructor
   \param mode Preferred fitting mode
 */
QwtCurveFitter::QwtCurveFitter( Mode mode )
    : m_mode( mode )
{
}

//! Destructor
QwtCurveFitter::~QwtCurveFitter()
{
}

//! \return Preferred fitting mode
QwtCurveFitter::Mode QwtCurveFitter::mode() const
{
    return m_mode;
}
