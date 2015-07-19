#!/bin/bash
#
# Since running everything at once yields inconsistent result, this script
# runs each benchmark one by one.
# An optional numeric argument can be used to run multiple iterations.
#

if [ ! -z $1 ]; then
  QPB_BENCH_EXTRA_ARG="-iterations $1"
fi

OUT=$(cd $(dirname ${BASH_SOURCE[0]}) && pwd)/out

$OUT/bin/test-protobuf-qml -input $OUT/test/protobuf/tst_benchmark.qml SerializationBenchmark::benchmark_v4cb $QPB_BENCH_EXTRA_ARG

$OUT/bin/test-protobuf-qml -input $OUT/test/protobuf/tst_benchmark.qml SerializationBenchmark::benchmark_v4 $QPB_BENCH_EXTRA_ARG

$OUT/bin/test-protobuf-qml -input $OUT/test/protobuf/tst_benchmark.qml SerializationBenchmark::benchmark_lecagy $QPB_BENCH_EXTRA_ARG
