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
 * Message handlers
 */

class error_message_handler : public kdu_core::kdu_message {
 public:
  error_message_handler() : handler(NULL) {}

  void put_text(const char* msg) {
    if (this->handler)
      this->handler(msg);
  }

 /*virtual void flush(bool end_of_message=false) {
   if (end_of_message) {
     throw kdu_core::kdu_exception();
   }
 }*/

  void set_handler(kdu_message_handler_func handler) {
    this->handler = handler;
  }

 private:
  kdu_message_handler_func handler;
};

class warning_message_handler : public kdu_core::kdu_message {
 public:
  warning_message_handler() : handler(NULL) {}

  void put_text(const char* msg) {
    if (this->handler)
      this->handler(msg);
  }
  void set_handler(kdu_message_handler_func handler) {
    this->handler = handler;
  }

 private:
  kdu_message_handler_func handler;
};

void kdu_register_error_handler(kdu_message_handler_func handler) {
  static error_message_handler error_handler;

  error_handler.set_handler(handler);

  kdu_core::kdu_customize_errors(&error_handler);
}

void kdu_register_warning_handler(kdu_message_handler_func handler) {
  static warning_message_handler warning_handler;

  warning_handler.set_handler(handler);

  kdu_core::kdu_customize_warnings(&warning_handler);
}

/**
 *  kdu_stripe_decompressor
 */

void kdu_stripe_decompressor_options_init(kdu_stripe_decompressor_options *opts) {
  opts->force_precise = 0;
  opts->want_fastest = 0;
  opts->reduce = 0;
}


int kdu_stripe_decompressor_new(kdu_stripe_decompressor** out) {
  try {
    *out = new kdu_supp::kdu_stripe_decompressor();
  } catch (...) {
    return 1;
  }
  return 0;
}

void kdu_stripe_decompressor_delete(kdu_stripe_decompressor* dec) {
  delete dec;
}

void kdu_stripe_decompressor_start(kdu_stripe_decompressor* dec,
                                   kdu_codestream* cs,
                                   const kdu_stripe_decompressor_options *opts) {
  dec->start(*cs, opts->force_precise, opts->want_fastest);
}

int kdu_stripe_decompressor_pull_stripe(kdu_stripe_decompressor* dec,
                                        unsigned char* pixels,
                                        const int* stripe_heights) {
  return !dec->pull_stripe(pixels, stripe_heights);
}

int kdu_stripe_decompressor_finish(kdu_stripe_decompressor* dec) {
  return !dec->finish();
}

/**
 *  kdu_stripe_compressor
 */

void kdu_stripe_compressor_options_init(kdu_stripe_compressor_options *opts) {
  opts->force_precise = 0;
  opts->want_fastest = 0;
  opts->rate_count = 0;
  opts->slope_count = 0;
}

int kdu_stripe_compressor_new(kdu_stripe_compressor** enc) {
  try {
    *enc = new kdu_supp::kdu_stripe_compressor();
  } catch (...) {
    return 1;
  }

  return 0;
}

void kdu_stripe_compressor_delete(kdu_stripe_compressor* enc) {
  delete enc;
}

static kdu_core::kdu_long get_bpp_dims(kdu_codestream &codestream)
{
  int comps = codestream.get_num_components();
  int max_width = 0;
  int max_height = 0;

  for (int n = 0; n < comps; n++) { 
    kdu_core::kdu_dims dims;
    
    codestream.get_dims(n, dims);

    max_width = std::max(dims.size.x, max_width);
    max_height = std::max(dims.size.y, max_height);
  }

  return ((kdu_core::kdu_long) max_height) * ((kdu_core::kdu_long) max_width);
}

int kdu_stripe_compressor_start(kdu_stripe_compressor* enc,
                                 kdu_codestream* cs,
                                 const kdu_stripe_compressor_options *opts
                                 ) {
  kdu_core::kdu_uint16 slope[KDU_MAX_LAYER_COUNT];
  kdu_core::kdu_long size[KDU_MAX_LAYER_COUNT];
  int layer_count;

  for(int i = 0; i < opts->slope_count; i++) {
    slope[i] = (kdu_core::kdu_uint16) opts->slope[i];
  }

  for(int i = 0; i < opts->rate_count; i++) {
    size[i] = get_bpp_dims(*cs) * 0.125 * opts->rate[i];
  }

  if (opts->rate_count > 0 && opts->slope_count > 0 && opts->slope_count != opts->rate_count)
    return 1;

  layer_count = opts->rate_count ? opts->rate_count : opts->slope_count;

  cs->access_siz()->finalize_all();

  enc->start(*cs, /* codestream */
              layer_count, /* num_layer_specs */
              opts->rate_count ? size : NULL, /* layer_sizes */
              opts->slope_count ? slope : NULL, /* layer_slopes */
              0, /* min_slope_threshold */
              false, /* no_auto_complexity_control*/
              opts->force_precise, /* force_precise */
              true, /* record_layer_info_in_comment */
              0, /* size_tolerance */
              0, /* num_components */
              opts->want_fastest
              );
  return 0;
}

