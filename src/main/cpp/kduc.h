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

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus

#include <vector>
#include "kdu_stripe_compressor.h"
#include "kdu_stripe_decompressor.h"
#include "kdu_elementary.h"

typedef kdu_supp::kdu_stripe_decompressor kdu_stripe_decompressor;
typedef kdu_supp::kdu_stripe_compressor kdu_stripe_compressor;
typedef kdu_supp::kdu_codestream kdu_codestream;
typedef kdu_supp::kdu_compressed_source kdu_compressed_source;
typedef kdu_core::siz_params kdu_siz_params;

class mem_compressed_target : public kdu_core::kdu_compressed_target {
 public:
  mem_compressed_target(): backtrack(-1) {}

  bool close() {
    this->buf.clear();
    return true;
  }

  bool write(const kdu_core::kdu_byte* buf, int num_bytes) {
    if (this->backtrack < 0) {
      std::copy(buf, buf + num_bytes, std::back_inserter(this->buf));
    } else if (num_bytes > this->backtrack) {
      return false;
    } else {
      std::copy(buf, buf + num_bytes, this->buf.end() - this->backtrack);
      this->backtrack -= num_bytes;
    }

    return true;
  }

  void set_target_size(kdu_core::kdu_long num_bytes) {
    this->buf.reserve(num_bytes);
  }

  bool prefer_large_writes() const { return false; }

  std::vector<uint8_t>& get_buffer() { return this->buf; }

  bool start_rewrite(kdu_core::kdu_long backtrack) {
    if (backtrack > this->buf.size() || backtrack < 0)
      return false;

    this->backtrack = backtrack;
    return true;
  }

  bool end_rewrite() {
    if (this->backtrack < -1)
      return false;

    this->backtrack = -1;
    return true;
  }

