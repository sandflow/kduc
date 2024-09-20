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
#include <stdlib.h>

static int is_error = 0;

void print_message(const char* msg) {
  printf("%s", msg);
  fflush(stdout);
}

void exit_with_error(const char* msg) {
  printf("%s", msg);
  fflush(stdout);
  exit(-1);
}

int main(void) {
  int height = 480;
  int width = 640;
  int num_comps = 3;
  int ret;

  unsigned char *y_pixels;
  unsigned char *uv_pixels;
  mem_compressed_target *target = NULL;
  kdu_codestream *cs = NULL;
  kdu_stripe_compressor *enc = NULL;
  kdu_siz_params *siz = NULL;

  unsigned char *buf;
  int buf_sz;

  /* register message handlers */

  kdu_register_error_handler(&exit_with_error);
  kdu_register_warning_handler(&exit_with_error);
  kdu_register_info_handler(&print_message);

  /* create image */

  y_pixels = malloc(height * width);
  if (! y_pixels)
    return 1;

  for(int i = 0; i < height * width; i++)
    y_pixels[i] = (unsigned char) (i & 0xFF);

  uv_pixels = malloc(height * width / 4);
  if (! uv_pixels)
    return 1;

  for(int i = 0; i < height * width / 4; i++)
    uv_pixels[i] = (unsigned char) (i & 0xFF);

  /* initialize siz */

  ret = kdu_siz_params_new(&siz);
  if (ret)
    return ret;

  kdu_siz_params_set_num_components(siz, num_comps);
  kdu_siz_params_set_precision(siz, 0, 8);
  kdu_siz_params_set_size(siz, 0, height, width);
  kdu_siz_params_set_size(siz, 1, height / 2, width / 2);
  kdu_siz_params_set_size(siz, 2, height / 2, width / 2);
  kdu_siz_params_set_signed(siz, 0, 0);

  /* allocate output codestream */

  ret = kdu_compressed_target_mem_new(&target);
  if (ret)
    return ret;

  /* init codestream */

  ret = kdu_codestream_create_from_target(target, siz, &cs);
  if (ret)
    return ret;

  /* compressor */

  ret = kdu_stripe_compressor_new(&enc);
  if (ret)
    return ret;

  kdu_stripe_compressor_options opts;

  kdu_stripe_compressor_options_init(&opts);

  int stripe_heights[3] = {height, height / 2, height / 2};
  int precisions[3] = {8, 8, 8};
  unsigned char* pixels[3] = {y_pixels, uv_pixels, uv_pixels};

  ret = kdu_stripe_compressor_start(enc, cs, &opts);
  if (ret)
    return ret;

  int stop = 0;
  while (!stop) {
    stop = kdu_stripe_compressor_push_stripe_planar(enc, pixels, stripe_heights, NULL, NULL, precisions);
  }

  ret = kdu_stripe_compressor_finish(enc);
  if (ret)
    return ret;

  kdu_codestream_textualize_params(cs, &print_message);

  kdu_compressed_target_bytes(target, &buf, &buf_sz);

  if (buf_sz == 0)
    return 1;

  FILE *j2c_fd = fopen("test_encoder.yuv.j2c", "wb");

  if ((fwrite(buf, 1, buf_sz, j2c_fd) != buf_sz))
    return 1;

  fclose(j2c_fd);

  free(y_pixels);

  free(uv_pixels);

  kdu_stripe_compressor_delete(enc);

  kdu_codestream_delete(cs);

  kdu_compressed_target_mem_delete(target);

  kdu_siz_params_delete(siz);

  return 0;
}
