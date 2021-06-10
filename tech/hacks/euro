#!/usr/bin/perl -w
#
# $Id: euro,v 1.1 2007/08/26 12:45:15 paula Exp $
#
# descricao: verifica combinacao do euromilhoes, com consulta da pagina
#
# autor: Paula Valenca, Feb 17 2007
#

use strict;

use LWP;
use HTML::TokeParser;
use Getopt::Std;
#use Data::Dumper;

LWP::Debug::level('+');

my $euroURL = 'https://www.jogossantacasa.pt/web/SCCartazResult/euroMilhoes'; 
my $euroBD = '.euroBD';

my %premios = (
	       1 => '52', 
	       2 => '51',
	       3 => '50',
	       4 => '42',
	       5 => '41',
	       6 => '40',
	       7 => '32',
	       8 => '31',
	       9 => '22',
	       10 => '30',
	       11 => '12',
	       12 => '21' 
	       );

my $debug = 1;

my %opts;

getopt( 'c:uh', \%opts );

if ( exists $opts{h} ) {
        print "Uso: $0 [ -h | -u | -c [<combinacao>] ]\n\n";
        print "       -h, esta ajuda\n";
        print "       -u, guardar nova aposta\n";
        print "       -c, comparar a ultima aposta com a combinacao vencedora.\n";
        print "           caso a combinacao nao seja dada, recolhe a ultima combinacao na pagina oficial.\n";
        exit 0;
}

if ( exists $opts{u} ) {
    update ();
}
elsif ( exists $opts{c} ) {
    check( \%opts );
}
else {
    print STDERR "Missing option.\nType $0 -h for help.\n";
    exit 1;
}

1;

sub update
{
    open my $bd, ">$euroBD" or die "error writing to $euroBD\n";
    
    my $combinacao;
    print "Introduza as combinações uma a uma, separando os numeros por espacos ( enter para terminar )\n";
    do
    {
	print "Combinacao: ";
	$combinacao = <STDIN>;
	chomp $combinacao;
	if( $combinacao ) {
	    my @c = valida( $combinacao );
	    if(@c) {
		print $bd (join ' ', @c)."\n";
	    }
	    else {
		print STDERR "Combinacao invalida. Tente de novo.\n"; 
	    }
	}
    } while ( $combinacao );
    print "Terminado.\n";
    close $bd;
    return 1;
}

sub check  {
    my $opts = shift;
    my (@win, $urlwin, $parser);

    if( $opts->{c} ) {
	@win = valida( $opts->{c} ) or die "Combinacao invalida. Tente de novo: $opts->{c}\n";
    }
    else {
	($urlwin, $parser) = recolhe_pagina();
	@win = valida( $urlwin) or die "Erro ao recuperar a chave da pagina oficial\n";
    }
    print "A comparar combinacoes com a chave ", (join ' - ', @win), "\n\n";
    open my $bd, "<$euroBD" or die "error writing to $euroBD\n";
    my @premios = ();
    while (<$bd>) {
	my @combinacao = valida( $_ )  or die "Combinacao invalida. Tente de novo: $_\n";
	my ($n, $e) = ( scalar( repeats( (@combinacao[0..4], @win[0..4]) )),
			scalar( repeats( (@combinacao[5..6], @win[5..6]) )));
	my @x = grep {$premios{$_} eq "$n$e" } (keys %premios);
	if( @x ) {
	    print "Premio: $x[0] (", (join " - ", @combinacao), ")\n";
	    push @premios, $x[0];
	}
    }
    close $bd;
    print "\nNumero de combinacoes com premio: ", scalar(@premios), "\n";

    if ( !$opts->{c} && $parser) {
	my $gain = 0;
	$gain += $_ foreach (recolhe_ganhos(\@premios, $parser));
	print "Previsao de ganhos: $gain euros\n";
    }
    return 1;
}


sub valida
{
    my $input = shift;
    my @combinacao = split '\s+', $input;
    return () if( @combinacao != 7 );

    return () if( (@combinacao = ( toSet( @combinacao[0..4] ), toSet( @combinacao[5..6] ))) != 7);

    foreach my $n (@combinacao[0..4]) {
	if( ($n !~ /^\d+$/) || $n < 1 || $n > 50 ) {
	    return ();
	}
    }

    foreach my $n (@combinacao[5..6]) {
	if( $n !~ /^\d+$/ || $n < 1 || $n > 9 ) {
	    return ();
	}
    }

    return @combinacao;
}

sub recolhe_pagina
{
#    my $ua   = LWP::UserAgent->new;
#    my $req  = HTTP::Request->new(GET => $euroURL );
#    $ENV{HTTPS_CA_DIR}    = '/etc/ssl/certs/';
#    my $res = $ua->request($req);
#    $res->is_success or die "$euroURL: " , $res->message,"\n";
#    print Dumper($res);
#
#    my $html = $res->content;
    my $html =`curl $euroURL 2>> /dev/null`;
    my $parser = HTML::TokeParser->new(\$html) or die "TokeParser error $!  ";
    my ($token, $text);
    do {
	$token = $parser->get_tag('table');
        $text = $parser->get_trimmed_text('/table');
    } while ($token && $text !~ /Chave:/);

    if( my @win = $text =~ /Chave:\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+(\d+)\s+\+\s+(\d+)\s+(\d+)/ ) {
	return ((join ' ', @win), $parser);
    }
    else {
	return ("", undef); 
    }
}

sub recolhe_ganhos
{
    my ($premios, $parser) = @_;

    my $e = chr 0x20AC; # UTF euro
    my ($token, $text, @gain);
    do {
	$token = $parser->get_tag('table');
        $text = $parser->get_trimmed_text('/table');
    } while ($token && $text !~ /1.. Pr.mio/);
    for my $premio (@$premios) {
	my (@x) = $text =~ /$premio.. Pr.mio[^$e]*$e (\d*)\.?(\d*)\.?(\d+),(\d+)/o;
	my $dec = pop @x;
	push @gain, ((join '', @x).".$dec");
    }
    return @gain;
}

sub toSet
{
    my @a = @_;
    my %s;
    @s{@a} = undef;
    @a = sort {( $a =~ /^\d+$/ && $b =~ /^\d+$/ ) ?
		   $a <=> $b :
		   "$a" cmp "$b" } 
         keys %s;
    return @a;
}

sub repeats
{
    my @a = @_;
    my %s;
    my @repeats;
    foreach my $elem (@a) {
	push @repeats, $elem if( $s{$elem}++ == 1);
    }
    return @repeats;
}

sub intersect
{
    my ($a, $b) = @_;
    my @i = repeats( (union(@$a), union(@$b)) );
    return \@i;
}
