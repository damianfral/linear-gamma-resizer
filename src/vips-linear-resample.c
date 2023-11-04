#include <vips/vips.h>

int main() {
  if (VIPS_INIT(NULL) != 0) {
    vips_error_exit("Error initializing VIPS");
  }

  VipsImage *input_image = vips_image_new_from_file("test.jpg", NULL);
  VipsImage *resized_image_4K = vips_image_new();
  VipsImage *resized_image_1K = vips_image_new();
  VipsImage *resized_thumbnail = vips_image_new();

  if (input_image == NULL) {
    vips_error_exit("Error loading input image");
  }

  vips_colourspace(input_image, &input_image, VIPS_INTERPRETATION_scRGB, NULL);

  double scaleV = 2160.0 / vips_image_get_height(input_image);
  double scaleH = 3840.0 / vips_image_get_width(input_image);
  double scale;

  if (scaleV < scaleH) {
    scale = scaleV;
  } else {
    scale = scaleH;
  }

  vips_resize(input_image, &resized_image_4K, scale, NULL);
  vips_resize(resized_image_4K, &resized_image_1K, 0.5, NULL);
  vips_resize(resized_image_1K, &resized_thumbnail, 0.25, NULL);

  vips_colourspace(resized_image_4K, &resized_image_4K,
                   VIPS_INTERPRETATION_sRGB, NULL);
  vips_colourspace(resized_image_1K, &resized_image_1K,
                   VIPS_INTERPRETATION_sRGB, NULL);
  vips_colourspace(resized_thumbnail, &resized_thumbnail,
                   VIPS_INTERPRETATION_sRGB, NULL);

  // Save the resized image as AVIF with no chroma subsampling
  if (vips_image_write_to_file(
          resized_image_4K,
          "output-4K.avif[Q=60,subsample_mode=VIPS_FOREIGN_SUBSAMPLE_OFF]",
          NULL) != 0) {
    vips_error_exit("Error saving AVIF output image");
  }

  if (vips_image_write_to_file(
          resized_image_1K,
          "output-1K.avif[Q=50,subsample_mode=VIPS_FOREIGN_SUBSAMPLE_OFF]",
          NULL) != 0) {
    vips_error_exit("Error saving AVIF output image");
  }

  if (vips_image_write_to_file(resized_thumbnail, "output-thumb.avif[Q=50]",
                               NULL) != 0) {
    vips_error_exit("Error saving AVIF output image");
  }

  // Clean up
  g_object_unref(input_image);
  g_object_unref(resized_image_4K);
  g_object_unref(resized_image_1K);
  g_object_unref(resized_thumbnail);
  vips_shutdown();

  return 0;
}
