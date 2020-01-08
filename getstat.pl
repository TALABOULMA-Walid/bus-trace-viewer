#!/usr/local/bin/perl

foreach $file (<*.trc.gz>) {
	`gunzip $file`;
	chop($file);
	chop($file);
	chop($file);
	print "$file\n";
	`/export/home/davewang/btv/src/tracestat k6 $file printall`;
	`gzip $file`;
}

