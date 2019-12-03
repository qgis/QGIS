/*
  Copyright 2008 Brad Hards  <bradh@frogmouth.net>
  Copyright 2009 Inge Wallin <inge@lysator.liu.se>
  Copyright 2015 Ion Vasilief <ion_vasilief@yahoo.fr>
*/

#ifndef EMFPARSER_H
#define EMFPARSER_H

#include "EmfOutput.h"

#include <QString>
#include <QRect> // also provides QSize

/**
   \file

   Primary definitions for EMF parser
*/

/**
   Namespace for Enhanced Metafile (EMF) classes
*/
namespace QEmf
{

/**
	%Parser for an EMF format file
 */
class EmfParser
{
public:
	EmfParser();
	~EmfParser();

	/**
	 * Load an EMF file
	 *
	 * \param fileName the name of the file to load
	 *
	 * \return true on successful load, or false on failure
	 */
	bool load( const QString &fileName );
	/**
	 * Load an EMF file
	 *
	 * \param contents a QByteArray containing the contents of the EMF.
	 *
	 * \return true on successful load, or false on failure
	 */
	bool load(const QByteArray &contents);


	/**
	 * Load an EMF file from a stream
	 *
	 * \param stream the stream to read from
	 *
	 * \return true on successful load, or false on failure
	 */
	bool loadFromStream(QDataStream &stream);

	/**
	   Set the output strategy for the parser

	   \param output pointer to a strategy implementation
	*/
	void setOutput( AbstractOutput *output );

	/**
	   Set the parsing strategy

	   \param headerOnly specify if only the header should be parsed
	*/
	void setParseHeaderOnly(bool headerOnly = true){mHeaderOnly = headerOnly;}

	/**
	   The bounding box of the file content, in device units
	*/
	QRect bounds() const {return mBounds;}

private:
	// read a single EMF record
	bool readRecord( QDataStream &stream );

	// temporary function to soak up unneeded bytes
	void soakBytes( QDataStream &stream, int numBytes );

	// temporary function to dump output bytes
	void outputBytes( QDataStream &stream, int numBytes );

	// Pointer to the output strategy
	AbstractOutput *mOutput;

	// flag specifying if only the header should be parsed
	bool mHeaderOnly;

	// bounding box of the file content, in device units
	QRect mBounds;
};

}

#endif
