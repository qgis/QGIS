#!/usr/bin/perl
use strict;
use warnings;
use File::Basename;

# TODO add contexts for
# "multiline function signatures"
# docustrings for QgsFeature::QgsAttributes

my $headerfile = $ARGV[0];

open(my $header, "<", $headerfile) || die "Couldn't open '".$headerfile."' for reading because: ".$!;

# contexts
my $SIP_RUN = 0;
my $HEADER_CODE = 0;
my $PRIVATE_SECTION = 0;
my $MULTILINE_DEFINITION = 0;

my $comment = '';
my $nesting_index = 0;
my $private_section_line = '';
my $line;

print "/******************************************************************\n";
print " * This file has been generated automatically by sipify.pl        *\n";
print " * Do not edit manually ! Edit header file and generate it again. *\n";
print " *****************************************************************/\n";


while(!eof $header){
    $line = readline $header;
    #print $line;

    # Skip preprocessor stuff
    if ($line =~ m/^\s*#/){
        if ( $line =~ m/^\s*#ifdef SIP_RUN/){
            $SIP_RUN = 1;
            if ($PRIVATE_SECTION == 1){
                print $private_section_line;
            }
            next;
        }
        if ( $SIP_RUN == 1 ){
            if ( $line =~ m/^\s*#endif/ ){
                if ( $nesting_index == 0 ){
                    $SIP_RUN = 0;
                    next;
                }
                else {
                    $nesting_index--;
                }
            }
            if ( $line =~ m/^\s*#if(def)?\s+/ ){
                $nesting_index++;
            }

            # if there is an else at this level, code will be ignored i.e. not SIP_RUN
            if ( $line =~ m/^\s*#else/ && $nesting_index == 0){
                while(!eof $header){
                    $line = readline $header;
                    if ( $line =~ m/^\s*#if(def)?\s+/ ){
                        $nesting_index++;
                    }
                    elsif ( $line =~ m/^\s*#endif/ ){
                        if ( $nesting_index == 0 ){
                            $SIP_RUN = 0;
                            last;
                        }
                        else {
                            $nesting_index--;
                        }
                    }
                }
                next;
            }
        }
        elsif ( $line =~ m/^\s*#ifndef SIP_RUN/){
            # code is ignored here
            while(!eof $header){
                $line = readline $header;
                if ( $line =~ m/^\s*#if(def)?\s+/ ){
                    $nesting_index++;
                }
                elsif ( $line =~ m/^\s*#else/ && $nesting_index == 0 ){
                    # code here will be printed out
                    if ($PRIVATE_SECTION == 1){
                        print $private_section_line;
                    }
                    $SIP_RUN = 1;
                    last;
                }
                elsif ( $line =~ m/^\s*#endif/ ){
                    if ( $nesting_index == 0 ){
                        $SIP_RUN = 0;
                        last;
                    }
                    else {
                        $nesting_index--;
                    }
                }
            }
            next;
        }
        else {
            next;
        }
    }

    # TYPE HEADER CODE
    if ( $HEADER_CODE && $SIP_RUN == 0 ){
        $HEADER_CODE = 0;
        print "%End\n";
    }

    # Skip forward declarations
    if ($line =~ m/^\s*class \w+;$/){
        next;
    }
    # Skip Q_OBJECT, Q_PROPERTY, Q_ENUM, Q_GADGET
    if ($line =~ m/^\s*Q_(OBJECT|ENUMS|PROPERTY|GADGET|DECLARE_METATYPE).*?$/){
        next;
    }

    # SIP_SKIP
    if ( $line =~ m/SIP_SKIP/ ){
      next;
    }

    # Private members (exclude SIP_RUN)
    if ( $line =~ m/^\s*private( slots)?:/ ){
        $PRIVATE_SECTION = 1;
        $private_section_line = $line;
        next;
    }
    if ( $PRIVATE_SECTION == 1 ){
        if ( $SIP_RUN == 0){
            if ( $line =~ m/^\s*(public|protected)( slots)?:.*$/ || $line =~ m/^\s*\};.*$/){
                $PRIVATE_SECTION = 0;
            }
            else {
                next;
            }
        }
    }

    # Detect comment block
    if ($line =~ m/^\s*\/\*/){
        do {no warnings 'uninitialized';
            $comment = $line =~ s/^\s*\/\*(\*)?(.*)$/$2/r;
        };
        $comment =~ s/^\s*$//;
        while(!eof $header){
            $line = readline $header;
            $comment .= $line =~ s/\s*\*?(.*?)(\/)?$/$1/r;
            if ( $line =~ m/\*\/$/ ){
                last;
            }
        }
        $comment =~ s/(\n)+$//;
        #print $comment;
        next;
    }

    # save comments and do not print them, except in SIP_RUN
    if ( $SIP_RUN == 0 ){
        if ( $line =~ m/^\s*\/\// ){
            $line =~ s/^\s*\/\/\!*\s*(.*)\n?$/$1/;
            $comment = $line;
            next;
        }
    }

    # class declaration started
    if ( $line =~ m/^(\s*class)\s*([A-Z]+_EXPORT)(\s+\w+)(\s*\:.*)?$/ ){
        $line = "$1$3";
        # Inheritance
        if ($4){
            my $m = $4;
            $m =~ s/public //g;
            $m =~ s/,?\s*private \w+//;
            $m =~ s/(\s*:)?\s*$//;
            $line .= $m;
        }

        $line .= "\n{\n";
        if ( $comment !~ m/^\s*$/ ){
            $line .= "%Docstring\n$comment\n%End\n";
        }
        $line .= "\n%TypeHeaderCode\n#include \"" . basename($headerfile) . "\"\n";

        print $line;

        my $skip;
        # Skip opening curly bracket, we already added that above
        $skip = readline $header;
        $skip =~ m/^\s*{\s$/ || die "Unexpected content on line $line";

        $comment = '';
        $HEADER_CODE = 1;
        next;
    }

    # Enum declaration
    if ( $line =~ m/^\s*enum\s+\w+.*?$/ ){
        print $line;
        $line = readline $header;
        $line =~ m/^\s*\{\s*$/ || die 'Unexpected content: enum should be followed by {';
        print $line;
        while(!eof $header){
            $line = readline $header;
            if ($line =~ m/\};/){
                last;
            }
            $line =~ s/(\s*\w+)(\s*=\s*\w+.*?)?(,?).*$/$1$3/;
            print $line;
        }
        print $line;
        # enums don't have Docstring apparently
        next;
    }

    do {no warnings 'uninitialized';
        # remove keywords
        $line =~ s/\s*override( SIP_\w+(\(.+\))?)?;/$1;/;
        $line =~ s/^(\s*)?(const )?(virtual |static )?inline /$1$2$3/;
        $line =~ s/\bnullptr\b/0/g;

        # remove constructor definition
        if ( $line =~  m/^(\s*)?(explicit )?(\w+)\(([^()]*\([^()]*\)[^()]*)*\)(?!;)$/ ){
            my $newline = $line =~ s/\n/;\n/r;
            my $nesting_index = 0;
            while(!eof $header){
                $line = readline $header;
                if ( $nesting_index == 0 ){
                    if ( $line =~ m/^\s*(:|,)/ ){
                        next;
                    }
                    $line =~ m/^\s*\{/ or die 'Constructor definition misses {';
                    if ( $line =~ m/^\s*\{.*?\}/ ){
                        last;
                    }
                    $nesting_index = 1;
                    next;
                }
                else {
                    $nesting_index += $line =~ tr/\{//;
                    $nesting_index -= $line =~ tr/\}//;
                    if ($nesting_index eq 0){
                        last;
                    }
                }
            }
            $line = $newline;
        }

        # remove function bodies
        if ( $line =~  m/^(\s*)?(const )?(virtual |static )?((\w+(<.*?>)?\s+(\*|&)?)?(\w+|operator.)\(.*?(\(.*\))*.*\)( const)?)\s*(\{.*\})?(?!;)(\s*\/\/.*)?$/ ){
            my $newline = "$1$2$3$4;\n";
            if ($line !~ m/\{.*?\}$/){
                $line = readline $header;
                if ( $line =~ m/^\s*\{\s*$/ ){
                    while(!eof $header){
                        $line = readline $header;
                        if ( $line =~ m/\}\s*$/ ){
                            last;
                        }
                    }
                }
            }
            $line = $newline;
        }
    };

    # deleted functions
    if ( $line =~  m/^(\s*)?(const )?(virtual |static )?((\w+(<.*?>)?\s+(\*|&)?)?(\w+|operator.)\(.*?(\(.*\))*.*\)( const)?)\s*= delete;(\s*\/\/.*)?$/ ){
      $comment = '';
      next;
    }

    $line =~ s/\bSIP_FACTORY\b/\/Factory\//;
    $line =~ s/\bSIP_OUT\b/\/Out\//g;
    $line =~ s/\bSIP_INOUT\b/\/In,Out\//g;
    $line =~ s/\bSIP_TRANSFER\b/\/Transfer\//g;
    $line =~ s/\bSIP_KEEPREFERENCE\b/\/KeepReference\//;
    $line =~ s/\bSIP_TRANSFERTHIS\b/\/TransferThis\//;
    $line =~ s/\bSIP_TRANSFERBACK\b/\/TransferBack\//;

    $line =~ s/SIP_PYNAME\(\s*(\w+)\s*\)/\/PyName=$1\//;
    $line =~ s/(\w+)(\<(?>[^<>]|(?2))*\>)?\s+SIP_PYTYPE\(\s*\'?([^()']+)(\(\s*(?:[^()]++|(?2))*\s*\))?\'?\s*\)/$3/g;
    $line =~ s/=\s+[^=]*?\s+SIP_PYDEFAULTVALUE\(\s*\'?([^()']+)(\(\s*(?:[^()]++|(?2))*\s*\))?\'?\s*\)/= $1/g;

    # fix astyle placing space after % character
    $line =~ s/\s*% (MappedType|TypeHeaderCode|ConvertFromTypeCode|ConvertToTypeCode|MethodCode|End)/%$1/;
    $line =~ s/\/\s+GetWrapper\s+\//\/GetWrapper\//;

    print $line;

    # multiline definition (parenthesis left open)
    if ( $MULTILINE_DEFINITION == 1 ){
      if ( $line =~ m/^[^()]*([^()]*\([^()]*\)[^()]*)*\)[^()]*$/){
          $MULTILINE_DEFINITION = 0;
      }
      else
      {
        next;
      }
    }
    elsif ( $line =~ m/^[^()]+\([^()]*([^()]*\([^()]*\)[^()]*)*[^)]*$/ ){
      $MULTILINE_DEFINITION = 1;
      next;
    }

    # write comment
    if ( $line =~ m/^\s*$/ || $line =~ m/\/\// || $line =~ m/\s*typedef / ){
        $comment = '';
    }
    elsif ( $comment !~ m/^\s*$/ ){
        print "%Docstring\n$comment\n%End\n";
        $comment = '';
    }
}
