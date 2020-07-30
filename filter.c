#include <stdio.h>
#include <stdlib.h>
#define PR(i, j) inPPM->red[((i) * (inPPM->width)) + (j)]
#define PG(i, j) inPPM->green[((i) * (inPPM->width)) + (j)]
#define PB(i, j) inPPM->blue[((i) * (inPPM->width)) + (j)]
#define K(i, j) kernel->elements[((i) * (kernel->dimension)) + (j)]

typedef struct
{
    int width, height, *red, *green, *blue;
} PPM;
typedef struct
{
    int dimension, scale, *elements;
} Kernel;

PPM* readPPM(char* inPPMname)
{
    int counter1, counter2, rgbsize;
    char filetype[4];
    PPM* inPPM;
    FILE *file;

    file = fopen(inPPMname, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Cannot open %s, must already exist to read.\n", inPPMname);
        return NULL;
    }

    fscanf(file, "%c%c", &filetype[0], &filetype[1]); 
    if (filetype[0] != 'P' || filetype[1] != '3')
    {
        fprintf(stderr, "Incorrect format of %s, must be P3.\n", inPPMname);
        return NULL;
    }

    inPPM = malloc(sizeof(PPM));
    if (inPPM == NULL)
    {
        fprintf(stderr, "Cannot allocate PPM memory for %s.\n", inPPMname);
        return NULL;
    }

    fscanf(file, "%d", &inPPM->width);
    fscanf(file, "%d", &inPPM->height);
    if(inPPM->width < 0 || inPPM->height < 0)
    {
        fprintf(stderr, "Incorrect dimensions for %s, must be both greater than 0.\n", inPPMname);
    }

    fscanf(file, "%d", &rgbsize);

    inPPM->red = malloc((inPPM->width) * (inPPM->height) * sizeof(int));
    inPPM->green = malloc((inPPM->width) * (inPPM->height) * sizeof(int));
    inPPM->blue = malloc((inPPM->width) * (inPPM->height) * sizeof(int));
    if (inPPM->red == NULL || inPPM->green == NULL || inPPM->blue == NULL)
    {
        fprintf(stderr, "Cannot allocate PPM RGB memory for %s.\n", inPPMname);
    }

    for(counter1 = 0; counter1 < inPPM->height; counter1++)
    {
        for(counter2 = 0; counter2 < inPPM->width; counter2++)
        {
            fscanf(file, "%d", &PR(counter1, counter2));
            fscanf(file, "%d", &PG(counter1, counter2));
            fscanf(file, "%d", &PB(counter1, counter2));
            if (PR(counter1, counter2) < 0 || PR(counter1, counter2) > 255 || PG(counter1, counter2) < 0 || PG(counter1, counter2) > 255 || PB(counter1, counter2) < 0 || PB(counter1, counter2) > 255)
            {       
                fprintf(stderr, "Incorrect RGB element size detected in %s, must be greater than or equal to 0 and less than or equal to 255.\n", inPPMname);
            }
        }
    }

    fclose(file);

    return inPPM;
}

Kernel* readKernel(char* kernelname)
{
    int counter1, counter2;
    Kernel* kernel;
    FILE* file;

    file = fopen(kernelname, "r");
    if (file == NULL)
    {
        fprintf(stderr, "Canot open %s, must already exist to read.\n", kernelname);
        return NULL;
    }

    kernel = malloc(sizeof(Kernel));
    if (kernel == NULL)
    {
        fprintf(stderr, "Cannot allocate kernel memory for %s.\n", kernelname);
        return NULL;
    }

    fscanf(file, "%d", &kernel->dimension);
    fscanf(file, "%d", &kernel->scale);
    if (kernel->dimension <= 0)
    {
        fprintf(stderr, "Incorrect dimension for %s, must be greater than 0.\n", kernelname);
        return NULL;
    }

    kernel->elements = malloc((kernel->dimension) * (kernel->dimension) * sizeof(int));
    if (kernel->elements == NULL)
    {
        fprintf(stderr, "Cannot allocate kernel elements memory for %s.\n", kernelname);
    }

    for(counter1 = 0; counter1 < kernel->dimension; counter1++)
    {
        for(counter2 = 0; counter2 < kernel->dimension; counter2++)
        {
            fscanf(file, "%d", &K(counter1, counter2));
        }
    }

    fclose(file);

    return kernel;
}

