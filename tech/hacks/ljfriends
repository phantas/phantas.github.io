#!/usr/local/bin/perl -w
#
# $Id: ljfriends.pl,v 1.4 2008/06/06 22:30:27 paula Exp paula $
#
# description: Create RSS feed of a user's LJ friends' page
#
# author: Paula Valenca, Feb 2007 
#
# requires: a paid, permanent or early adopter account, unfortunately...
#           ask brad fitzpatrick nicely to extend the API if you're free...
#
# todo: - convert or add the functionality: CGI
#       - break down and package it better (less of an hack)
#       - add documentation
#		- try to get round the HTML::Tidy issue <-- important
#
# warning: this script is not endorsed by LiveJournal and is provided as is.
#          In particular, DO NOT MAKE THE SCRIPT USABLE PUBLICALLY IN A SERVER
#          - there is a security loophole that may be used to hang or 
#          crash your script/server.
#
# troubleshooting:
#          this script uses HTML::Tidy which frequently has instalation conflicts.
#          Ensure you have a working instalation of the module that passes
#          all tests. Issues may likely arise from there. If it still
#          doesn't work, contact me at me@paulavalenca.org.

use strict;

use LWP;
use HTML::Tidy 1.07; # version 1.07_01 since the 1.06 has bugs
use HTML::Entities qw(encode_entities_numeric);
use XML::XPath;
use Getopt::Std;

# use Data::Dumper; # for debugging purposes
# use LJ::Simple; # not needed for now but might be useful in the future


my $debug = 0;
my $feedfile = undef;

my $styleid = 66435; # best system style for parsing

my %opts;
getopts( 'f:l:u:ph', \%opts );
my $lj = load_user( \%opts );

if (! defined $lj ) {
    die "Error loading user\n";
}

undef $/; #read until EOF

my $res;

# for now, authenticate whenever password is present.
# might save the cookies to a file at a later time...
if ($lj->{pass}) {
    $res = $lj->{ua}->post(
			   "http://www.livejournal.com/login.bml?ret=1", 
			   [user => $lj->{login},
			    password => $lj->{pass},
			    remember_me => 'on']
			   );
    
    if( # $res->is_success || 
	$res->content =~ /There was an error processing your request/ ) {
	die $res->status_line, " or there was an error processing your request\n";
    }
}

my $req = HTTP::Request->new(GET => "$lj->{raw}");
$res = $lj->{ua}->request($req);
$res->is_success or die $res->status_line,"\n";

my $source = $res->content;

my $xp = XML::XPath->new(xml => tidy($source));

my $title = extract_title( $xp );
my @cleanitems = extract_entries( $xp );
my $feed = create_view_rss( $lj, $title, \@cleanitems );

if( $feedfile ) {
    open my $feedhandle, ">$feedfile" or die "error writing to $feedfile\n";
    print $feedhandle $feed;
    close $feedhandle;
}
else { # assume STDOUT
    print $feed;
}

# print tidy($source) if $debug;
# print Dumper($lj) if $debug;


###
#
# Parsing of Friend's page
#
# I'll have to place this in an external file/module and, ideally,
# parsing of other styles could be used, maintaining the interface. 
#

sub convert_date
{
    my $str = shift;

    my ($day, $month, $year, $hour, $mins, $am_pm) = $str =~ 
	/(\d\d?).. (\w\w\w)\w+ (\d\d\d\d) (\d\d?):(\d\d)([ap])m/;

    $day = ( length $day == 1 ) ? "0".$day : $day;
    $hour = "00" if ($hour eq "12");
    $hour = ( length $hour == 1 ) ? "0".$hour : $hour;
    $hour += ( $am_pm eq 'p' ) ? 12 : 0;

    return "$day $month $year $hour:$mins GMT";
}

sub tidy
{
    my $html = shift;

    my $tidy = HTML::Tidy->new({
                                    output_xhtml => 1,
                                    numeric_entities => 1,
                                });
    $tidy->ignore( type => TIDY_WARNING );
    $html = $tidy->clean( $html );
    return $html;
}

sub extract_title
{
    my $xp = shift;

    my $title = $xp->find('/html/head/title/text()');
    return $title;
}

