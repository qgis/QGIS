#WCS response decoder.
#Decodes response from a WCS (either a Coverages XML document or a Multipart MIME) and extracts the urls of the coverage data.
#Copyright (c) 2007 STFC <http://www.stfc.ac.uk>
#Author: Dominic Lowe, STFC
#contact email: d.lowe@rl.ac.uk
#
# Multipart MIME decoding based on http://aspn.activestate.com/ASPN/Cookbook/Python/Recipe/86676

#example: used in conjunction with ows lib wcs:

#from owslib import wcsdecoder
#u=wcs.getcoverage(identifier=['TuMYrRQ4'], timeSequence=['2792-06-01T00:00:00.0'], bbox=(-112,36,-106,41),format='application/netcdf', store='true')
#decoder=wcsdecoder.WCSDecoder(u)
#decoder.getCoverages()

import os
from owslib.etree import etree
import email
import errno

class WCSDecoder(object):
    def __init__(self, u):
        ''' initiate with a urllib  url object.'''
        self.u=u
        self._getType()

    def _getType(self):
        ''' determine whether it is a Multipart Mime or a Coverages XML file'''
        
        #what's the best way to test this? 
        #for now read start of file
        tempu=self.u
        if tempu.readline()[:14] == '<?xml version=':              
            self.urlType='XML'       
        else:
            self.urlType='Multipart'
        
      
    def getCoverages(self, unpackdir='./unpacked'):
        if self.urlType=='XML': 
            paths=[]              
            u_xml = self.u.read()
            u_tree = etree.fromstring(u_xml)
            for ref in u_tree.findall('{http://www.opengis.net/wcs/1.1}Coverage/{http://www.opengis.net/wcs/1.1}Reference'):
                path = ref.attrib['{http://www.w3.org/1999/xlink}href']
                paths.append(path)         
            for ref in u_tree.findall('{http://www.opengis.net/wcs/1.1.0/owcs}Coverage/{{http://www.opengis.net/wcs/1.1.0/owcs}Reference'):
                path = ref.attrib['{http://www.w3.org/1999/xlink}href']
                paths.append(path)         
        elif self.urlType=='Multipart':
            #Decode multipart mime and return fileobjects
            u_mpart=self.u.read()
            mpart =MpartMime(u_mpart)
            paths= mpart.unpackToDir(unpackdir)
        return paths

class MpartMime(object):
    def __init__ (self,mpartmime):
        """ mpartmime is a multipart mime file  that has already been read in."""
        self.mpartmime=mpartmime
        
    def unpackToDir(self, unpackdir):
        """ unpacks contents of Multipart mime to a given directory"""
        
        names=[]
        #create the directory if it doesn't exist:
        try:
            os.mkdir(unpackdir)
        except OSError, e:
            # Ignore directory exists error
            if e.errno <> errno.EEXIST:
                raise
               
        #now walk through the multipart mime and write out files
        msg = email.message_from_string(self.mpartmime)
        counter =1
        for part in msg.walk():
            # multipart/* are just containers, ignore
            if part.get_content_maintype() == 'multipart':
                continue            
            # Applications should really check the given filename so that an
            # email message can't be used to overwrite important files
            filename = part.get_filename()
            if not filename:
                try:
                    ext = mimetypes.guess_extension(part.get_type())
                except:
                    ext=None
                if not ext:
                    # Use a generic extension
                    ext = '.bin'
                filename = 'part-%03d%s' % (counter, ext)                
            fullpath=os.path.join(unpackdir, filename)
            names.append(fullpath)
            fp = open(fullpath, 'wb')
            fp.write(part.get_payload(decode=True))
            fp.close()
        return names
