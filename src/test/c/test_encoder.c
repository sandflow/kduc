#include <kduc.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  int height = 480;
  int width = 640;
  int num_comps = 3;
  unsigned char *pixels;
  mem_compressed_target *target;
  int ret;

  kdu_codestream *cs;
  mem_compressed_target *source;
  kdu_stripe_compressor *enc;
  kdu_siz_params *siz;

  unsigned char *buf;
  int buf_sz;

  /* create image */

  pixels = malloc(height * width * num_comps);
  if (! pixels)
    return 1;

  for(int i = 0; i < height * width * num_comps; i++)
    pixels[i] = (unsigned char) (i & 0xFF);

  /* initialize siz */

  ret = kdu_siz_params_new(&siz);
  if (ret)
    return ret;

  kdu_siz_params_set_num_components(siz, num_comps);
  kdu_siz_params_set_precision(siz, 0, 8);
  kdu_siz_params_set_size(siz, 0, height, width);
  kdu_siz_params_set_signed(siz, 0, 0);

  /* allocate output codestream */

  ret = kdu_compressed_target_mem_new(&target);
  if (ret)
    return ret;

  /* init codestream */

  ret = kdu_codestream_create_from_target(target, siz, &cs);
  if (ret)
    return ret;

  ret = kdu_codestream_parse_params(cs, "Cmodes=HT");
  if (ret)
    return ret;

  /* compressor */

  ret = kdu_stripe_compressor_new(&enc);
  if (ret)
    return ret;

  kdu_stripe_compressor_options opts;

  kdu_stripe_compressor_options_init(&opts);

  int stripe_heights[3] = {height, height, height};

  kdu_stripe_compressor_start(enc, cs, &opts);

  int stop = 0;
  while (!stop) {
    stop = kdu_stripe_compressor_push_stripe(enc, pixels, stripe_heights);
  }

  ret = kdu_stripe_compressor_finish(enc);
  if (ret)
    return ret;

  kdu_compressed_target_bytes(target, &buf, &buf_sz);

  if (buf_sz == 0)
    return 1;

  FILE *j2c_fd = fopen("test_encoder.j2c", "wb");

  if ((fwrite(buf, 1, buf_sz, j2c_fd) != buf_sz))
    return 1;

  fclose(j2c_fd);

  free(pixels);

  kdu_stripe_compressor_delete(enc);

  kdu_codestream_delete(cs);

  kdu_compressed_target_mem_delete(target);

  kdu_siz_params_delete(siz);

  return 0;
}
