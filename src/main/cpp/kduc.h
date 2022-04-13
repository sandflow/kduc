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
#include <vector>

typedef kdu_supp::kdu_stripe_decompressor kdu_stripe_decompressor;
typedef kdu_supp::kdu_stripe_compressor kdu_stripe_compressor;
typedef kdu_supp::kdu_codestream kdu_codestream;
typedef kdu_supp::kdu_compressed_source kdu_compressed_source;
typedef kdu_core::siz_params kdu_siz_params;

class mem_compressed_target : public kdu_core::kdu_compressed_target {
 public:
  mem_compressed_target() {}

  bool close() {
    this->buf.clear();
    return true;
  }

  bool write(const kdu_core::kdu_byte* buf, int num_bytes) {
    std::copy(buf, buf + num_bytes, std::back_inserter(this->buf));
    return true;
  }

  void set_target_size( kdu_core::kdu_long num_bytes) { this->buf.reserve(num_bytes); }

  bool prefer_large_writes() const { return false; }

  std::vector<uint8_t>& get_buffer() { return this->buf; }

 private:
  std::vector<uint8_t> buf;
};

extern "C" {

#else

typedef struct kdu_stripe_decompressor kdu_stripe_decompressor;
typedef struct kdu_stripe_compressor kdu_stripe_compressor;
typedef struct kdu_codestream kdu_codestream;
typedef struct kdu_compressed_source kdu_compressed_source;
typedef struct mem_compressed_target mem_compressed_target;
typedef struct siz_params kdu_siz_params;

#endif

/**
 * kdu_codestream
 */

int kdu_codestream_create_from_source(kdu_compressed_source* source, kdu_codestream** out);
void kdu_codestream_get_size(kdu_codestream* cs, int comp_idx, int *height, int *width);
int kdu_codestream_get_num_components(kdu_codestream* cs);

int kdu_codestream_create_from_target(mem_compressed_target* target, kdu_siz_params* sz, kdu_codestream** cs);
int kdu_codestream_parse_params(kdu_codestream* cs, const char* params);

void kdu_codestream_delete(kdu_codestream* cs);

/**
 * kdu_compressed_source_buffered
 */

int kdu_compressed_source_buffered_new(const unsigned char* cs, unsigned long int len, kdu_compressed_source** out);
void kdu_compressed_source_buffered_delete(kdu_compressed_source* cs);

/**
 * mem_compressed_target
 */

int kdu_compressed_target_mem_new(mem_compressed_target** target);
void kdu_compressed_target_mem_delete(mem_compressed_target* target);
void kdu_compressed_target_bytes(mem_compressed_target* target, unsigned char **data, int *sz);

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

/**
 * kdu_stripe_compressor
 */

int kdu_stripe_compressor_new(kdu_stripe_compressor** enc);
void kdu_stripe_compressor_delete(kdu_stripe_compressor* enc);
void kdu_stripe_compressor_start(kdu_stripe_compressor* enc,
                                   kdu_codestream* cs);
int kdu_stripe_compressor_push_stripe(kdu_stripe_compressor* enc,
                                         unsigned char* pixels,
                                         const int* stripe_heights);
int kdu_stripe_compressor_finish(kdu_stripe_compressor* enc);

/**
 * kdu_siz_params
 */

int kdu_siz_params_new(kdu_siz_params** sz);

int kdu_siz_params_parse_string(kdu_siz_params* sz, const char* args);
void kdu_siz_params_set_size(kdu_siz_params* sz, int comp_idx, int height, int width);
void kdu_siz_params_set_precision(kdu_siz_params* sz, int comp_idx, int prec);
void kdu_siz_params_set_signed(kdu_siz_params* sz, int comp_idx, int is_signed);
void kdu_siz_params_set_num_components(kdu_siz_params* sz, int num_comps);

void kdu_siz_params_delete(kdu_siz_params* sz);

#ifdef __cplusplus
}
#endif

#endif