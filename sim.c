#include <stdio.h>
#include <stdlib.h>
#include <math.h>

long long getTotalLines(FILE *fp);
long long totalLines;
#define IGNORE_BITS 2

struct instruction_t      // 3-Dimensional Coordinate Point
{
  unsigned int pCounter;
  long long pt_index;
  unsigned int pt_value;
  unsigned int actual_prediction;
  unsigned char pt_value_updated;
};

typedef struct instruction_t Instruction;
long long true_prediction;

int main(int argc, char *argv[]) //sim bimodal <M> <tracefile>
{
  FILE *inputFD;
  char string[100];

  unsigned int m_value = 0;
  unsigned int m_bits = 0;
  unsigned int n_bits = 0;

  // if (argc != 3)                                                                // Make sure the number of input is correct
  // {
  //   printf("Error! Invalid number of Arguments. Please run program as ./submission inputFile.txt outputFile.txt\n");
  //   return -1;
  // }

  m_bits = atoi(argv[2]); // Getting m bits

  printf("Enter the n # bits: \n");
  scanf("%d", &n_bits);

  for (unsigned int i = 0; i < m_bits; i++)    // Based on m value, creating the m_value
  {
    m_value = (m_value << 1) | 1;
  }
  m_value = m_value << IGNORE_BITS;
  printf("Bit: %d\n", m_value);

  inputFD = fopen(argv[3], "r");                                                // Open file for Reading the input
  if (inputFD == NULL)
  {
    perror("Error opening the input file");
    return -1;
  }

  totalLines = getTotalLines(inputFD);

  long long size = totalLines;
  Instruction *instruction = (Instruction* )malloc(totalLines * sizeof(Instruction));
  unsigned char temp_prediction = ' ';

  size = 100000;
  for (long long i=0; i<size; i++)
  {
    if(instruction[i].pt_value_updated == 0)        // Initiate all PT values to 4
    {
      instruction[i].pt_value = 4;
      instruction[i].pt_value_updated = 1;
    }

    fgets(string, 100, inputFD);
    sscanf(string, "%x %c", &(instruction[i].pCounter), &temp_prediction);
    instruction[i].actual_prediction = (temp_prediction == 't' ? 1 : 0);

    printf("<Line #%lld>	%x	%c\n", i, instruction[i].pCounter, temp_prediction);

    instruction[i].pt_index = instruction[i].pCounter & m_value;
    instruction[i].pt_index = instruction[i].pt_index >> IGNORE_BITS;
    printf("\tPT index:	%lld\n", instruction[i].pt_index);

    if(instruction[instruction[i].pt_index].pt_value_updated == 0)        // Initiate all PT values to 4
    {
      instruction[instruction[i].pt_index].pt_value = 4;
      instruction[instruction[i].pt_index].pt_value_updated = 1;
    }
    printf("\tPT value:	%d\n", instruction[instruction[i].pt_index].pt_value);

    if (instruction[instruction[i].pt_index].pt_value >= 4)
    {
      printf("\tPrediction:	true\n");
      if (instruction[i].actual_prediction == 1)
      {
        if(instruction[instruction[i].pt_index].pt_value < 7 && instruction[instruction[i].pt_index].pt_value >= 0)
        {
          instruction[instruction[i].pt_index].pt_value++;
        }
        instruction[i].pt_value_updated = 1;
        true_prediction++;
      }
      else
      {
        if(instruction[instruction[i].pt_index].pt_value <= 7 && instruction[instruction[i].pt_index].pt_value > 0)
        {
          instruction[instruction[i].pt_index].pt_value--;
        }
        instruction[i].pt_value_updated = 1;
      }
    }
    else
    {
      printf("\tPrediction:	false\n");
      if (instruction[i].actual_prediction == 1)
      {
        if(instruction[instruction[i].pt_index].pt_value < 7 && instruction[instruction[i].pt_index].pt_value >= 0)
        {
          instruction[instruction[i].pt_index].pt_value++;
        }
        instruction[i].pt_value_updated = 1;
      }
      else
      {
        if(instruction[instruction[i].pt_index].pt_value <= 7 && instruction[instruction[i].pt_index].pt_value > 0)
        {
          instruction[instruction[i].pt_index].pt_value--;
        }
        instruction[i].pt_value_updated = 1;
        true_prediction++;
      }
    }
    printf("\tNew PT value:	%d\n", instruction[instruction[i].pt_index].pt_value);

  }

  printf("OUTPUT\n");
  printf("number of predictions:		%lld\n", size);
  printf("number of mispredictions:	%lld\n", (long long)(size - true_prediction));
  printf("misprediction rate:		%.2f%\n", ((float)(size - true_prediction)/size)*100);

  unsigned int sizeofMtable = pow(2, m_bits);

  printf("FINAL BIMODAL CONTENTS\n");

  for (unsigned int i = 0; i < sizeofMtable; i++)
  {
    printf("%d\t%d\n",i,  instruction[i].pt_value);
  }

  fclose(inputFD);
  free(instruction);

  return 0;
}

long long getTotalLines(FILE *fp)
{
  long long totalLines = 0;
  char string[100];

  while(!feof(fp)) {
    fgets(string, 100, fp);
    totalLines++;
  }

  totalLines--;

  if(fseek(fp, 0L, SEEK_SET) == EOF) {
    perror("Error while seeking to begining of file");
    exit(0);
  }

  return totalLines;
}
