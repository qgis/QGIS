/*
  Copyright 2008 Brad Hards  <bradh@frogmouth.net>
  Copyright 2009 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#include "EmfOutput.h"
#include "EmfParser.h"

namespace QEmf
{

bool AbstractOutput::load(const QByteArray &contents)
{
	EmfParser parser;
	parser.setOutput(this);
	return parser.load(contents);
}

bool AbstractOutput::load(const QString& fileName)
{
	EmfParser parser;
	parser.setOutput(this);
	return parser.load(QString(fileName));
}

} // xnamespace...
