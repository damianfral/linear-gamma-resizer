#include <vips/vips.h>

double get_scale_factor(int width, int height, VipsImage *img) {
  double scale;
  double scaleV = (double)height / vips_image_get_height(img);
  double scaleH = (double)width / vips_image_get_width(img);

  if (scaleV < scaleH) {
    scale = scaleV;
  } else {
    scale = scaleH;
  }
  return scale;
}

VipsImage *scale_image(int width, int height, VipsImage *img) {
  double scale = get_scale_factor(width, height, img);
  VipsImage *new_image = vips_image_new();
  vips_resize(img, &new_image, scale, NULL);
  return new_image;
}

void export_image(VipsImage *img, char *filepath) {
  char avif_options[64] = "[Q=66,subsample_mode=VIPS_FOREIGN_SUBSAMPLE_OFF]";
  char avif_output[256] = "";
  strcat(avif_output, filepath);
  strcat(avif_output, avif_options);
  vips_colourspace(img, &img, VIPS_INTERPRETATION_sRGB, NULL);
  // Save the resized image as AVIF with no chroma subsampling
  if (vips_image_write_to_file(img, avif_output, NULL) != 0) {
    vips_error_exit("Error saving AVIF output image");
  }
}

int main() {
  if (VIPS_INIT(NULL) != 0) {
    vips_error_exit("Error initializing VIPS");
  }

  VipsImage *input_image = vips_image_new_from_file("test.jpg", NULL);

  if (input_image == NULL) {
    vips_error_exit("Error loading input image");
  }

  vips_colourspace(input_image, &input_image, VIPS_INTERPRETATION_scRGB, NULL);

  VipsImage *resized_image_4K = scale_image(3840, 2160, input_image);
  VipsImage *resized_image_1K = scale_image(1920, 1080, input_image);
  VipsImage *resized_image_thumb = scale_image(480, 270, input_image);

  // Save the resized image as AVIF with no chroma subsampling
  export_image(resized_image_4K, "output-4K.avif");
  export_image(resized_image_1K, "output-HD.avif");
  export_image(resized_image_thumb, "output-thumb.avif");
  // Clean up
  g_object_unref(input_image);
  g_object_unref(resized_image_4K);
  g_object_unref(resized_image_1K);
  g_object_unref(resized_image_thumb);
  vips_shutdown();

  return 0;
}
