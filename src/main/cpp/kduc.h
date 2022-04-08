/*
 * Copyright (c) 2022, Sandflow Consulting LLC
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef KDUC_H
#define KDUC_H

#include <stdint.h>

#ifdef __cplusplus

#include "kdu_stripe_compressor.h"
#include "kdu_stripe_decompressor.h"

typedef kdu_supp::kdu_stripe_decompressor kdu_stripe_decompressor;
typedef kdu_supp::kdu_codestream kdu_codestream;
typedef kdu_supp::kdu_compressed_source kdu_compressed_source;

extern "C" {

#else

typedef struct kdu_stripe_decompressor kdu_stripe_decompressor;
typedef struct kdu_codestream kdu_codestream;
typedef struct kdu_compressed_source kdu_compressed_source;

#endif

/**
 * kdu_codestream
 */

int kdu_codestream_create_from_source(kdu_compressed_source* source, kdu_codestream** out);
void kdu_codestream_get_size(kdu_codestream* cs, int comp_idx, int *height, int *width);
int kdu_codestream_get_num_components(kdu_codestream* cs);
void kdu_codestream_delete(kdu_codestream* cs);

/**
 * kdu_compressed_source_buffered
 */

int kdu_compressed_source_buffered_new(const unsigned char* cs, unsigned long int len, kdu_compressed_source** out);
void kdu_compressed_source_buffered_delete(kdu_compressed_source* cs);

/**
 * kdu_stripe_decompressor
 */

int kdu_stripe_decompressor_new(kdu_stripe_decompressor** out);
void kdu_stripe_decompressor_delete(kdu_stripe_decompressor* dec);
void kdu_stripe_decompressor_start(kdu_stripe_decompressor* dec,
                                   kdu_codestream* cs);
int kdu_stripe_decompressor_pull_stripe(kdu_stripe_decompressor* dec,
                                         unsigned char* pixels,
                                         const int* stripe_heights);
int kdu_stripe_decompressor_finish(kdu_stripe_decompressor* dec);

#ifdef __cplusplus
}
#endif

#endif