#!/usr/bin/perl
if ($#ARGV < 1) {die "Usage: $0 one_file another_file\n";}
$file1 = shift(@ARGV);
$file2 = shift(@ARGV);

# first strip off blanks at end
open(F1, $file1);
open(O1, "> t1");
while (<F1>){
    $_ =~ s/\s*\n$//;
    print O1 "$_\n";
}
close(F1);
close(O1);
open(F2, $file2);
open(O2, "> t2");
while (<F2>){
    $_ =~ s/\s*\n$//;
    print O2 "$_\n";
}
close(F2);
close(O2);

open(OUT, "> t3");
open(DIFF, "diff t1 t2 |");
$store = "NO";
while(<DIFF>){
    if (/^[0-9]+/){
	$line = $_;
	next;
    }
    if (/Message-ID:/ 
	|| / [-,+][0-9]{2}00 - - -$/
	|| /^[<>] [0-9]+:[0-9]+$/
	|| ($open_done > 1 && /^[<>] This petition was initiated on/)
	|| ($open_done > 1 && /^[<>] This poll was initiated on/)
	|| ($close_done > 1 && /^[<>] but has been closed since/)
	|| ($open_done > 1 && /^[<>] This poll was opened for voting on/ )
	|| /^[<>] [Oo]n [A-Z][a-z]{2}, [0-9,\s]{2} [A-Z][a-z]{2} 200/
	|| /^[<>]\s*[A-Z][a-z]{2} [A-Z][a-z]{2} [0-9,\s]{2} [0-2][0-9]:/){
	next;
    }
    if (/^[<>] This poll was opened for voting on/ 
	|| /^[<>] This petition was initiated on/ ){
	$open_done++;
    }
    if (/^[<>] but has been closed since/){
	$close_done++;
    }
    if (/^---/ && $line ne ""){
	next;
    }
    print OUT $line;
    $line = "";
    print OUT $_;
}
close(DIFF);
close(OUT);
unlink(t1);
unlink(t2);

open(IN, "t3");
open(OUT, "> compare.out");
print OUT "<<<< $file1\n";
print OUT ">>>> $file2\n";
print OUT "---\n";
$in = <IN>;
$reading = 1;
while ($reading == 1) {
    $line = $in;
    $new = "";
    while ($reading == 1) {
	unless ($in = <IN>){
	    $reading = 0;
	}
	if ($in =~ /^[0-9]+/) { 
	    last;
	}
	$new = join("", $new, $in);
    }
#    print "Got:\n$new";
    $found = 0;
    for ($j = 0; $j <= $#entry; $j++) {
#	print "Checking $j:\n$entry[$j]\nxxx\n";
	if ($new eq $entry[$j]){
	    $lines[$j] = join("",$lines[$j],$line);
	    $found = 1;
	    last;
	}
    }
    if ($found == 0) {
	push(@entry, $new);
	push(@lines, $line);
	
#	print "Stored \n$entry[$#entry] for line $lines[$#lines]\n";
    }
}

for ($j = 0; $j <= $#entry; $j++){
    print OUT $lines[$j];
    print OUT $entry[$j];
}
close(IN);
close(OUT);

