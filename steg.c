#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <time.h>


static const size_t NUM_COLS = 2;

typedef struct Pixel{

  int red;
  int green;
  int blue;

}pixel;

typedef struct PPM{

  char code[3];
  char comment[500];
  int width, height;
  int max;
  pixel * pixels;

}ppm;

struct PPM * getPPM(FILE * fd){

  char buffer[3];

  ppm * image;

  //allocates memory for the image
  image = malloc(sizeof(ppm));

  if(image == NULL){
    printf("Failed to allocate memory for image\n\n");
    exit(0);
  }

  //gets first and second character from file
  fscanf(fd, "%c", &buffer[0]);
  fscanf(fd, "%c", &buffer[1]);
  //gets the end of line character
  fscanf(fd, "%c", &buffer[2]);

  //checks image is of type P3 and stores information if it is
  //else show error and exit program
  if(buffer[0] != 'P' || buffer[1] != '3'){
    printf("Not P3 file format!\n\n");
    exit(0);
  }else{
    image->code[0] = 'P';
    image->code[1] = '3';
    image->code[2] = '\0';
  }

  //Will remove white space and get to comment(s) or dimensions
  int c = getc(fd);
  while(c == '\n'){
    c = getc(fd);
  }

  int i=0;
  //Will store the comments
  while(c == '#'){
    //adds characters until end of line
    do{
      image->comment[i] = c;
      c = getc(fd);
      i++;
   }while((c != '\n'));
   //adds end of line character
   image->comment[i] = c;
   //remove white space and get to next comment or dimensions
   while(c == '\n'){
     c = getc(fd);
   }
    i++;
  }
  //c at this point will hold the first number in the width, so undo this
  ungetc(c,fd);

  //adds end of line character
  image->comment[i] = '\0';

  //reads width, height and max and returns error if fails
  if(fscanf(fd, "%d %d %d", &image->width, &image->height, &image->max) != 3){
    printf("Image size is invalid! Width: %d, Height: %d\n\n", image->width, image->height);
    exit(0);
  }

  int numberOfPixels = (image->height * image->width);
  //allocates memory for pixels
  image->pixels = malloc( numberOfPixels * sizeof(pixel));

  //reads in pixels and returns error, exiting the program, if it can't read the read, green and blue pixel
  for(int i = 0; i<numberOfPixels; i++){

    if(fscanf(fd, "%d %d %d", &image->pixels[i].red, &image->pixels[i].green, &image->pixels[i].blue) != 3){
      printf("Error reading pixels!");
      exit(0);
    }

  }

  return image;

}//end of getPPM method


int cmpfunc (const void * a, const void * b){
   return ( *(int*)a - *(int*)b );
}

void showPPM(struct PPM * image){


  int numberOfPixels = (image->height * image->width);

  printf("\nImage Code: %s\n", image->code);
  printf("Comment: %s", image->comment);
  printf("Image width: %d\n", image->width);
  printf("Image height: %d\n", image->height);
  printf("Maximum colour value: %d\n", image->max);

  //loop for number of pixels displaying each one
  for(int i = 0; i<numberOfPixels; i++){
    //take new line if end of row
    if((i % image->width) == 0){
      printf("\n");
    }
      printf("%3d %3d %3d\t", image->pixels[i].red, image->pixels[i].green, image->pixels[i].blue);

  }

  printf("\n");

}//end of showPPM


struct PPM * encode(char * text, struct PPM * image){

  int numberOfPixels = (image->height * image->width);

  int sizeOfMessage = strlen(text);

  //checks if encoding is possible
  while(sizeOfMessage > numberOfPixels){
    printf("Not enough pixels to hide message. Please use a different picture or enter a smaller message.\n");
    exit(0);
  }

  //stores the randomly generated numbers for where to store the characters
  int pixelIndex[sizeOfMessage];

  srand(time(NULL));

  //generates random numbers for the amount of characters in the message
  for(int i=0; i < sizeOfMessage; i++){

    bool hasBeenGeneratedBefore = false;

    //produces random number between 0 and numberOfPixels - 1
    int newRandomNumber = (int) (rand() % numberOfPixels);

    //loops for amount of numbers already generated and stored so can checks
    //if the newly generated number has already been generated
    for(int y =0; y < i; y++){

      if(newRandomNumber == pixelIndex[y])
        hasBeenGeneratedBefore = true;
    }

    if(hasBeenGeneratedBefore == true){
      i--;
    }else{
      pixelIndex[i] = newRandomNumber;
    }

  }

  //performs quicksort on pixelIndex array to sort numbers in ascending order
  qsort(pixelIndex, sizeOfMessage, sizeof(int), cmpfunc);


