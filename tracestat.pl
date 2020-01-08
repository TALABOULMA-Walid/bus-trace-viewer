#!/usr/bin/perl

$BTV_SRC_DIR 		= "/home/davewang/btv/src";
$TRACE_DIR		= "/home/davewang/btv/traces/mase_256K_64_2G/";
$TRACE_TYPE		= "mase";
$starttime = time();
if($ARGV[0] eq "all"){
	print "Processing $ARGV[0]\n";
	chdir($TRACE_DIR);
	foreach $file (<*.trc>) {
		$oldtime = time();
		print "Processing $file\n";
		`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/$file -analysis_type dram`;
		$elapsedtime = time() - $oldtime;
		print "$file took $elapsedtime seconds of analysistime\n";
	}
} else {
	print "Processing $ARGV[0]\n";
	$oldtime = time();
	`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/$ARGV[0].trc -analysis_type dram`;
	$elapsedtime = time() - $oldtime;
	print "$elapsedtime\n";
}
$totaltime =  time() - $starttime;
print "Total time $totaltime\n";
