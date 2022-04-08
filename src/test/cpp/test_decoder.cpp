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

#include <fstream>
#include <vector>
#include "kduc.h"

int main(int argc, char** argv) {
  int height;
  int width;
  int num_comps;

  kdu_compressed_source* source;
  kdu_codestream* cs;
  kdu_stripe_decompressor* d;

  std::vector<uint8_t> pixels;

  std::ifstream j2c_file("resources/counter-00000.j2c", std::ifstream::ate | std::ifstream::in | std::ifstream::binary);
  std::streamsize size = j2c_file.tellg();
  j2c_file.seekg(0, std::ios::beg);

  std::vector<unsigned char> j2c_buffer(size);
  j2c_file.read((char*)j2c_buffer.data(), size);

  kdu_compressed_source_buffered_new(j2c_buffer.data(), j2c_buffer.size(), &source);

  kdu_codestream_create_from_source(source, &cs);

  kdu_codestream_get_size(cs, 0, &height, &width);

  num_comps = kdu_codestream_get_num_components(cs);

  kdu_stripe_decompressor_new(&d);

  pixels.resize(width * height * num_comps);

  int stripe_heights[4] = {height, height, height, height};

  kdu_stripe_decompressor_start(d, cs);

  kdu_stripe_decompressor_pull_stripe(d, pixels.data(), stripe_heights);

  kdu_stripe_decompressor_finish(d);

  kdu_stripe_decompressor_delete(d);

  kdu_codestream_delete(cs);

  kdu_compressed_source_buffered_delete(source);

  return 0;
}