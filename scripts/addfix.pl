#!/usr/bin/perl
use strict;
use warnings;

my @fixes = qw/
lib2to3.fixes.fix_apply
lib2to3.fixes.fix_basestring
lib2to3.fixes.fix_dict
lib2to3.fixes.fix_except
lib2to3.fixes.fix_exec
lib2to3.fixes.fix_exitfunc
lib2to3.fixes.fix_filter
lib2to3.fixes.fix_funcattrs
lib2to3.fixes.fix_getcwdu
lib2to3.fixes.fix_has_key
lib2to3.fixes.fix_idioms
lib2to3.fixes.fix_input
lib2to3.fixes.fix_intern
lib2to3.fixes.fix_isinstance
lib2to3.fixes.fix_itertools
lib2to3.fixes.fix_itertools_imports
lib2to3.fixes.fix_long
lib2to3.fixes.fix_map
lib2to3.fixes.fix_methodattrs
lib2to3.fixes.fix_ne
lib2to3.fixes.fix_nonzero
lib2to3.fixes.fix_numliterals
lib2to3.fixes.fix_operator
lib2to3.fixes.fix_paren
lib2to3.fixes.fix_raw_input
lib2to3.fixes.fix_reduce
lib2to3.fixes.fix_renames
lib2to3.fixes.fix_repr
lib2to3.fixes.fix_standarderror
lib2to3.fixes.fix_sys_exc
lib2to3.fixes.fix_throw
lib2to3.fixes.fix_tuple_params
lib2to3.fixes.fix_types
lib2to3.fixes.fix_ws_comma
lib2to3.fixes.fix_xreadlines
lib2to3.fixes.fix_zip

libfuturize.fixes.fix_cmp
libfuturize.fixes.fix_execfile
libfuturize.fixes.fix_future_builtins
libfuturize.fixes.fix_future_standard_library
libfuturize.fixes.fix_future_standard_library_urllib
libfuturize.fixes.fix_metaclass
libfuturize.fixes.fix_next_call
libfuturize.fixes.fix_object
libfuturize.fixes.fix_raise
libfuturize.fixes.fix_xrange_with_import

libpasteurize.fixes.fix_newstyle
/;

my %files;
for my $filename (glob "scripts/qgis_fixes/fix_*.py") {
	$files{$filename}=1;
}

for my $fix (@fixes) {
	my($f) = $fix =~ /\.(fix_.*)$/;

	my $p = $fix;
	$p =~ s#\.#/#g;

	open F, "/usr/lib/python2.7/$p.py" or open F, "/usr/lib/python2.7/dist-packages/$p.py" or die "$p not found";
	my $c;
	while(<F>) {
		last if ($c) = /^class (Fix[^(:]+)[(:]/;
	}
	close F;

	my $filename = "scripts/qgis_fixes/$f.py";
	my $content = "from $fix import $c\n";
	delete $files{$filename};

	#print "CHECK $filename: $content";

	if(-f $filename) {
		open F, $filename;
		my $f = <F>;
		close F;

		print "WRONG $filename:\n  FOUND:$f  EXPECTED:$f" if $f ne $content;
	} else {
		print "WRITE $filename: $content";

		open F, ">$filename";
		print F $content;
		close F;
	}
}

print "LOCAL FIXES:\n  ", join( "\n  ", keys %files), "\n";
