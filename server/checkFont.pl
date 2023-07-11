#!/usr/bin/perl

$b8c = -1;
while(<>) {
    if(/char 0x/) {
	if($b8c > 0 && $b8c != 7) {
	    print "Something at $_\n";
	}
	$b8c = 0;
    }

    if(/B8\(/) {
	$b8c++;
    }
}
