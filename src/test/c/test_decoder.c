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

#include <kduc.h>
#include <stdio.h>

int main(void) {
  int height;
  int width;
  int num_comps;
  int ret;
  int sampling_x, sampling_y;

  kdu_codestream *cs;
  kdu_compressed_source *source;
  kdu_stripe_decompressor *d;

  FILE *j2c_file = fopen("resources/counter-00000.j2c", "rb");

  fseek(j2c_file, 0L, SEEK_END);
  const long size = ftell(j2c_file);
  fseek(j2c_file, 0L, SEEK_SET);

  unsigned char j2c_buffer[size];
  fread(j2c_buffer, size, 1, j2c_file);

  ret = kdu_compressed_source_buffered_new(&j2c_buffer[0], size, &source);
  if (ret) return ret;

  ret = kdu_codestream_create_from_source(source, &cs);
  if (ret) return ret;

  kdu_codestream_get_size(cs, 0, &height, &width);

  kdu_codestream_get_subsampling(cs, 0, &sampling_x, &sampling_y);
  if (sampling_x != sampling_y || sampling_y != 1)
    return 1;

  num_comps = kdu_codestream_get_num_components(cs);

  ret = kdu_stripe_decompressor_new(&d);
  if (ret) return ret;

  unsigned char pixels[width * height * num_comps];

  int stripe_heights[4] = {height, height, height, height};
  int precisions[4] = {8, 8, 8, 8};

  kdu_stripe_decompressor_options opts;

  kdu_stripe_decompressor_options_init(&opts);

  kdu_stripe_decompressor_start(d, cs, &opts);

  int pull_strip_should_stop = 0;
  while(!pull_strip_should_stop) {
    pull_strip_should_stop = kdu_stripe_decompressor_pull_stripe(
        d, &pixels[0], stripe_heights, NULL, NULL, NULL, precisions, NULL);
  }

  ret = kdu_stripe_decompressor_finish(d);
  if (ret) return ret;

  kdu_stripe_decompressor_delete(d);

  kdu_codestream_delete(cs);

  kdu_compressed_source_buffered_delete(source);

  return 0;
}
