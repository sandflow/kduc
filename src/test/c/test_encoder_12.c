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
  int pad_pix_count = 4;
  int num_comps = 3;
  int ret;

  uint16_t *pixels;
  mem_compressed_target *target = NULL;
  kdu_codestream *cs = NULL;
  kdu_stripe_compressor *enc = NULL;
  kdu_siz_params *siz = NULL;

  unsigned char *buf;
  int buf_sz;

  /* register message handlers */

  kdu_register_error_handler(&print_message);
  kdu_register_warning_handler(&print_message);
  kdu_register_info_handler(&print_message);

  /* create image */

  pixels = malloc(height * (width + pad_pix_count) * num_comps * sizeof(*pixels));
  if (! pixels)
    return 1;

  for(int i = 0; i < height; i++) {
    for(int j = 0; j < width; j++) {
      int pix_offset = (i * (width + pad_pix_count) + j) * num_comps;
      pixels[pix_offset] = j << 12;
      pixels[pix_offset + 1] = j << 12;
      pixels[pix_offset + 2] = j << 12;
    }
  }

  /* initialize siz */

  ret = kdu_siz_params_new(&siz);
  if (ret)
    return ret;

  kdu_siz_params_set_num_components(siz, num_comps);
  kdu_siz_params_set_precision(siz, 0, 12);
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

  /* compressor */

  ret = kdu_stripe_compressor_new(&enc);
  if (ret)
    return ret;

  kdu_stripe_compressor_options opts;

  kdu_stripe_compressor_options_init(&opts);

  int stripe_heights[3] = {height, height, height};
  int precisions[3] = {12, 12, 12};
  int row_gaps[3] = {(width + pad_pix_count) * num_comps,(width + pad_pix_count) * num_comps, (width + pad_pix_count) * num_comps};
  bool is_signed[3] = {false, false, false};

  ret = kdu_stripe_compressor_start(enc, cs, &opts);
  if (ret)
    return ret;

  int stop = 0;
  while (!stop) {
    stop = kdu_stripe_compressor_push_stripe_16(
        enc, pixels, stripe_heights, NULL, NULL, row_gaps, precisions, is_signed);
  }

  ret = kdu_stripe_compressor_finish(enc);
  if (ret)
    return ret;

  kdu_codestream_textualize_params(cs, &print_message);

  kdu_compressed_target_bytes(target, &buf, &buf_sz);

  if (buf_sz == 0)
    return 1;

  FILE *j2c_fd = fopen("test_encoder_12.j2c", "wb");

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
