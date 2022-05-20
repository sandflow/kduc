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

#include <vector>
#include <kduc.h>
#include <iostream>

void print_message(const char* msg) {
  std::cout << msg;
}

int main(int argc, char** argv) {
  int height = 480;
  int width = 640;
  int num_comps = 3;

  std::vector<unsigned char> pixels(height * width * num_comps);
  mem_compressed_target *target = NULL;
  kdu_codestream *cs = NULL;
  mem_compressed_target *source = NULL;
  kdu_stripe_compressor *enc = NULL;
  kdu_siz_params *siz = NULL;

  unsigned char *buf;
  int buf_sz;

  kdu_register_error_handler(print_message);
  kdu_register_warning_handler(print_message);

  /* create image */

  for(int i = 0; i < pixels.size(); i++)
    pixels[i] = (unsigned char) (i & 0xFF);

  /* initialize siz */

  siz = new kdu_core::siz_params();

  siz->set(Scomponents, 0, 0, num_comps);
  siz->set(Sprecision, 0, 0, 8);
  siz->set(Sdims, 0, 0, height);
  siz->set(Sdims, 0, 1, width);
  siz->set(Ssigned, 0, 0, false);

  static_cast<kdu_core::kdu_params*>(siz)->finalize();

  /* allocate output codestream */

  target = new mem_compressed_target();

  /* init codestream */

  cs = new kdu_supp::kdu_codestream();

  cs->create(siz, target);

  //cs->access_siz()->parse_string("Qfactor=85");
  //cs->access_siz()->parse_string("Ctype=N");
  cs->access_siz()->parse_string("Creversible=yes");
  cs->access_siz()->parse_string("Clayers=9");
  cs->access_siz()->parse_string("Stiles={711,393}");
  cs->access_siz()->parse_string("Sorigin={39,71}");
  cs->access_siz()->parse_string("Stile_origin={17,69}");
  cs->access_siz()->parse_string("Cprecincts={128,128},{64,64}");
  cs->access_siz()->parse_string("Corder=PCRL");
  /* compressor */

  enc = new kdu_supp::kdu_stripe_compressor();

  int stripe_heights[3] = {height, height, height};

  cs->access_siz()->finalize_all();

  enc->start(*cs);

  enc->push_stripe(pixels.data(), stripe_heights);

  enc->finish();

  buf = target->get_buffer().data();
  buf_sz = target->get_buffer().size();

  if (buf_sz == 0)
    return 1;

  delete enc;

  cs->destroy();
  delete cs;

  delete target;

  delete siz;

  return 0;
}
