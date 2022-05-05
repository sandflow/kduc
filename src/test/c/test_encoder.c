#include <kduc.h>
#include <stdio.h>
#include <stdlib.h>

static int is_error = 0;

void print_message(const char* msg) {
  printf("%s", msg);
  fflush(stdout);
}

int main(void) {
  int height = 480;
  int width = 640;
  int num_comps = 3;
  int ret;

  unsigned char *pixels;
  mem_compressed_target *target = NULL;
  kdu_codestream *cs = NULL;
  kdu_stripe_compressor *enc = NULL;
  kdu_siz_params *siz = NULL;

  unsigned char *buf;
  int buf_sz;

  /* register message handlers */

  kdu_register_error_handler(&print_message);
  kdu_register_warning_handler(&print_message);

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

  ret = kdu_codestream_parse_params(cs, "Ctype=N");
  if (ret)
    return ret;

  ret = kdu_codestream_parse_params(cs, "Qweights=1.732051,1.805108,1.573402");
  if (ret)
    return ret;

  ret = kdu_codestream_parse_params(cs, "Qfactor=85");
  if (ret)
    return ret;

  /* compressor */

  ret = kdu_stripe_compressor_new(&enc);
  if (ret)
    return ret;

  kdu_stripe_compressor_options opts;

  kdu_stripe_compressor_options_init(&opts);

  int stripe_heights[3] = {height, height, height};

  ret = kdu_stripe_compressor_start(enc, cs, &opts);
  if (ret)
    return ret;

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
