  Liberation Fonts
  =================

  The Liberation Fonts is font collection which aims to provide document 
  layout compatibility as usage of Times New Roman, Arial, Courier New.


  Requirements
  =================

  * [fontforge](http://fontforge.sourceforge.net)
  * [python fonttools](https://pypi.org/project/fonttools/)


 Install
  ============

  1. Get source
        ```
   	    $ git clone https://github.com/liberationfonts/liberation-fonts.git
        ```
    
     - Or downloading the tar.gz file from [releases](https://github.com/fontforge/fontforge/releases).

     - Extract the tar file:
  	    ```
	    $ tar zxvf liberation-fonts-[VERSION].tar.gz
        ```
  2. Build from the source
        ```
    	$ cd liberation-fonts    or   $ cd liberation-fonts-[VERSION]
    	$ make
    	```	
     The binary font files will be available in 'liberation-fonts-ttf-[VERSION]' directory.

  3. Install to system
        
        Fedora Users : 
        - One can manually install the fonts by copying the TTFs to `~/.fonts` for user wide usage 
        - and/or to `/usr/share/fonts/liberation` for system-wide availability. 
        - Then, run `fc-cache` to let that cached.

        Other distributions : 
        please check out corresponding documentation.


  Usage
  ==========

  Simply select preferred liberation font in applications and start using.


   License
  ============

  This Font Software is licensed under the SIL Open Font License,
  Version 1.1.

  Please read file "LICENSE" for details.


   For Maintainers
  ====================

  1. Before packaging a new release based on a new source tarball, you have to update the version suffix in the Makefile:
        ```
        VER = [NEW_VERSION]
        ```
  2. After updating Makefile VER attribute, update all font metadata by executing:
        ```
        $ make versionupdate
        ```
        can verfy changes using ftinfo/otfinfo or fontforge itself. 
  3. It is highly recommended that file 'ChangeLog' is updated to reflect changes.

  4. Create a tarball with the following command:
        ```
        $ make dist
        ```
        The new versioned tarball will be available in the dist/ folder as `liberation-fonts-[NEW_VERSION].tar.gz.`
  5. Create github tag for that [NEW_VERSION] and upload dist tarball 

  Credits
 ============

  Please read file "AUTHORS" for list of contributors.