int kdu_stripe_compressor_push_stripe(kdu_stripe_compressor* enc,
                                      unsigned char* pixels,
                                      const int* stripe_heights) {
  return !enc->push_stripe(pixels, stripe_heights);
}

int kdu_stripe_compressor_finish(kdu_stripe_compressor* enc) {
  return !enc->finish();
}


/**
 *  kdu_codestream
 */

int kdu_codestream_create_from_source(kdu_compressed_source* source, kdu_codestream** out) {
  try {
    *out = new kdu_supp::kdu_codestream();

    (*out)->create(source);
  } catch (...) {
    return 1;
  }
  return 0;
}


int kdu_codestream_create_from_target(mem_compressed_target* target, kdu_siz_params* sz, kdu_codestream** cs) {
  try {
    *cs = new kdu_supp::kdu_codestream();

    static_cast<kdu_core::kdu_params*>(sz)->finalize();

    (*cs)->create(sz, target);
  } catch (...) {
    return 1;
  }
  return 0;
}

void kdu_codestream_get_size(kdu_codestream* cs, int comp_idx, int* height, int* width) {
  kdu_core::kdu_dims dims;
  cs->get_dims(comp_idx, dims);
  *height = dims.size.y;
  *width = dims.size.x;
}

int kdu_codestream_get_num_components(kdu_codestream* cs) {
  return cs->get_num_components();
}

void kdu_codestream_delete(kdu_codestream* cs) {
  cs->destroy();
  delete cs;
}

int kdu_codestream_parse_params(kdu_codestream* cs, const char* params) {
  if ((!cs->access_siz()->parse_string(params)))
    return 1;

  return 0;
}

void kdu_codestream_discard_levels(kdu_codestream* cs, int discard_levels) {
  cs->apply_input_restrictions(0, 0, discard_levels, 0, NULL);
}

/**
 *  kdu_compressed_source_buffered
 */

int kdu_compressed_source_buffered_new(const unsigned char* cs, const unsigned long int len, kdu_compressed_source** out) {
  try {
    *out = new kdu_core::kdu_compressed_source_buffered((kdu_core::kdu_byte*) cs, len);
  } catch (...) {
    return 1;
  }
  return 0;
}

void kdu_compressed_source_buffered_delete(kdu_compressed_source* cs) {
  delete cs;
}

/**
 * kdu_siz_params
 */

int kdu_siz_params_new(kdu_siz_params** sz) {
  try {
    *sz = new kdu_core::siz_params();
  } catch (...) {
    return 1;
  }
  return 0;
}

void kdu_siz_params_delete(kdu_siz_params* sz) {
  delete sz;
}


int kdu_siz_params_parse_string(kdu_siz_params* sz, const char* args) {
  if ((!sz->parse_string(args)))
    return 1;

  return 0;
}

void kdu_siz_params_set_size(kdu_siz_params* sz, int comp_idx, int height, int width) {
  sz->set(Sdims, comp_idx, 0, height);
  sz->set(Sdims, comp_idx, 1, width);
}

void kdu_siz_params_set_precision(kdu_siz_params* sz, int comp_idx, int prec) {
  sz->set(Sprecision, comp_idx, 0, prec);
}

void kdu_siz_params_set_signed(kdu_siz_params* sz, int comp_idx, int is_signed) {
  sz->set(Ssigned, comp_idx, 0, is_signed);
}

void kdu_siz_params_set_num_components(kdu_siz_params* sz, int num_comps) {
  sz->set(Scomponents, 0, 0, num_comps);
}

/**
 * mem_compressed_target
 */

int kdu_compressed_target_mem_new(mem_compressed_target** target) {
  *target = new mem_compressed_target();

  if (!*target) return 1;

  return 0;
}

void kdu_compressed_target_mem_delete(mem_compressed_target* target) {
  delete target;
}

void kdu_compressed_target_bytes(mem_compressed_target* target, unsigned char** data, int* sz) {
  *data = target->get_buffer().data();
  *sz = target->get_buffer().size();
}
