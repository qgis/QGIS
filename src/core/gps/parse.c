/*
 *
 * NMEA library
 * URL: http://nmea.sourceforge.net
 * Author: Tim (xtimor@gmail.com)
 * Licence: http://www.gnu.org/licenses/lgpl.html
 * $Id: parse.c 17 2008-03-11 11:56:11Z xtimor $
 *
 */

/**
 * \file parse.h
 * \brief Functions of a low level for analysis of
 * packages of NMEA stream.
 *
 * \code
 * ...
 * ptype = nmea_pack_type(
 *     (const char *)parser->buffer + nparsed + 1,
 *     parser->buff_use - nparsed - 1);
 *
 * if(0 == (node = malloc(sizeof(nmeaParserNODE))))
 *     goto mem_fail;
 *
 * node->pack = 0;
 *
 * switch(ptype)
 * {
 * case GPGGA:
 *     if(0 == (node->pack = malloc(sizeof(nmeaGPGGA))))
 *         goto mem_fail;
 *     node->packType = GPGGA;
 *     if(!nmea_parse_GPGGA(
 *         (const char *)parser->buffer + nparsed,
 *         sen_sz, (nmeaGPGGA *)node->pack))
 *     {
 *         free(node);
 *         node = 0;
 *     }
 *     break;
 * case GPGSA:
 *     if(0 == (node->pack = malloc(sizeof(nmeaGPGSA))))
 *         goto mem_fail;
 *     node->packType = GPGSA;
 *     if(!nmea_parse_GPGSA(
 *         (const char *)parser->buffer + nparsed,
 *         sen_sz, (nmeaGPGSA *)node->pack))
 *     {
 *         free(node);
 *         node = 0;
 *     }
 *     break;
 * ...
 * \endcode
 */

#include "tok.h"
#include "parse.h"
#include "context.h"
#include "gmath.h"
#include "units.h"

#include <string.h>
#include <stdio.h>

int _nmea_parse_time( const char *buff, int buff_sz, nmeaTIME *res )
{
  int success = 0;

  switch ( buff_sz )
  {
    case sizeof( "hhmmss" ) - 1:
      success = ( 3 == nmea_scanf( buff, buff_sz,
                                   "%2d%2d%2d", &( res->hour ), &( res->min ), &( res->sec )
                                 ) );
      break;
    case sizeof( "hhmmss.s" ) - 1:
      success = ( 4 == nmea_scanf( buff, buff_sz,
                                   "%2d%2d%2d.%d", &( res->hour ), &( res->min ), &( res->sec ), &( res->msec )
                                 ) );
      res->msec = res->msec * 100;   // tenths sec * 100 = thousandths
      break;
    case sizeof( "hhmmss.ss" ) - 1:
      success = ( 4 == nmea_scanf( buff, buff_sz,
                                   "%2d%2d%2d.%d", &( res->hour ), &( res->min ), &( res->sec ), &( res->msec )
                                 ) );
      res->msec = res->msec * 10;   // hundredths sec * 10 = thousandths
      break;
    case sizeof( "hhmmss.sss" ) - 1:
      success = ( 4 == nmea_scanf( buff, buff_sz,
                                   "%2d%2d%2d.%d", &( res->hour ), &( res->min ), &( res->sec ), &( res->msec )
                                 ) );
      // already thousandths
      break;
    default:
      nmea_error( "Parse of time error (format error)!" );
      success = 0;
      break;
  }

  return ( success ? 0 : -1 );
}

/**
 * \brief Define packet type by header (nmeaPACKTYPE).
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @return The defined packet type
 * @see nmeaPACKTYPE
 */
int nmea_pack_type( const char *buff, int buff_sz )
{
  static const char *pheads[] =
  {
    "GPGGA",
    "GPGSA",
    "GPGSV",
    "GPRMC",
    "GPVTG",
  };

  NMEA_ASSERT( buff );

  if ( buff_sz < 5 )
    return GPNON;
  else if ( 0 == memcmp( buff, pheads[0], 5 ) )
    return GPGGA;
  else if ( 0 == memcmp( buff, pheads[1], 5 ) )
    return GPGSA;
  else if ( 0 == memcmp( buff, pheads[2], 5 ) )
    return GPGSV;
  else if ( 0 == memcmp( buff, pheads[3], 5 ) )
    return GPRMC;
  else if ( 0 == memcmp( buff, pheads[4], 5 ) )
    return GPVTG;

  return GPNON;
}

