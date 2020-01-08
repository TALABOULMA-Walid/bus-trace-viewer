#!/usr/local/bin/perl

$BTV_SRC_DIR 		= "/home/davewang/btv/src";
$TRACE_DIR		= "/home/davewang/mase_traces";
$TRACE_TYPE		= "mase";

`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/gzip.trc -analysis_type dram`;

`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/applu.trc -analysis_type dram`;

