#!/bin/bash -f
# ****************************************************************************
# Vivado (TM) v2020.2 (64-bit)
#
# Filename    : simulate.sh
# Simulator   : Xilinx Vivado Simulator
# Description : Script for simulating the design by launching the simulator
#
# Generated by Vivado on Mon May 09 10:05:45 NZST 2022
# SW Build 3064766 on Wed Nov 18 09:12:47 MST 2020
#
# Copyright 1986-2020 Xilinx, Inc. All Rights Reserved.
#
# usage: simulate.sh
#
# ****************************************************************************
set -Eeuo pipefail
# simulate design
echo "xsim testbench_behav -key {Behavioral:sim_1:Functional:testbench} -tclbatch testbench.tcl -protoinst "protoinst_files/system.protoinst" -view /home/acoustics/Documents/RP/RP_feedback/feedback.srcs/sim_1/imports/tmp/testbench_behav.wcfg -log simulate.log"
xsim testbench_behav -key {Behavioral:sim_1:Functional:testbench} -tclbatch testbench.tcl -protoinst "protoinst_files/system.protoinst" -view /home/acoustics/Documents/RP/RP_feedback/feedback.srcs/sim_1/imports/tmp/testbench_behav.wcfg -log simulate.log