### parse HTML and create a list of entries
sub extract_entries
{
    my $xp = shift;
    my @items = ();
    my $days = $xp->findnodes('//div[@class="day"]');
    foreach my $node ($days->get_nodelist)
    {
	my $day = $xp->findvalue('./h3', $node);
	my $entries = $xp->findnodes('./div[@class="entry"]', $node);
	foreach my $entry ($entries->get_nodelist)
	{
	    my $item = {
		ditemid => undef,
		subject => undef,
		author  => undef, # required
		altauthor =>  undef,
		createtime => undef, # required
		event => "", # required
		comments => undef, # related with having a ditemid or not
		tags => undef,
		music => undef,
		mood => undef,
		security => undef
		};
	    
	    my $entrytext = ($xp->findnodes('./div[@class="entrytext"]', $entry)->get_nodelist)[0];
	    my $comments = ($xp->findnodes('./div[@class="talklinks"]', $entry)->get_nodelist)[0];
	    if ($comments) {
		$item->{comments} = 1;
		my $commentlink = ($xp->findnodes('./a', $comments)->get_nodelist)[-1]->toString;
		($item->{ditemid}) = $commentlink =~ /\/(\d+)\.html/;
	    }
	    
	    # entryheading

	    if( $xp->exists('./span/img[@alt="[Protected]"]', $entrytext) ) {
		$item->{security} = "usemask";
	    }
	    elsif( $xp->exists('./span/img[@alt="[Private]"]', $entrytext) ) {
		$item->{security} = "private";
	    }
	    else {
		$item->{security} = "public";
	    }

	    my $date = "$day " . $xp->findvalue('./span/span[@class="timestamp"]', $entrytext);
	    $item->{createtime} = convert_date($date);
	    $item->{altauthor} = $xp->findvalue('./span/span[@class="altposter"]/a', $entrytext);
	    $item->{altauthor} = "__SYNDICATED__" if( $xp->exists('./p[@class="ljsyndicationlink"]', $entrytext) );
	    $item->{author} = $xp->findvalue('./span/span[not(@class)]/a', $entrytext);
	    $item->{subject} = $xp->findvalue('./span/span[@class="subject"]', $entrytext);

	    # currents
	    ($item->{mood} = $xp->findvalue('div/div[@class="currentmood"]/text()', $entrytext)) =~ s/^\s*//;
	    ($item->{music} = $xp->findvalue('div/div[@class="currentmusic"]/text()', $entrytext)) =~ s/^\s*//;

	    # and finally, the event: everything else

	    my @cnodes = $entrytext->getChildNodes;

	    @cnodes = grep { 
		my $cn = $_;
		! ( $cn->getNodeType == XML::XPath::Node::ELEMENT_NODE 
		    and $cn->getName eq 'span' and $cn->getAttribute('class') eq "entryheading" )
		and 
		! ( $cn->getNodeType == XML::XPath::Node::ELEMENT_NODE 
		    and $cn->getName eq 'div' and $cn->getAttribute('class') eq "currents" )
		and 
		! ( $cn->getNodeType == XML::XPath::Node::TEXT_NODE and $cn->toString eq "\n");
	    } @cnodes;
 
	    $item->{event} = join "", ( map { encode_entities_numeric( $_->toString ); } @cnodes );
	    push @items, $item;
	}
    }
    return @items;
}

#
# End of Friend's page parsing
#
###


sub load_user
{
    my $opts = shift;
    
    if( exists $opts->{h} ) {
	print "Usage: $0 [ -h | -p | -f <rssfile> | -u <ljuser> ] -l <login>[:<passwd>]\n\n";
	print "\tif <passwd> is not given,protected entries will not be shown\n";
	print "\tunless -p is used, in which case user is prompted for password\n";
	exit 0;
    }

    $feedfile = $opts->{f} if( exists $opts->{f} );

    ( exists $opts->{l} ) or die "Missing login.\nType $0 -h for usage.";
    
    my ( $login, $passwd ) = split (':', $opts->{l});
    my $user = ( exists $opts->{u} ) ? $opts->{u} : $login;
    my $link = "http://$login.livejournal.com";
    my $friends = "http://$user.livejournal.com/friends";
    my $raw = "http://$user.livejournal.com/data/customview?styleid=$styleid";
    if( ! $passwd ) {
	if( exists $opts->{p} ) {
	    print "Password: ";
	    system("stty -echo");
	    $passwd = <STDIN>;
	    system("stty echo");
	    print "\n";
	    chomp($passwd);
	} 
	else {
	    $passwd = undef;
	    print STDERR "No password given, assume no authentication\n" 
		if $debug;
	}
    }

    my $ua = LWP::UserAgent->new(keep_alive => 1);
    if( $passwd ) {
	$ua->cookie_jar( {} );
	$ua->credentials("$link:80", "lj", $login => $passwd);
	$friends .="?auth=digest";
	$raw .= "&checkcookies=1";
    }
    
    # include any LJ::Simple User autentication here
    # and extend lj appropriately
    
    my $lj = {
	      login => $login,
	      pass => $passwd,
	      link  => $link,
	      friends => $friends,
	      raw => $raw,
	      ua => $ua
	      };
    return $lj;
}


