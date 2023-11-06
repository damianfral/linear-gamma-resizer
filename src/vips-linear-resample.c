#include <omp.h>
#include <stdio.h>
#include <vips/vips.h>

double get_scale_factor(int width, int height, VipsImage *img) {
  double scale;
  double scaleV = (double)height / vips_image_get_height(img);
  double scaleH = (double)width / vips_image_get_width(img);

  if (scaleV < scaleH)
    return scaleV;
  else
    return scaleH;
}

VipsImage *scale_image(int width, int height, VipsImage *img) {
  double scale = get_scale_factor(width, height, img);
  VipsImage *new_image = vips_image_new();
  vips_resize(img, &new_image, scale, NULL);
  return new_image;
}

void export_image(VipsImage *img, const char *filepath) {
  const char avif_options[] =
      "[Q=66,subsample_mode=VIPS_FOREIGN_SUBSAMPLE_OFF,effort=0]";
  char avif_output[512] = "";
  strcat(avif_output, filepath);
  strcat(avif_output, avif_options);
  vips_colourspace(img, &img, VIPS_INTERPRETATION_sRGB, NULL);
  // Save the resized image as AVIF with no chroma subsampling
  if (vips_image_write_to_file(img, avif_output, NULL) != 0)
    vips_error_exit("Error saving AVIF output image");
}

void export_image_vif(VipsImage *img) {
  vips_colourspace(img, &img, VIPS_INTERPRETATION_sRGB, NULL);
  // Save the resized image as AVIF with no chroma subsampling
  if (vips_image_write_to_file(img, "output.vips", NULL) != 0)
    vips_error_exit("Error saving AVIF output image");
  g_object_unref(img);
}

void remove_extension(const char *filepath, char *new_filepath) {
  int size = strlen(filepath);
  int i;
  for (i = size - 1; i > 0; i--) {
    if (filepath[i] == '.') {
      break;
    }
  }
  if (i == 0)
    strcpy(new_filepath, filepath);
  else
    strncpy(new_filepath, filepath, i);
  new_filepath[i] = '\0';
}

void add_extension(const char *extension, const char *filepath, char *result) {
  strcpy(result, filepath);
  strcat(result, extension);
}

void replace_extension(const char *new_extension, const char *filepath,
                       char *result) {
  char filepath_no_extension[512];
  remove_extension(filepath, filepath_no_extension);
  add_extension(new_extension, filepath_no_extension, result);
}

const int linearRGB = VIPS_INTERPRETATION_RGB16;

int main(int argc, char *argv[]) {
  if (VIPS_INIT(NULL) != 0)
    vips_error_exit("Error initializing VIPS");
  VipsImage **input_images = malloc((argc - 1) * sizeof *input_images);
  VipsImage **resized_images = malloc((argc - 1) * sizeof *resized_images);

  const int sizes[3][2] = {{3840, 2160}, {1920, 1080}, {480, 270}};
  const char *extensions[3] = {"-lg.avif", "-md.avif", "-sm.avif"};

#pragma omp parallel for
  for (int i = 1; i < argc; i++) {
    const char *input_image_path = argv[i];
    VipsImage *input_image = input_images[i - 1];
    VipsImage *resized_image = resized_images[i - 1];
    fprintf(stderr, "Loading image %s\n", input_image_path);
    input_image = vips_image_new_from_file(input_image_path, NULL);

    if (input_image == NULL)
      vips_error_exit("Error loading input image");

    vips_colourspace(input_image, &input_image, linearRGB, NULL);

#pragma omp parallel for
    for (int j = 0; j < 3; j++) {
      char resized_image_path[512] = "";
      replace_extension(extensions[j], input_image_path, resized_image_path);
      resized_image = scale_image(sizes[j][0], sizes[j][1], input_image);
      printf("%s\n", resized_image_path);
      export_image(resized_image, resized_image_path);
    }
    g_object_unref(input_image);
  }
  // Clean up
  vips_shutdown();
  fflush(stdout);
  free(input_images);
  free(resized_images);

  return 0;
}
