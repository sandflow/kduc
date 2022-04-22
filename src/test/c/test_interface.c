#include <kduc.h>
#include <stdio.h>

int main(void) {
  int height;
  int width;
  int num_comps;
  int ret;

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

  num_comps = kdu_codestream_get_num_components(cs);

  ret = kdu_stripe_decompressor_new(&d);
  if (ret) return ret;

  unsigned char pixels[width * height * num_comps];

  int stripe_heights[4] = {height, height, height, height};

  kdu_stripe_decompressor_options opts;

  kdu_stripe_decompressor_options_init(&opts);

  kdu_stripe_decompressor_start(d, cs, &opts);

  int pull_strip_should_stop = 0;
  while(!pull_strip_should_stop) {
    pull_strip_should_stop = kdu_stripe_decompressor_pull_stripe(d, &pixels[0], stripe_heights);
  }

  ret = kdu_stripe_decompressor_finish(d);
  if (ret) return ret;

  kdu_stripe_decompressor_delete(d);

  kdu_codestream_delete(cs);

  kdu_compressed_source_buffered_delete(source);

  return 0;
}
