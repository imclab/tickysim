#!/bin/bash
#
# Utility script which runs an experiment on a cluster of lab machines
# This script is an absolute mess but very handy non-the-less.
#
# Usage:
#   ./util/run_experiment.sh path/to/config/file.config

# Relative to project root
CONFIG_FILE="$1"

# Count the number of opening brackets in the "Groups" array to yield the number
# of experimental groups. (Note: This is not foolproof. Do not allow fools to
# operate this software.)
NUM_GROUPS="$(( $( grep -A9999 "groups:" "$CONFIG_FILE" \
                   | grep -m1 -B9999 ");" \
                   | sed -re "s/#.*$//" \
                   | tr -c -d "(" \
                   | wc -c \
                 ) - 1 ))"

# Pull this value out of the config file. (Note: This is not foolproof. Do not
# allow fools to operate this software.)
NUM_SAMPLES="$(sed -nre "s/.*num_samples: *([0-9]+) *;.*/\1/p" < "$CONFIG_FILE")"

RESULTS_PREFIX="$(sed -nre 's/.*results_directory: *"([^"]*)" *;.*/\1/p' < "$CONFIG_FILE")"
RESULT_FILES=(global_counters.dat per_node_counters.dat packet_details.dat simulator.dat)
RESULTS_DIR="$(dirname "$RESULTS_PREFIX${RESULT_FILES[0]}" )"

CLUSTER_HEAD_NODE=kilburn.cs.man.ac.uk

PARALLEL_PROFILE=cluster128

echo =====================
echo Experiment Parameters
echo =====================
echo
echo Config file: $CONFIG_FILE
echo
echo Number of groups: $NUM_GROUPS
echo Number of samples: $NUM_SAMPLES
echo
echo Results file prefix: $RESULTS_PREFIX
echo Results dir: $RESULTS_DIR
echo Results files: ${RESULT_FILES[@]}
echo
echo Cluster head node: $CLUSTER_HEAD_NODE
echo GNU Parallel Profile: $PARALLEL_PROFILE
echo
echo ==================================
echo Packaging tickysim for the cluster
echo ==================================
make -s dist && \
scp tickysim-0.1.tar.gz $CLUSTER_HEAD_NODE: &&
ssh $CLUSTER_HEAD_NODE "rm -rf tickysim-0.1/; \
                        echo ======================================= && \
                        echo Unpacking/compiling tickysim on cluster && \
                        echo ======================================= && \
                        tar xzfm tickysim-0.1.tar.gz && \
                        cd tickysim-0.1 && \
                        mkdir -p \"$RESULTS_DIR\" && \
                        ./configure -q && \
                        make -s && \
                        echo ================== && \
                        echo Running simulation && \
                        echo ================== && \
                        time parallel -J$PARALLEL_PROFILE -j1 \
                          -a <(seq $NUM_GROUPS) \
                          -a <(seq $NUM_SAMPLES) \
                          ./src/tickysim_spinnaker \
                            \"$CONFIG_FILE\" \
                            measurements.results_directory=\"${RESULTS_PREFIX}g{1}_s{2}_\" \
                            experiment.parallel.group={1} \
                            experiment.parallel.sample={2} \
                        ; \
                        echo ================= && \
                        echo Collating results && \
                        echo ================= && \
                        for RESULT_FILE in ${RESULT_FILES[@]}; do \
                          [ ! -f \"${RESULTS_PREFIX}g1_s1_\$RESULT_FILE\" ] && continue ; \
                          ( head -n1 \"${RESULTS_PREFIX}g1_s1_\$RESULT_FILE\"; \
                            tail -q -n+2 \"${RESULTS_PREFIX}\"g*_s*_\"\$RESULT_FILE\" | sort -n; \
                          ) > \"${RESULTS_PREFIX}\$RESULT_FILE\"; \
                          rm \"${RESULTS_PREFIX}\"g*_s*_\"\$RESULT_FILE\"; \
                        done
                        " && \
echo ====================== && \
echo Downloading results... && \
echo ====================== && \
mkdir -p "$RESULTS_DIR"
scp "${RESULT_FILES[@]/#/$CLUSTER_HEAD_NODE:tickysim-0.1/${RESULTS_PREFIX}}" "$RESULTS_DIR/"

