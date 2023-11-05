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

void export_image(VipsImage *img, char *filepath) {
  char avif_options[] = "[Q=66,subsample_mode=VIPS_FOREIGN_SUBSAMPLE_OFF]";
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
}

void remove_file_extension(const char *filepath, char *new_filepath) {
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
}

void add_file_extension(const char *extension, const char *filepath,
                        char *result) {
  strcpy(result, filepath);
  strcat(result, extension);
}

void replace_file_extension(const char *new_extension, const char *filepath,
                            char *result) {
  char filepath_no_extension[512];
  remove_file_extension(filepath, filepath_no_extension);
  add_file_extension(new_extension, filepath_no_extension, result);
}

int main(int argc, char *argv[]) {
  if (VIPS_INIT(NULL) != 0)
    vips_error_exit("Error initializing VIPS");
  char *input_image_path;
  char image_4K_path[512];
  char image_1K_path[512];
  char image_thumb_path[512];
  VipsImage *input_image;
  VipsImage *image_4K;
  VipsImage *image_1K;
  VipsImage *image_thumb;

  for (int i = 1; i < argc; i++) {
    input_image_path = argv[i];
    replace_file_extension("-4K.avif", input_image_path, image_4K_path);
    replace_file_extension("-1K.avif", input_image_path, image_1K_path);
    replace_file_extension("-thumb.avif", input_image_path, image_thumb_path);

    printf("Writting %s %s %s\n", image_4K_path, image_1K_path,
           image_thumb_path);
    printf("Processing image %s\n", input_image_path);

    fflush(stdout);

    input_image = vips_image_new_from_file(input_image_path, NULL);

    if (input_image == NULL)
      vips_error_exit("Error loading input image");

    vips_colourspace(input_image, &input_image, VIPS_INTERPRETATION_scRGB,
                     NULL);
    // export_image_vif(input_image);

    image_4K = scale_image(3840, 2160, input_image);
    image_1K = scale_image(1920, 1080, input_image);
    image_thumb = scale_image(480, 270, input_image);

    // Save the resized image as AVIF with no chroma subsampling
    export_image(image_4K, image_4K_path);
    export_image(image_1K, image_1K_path);
    export_image(image_thumb, image_thumb_path);
  }
  // Clean up
  g_object_unref(input_image);
  g_object_unref(image_4K);
  g_object_unref(image_1K);
  g_object_unref(image_thumb);
  vips_shutdown();

  return 0;
}