  //replaces the red value of the pixel at the index of the number stored in the
  //random number array at the current loop index with the ASCII of the
  //character at the current loop index
  for(int i=0; i < sizeOfMessage; i++){
    image->pixels[pixelIndex[i]].red = text[i];
  }


  return image;

}//end of encode

char* stradd(const char* a, const char* b){
  //length of new string
  size_t len = strlen(a) + strlen(b);
  //creates pointer with length of both pointers combined + 1(for end of line character)
  char *ret = (char*)malloc(len * sizeof(char) + 1);
  *ret = '\0';
  //returns the concatenation of both pointers
  return strcat(strcat(ret, a) ,b);
}


char * decode(struct PPM * i1, struct PPM * i2){

  int numberOfPixels = (i2->height * i2->width);
  char * secret = "";


  for(int i = 0; i<numberOfPixels; i++){

    //compares red pixels between original image and altered image
    if(i1->pixels[i].red != i2->pixels[i].red){

      char * c;
      c = malloc(sizeof(char));

      //changes the ascii value to character
      c[0] = i2->pixels[i].red;
      //adds the character to the char pointer
      secret = stradd(secret, c);

    }

  }//end of loop

  return secret;

}//end of decode method

void writePPM(const char *filename, struct PPM *img){
    FILE *fp;

    fp = fopen(filename, "wb");

    if (!fp) {
      fprintf(stderr, "Unable to open file '%s'\n", filename);
      exit(1);
    }

    //writes code, comments, width, height and max to file
    fprintf(fp,"%s\n%s%d %d\n%d", img->code, img->comment,img->width, img->height,
    img->max);

    int numberOfPixels = (img->height * img->width);

    //loop for number of pixels
    for(int i=0; i < numberOfPixels; i++){

      //take new line if width is reached
      if((i % img->width) ==0){
        fprintf(fp, "\n");
      }

      //write pixel values
      fprintf(fp, "%3d %3d %3d\t", img->pixels[i].red, img->pixels[i].green, img->pixels[i].blue);

    }

    fclose(fp);
}

void handleEncoding(const char * originalFilepath, const char * alteredFilepath){


  FILE * originalImageFile;

  //opens file for the original image
  originalImageFile = fopen(originalFilepath, "r");


  if (!originalImageFile) {
    fprintf(stderr, "Unable to open file '%s'\n", originalFilepath);
    exit(1);
  }

  //gets information from image and stores it in struct
  struct PPM * originalImage = getPPM(originalImageFile);

  int numberOfPixels = (originalImage->height * originalImage->width);

  //allocates memory for message and gets message from user
  char *message = malloc(sizeof(char) * (numberOfPixels + 1));
  printf("\nPlease enter a secret message: ");

  //stores message
  scanf("%[^\n]s", message);

  //encodes image with message
  encode(message, originalImage);

  //shows original image information
  showPPM(originalImage);

  //writes new image to file (originalImage info is changed hence the name but is
  //saved with a different filename so the original file will still contain the original
  //information)

  writePPM(alteredFilepath, originalImage);

}//end of handleEncoding

void handleDecoding(const char * originalFilePath, const char* alteredFilePath){

  //opens file for original image
  FILE * originalImageFile = fopen(originalFilePath, "r");
  //opens file for image with hidden message
  FILE * alteredImageFile = fopen(alteredFilePath, "r");

  if (!originalImageFile) {
    fprintf(stderr, "Unable to open file '%s'\n", originalFilePath);
    exit(1);
  }

  if (!alteredImageFile) {
    fprintf(stderr, "Unable to open file '%s'\n", alteredFilePath);
    exit(1);
  }

  //gets information from image and stores it in struct
  struct PPM * originalImage = getPPM(originalImageFile);
  struct PPM * alteredImage = getPPM(alteredImageFile);

  if(originalImage->width != alteredImage->width && originalImage->height !=
  alteredImage->height){
    printf("\nThis is not the original image as the dimensions differ!\n\n");
    exit(0);
  }

  //decodes and prints out hidden message by comparing both structs
  char * secret = decode(originalImage, alteredImage);
  printf("\nSecret message is : %s\n\n", secret);

}//end of handleDecoding

int main( int argc, const char* argv[] ){


  //if the number of arguments is not 4 exit the program
  if(argc != 4){
    printf("\nThere should be 4 arguments!\n\n");
    exit(0);
  }

  //d is the command for decoding
  if(strcmp(argv[1],"d") == 0){
    handleDecoding(argv[2], argv[3]);
  }//e is argument for encoding
  else if (strcmp(argv[1],"e")==0){
    handleEncoding(argv[2], argv[3]);
  }
  else//will warn user of an unrecognised character argument
    printf("\nWrong character entry for 2nd argument. Enter 'E' for encoding, 'D' for deconding.\n\n");


  return 0;
}