 private:
  std::vector<uint8_t> buf;
  kdu_core::kdu_long backtrack;
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
 * constants
 *
 */

#define KDU_MAX_LAYER_COUNT 32

#define KDU_MAX_COMPONENT_COUNT 8

/**
 * message handlers
 */

typedef void (*kdu_message_handler_func)(const char*);

void kdu_register_error_handler(kdu_message_handler_func handler);

void kdu_register_warning_handler(kdu_message_handler_func handler);

void kdu_register_info_handler(kdu_message_handler_func handler);

void kdu_register_debug_handler(kdu_message_handler_func handler);

/**
 * kdu_codestream
 */

int kdu_codestream_create_from_source(kdu_compressed_source* source,
                                      kdu_codestream** out);

void kdu_codestream_discard_levels(kdu_codestream* cs, int discard_levels);

void kdu_codestream_get_size(kdu_codestream* cs,
                             int comp_idx,
                             int* height,
                             int* width);

int kdu_codestream_get_num_components(kdu_codestream* cs);

int kdu_codestream_get_depth(kdu_codestream* cs, int comp_idx);

bool kdu_codestream_get_signed(kdu_codestream* cs, int comp_idx);

void kdu_codestream_get_subsampling(kdu_codestream* cs,
                                    int comp_idx,
                                    int* x,
                                    int* y);

int kdu_codestream_create_from_target(mem_compressed_target* target,
                                      kdu_siz_params* sz,
                                      kdu_codestream** cs);

int kdu_codestream_parse_params(kdu_codestream* cs, const char* params);

void kdu_codestream_textualize_params(kdu_codestream* cs,
                                      kdu_message_handler_func handler);

void kdu_codestream_delete(kdu_codestream* cs);

/**
 * kdu_compressed_source_buffered
 */

int kdu_compressed_source_buffered_new(const unsigned char* cs,
                                       unsigned long int len,
                                       kdu_compressed_source** out);

void kdu_compressed_source_buffered_delete(kdu_compressed_source* cs);

/**
 * mem_compressed_target
 */

int kdu_compressed_target_mem_new(mem_compressed_target** target);

void kdu_compressed_target_mem_delete(mem_compressed_target* target);

void kdu_compressed_target_bytes(mem_compressed_target* target,
                                 unsigned char** data,
                                 int* sz);

/**
 * kdu_stripe_decompressor
 */

typedef struct kdu_stripe_decompressor_options {
  bool force_precise;
  bool want_fastest;
  int reduce;
} kdu_stripe_decompressor_options;

void kdu_stripe_decompressor_options_init(
    kdu_stripe_decompressor_options* opts);

int kdu_stripe_decompressor_new(kdu_stripe_decompressor** out);

void kdu_stripe_decompressor_delete(kdu_stripe_decompressor* dec);

void kdu_stripe_decompressor_start(kdu_stripe_decompressor* dec,
                                   kdu_codestream* cs,
                                   const kdu_stripe_decompressor_options* opts);

int kdu_stripe_decompressor_pull_stripe(kdu_stripe_decompressor* dec,
                                        unsigned char* pixels,
                                        const int* stripe_heights,
                                        const int* sample_offsets,
                                        const int* sample_gaps,
                                        const int* row_gaps,
                                        const int* precisions,
                                        const int* pad_flags);

int kdu_stripe_decompressor_pull_stripe_planar(kdu_stripe_decompressor* dec,
                                               unsigned char* pixels[],
                                               const int* stripe_heights,
                                               const int* sample_gaps,
                                               const int* row_gaps,
                                               const int* precisions,
                                               const int* pad_flags);

int kdu_stripe_decompressor_pull_stripe_16(kdu_stripe_decompressor* dec,
                                           int16_t* pixels,
                                           const int* stripe_heights,
                                           const int* sample_offsets,
                                           const int* sample_gaps,
                                           const int* row_gaps,
                                           const int* precisions,
                                           const bool* is_signed,
                                           const int* pad_flags);

int kdu_stripe_decompressor_pull_stripe_planar_16(kdu_stripe_decompressor* dec,
                                                  int16_t* pixels[],
                                                  const int* stripe_heights,
                                                  const int* sample_gaps,
                                                  const int* row_gaps,
                                                  const int* precisions,
                                                  const bool* is_signed,
                                                  const int* pad_flags);

int kdu_stripe_decompressor_finish(kdu_stripe_decompressor* dec);

/**
 * kdu_stripe_compressor
 */

typedef struct kdu_stripe_compressor_options {
  bool force_precise;
  bool want_fastest;
  float tolerance;                    /* [0..0.5] */
  int rate_count;                     /* [0..KDU_MAX_LAYER_COUNT] */
  float rate[KDU_MAX_LAYER_COUNT];    /* target compression in bpp (see `-rate` in `kdu_compress`) */
  int slope_count;                    /* [0..KDU_MAX_LAYER_COUNT] */
  int slope[KDU_MAX_LAYER_COUNT];     /* distortion-length slope (see `kdu_stripe_compressor.h`) */
} kdu_stripe_compressor_options;

void kdu_stripe_compressor_options_init(kdu_stripe_compressor_options* opts);

int kdu_stripe_compressor_new(kdu_stripe_compressor** enc);

void kdu_stripe_compressor_delete(kdu_stripe_compressor* enc);

int kdu_stripe_compressor_start(kdu_stripe_compressor* enc,
                                kdu_codestream* cs,
                                const kdu_stripe_compressor_options* opts);

int kdu_stripe_compressor_push_stripe(kdu_stripe_compressor* enc,
                                      unsigned char* pixels,
                                      const int* stripe_heights,
                                      const int* sample_offsets,
                                      const int* sample_gaps,
                                      const int* row_gaps,
                                      const int* precisions);

int kdu_stripe_compressor_push_stripe_16(kdu_stripe_compressor* enc,
                                         int16_t* pixels,
                                         const int* stripe_heights,
                                         const int* sample_offsets,
                                         const int* sample_gaps,
                                         const int* row_gaps,
                                         const int* precisions,
                                         const bool* is_signed);

int kdu_stripe_compressor_push_stripe_planar(kdu_stripe_compressor* enc,
                                             unsigned char* pixels[],
                                             const int* stripe_heights,
                                             const int* sample_gaps,
                                             const int* row_gaps,
                                             const int* precisions);

int kdu_stripe_compressor_push_stripe_planar_16(kdu_stripe_compressor* enc,
                                                int16_t* pixels[],
                                                const int* stripe_heights,
                                                const int* sample_gaps,
                                                const int* row_gaps,
                                                const int* precisions,
                                                const bool* is_signed);

int kdu_stripe_compressor_finish(kdu_stripe_compressor* enc);

/**
 * kdu_siz_params
 */

int kdu_siz_params_new(kdu_siz_params** sz);

int kdu_siz_params_parse_string(kdu_siz_params* sz, const char* args);

void kdu_siz_params_set_size(kdu_siz_params* sz,
                             int comp_idx,
                             int height,
                             int width);

void kdu_siz_params_set_precision(kdu_siz_params* sz, int comp_idx, int prec);

void kdu_siz_params_set_signed(kdu_siz_params* sz, int comp_idx, int is_signed);

void kdu_siz_params_set_num_components(kdu_siz_params* sz, int num_comps);

void kdu_siz_params_delete(kdu_siz_params* sz);

#ifdef __cplusplus
}
#endif

#endif
