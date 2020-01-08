#!/usr/bin/perl

$BTV_SRC_DIR 		= "/home/davewang/btv/src";
$TRACE_DIR		= "/home/davewang/mase_traces";
$TRACE_TYPE		= "mase";

$oldtime = time();
`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/$ARGV[0].trc -analysis_type dram`;
$elapsedtime = time() - $oldtime;
print "$elapsedtime\n";