/**
 * \brief Find tail of packet ("\r\n") in buffer and check control sum (CRC).
 * @param buff a constant character pointer of packets buffer.
 * @param buff_sz buffer size.
 * @param res_crc a integer pointer for return CRC of packet (must be defined).
 * @return Number of bytes to packet tail.
 */
int nmea_find_tail( const char *buff, int buff_sz, int *res_crc )
{
  static const int tail_sz = 3 /* *[CRC] */ + 2 /* \r\n */;

  const char *end_buff = buff + buff_sz;
  int nread = 0;
  int crc = 0;

  NMEA_ASSERT( buff && res_crc );

  *res_crc = -1;

  for ( ; buff < end_buff; ++buff, ++nread )
  {
    if (( '$' == *buff ) && nread )
    {
      buff = 0;
      break;
    }
    else if ( '*' == *buff )
    {
      if ( buff + tail_sz <= end_buff && '\r' == buff[3] && '\n' == buff[4] )
      {
        *res_crc = nmea_atoi( buff + 1, 2, 16 );
        nread = buff_sz - ( int )( end_buff - ( buff + tail_sz ) );
        if ( *res_crc != crc )
        {
          *res_crc = -1;
          buff = 0;
        }
      }

      break;
    }
    else if ( nread )
      crc ^= ( int ) * buff;
  }

  if ( *res_crc < 0 && buff )
    nread = 0;

  return nread;
}

/**
 * \brief Parse GGA packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
int nmea_parse_GPGGA( const char *buff, int buff_sz, nmeaGPGGA *pack )
{
  char time_buff[NMEA_TIMEPARSE_BUF];

  NMEA_ASSERT( buff && pack );

  memset( pack, 0, sizeof( nmeaGPGGA ) );

  nmea_trace_buff( buff, buff_sz );

  if ( 14 != nmea_scanf( buff, buff_sz,
                         "$GPGGA,%s,%f,%C,%f,%C,%d,%d,%f,%f,%C,%f,%C,%f,%d*",
                         &( time_buff[0] ),
                         &( pack->lat ), &( pack->ns ), &( pack->lon ), &( pack->ew ),
                         &( pack->sig ), &( pack->satinuse ), &( pack->HDOP ), &( pack->elv ), &( pack->elv_units ),
                         &( pack->diff ), &( pack->diff_units ), &( pack->dgps_age ), &( pack->dgps_sid ) ) )
  {
    nmea_error( "GPGGA parse error!" );
    return 0;
  }

  if ( 0 != _nmea_parse_time( &time_buff[0], ( int )strlen( &time_buff[0] ), &( pack->utc ) ) )
  {
    nmea_error( "GPGGA time parse error!" );
    return 0;
  }

  return 1;
}

/**
 * \brief Parse GSA packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
int nmea_parse_GPGSA( const char *buff, int buff_sz, nmeaGPGSA *pack )
{
  NMEA_ASSERT( buff && pack );

  memset( pack, 0, sizeof( nmeaGPGSA ) );

  nmea_trace_buff( buff, buff_sz );

  if ( 17 != nmea_scanf( buff, buff_sz,
                         "$GPGSA,%C,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%f,%f,%f*",
                         &( pack->fix_mode ), &( pack->fix_type ),
                         &( pack->sat_prn[0] ), &( pack->sat_prn[1] ), &( pack->sat_prn[2] ), &( pack->sat_prn[3] ), &( pack->sat_prn[4] ), &( pack->sat_prn[5] ),
                         &( pack->sat_prn[6] ), &( pack->sat_prn[7] ), &( pack->sat_prn[8] ), &( pack->sat_prn[9] ), &( pack->sat_prn[10] ), &( pack->sat_prn[11] ),
                         &( pack->PDOP ), &( pack->HDOP ), &( pack->VDOP ) ) )
  {
    nmea_error( "GPGSA parse error!" );
    return 0;
  }

  return 1;
}

/**
 * \brief Parse GSV packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
int nmea_parse_GPGSV( const char *buff, int buff_sz, nmeaGPGSV *pack )
{
  int nsen, nsat;

  NMEA_ASSERT( buff && pack );

  memset( pack, 0, sizeof( nmeaGPGSV ) );

  nmea_trace_buff( buff, buff_sz );

  nsen = nmea_scanf( buff, buff_sz,
                     "$GPGSV,%d,%d,%d,"
                     "%d,%d,%d,%d,"
                     "%d,%d,%d,%d,"
                     "%d,%d,%d,%d,"
                     "%d,%d,%d,%d*",
                     &( pack->pack_count ), &( pack->pack_index ), &( pack->sat_count ),
                     &( pack->sat_data[0].id ), &( pack->sat_data[0].elv ), &( pack->sat_data[0].azimuth ), &( pack->sat_data[0].sig ),
                     &( pack->sat_data[1].id ), &( pack->sat_data[1].elv ), &( pack->sat_data[1].azimuth ), &( pack->sat_data[1].sig ),
                     &( pack->sat_data[2].id ), &( pack->sat_data[2].elv ), &( pack->sat_data[2].azimuth ), &( pack->sat_data[2].sig ),
                     &( pack->sat_data[3].id ), &( pack->sat_data[3].elv ), &( pack->sat_data[3].azimuth ), &( pack->sat_data[3].sig ) );

  nsat = ( pack->pack_index - 1 ) * NMEA_SATINPACK;
  nsat = ( nsat + NMEA_SATINPACK > pack->sat_count ) ? pack->sat_count - nsat : NMEA_SATINPACK;
  nsat = nsat * 4 + 3 /* first three sentence`s */;

  if ( nsen < nsat || nsen > ( NMEA_SATINPACK * 4 + 3 ) )
  {
    nmea_error( "GPGSV parse error!" );
    return 0;
  }

  return 1;
}

