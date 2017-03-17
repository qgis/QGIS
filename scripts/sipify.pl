#!/usr/bin/perl
use strict;
use warnings;
use File::Basename;

my $headerfile = $ARGV[0];

open(my $header, "<", $headerfile) || die "Couldn't open '".$headerfile."' for reading because: ".$!;

my $GLOBAL = 1;
my $DOXYGEN_BLOCK = 2;

# TODO add contexts for
# "private: " -> skip
# "function bodies" -> skip
# "multiline function signatures"
# "enums" -> remove assigned values

my $context = $GLOBAL;
my $comment;

while(!eof $header) {
    my $line = readline $header;

    # Skip preprocessor stuff
    if ($line =~ /^\s*#/) {
        if ($line !~ /NO_SIPIFY/) {
            next;
        }
    }
    # Skip forward declarations
    if ($line =~ m/^\s*class \w+;$/) {
        next;
    }
    # Skip Q_OBJECT, Q_PROPERTY, Q_ENUM, Q_GADGET
    if ($line =~ m/^\s*Q_(OBJECT|ENUMS|PROPERTY|GADGET).*?$/) {
        next;
    }

    # Doxygen comment started
    if ($DOXYGEN_BLOCK != $context) {
        if ($line =~ m/\s*\/\*\*/) {
            $context = $DOXYGEN_BLOCK;
            $comment = "";
        }
        # class declaration started
        if ($line =~ m/^(\s*class)\s*([A-Z]+_EXPORT)(\s+\w+)(\s*\:.*)?$/) {
            $line = "$1$3";
            # Inheritance
            if ($4) {
                my $m;
                $m = $4;
                $m =~ s/public //g;
                $m =~ s/\s?private \w+,?//;
                $line .= $m;
            }
            $line .= "\n{\n%TypeHeaderCode\n#include \"" . basename($headerfile) . "\"\n%End";
            $line .= "\n%Docstring\n$comment\n%End\n";

            my $skip;
            # Skip opening curly bracket, we already added that above
            $skip = readline $header;
            $skip =~ m/^\s*{\s$/ || die "Unexpected content on line $line";
        }

        $line =~ s/SIP_FACTORY/\/Factory\//;
        $line =~ s/SIP_SKIP/\/\//;
        $line =~ s/SIP_OUT/\/Out\//g;
        $line =~ s/SIP_INOUT/\/In,Out\//g;
        $line =~ s/SIP_TRANSFER/\/Transfer\//g;
        $line =~ s/SIP_PYNAME\((\w+)\)/\/PyName=$1\//;
        $line =~ s/SIP_KEEPREFERENCE\((\w+)\)/\/KeepReference\//;
        $line =~ s/SIP_TRANSFERTHIS\((\w+)\)/\/TransferThis\//;
        $line =~ s/SIP_TRANSFERBACK\((\w+)\)/\/TransferBack\//;

        $line =~ s/override;/;/g;
    }

    # In block comment
    if ($DOXYGEN_BLOCK == $context) {
        if ($line =~ m/\*\//) {
            $context = $GLOBAL;
        }
        $line =~ m/\s*\*?(.*)/;
        $comment .= "$1\n";
        next;
    }

    print $line;
}