int writePPM(PPM* inPPM, Kernel* kernel, char* outPPMname)
{
    int counter1, counter2, counter3, counter4, reddotprod, bluedotprod, greendotprod;
    FILE* file;

    file = fopen(outPPMname, "w");
    if (file == NULL)
    {
        fprintf(stderr, "Canot open %s for writing.\n", outPPMname);
        return 1;
    }

    fprintf(file, "P%d\n%d %d\n%d\n", 3, inPPM->width, inPPM->height, 255);

    for (counter1 = 0 - (kernel->dimension / 2) ; counter1 < inPPM->height - (kernel->dimension / 2); counter1++)
    {
        for (counter2 = 0 - (kernel->dimension / 2); counter2 < inPPM->width - (kernel->dimension / 2); counter2++)
        {
            reddotprod = 0;
            greendotprod = 0;
            bluedotprod = 0;

            for (counter3 = 0; counter3 < kernel->dimension; counter3++)
            {
                for (counter4 = 0; counter4 < kernel->dimension; counter4++)
                {
                    if (counter1 + counter3 >= 0 && counter1 + counter3 < inPPM->height && counter2 + counter4 >= 0 && counter2 + counter4 < inPPM->width)
                    {
                        reddotprod += PR(counter1 + counter3, counter2 + counter4) * K(counter3, counter4);
                        greendotprod += PG(counter1 + counter3, counter2 + counter4) * K(counter3, counter4);
                        bluedotprod += PB(counter1 + counter3, counter2 + counter4) * K(counter3, counter4);
                    }
                }
            }

            reddotprod = reddotprod / kernel->scale;
            greendotprod = greendotprod / kernel->scale;
            bluedotprod = bluedotprod / kernel->scale;

            if (reddotprod < 0)
            {
                fprintf(file, "%d ", 0);
            }
            else if (reddotprod > 255)
            {
                fprintf(file, "%d ", 255);
            }
            else
            {
                fprintf(file, "%d ", reddotprod);
            }
            if (greendotprod < 0)
            {
                fprintf(file, "%d ", 0);
            }
            else if (greendotprod > 255)
            {
                fprintf(file, "%d ", 255);
            }
            else
            {
                fprintf(file, "%d ", greendotprod);
            }
            if (bluedotprod < 0)
            {
                fprintf(file, "%d ", 0);
            }
            else if (bluedotprod > 255)
            {
                fprintf(file, "%d ", 255);
            }
            else
            {
                fprintf(file, "%d ", bluedotprod);
            }
        }

        fprintf(file, "\n");
    }

    fclose(file);

    return 0;
}

int main(int argc, char** argv)
{
    if (argc != 4)
    {
        fprintf(stderr, "Usage: ./filter input.ppm kernel output.ppm\n");
        return 1;
    }

    int checker;
    char* inPPMname = argv[1];
    char* kernelname = argv[2];
    char* outPPMname = argv[3];

    PPM* inPPM = readPPM(inPPMname);
    if (inPPM == NULL)
    {
        fprintf(stderr, "Could not read input PPM.\n");
        return 1;
    }

    Kernel* kernel = readKernel(kernelname);
    if (kernel == NULL)
    {
        fprintf(stderr, "Could not read input kernel.\n");
        return 1;
    }

    checker = writePPM(inPPM, kernel, outPPMname);
    if (checker != 0)
    {
        fprintf(stderr, "Could not write output PPM.\n");
        return 1;
    }

    free(inPPM->red), free(inPPM->green), free(inPPM->blue), free(inPPM);
    free(kernel->elements), free(kernel);

    return 0;
}