#!/usr/bin/perl
$ARGC = @ARGV;
if ( $ARGC < 0 ){
  printf ("transpose [separator] < input-file > output-file\n");
  exit (1);
}
@lines = <STDIN>;
$count = 0;
$m = 0;

$sep = " ";
if ( $ARGC > 0 ){ $sep = $ARGV[0]; } 

foreach $trans( @lines ) {
  @eles = split($sep, $trans);
  $all = @eles;
  foreach $item( @eles ){
    $item =~ s/\r//;
    $item =~ s/\n//;
    push ( @{$t[$item]}, $count );
    if ( $item > $m ){ $m = $item; }
  }
  $count++;
}

for ( $i=0 ; $i<=$m ; $i++ ){ print "@{$t[$i]}\n";}