/**
 * \brief Parse RMC packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
int nmea_parse_GPRMC( const char *buff, int buff_sz, nmeaGPRMC *pack )
{
  int nsen;
  char time_buff[NMEA_TIMEPARSE_BUF];

  NMEA_ASSERT( buff && pack );

  memset( pack, 0, sizeof( nmeaGPRMC ) );

  nmea_trace_buff( buff, buff_sz );

  nsen = nmea_scanf( buff, buff_sz,
                     "$GPRMC,%s,%C,%f,%C,%f,%C,%f,%f,%2d%2d%2d,%f,%C,%C*",
                     &( time_buff[0] ),
                     &( pack->status ), &( pack->lat ), &( pack->ns ), &( pack->lon ), &( pack->ew ),
                     &( pack->speed ), &( pack->direction ),
                     &( pack->utc.day ), &( pack->utc.mon ), &( pack->utc.year ),
                     &( pack->declination ), &( pack->declin_ew ), &( pack->mode ) );

  if ( nsen != 13 && nsen != 14 )
  {
    nmea_error( "GPRMC parse error!" );
    return 0;
  }

  if ( 0 != _nmea_parse_time( &time_buff[0], ( int )strlen( &time_buff[0] ), &( pack->utc ) ) )
  {
    nmea_error( "GPRMC time parse error!" );
    return 0;
  }

  if ( pack->utc.year < 90 )
    pack->utc.year += 100;
  pack->utc.mon -= 1;

  return 1;
}

/**
 * \brief Parse VTG packet from buffer.
 * @param buff a constant character pointer of packet buffer.
 * @param buff_sz buffer size.
 * @param pack a pointer of packet which will filled by function.
 * @return 1 (true) - if parsed successfully or 0 (false) - if fail.
 */
int nmea_parse_GPVTG( const char *buff, int buff_sz, nmeaGPVTG *pack )
{
  NMEA_ASSERT( buff && pack );

  memset( pack, 0, sizeof( nmeaGPVTG ) );

  nmea_trace_buff( buff, buff_sz );

  if ( 8 != nmea_scanf( buff, buff_sz,
                        "$GPVTG,%f,%C,%f,%C,%f,%C,%f,%C*",
                        &( pack->dir ), &( pack->dir_t ),
                        &( pack->dec ), &( pack->dec_m ),
                        &( pack->spn ), &( pack->spn_n ),
                        &( pack->spk ), &( pack->spk_k ) ) )
  {
    nmea_error( "GPVTG parse error!" );
    return 0;
  }

  if ( pack->dir_t != 'T' ||
       pack->dec_m != 'M' ||
       pack->spn_n != 'N' ||
       pack->spk_k != 'K' )
  {
    nmea_error( "GPVTG parse error (format error)!" );
    return 0;
  }

  return 1;
}

/**
 * \brief Fill nmeaINFO structure by GGA packet data.
 * @param pack a pointer of packet structure.
 * @param info a pointer of summary information structure.
 */
