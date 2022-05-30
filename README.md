# kduc
          __        __  
    |__/ |  \ |  | /  ` 
    |  \ |__/ \__/ \__,

## Overview

_kduc_ is a C99 interface to the `kdu_supp::kdu_stripe_compressor` and
`kdu_supp::kdu_stripe_decompressor` classes exposed by the [Kakadu
SDK](https://kakadusoftware.com/).

_IMPORTANT_: While _kduc_ is published under an [open-source
license](./LICENSE.txt), the Kakadu SDK is a commercial library licensed under a
restrictive license. The _kduc_ license does not extend to the Kakadu SDK and a
separate license for Kakadu SDK must be obained.

## Prerequisites

* Kakadu SDK library files
* C99 and C++03 toolchains
* CMake

## Quick start

    git clone https://github.com/sandflow/kduc.git
    cd kduc
    mkdir build
    cd build
    cmake -DKDU_LIBRARY=<path to libkdu.a> \
          -DKDU_AUX_LIBRARY=<path to libkdu_aux.a> \
          -DKDU_INCLUDE_DIR=<path to Kakadu SDK include headers> \
          -DCMAKE_INSTALL_PREFIX=<path where to install the kduc library>
          ..
    ctest
    make install

## Unit tests and samples

[src/test/c](./src/test/c) contains unit tests, which also serve as usage
examples for the interface. Complete documentation of the
`kdu_supp::kdu_stripe_compressor` and `kdu_supp::kdu_stripe_decompressor`
classes are provided in the Kakadu SDK.
