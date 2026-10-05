#include "kernel.h"
#include <string.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>


// Writes a file, evicts it from the page cache, then maps and reads it to force major faults.
int generate_pagefault() {
    const char* path = "fault.bin";
    struct image img = {0};
    img.width = 2048;
    img.height = 2048;
    size_t npix = (size_t)img.width * img.height;
    img.pixels = calloc(npix, sizeof(struct pixel));
    if (img.pixels == NULL) {
        return 1;
    }

    int rc = saveimage_mmap((char*)path, &img);
    free(img.pixels);
    if (rc != 0) {
        return 1;
    }

    int fd = open(path, O_RDONLY);
    if (fd != -1) {
        fdatasync(fd);
        posix_fadvise(fd, 0, 0, POSIX_FADV_DONTNEED);
        close(fd);
    }

    struct image in = {0};
    in.width = img.width;
    in.height = img.height;
    if (loadimage_mmap((char*)path, &in) != 0) {
        unlink(path);
        return 1;
    }

    volatile long sum = 0;
    for (size_t i = 0; i < npix; i += 4096 / sizeof(struct pixel)) {
        sum += in.pixels[i].r;
    }

    munmap((char*)in.pixels - sizeof(struct image), sizeof(struct image) + npix * sizeof(struct pixel));
    unlink(path);
    return 0;
}

int fun_convert(char** argv) {
    struct image img = {0};
    img.width = atoi(argv[3]);
    img.height = atoi(argv[4]);

    if (img.width <= 0 || img.height <= 0) {
        printf("Invalid image dimensions: %dx%d\n", img.width, img.height);
        return 1;
    }

    if (loadimage(argv[2], &img) != 0) {
        printf("Could not load image: %s\n", argv[2]);
        free(img.pixels);
        return 1;
    }

    int rc = saveimage_mmap(argv[5], &img);
    free(img.pixels);
    if (rc != 0) {
        printf("Could not save image: %s\n", argv[5]);
        return 1;
    }
    return 0;
}

int fun_uconvert(char** argv) {
    struct image img = {0};
    img.width = atoi(argv[3]);
    img.height = atoi(argv[4]);

    if (img.width <= 0 || img.height <= 0) {
        printf("Invalid image dimensions: %dx%d\n", img.width, img.height);
        return 1;
    }

    size_t length = sizeof(struct image) + (size_t)img.width * img.height * sizeof(struct pixel);
    if (loadimage_mmap(argv[2], &img) != 0) {
        printf("Could not load image: %s\n", argv[2]);
        return 1;
    }

    int rc = saveimage(argv[5], &img);
    munmap((char*)img.pixels - sizeof(struct image), length);
    if (rc != 0) {
        printf("Could not save image: %s\n", argv[5]);
        return 1;
    }
    return 0;
}

int fun_mmap(char** argv) {
    struct image img = {0};
    img.width = atoi(argv[3]);
    img.height = atoi(argv[4]);

    if (img.width <= 0 || img.height <= 0) {
        printf("Invalid image dimensions: %dx%d\n", img.width, img.height);
        return 1;
    }

    size_t length = sizeof(struct image) + (size_t)img.width * img.height * sizeof(struct pixel);
    if (loadimage_mmap(argv[2], &img) != 0) {
        printf("Could not load image: %s\n", argv[2]);
        return 1;
    }
    char* base = (char*)img.pixels - sizeof(struct image);

    int kernel[3][3] = {{1,1,1},{1,1,1},{1,1,1}};

    struct image* result = apply_kernel(&img, (int*)kernel, 3, 1.0f / 9.0f);
    if (result == NULL) {
        printf("Could not apply kernel\n");
        munmap(base, length);
        return 1;
    }

    int rc = saveimage_mmap(argv[5], result);
    free(result->pixels);
    free(result);
    munmap(base, length);
    if (rc != 0) {
        printf("Could not save image: %s\n", argv[5]);
        return 1;
    }
    return 0;
}

int fun_apply_kernel(char** argv) {
    struct image newimg = {0};
    
    newimg.width = atoi(argv[3]);
    newimg.height = atoi(argv[4]);

    if (newimg.width <= 0 || newimg.height <= 0) {
        printf("Invalid image dimensions: %dx%d\n", newimg.width, newimg.height);
        return 1;
    }

    if (loadimage(argv[2], &newimg) != 0) {
        printf("Could not load image: %s\n", argv[2]);
        free(newimg.pixels);
        return 1;
    }

    //allocate the space needed for one image and load the image

    int kernel[3][3] = {{1,1,1},{1,1,1},{1,1,1}};

    struct image* result =  apply_kernel(&newimg, (int*)kernel, 3, 1.0f / 9.0f);
    if (result == NULL) {
        printf("Could not apply kernel\n");
        free(newimg.pixels);
        return 1;
    }
    
    int save_result = saveimage(argv[5], result);
    free(result->pixels);
    free(result);
    free(newimg.pixels);
    if (save_result != 0) {
        printf("Could not save image: %s\n", argv[5]);
        return 1;
    }

    return 0;
}

int main(int argc, char** argv){
    // TODO: parse the arguments in argv. 
    // You can expect argv[1] to be the mode
    // You can expect argv[2] to be the filepath
    // You can expect argv[3] to be the integer width
    // You can expect argv[4] to be the integer height
    // You can expect argv[5] to be the output filepath.

    if(argc != 6) {
        printf("Incorrect number of arguments. Expected: ./cli <MODE=kernel|mmap|convert|uconvert|fault> <input_image> <width> <height> <output_image_path>\n");
        return -1;
    }

    // call correct function based on mode    
    if(strcmp(argv[1], "kernel") == 0) {
        // Call the kernel function here
        return fun_apply_kernel(argv);
    } else if(strcmp(argv[1], "mmap") == 0) {
        return fun_mmap(argv);
    } else if(strcmp(argv[1], "convert") == 0) {
        return fun_convert(argv);
    } else if(strcmp(argv[1], "uconvert") == 0) {
        return fun_uconvert(argv);
    } else if(strcmp(argv[1], "fault") == 0) {
        return generate_pagefault();
    } else {
        printf("Unknown mode: %s\n", argv[1]);
        return -1;
    }
}