###
#
# with regards to $cleanitems:
# - only using the following fields
#   ditemid, subject, event, createtime, music, mood, security
# - tags is here but most system styles do not show them
# - itemid can actually be obtained via the 66435 style but is not needed here
# - if user chose to disallow comments, no system styles (but one) shows link
#   so ditemid will have to be undef...
# - the ppid field needs to be checked... for now it's not used
# - added the field author (and alt_author),
#   no distinction is made between users and syndicated feeds
# - assuming fields are formatted for XML, this might require future checking
#
sub create_view_rss
{
    my ($lj, $title, $cleanitems) = @_;

    # most of what follows comes from ljfeed.pl from LJ's /trunk
    
    my ($friends, $ret);
    my $date = localtime();
    ($friends = $lj->{friends}) =~ s/\?auth=digest$//;

    # header
    $ret .= "<?xml version='1.0' encoding='utf-8' ?>\n";
    $ret .= "<!--  If you are running a bot please visit this policy page outlining rules " .
	    "you must respect. http://www.livejournal.com/bots/  -->\n";
    $ret .= "<rss version='2.0' xmlns:lj='http://www.livejournal.org/rss/lj/1.0/'>\n";

    # channel attributes
    $ret .= "<channel>\n";
    $ret .= "  <title>$title</title>\n";
    $ret .= "  <link>$friends</link>\n";
    $ret .= "  <description>$title - LiveJournal.com</description>\n";
    $ret .= "  <lastBuildDate>$date</lastBuildDate>\n\n";

    # for simplicity sake, image is not added
    # this could be easily solved by fetching the user's RSS feed

    # output individual item blocks

    foreach my $it (@$cleanitems)
    {
	my $ditemurl;
	if( !$it->{altauthor} ) {
	    $ditemurl = "http://$it->{author}.livejournal.com";
	}
	elsif( $it->{altauthor} eq "__SYNDICATED__") {
	    $ditemurl = "http://syndicated.livejournal.com/$it->{author}";
	}
	else {
	    $ditemurl = "http://community.livejournal.com/$it->{author}";
	}
	$ditemurl .= ( $it->{ditemid} ) ? "/$it->{ditemid}.html" : "";
	my $author = ( ( $it->{altauthor} && $it->{altauthor} ne "__SYNDICATED__" ) 
		    ? "$it->{altauthor} in " : "") 
	          . $it->{author};

        $ret .= "<item>\n";
        $ret .= "  <guid isPermaLink='true'>$ditemurl</guid>\n";
        $ret .= "  <pubDate>$it->{createtime}</pubDate>\n";
        $ret .= "  <title>$it->{subject}</title>\n" if $it->{subject};
        $ret .= "  <author>$author</author>\n";
        $ret .= "  <link>$ditemurl</link>\n";
        $ret .= "  <description>$it->{event}</description>\n";
        if ($it->{comments}) {
            $ret .= "  <comments>$ditemurl</comments>\n";
        }
        $ret .= "  <category>$_</category>\n" foreach @{$it->{tags} || []};
        $ret .= "  <lj:music>$it->{music}</lj:music>\n" if $it->{music};
        $ret .= "  <lj:mood>$it->{mood}</lj:mood>\n" if $it->{mood};
        $ret .= "  <lj:security>$it->{security}</lj:security>\n" if $it->{security};
        $ret .= "</item>\n";
    }

    $ret .= "</channel>\n";
    $ret .= "</rss>\n";

    return $ret;
}

#
# $Log: ljfriends.pl,v $
# Revision 1.4  2008/06/06 22:30:27  paula
# minor tweaks, mainly revisiting. LJ seems to have changed somethings on the login page
# since it's now returning unsuccessful despite working...
#
# Revision 1.3  2007/02/10 22:27:02  paula
# version 0.01
# working personally
# next step: testing for bugs and adding features
#
# Revision 1.2  2007/02/08 03:44:19  paula
# authentication via cookies and create_view_rss
# missing parsing of html to create a list of entries
# .
#
# Revision 1.1  2007/02/06 19:40:11  paula
# Initial revision
#
#
