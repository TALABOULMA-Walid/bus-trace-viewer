#!/usr/local/bin/perl

$BTV_SRC_DIR 		= "/home/davewang/btv/src";
$TRACE_DIR		= "/home/davewang/mase_traces";
$TRACE_TYPE		= "mase";

#`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/gzip.trc -analysis_type dram`;
#`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/applu.trc -analysis_type dram`;
#`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/mgrid.trc -analysis_type dram`;
`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/apsi.trc -analysis_type dram`;
`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/art.trc -analysis_type dram`;
`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/gcc.trc -analysis_type dram`;
`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/mcf.trc -analysis_type dram`;
`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/ammp.trc -analysis_type dram`;
`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/crafty.trc -analysis_type dram`;
`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/parser.trc -analysis_type dram`;
`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/vortex.trc -analysis_type dram`;
`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/gap.trc -analysis_type dram`;
`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/galgel.trc -analysis_type dram`;
`$BTV_SRC_DIR/tracestat -trace_type $TRACE_TYPE -trace_file $TRACE_DIR/equake.trc -analysis_type dram`;


