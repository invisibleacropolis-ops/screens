/*
  stb_image.h - v2.28 - public domain image loader - http://nothings.org/stb
  no warranty implied; use at your own risk
*/
#ifndef STB_IMAGE_H
#define STB_IMAGE_H

#ifdef __cplusplus
extern "C" {
#endif

extern int stbi_set_flip_vertically_on_load(int flag_true_if_should_flip);
extern unsigned char *stbi_load(char const *filename, int *x, int *y,
                                int *channels_in_file,
                                int desired_channels);
extern void stbi_image_free(void *retval_from_stbi_load);

#ifdef __cplusplus
}
#endif

#ifdef STB_IMAGE_IMPLEMENTATION

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned char stbi_uc;

typedef struct
{
  int (*read)(void *user, char *data, int size);
  void (*skip)(void *user, int n);
  int (*eof)(void *user);
} stbi_io_callbacks;

static int stbi__flip_vertically_on_load = 0;

int stbi_set_flip_vertically_on_load(int flag_true_if_should_flip) {
  stbi__flip_vertically_on_load = flag_true_if_should_flip;
  return 1;
}

static int stbi__stdio_read(void *user, char *data, int size) {
  return (int)fread(data, 1, size, (FILE *)user);
}

static void stbi__stdio_skip(void *user, int n) {
  fseek((FILE *)user, n, SEEK_CUR);
}

static int stbi__stdio_eof(void *user) { return feof((FILE *)user); }

static stbi_io_callbacks stbi__stdio_callbacks = {stbi__stdio_read,
                                                  stbi__stdio_skip,
                                                  stbi__stdio_eof};

static unsigned char *stbi__loadppm(const char *filename, int *x, int *y,
                                    int *comp) {
  FILE *f = fopen(filename, "rb");
  if (!f) {
    return NULL;
  }

  char header[3] = {0};
  if (fscanf(f, "%2s", header) != 1 || strcmp(header, "P6") != 0) {
    fclose(f);
    return NULL;
  }

  int width = 0;
  int height = 0;
  int maxval = 0;
  if (fscanf(f, "%d %d %d", &width, &height, &maxval) != 3) {
    fclose(f);
    return NULL;
  }

  if (maxval <= 0 || maxval > 255 || width <= 0 || height <= 0) {
    fclose(f);
    return NULL;
  }

  fgetc(f);

  size_t dataSize = (size_t)width * (size_t)height * 3;
  stbi_uc *data = (stbi_uc *)malloc(dataSize);
  if (!data) {
    fclose(f);
    return NULL;
  }

  if (fread(data, 1, dataSize, f) != dataSize) {
    free(data);
    fclose(f);
    return NULL;
  }

  fclose(f);

  if (stbi__flip_vertically_on_load && height > 1) {
    int rowSize = width * 3;
    stbi_uc *tmp = (stbi_uc *)malloc(rowSize);
    if (tmp) {
      for (int row = 0; row < height / 2; ++row) {
        stbi_uc *rowPtr = data + row * rowSize;
        stbi_uc *oppPtr = data + (height - 1 - row) * rowSize;
        memcpy(tmp, rowPtr, rowSize);
        memcpy(rowPtr, oppPtr, rowSize);
        memcpy(oppPtr, tmp, rowSize);
      }
      free(tmp);
    }
  }

  *x = width;
  *y = height;
  *comp = 3;
  return data;
}

unsigned char *stbi_load(char const *filename, int *x, int *y,
                         int *channels_in_file, int desired_channels) {
  (void)desired_channels;
  if (!filename) {
    return NULL;
  }

  const char *ext = strrchr(filename, '.');
  if (ext && (strcmp(ext, ".ppm") == 0 || strcmp(ext, ".PPM") == 0)) {
    return stbi__loadppm(filename, x, y, channels_in_file);
  }

  return NULL;
}

void stbi_image_free(void *retval_from_stbi_load) { free(retval_from_stbi_load); }

#endif // STB_IMAGE_IMPLEMENTATION

#endif // STB_IMAGE_H
