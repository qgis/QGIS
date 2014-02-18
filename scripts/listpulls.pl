#!/usr/bin/perl

use strict;
use warnings;

use JSON;
use LWP::UserAgent;

my $ua = LWP::UserAgent->new(ssl_opts => { verify_hostname => 0 });
my $res = $ua->get( "https://api.github.com/repos/qgis/qgis/pulls" );

die "pull request retrieval failed: " . $res->status_line unless $res->is_success;

my %assigned;

printf "%5s %-16s %s\n", "#", "Assignee", "Title";
foreach my $pull ( sort { $a->{number} <=> $b->{number} } @{ JSON::from_json( $res->decoded_content ) } ) {
	my $assignee = $pull->{assignee}->{login};
	$assignee = "" unless defined $assignee;

	push @{ $assigned{$assignee} }, $pull->{number};

	printf "%5d %-16s %s\n", $pull->{number}, $assignee || "", $pull->{title};
}

print "\nASSIGNMENTS:\n";

foreach my $assignee ( sort keys %assigned ) {
	printf "%-22s %s\n", $assignee || "unassigned", join( ", ", sort { $a <=> $b } @{ $assigned{$assignee} } );
}