void nmea_GPGGA2info( nmeaGPGGA *pack, nmeaINFO *info )
{
  NMEA_ASSERT( pack && info );

  info->utc.hour = pack->utc.hour;
  info->utc.min = pack->utc.min;
  info->utc.sec = pack->utc.sec;
  info->utc.msec = pack->utc.msec;
  info->sig = pack->sig;
  info->HDOP = pack->HDOP;
  info->elv = pack->elv;
  info->lat = (( pack->ns == 'N' ) ? pack->lat : -( pack->lat ) );
  info->lon = (( pack->ew == 'E' ) ? pack->lon : -( pack->lon ) );
  info->smask |= GPGGA;
}

/**
 * \brief Fill nmeaINFO structure by GSA packet data.
 * @param pack a pointer of packet structure.
 * @param info a pointer of summary information structure.
 */
void nmea_GPGSA2info( nmeaGPGSA *pack, nmeaINFO *info )
{
  int i, j, nuse = 0;

  NMEA_ASSERT( pack && info );

  info->fix = pack->fix_type;
  info->PDOP = pack->PDOP;
  info->HDOP = pack->HDOP;
  info->VDOP = pack->VDOP;

  for ( i = 0; i < NMEA_MAXSAT; ++i )
  {
    for ( j = 0; j < info->satinfo.inview; ++j )
    {
      if ( pack->sat_prn[i] && pack->sat_prn[i] == info->satinfo.sat[j].id )
      {
        info->satinfo.sat[j].in_use = 1;
        nuse++;
      }
    }
  }

  info->satinfo.inuse = nuse;
  info->smask |= GPGSA;
}

/**
 * \brief Fill nmeaINFO structure by GSV packet data.
 * @param pack a pointer of packet structure.
 * @param info a pointer of summary information structure.
 */
void nmea_GPGSV2info( nmeaGPGSV *pack, nmeaINFO *info )
{
  int isat, isi, nsat;

  NMEA_ASSERT( pack && info );

  if ( pack->pack_index > pack->pack_count ||
       pack->pack_index * NMEA_SATINPACK > NMEA_MAXSAT )
    return;

  if ( pack->pack_index < 1 )
    pack->pack_index = 1;

  info->satinfo.inview = pack->sat_count;

  nsat = ( pack->pack_index - 1 ) * NMEA_SATINPACK;
  nsat = ( nsat + NMEA_SATINPACK > pack->sat_count ) ? pack->sat_count - nsat : NMEA_SATINPACK;

  for ( isat = 0; isat < nsat; ++isat )
  {
    isi = ( pack->pack_index - 1 ) * NMEA_SATINPACK + isat;
    info->satinfo.sat[isi].id = pack->sat_data[isat].id;
    info->satinfo.sat[isi].elv = pack->sat_data[isat].elv;
    info->satinfo.sat[isi].azimuth = pack->sat_data[isat].azimuth;
    info->satinfo.sat[isi].sig = pack->sat_data[isat].sig;
  }

  info->smask |= GPGSV;
}

/**
 * \brief Fill nmeaINFO structure by RMC packet data.
 * @param pack a pointer of packet structure.
 * @param info a pointer of summary information structure.
 */
void nmea_GPRMC2info( nmeaGPRMC *pack, nmeaINFO *info )
{
  NMEA_ASSERT( pack && info );

  if ( 'A' == pack->status )
  {
    if ( NMEA_SIG_BAD == info->sig )
      info->sig = NMEA_SIG_MID;
    if ( NMEA_FIX_BAD == info->fix )
      info->fix = NMEA_FIX_2D;
  }
  else if ( 'V' == pack->status )
  {
    info->sig = NMEA_SIG_BAD;
    info->fix = NMEA_FIX_BAD;
  }

  info->utc = pack->utc;
  info->lat = (( pack->ns == 'N' ) ? pack->lat : -( pack->lat ) );
  info->lon = (( pack->ew == 'E' ) ? pack->lon : -( pack->lon ) );
  info->speed = pack->speed * NMEA_TUD_KNOTS;
  info->direction = pack->direction;
  info->smask |= GPRMC;
}

/**
 * \brief Fill nmeaINFO structure by VTG packet data.
 * @param pack a pointer of packet structure.
 * @param info a pointer of summary information structure.
 */
void nmea_GPVTG2info( nmeaGPVTG *pack, nmeaINFO *info )
{
  NMEA_ASSERT( pack && info );

  info->direction = pack->dir;
  info->declination = pack->dec;
  info->speed = pack->spk;
  info->smask |= GPVTG;
}
