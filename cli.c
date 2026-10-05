#include "kernel.h"
#include <string.h>


int generate_pagefault() {

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
        // Call the mmap function here
    } else if(strcmp(argv[1], "convert") == 0) {
        // Call the convert function here
    } else if(strcmp(argv[1], "uconvert") == 0) {
        // Call the uconvert function here
    } else if(strcmp(argv[1], "fault") == 0) {
        return generate_pagefault();
    } else {
        printf("Unknown mode: %s\n", argv[1]);
        return -1;
    }
}
