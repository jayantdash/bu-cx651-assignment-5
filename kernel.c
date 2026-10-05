#include "loader.h"
#include <stdlib.h>
#include <string.h>

/** Returns p1 with each channel multiplied by scalar. */
struct pixel mul(struct pixel p1, float scalar) {
    return (struct pixel){r: p1.r * scalar, g: p1.g * scalar, b: p1.b * scalar};
}
/** Returns the channel-wise sum of p1 and p2. */
struct pixel add(struct pixel p1, struct pixel p2) {
    return (struct pixel){r: p1.r + p2.r, g: p1.g + p2.g, b: p1.b + p2.b};
}

/**
 * Applies a square kernel to an image (cross-correlation).
 *
 * Produces a new image where each output pixel is the weighted sum of
 * the ksize x ksize neighborhood centered on the corresponding input
 * pixel, multiplied by normalize. The kernel is applied as-is (not
 * flipped), so this is technically cross-correlation; the result is
 * identical to convolution for symmetric kernels.
 *
 * The input img is padded so that kernel operations that fall outside of the 
 * original image are multiplied by a black pixel (zero padding).
 *
 * img        Source image. Not modified.
 * kernel     Kernel weights in row-major order, containing ksize * ksize elements.
 * ksize      Width and height of the kernel. Should be odd
 * normalize  Scale factor applied to each weighted sum
 *                       (e.g., 1.0f / 9 for a 3x3 box blur).
 *
 * Returns a pointer to a newly allocated image with the same dimensions as img.
 *
 */
struct image* apply_kernel(struct image* img, int* kernel, int ksize, float normalize) {
    
    // Validate input parameters
    if (img == NULL || img->pixels == NULL || kernel == NULL || ksize <= 0 || ksize % 2 == 0) {
        return NULL;
    }

    struct image* result = malloc(sizeof(struct image));
    if (result == NULL) {
        return NULL;
    }    
    
    result->width = img->width;
    result->height = img->height;

    result->pixels = malloc(sizeof(struct pixel) * img->width * img->height);
    if (result->pixels == NULL) {
        // Free the previously allocated result structure if pixel allocation fails
        free(result);
        return NULL;
    }

    // Calculate the offset for the kernel center
    int offset = ksize / 2;

    // Apply the kernel to each pixel in the image
    for (int y = 0; y < img->height; y++) {
        // Iterate over each column in the current row
        for (int x = 0; x < img->width; x++) {
            // Initialize the accumulator for the current pixel
            struct pixel accumulator = {0, 0, 0};
            // Iterate over each position in the kernel
            for (int ky = 0; ky < ksize; ky++) {
                // Iterate over each column in the kernel
                for (int kx = 0; kx < ksize; kx++) {
                    int image_x = x + kx - offset;
                    int image_y = y + ky - offset;
                    // Check if the current kernel position maps to a valid image pixel
                    if (image_x >= 0 && image_x < img->width && image_y >= 0 && image_y < img->height) {
                        struct pixel p = img->pixels[image_x + image_y * img->width];
                        int kernel_value = kernel[kx + ky * ksize];
                        // Accumulate the weighted pixel value
                        accumulator = add(accumulator, mul(p, kernel_value));
                    }
                }
            }
            // Store the normalized accumulated value in the result image
            result->pixels[x + y * img->width] = mul(accumulator, normalize);
        }
    }
    return result;
}

