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

#include "kduc.h"
#include <vector>

/**
 *  kdu_stripe_decompressor
 */

kdu_stripe_decompressor* kdu_stripe_decompressor_new() {
  return new kdu_supp::kdu_stripe_decompressor();
}

void kdu_stripe_decompressor_delete(kdu_stripe_decompressor* dec) {
  delete dec;
}

void kdu_stripe_decompressor_start(kdu_stripe_decompressor* dec,
                                   kdu_codestream* cs) {
  dec->start(*cs);
}

void kdu_stripe_decompressor_pull_stripe(kdu_stripe_decompressor* dec,
                                         unsigned char* pixels,
                                         const int* stripe_heights) {
  dec->pull_stripe(pixels, stripe_heights);
}

void kdu_stripe_decompressor_finish(kdu_stripe_decompressor* dec) {
  dec->finish();
}

/**
 *  kdu_codestream
 */

kdu_codestream* kdu_codestream_create_from_source(kdu_compressed_source* source) {
  kdu_supp::kdu_codestream* cs = new kdu_supp::kdu_codestream();

  cs->create(source);

  return cs;
}

void kdu_codestream_get_size(kdu_codestream* cs, int comp_idx, int *height, int *width) {
  kdu_core::kdu_dims dims;
  cs->get_dims(comp_idx, dims);
  *height = dims.size.y;
  *width = dims.size.x;
}

int kdu_codestream_get_num_components(kdu_codestream* cs) {
  return cs->get_num_components();
}

void kdu_codestream_delete(kdu_codestream* cs) {
  delete cs;
}

/**
 *  kdu_compressed_source_buffered
 */

kdu_compressed_source* kdu_compressed_source_buffered_create(const unsigned char* cs, size_t len) {
  return new kdu_core::kdu_compressed_source_buffered((kdu_core::kdu_byte*) cs, len);
}

void kdu_compressed_source_buffered_delete(kdu_compressed_source* cs) {
  delete cs;
}