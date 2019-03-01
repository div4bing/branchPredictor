#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

long long getTotalLines(FILE *fp);
long long totalLines;
#define ENABLE_DEBUG 0  //Enable debug = 1, Disable debug = 0
#define IGNORE_BITS 2

#define BIMODAL_PREDICTOR 0
#define GSHARE_PREDICTOR 1
#define HYBRID_PREDICTOR 2

struct instruction_t      // Branch struct
{
  unsigned int branch_counter;
  unsigned int actual_prediction;
  long long chooser_index;
  unsigned int chooser_value;
  long long bimodal_pt_index;
  unsigned int bimodal_pt_value;
  long long gshare_pt_index;
  unsigned int gshare_pt_value;
  unsigned char chooser_value_updated;
  unsigned char bimodal_pt_value_updated;
  unsigned char gshare_pt_value_updated;
};

typedef struct instruction_t Instruction;

int getBimodalPrediction(char * argv[]);      // Process Bimodal Predictor
int getGsharePrediction(char * argv[]);       // Process Gshare Predictor
int getHybridPrediction(char * argv[]);       // Process Hybrid Predictor

long long true_prediction;
unsigned int global_predictor_type_flag = BIMODAL_PREDICTOR; // 0 = Bimodal, 1 = Gshare, 2 = Hybrid

int main(int argc, char *argv[])
{
  int ret = -1;

  if (!(strcmp(argv[1], "bimodal")))       // It's a Bimodal type
  {
    global_predictor_type_flag = BIMODAL_PREDICTOR;
  }
  else if(!(strcmp(argv[1], "gshare")))    // It's a gshare type
  {
    global_predictor_type_flag = GSHARE_PREDICTOR;
  }
  else if (!(strcmp(argv[1], "hybrid")))   // It's a hybrid type
  {
    global_predictor_type_flag = HYBRID_PREDICTOR;
  }
  else                        // Invalid predictor
  {
    printf("Error! Invalid Predictor type..\n");
    return -1;
  }

  printf("COMMAND\n");
  for (int c = 0; c < argc; c++)
  {
    printf("%s ", argv[c]);
  }
  printf("\n");

  if (global_predictor_type_flag == BIMODAL_PREDICTOR)
  {
    if (argc != 4)                                                                // Make sure the number of input is correct
    {
      printf("Error! Invalid number of Arguments. Please run program example ./sim bimodal 6 gcc_trace.txt\n");
      return -1;
    }

    ret = getBimodalPrediction(argv);
  }
  else if (global_predictor_type_flag == GSHARE_PREDICTOR)
  {
    if (argc != 5)                                                                // Make sure the number of input is correct
    {
      printf("Error! Invalid number of Arguments. Please run program example ./sim gshare 9 3 gcc_trace.txt\n");
      return -1;
    }

    ret = getGsharePrediction(argv);
  }
  else
  {
    if (argc != 7)                                                                // Make sure the number of input is correct
    {
      printf("Error! Invalid number of Arguments. Please run program example ./sim hybrid 8 14 10 5 gcc_trace.txt\n");
      return -1;
    }

    ret = getHybridPrediction(argv);
  }

  return ret;
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

//----------------------------------- BIMODAL -------------------------------------------------

int getBimodalPrediction(char *argv[])     //sim bimodal <M> <tracefile>
{
  FILE *inputFD;
  char string[100];
  unsigned int m_branch = 0;
  unsigned int m_bits = 0;

  m_bits = atoi(argv[2]); // Getting m bits

  for (unsigned int i = 0; i < m_bits; i++)    // Based on m value, creating the m_branch
  {
    m_branch = (m_branch << 1) | 1;
  }

  m_branch = m_branch << IGNORE_BITS;

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

#if ENABLE_DEBUG
  size = 100000;
#endif

  for (long long i=0; i<size; i++)
  {
    if(instruction[i].bimodal_pt_value_updated == 0)        // Initiate all PT values to 4
    {
      instruction[i].bimodal_pt_value = 4;
      instruction[i].bimodal_pt_value_updated = 1;
    }

    fgets(string, 100, inputFD);
    sscanf(string, "%x %c", &(instruction[i].branch_counter), &temp_prediction);
    instruction[i].actual_prediction = (temp_prediction == 't' ? 1 : 0);

#if ENABLE_DEBUG
    printf("<Line #%lld>	%x	%c\n", i, instruction[i].branch_counter, temp_prediction);
#endif

    instruction[i].bimodal_pt_index = instruction[i].branch_counter & m_branch;
    instruction[i].bimodal_pt_index = instruction[i].bimodal_pt_index >> IGNORE_BITS;

#if ENABLE_DEBUG
    printf("\tPT index:	%lld\n", instruction[i].bimodal_pt_index);
#endif

    if(instruction[instruction[i].bimodal_pt_index].bimodal_pt_value_updated == 0)        // Initiate all PT values to 4
    {
      instruction[instruction[i].bimodal_pt_index].bimodal_pt_value = 4;
      instruction[instruction[i].bimodal_pt_index].bimodal_pt_value_updated = 1;
    }

#if ENABLE_DEBUG
    printf("\tPT value:	%d\n", instruction[instruction[i].bimodal_pt_index].bimodal_pt_value);
#endif

    if (instruction[instruction[i].bimodal_pt_index].bimodal_pt_value >= 4)
    {

#if ENABLE_DEBUG
      printf("\tPrediction:	true\n");
#endif

      if (instruction[i].actual_prediction == 1)
      {
        if(instruction[instruction[i].bimodal_pt_index].bimodal_pt_value < 7 && instruction[instruction[i].bimodal_pt_index].bimodal_pt_value >= 0)
        {
          instruction[instruction[i].bimodal_pt_index].bimodal_pt_value++;
        }
        instruction[i].bimodal_pt_value_updated = 1;
        true_prediction++;
      }
      else
      {
        if(instruction[instruction[i].bimodal_pt_index].bimodal_pt_value <= 7 && instruction[instruction[i].bimodal_pt_index].bimodal_pt_value > 0)
        {
          instruction[instruction[i].bimodal_pt_index].bimodal_pt_value--;
        }
        instruction[i].bimodal_pt_value_updated = 1;
      }
    }
    else
    {

#if ENABLE_DEBUG
      printf("\tPrediction:	false\n");
#endif

      if (instruction[i].actual_prediction == 1)
      {
        if(instruction[instruction[i].bimodal_pt_index].bimodal_pt_value < 7 && instruction[instruction[i].bimodal_pt_index].bimodal_pt_value >= 0)
        {
          instruction[instruction[i].bimodal_pt_index].bimodal_pt_value++;
        }
        instruction[i].bimodal_pt_value_updated = 1;
      }
      else
      {
        if(instruction[instruction[i].bimodal_pt_index].bimodal_pt_value <= 7 && instruction[instruction[i].bimodal_pt_index].bimodal_pt_value > 0)
        {
          instruction[instruction[i].bimodal_pt_index].bimodal_pt_value--;
        }
        instruction[i].bimodal_pt_value_updated = 1;
        true_prediction++;
      }
    }

#if ENABLE_DEBUG
    printf("\tNew PT value:	%d\n", instruction[instruction[i].bimodal_pt_index].bimodal_pt_value);
#endif
  }

  printf("OUTPUT\n");
  printf("number of predictions:		%lld\n", size);
  printf("number of mispredictions:	%lld\n", (long long)(size - true_prediction));
  printf("misprediction rate:		%.2f%\n", ((round)(size - true_prediction)/size)*100);

  unsigned int sizeofPredictionTable = pow(2, m_bits);

  printf("FINAL BIMODAL CONTENTS\n");

  for (unsigned int i = 0; i < sizeofPredictionTable; i++)
  {
    printf("%d\t%d\n",i,  instruction[i].bimodal_pt_value);
  }

  fclose(inputFD);
  free(instruction);

  return 0;
}

//------------------------------------------ GSHARE ------------------------------------------

int getGsharePrediction(char *argv[])      //sim gshare <M> <N> <tracefile>
{
  FILE *inputFD;
  char string[100];
  unsigned int m_branch = 0;
  unsigned int m_bits = 0;
  unsigned int n_bits = 0;
  unsigned int multiplier=1;

  m_bits = atoi(argv[2]); // Getting m bits
  n_bits = atoi(argv[3]); // Getting n bits

  unsigned int *global_branch_history_register = (unsigned int *)malloc(n_bits * sizeof(unsigned int));    // Default is 0
  memset(global_branch_history_register, 0, sizeof(global_branch_history_register));
  unsigned int global_branch_history_register_value = 0;

  for (unsigned int i = 0; i < m_bits; i++)    // Based on m value, creating the m_branch
  {
    m_branch = (m_branch << 1) | 1;
  }

  m_branch = m_branch << IGNORE_BITS;

  inputFD = fopen(argv[4], "r");                                                // Open file for Reading the input
  if (inputFD == NULL)
  {
    perror("Error opening the input file");
    return -1;
  }

  totalLines = getTotalLines(inputFD);

  long long size = totalLines;
  Instruction *instruction = (Instruction *)malloc(totalLines * sizeof(Instruction));
  unsigned char temp_prediction = ' ';

#if ENABLE_DEBUG
  size = 100000;
#endif

  for (long long i=0; i<size; i++)
  {
    if(instruction[i].gshare_pt_value_updated == 0)        // Initiate all PT values to 4
    {
      instruction[i].gshare_pt_value = 4;
      instruction[i].gshare_pt_value_updated = 1;
    }

    fgets(string, 100, inputFD);
    sscanf(string, "%x %c", &(instruction[i].branch_counter), &temp_prediction);
    instruction[i].actual_prediction = (temp_prediction == 't' ? 1 : 0);

#if ENABLE_DEBUG
    printf("<Line #%lld>	%x	%c\n", i, instruction[i].branch_counter, temp_prediction);
#endif

    // Calculate global branch history register value
    multiplier=1;
    global_branch_history_register_value = 0;
    for (unsigned int j=0; j<n_bits; j++)
    {
      global_branch_history_register_value += global_branch_history_register[(n_bits-1)-j]*multiplier;
      multiplier *= 2;
    }

#if 0
    printf("global_branch_history_register_value: %d\n", global_branch_history_register_value);
#endif

    instruction[i].gshare_pt_index = instruction[i].branch_counter & m_branch;
    instruction[i].gshare_pt_index = ((instruction[i].gshare_pt_index >> IGNORE_BITS) ^ global_branch_history_register_value);    // XOR with Global Branch History Register

    // Shift the Global Branch History Register based on actual outcome
    for (unsigned int k = (n_bits - 1); k > 0; k--)
    {
      global_branch_history_register[k] = global_branch_history_register[k-1];
    }
    global_branch_history_register[0] = instruction[i].actual_prediction;

#if ENABLE_DEBUG
    printf("\tPT index:	%lld\n", instruction[i].gshare_pt_index);
#endif

    if(instruction[instruction[i].gshare_pt_index].gshare_pt_value_updated == 0)        // Initiate all PT values to 4
    {
      instruction[instruction[i].gshare_pt_index].gshare_pt_value = 4;
      instruction[instruction[i].gshare_pt_index].gshare_pt_value_updated = 1;
    }

#if ENABLE_DEBUG
    printf("\tPT value:	%d\n", instruction[instruction[i].gshare_pt_index].gshare_pt_value);
#endif

    if (instruction[instruction[i].gshare_pt_index].gshare_pt_value >= 4)
    {

#if ENABLE_DEBUG
      printf("\tPrediction:	true\n");
#endif

      if (instruction[i].actual_prediction == 1)
      {
        if(instruction[instruction[i].gshare_pt_index].gshare_pt_value < 7 && instruction[instruction[i].gshare_pt_index].gshare_pt_value >= 0)
        {
          instruction[instruction[i].gshare_pt_index].gshare_pt_value++;
        }
        instruction[i].gshare_pt_value_updated = 1;
        true_prediction++;
      }
      else
      {
        if(instruction[instruction[i].gshare_pt_index].gshare_pt_value <= 7 && instruction[instruction[i].gshare_pt_index].gshare_pt_value > 0)
        {
          instruction[instruction[i].gshare_pt_index].gshare_pt_value--;
        }
        instruction[i].gshare_pt_value_updated = 1;
      }
    }
    else
    {

#if ENABLE_DEBUG
      printf("\tPrediction:	false\n");
#endif

      if (instruction[i].actual_prediction == 1)
      {
        if(instruction[instruction[i].gshare_pt_index].gshare_pt_value < 7 && instruction[instruction[i].gshare_pt_index].gshare_pt_value >= 0)
        {
          instruction[instruction[i].gshare_pt_index].gshare_pt_value++;
        }
        instruction[i].gshare_pt_value_updated = 1;
      }
      else
      {
        if(instruction[instruction[i].gshare_pt_index].gshare_pt_value <= 7 && instruction[instruction[i].gshare_pt_index].gshare_pt_value > 0)
        {
          instruction[instruction[i].gshare_pt_index].gshare_pt_value--;
        }
        instruction[i].gshare_pt_value_updated = 1;
        true_prediction++;
      }
    }

#if ENABLE_DEBUG
    printf("\tNew PT value:	%d\n", instruction[instruction[i].gshare_pt_index].gshare_pt_value);
    printf("\tBHR now set to:\t");
    for (unsigned int j=0; j < n_bits; j++)
    {
      printf("[%d]", global_branch_history_register[j]);
    }
    printf("\n\n");
#endif
  }

  printf("OUTPUT\n");
  printf("number of predictions:		%lld\n", size);
  printf("number of mispredictions:	%lld\n", (long long)(size - true_prediction));
  printf("misprediction rate:		%.2f%\n", ((round)(size - true_prediction)/size)*100);

  unsigned int sizeofPredictionTable = pow(2, m_bits);

  printf("FINAL GSHARE CONTENTS\n");

  for (unsigned int i = 0; i < sizeofPredictionTable; i++)
  {
    printf("%d\t%d\n",i,  instruction[i].gshare_pt_value);
  }

  fclose(inputFD);
  free(instruction);
  free(global_branch_history_register);

  return 0;
}

//--------------------------------------------- HYBRID --------------------------------------

int getHybridPrediction(char * argv[])    //sim hybrid <K> <M1> <N> <M2> <tracefile>
{
  FILE *inputFD;
  char string[100];

  unsigned int k_bits = 0;
  unsigned int m1_branch = 0;
  unsigned int m2_branch = 0;
  unsigned int chooser_branch = 0;
  unsigned int m1_bits = 0;
  unsigned int n_bits = 0;
  unsigned int m2_bits = 0;
  unsigned int multiplier=1;

  k_bits = atoi(argv[2]); // Getting K bits
  m1_bits = atoi(argv[3]); // Getting M1 bits
  n_bits = atoi(argv[4]); // Getting N bits
  m2_bits = atoi(argv[5]); // Getting M2 bits

  unsigned int *global_branch_history_register = (unsigned int *)malloc(n_bits * sizeof(unsigned int));    // Default is 0
  memset(global_branch_history_register, 0, sizeof(global_branch_history_register));
  unsigned int global_branch_history_register_value = 0;


  // Gshare
  for (unsigned int i = 0; i < m1_bits; i++)    // Based on m1 value, creating the m1_branch
  {
    m1_branch = (m1_branch << 1) | 1;
  }
  m1_branch = m1_branch << IGNORE_BITS;

  // Bimodal
  for (unsigned int i = 0; i < m2_bits; i++)    // Based on m value, creating the m_branch
  {
    m2_branch = (m2_branch << 1) | 1;
  }
  m2_branch = m2_branch << IGNORE_BITS;

  // Chooser
  for (unsigned int i = 0; i < k_bits; i++)    // Based on k value, creating the chooser_branch
  {
    chooser_branch = (chooser_branch << 1) | 1;
  }
  chooser_branch = chooser_branch << IGNORE_BITS;



  inputFD = fopen(argv[6], "r");                                                // Open file for Reading the input
  if (inputFD == NULL)
  {
    perror("Error opening the input file");
    return -1;
  }

  totalLines = getTotalLines(inputFD);

  long long size = totalLines;
  Instruction *instruction = (Instruction *)malloc(totalLines * sizeof(Instruction));
  unsigned char temp_prediction = ' ';

#if ENABLE_DEBUG
  size = 100000;
#endif

  for (long long i=0; i<size; i++)
  {
    //Gshare
    if(instruction[i].gshare_pt_value_updated == 0)        // Initiate all PT values to 4
    {
      instruction[i].gshare_pt_value = 4;
      instruction[i].gshare_pt_value_updated = 1;
    }

    //Bimodal
    if(instruction[i].bimodal_pt_value_updated == 0)        // Initiate all PT values to 4
    {
      instruction[i].bimodal_pt_value = 4;
      instruction[i].bimodal_pt_value_updated = 1;
    }

    //Chooser
    if(instruction[i].chooser_value_updated == 0)        // Initiate all PT values to 1
    {
      instruction[i].chooser_value = 1;
      instruction[i].chooser_value_updated = 1;
    }

    fgets(string, 100, inputFD);
    sscanf(string, "%x %c", &(instruction[i].branch_counter), &temp_prediction);
    instruction[i].actual_prediction = (temp_prediction == 't' ? 1 : 0);

#if ENABLE_DEBUG
    printf("<Line #%lld>	%x	%c\n", i, instruction[i].branch_counter, temp_prediction);
#endif

    // Calculate global branch history register value
    multiplier=1;
    global_branch_history_register_value = 0;
    for (unsigned int j=0; j<n_bits; j++)
    {
      global_branch_history_register_value += global_branch_history_register[(n_bits-1)-j]*multiplier;
      multiplier *= 2;
    }

#if 0
    printf("global_branch_history_register_value: %d\n", global_branch_history_register_value);
#endif

    //Gshare
    instruction[i].gshare_pt_index = instruction[i].branch_counter & m1_branch;
    instruction[i].gshare_pt_index = ((instruction[i].gshare_pt_index >> IGNORE_BITS) ^ global_branch_history_register_value);    // XOR with Global Branch History Register

    //Bimodal
    instruction[i].bimodal_pt_index = instruction[i].branch_counter & m2_branch;
    instruction[i].bimodal_pt_index = instruction[i].bimodal_pt_index >> IGNORE_BITS;

    //Chooser
    instruction[i].chooser_index = instruction[i].branch_counter & chooser_branch;
    instruction[i].chooser_index = instruction[i].chooser_index >> IGNORE_BITS;

    // Shift the Global Branch History Register based on actual outcome
    for (unsigned int k = (n_bits - 1); k > 0; k--)
    {
      global_branch_history_register[k] = global_branch_history_register[k-1];
    }
    global_branch_history_register[0] = instruction[i].actual_prediction;

    //Bimodal
    if(instruction[instruction[i].bimodal_pt_index].bimodal_pt_value_updated == 0)        // Initiate all PT values to 4
    {
      instruction[instruction[i].bimodal_pt_index].bimodal_pt_value = 4;
      instruction[instruction[i].bimodal_pt_index].bimodal_pt_value_updated = 1;
    }

    //Chooser
    if(instruction[instruction[i].chooser_index].chooser_value_updated == 0)        // Initiate all PT values to 4
    {
      instruction[instruction[i].chooser_index].chooser_value = 1;
      instruction[instruction[i].chooser_index].chooser_value_updated = 1;
    }

#if ENABLE_DEBUG
    printf("\tCT index:	%lld\n", instruction[i].chooser_index);
    printf("\tCT value:	%d\n", instruction[instruction[i].chooser_index].chooser_value);
    printf("\tbimodal-PT index:	%lld\n", instruction[i].bimodal_pt_index);
    printf("\tbimodal-PT value:	%d\n", instruction[instruction[i].bimodal_pt_index].bimodal_pt_value);
#endif

    //Gshare
    if(instruction[instruction[i].gshare_pt_index].gshare_pt_value_updated == 0)        // Initiate all PT values to 4
    {
      instruction[instruction[i].gshare_pt_index].gshare_pt_value = 4;
      instruction[instruction[i].gshare_pt_index].gshare_pt_value_updated = 1;
    }

#if ENABLE_DEBUG
    printf("\tgshare-PT index:	%lld\n", instruction[i].gshare_pt_index);
    printf("\tgshare-PT value:	%d\n", instruction[instruction[i].gshare_pt_index].gshare_pt_value);
#endif

    // Chooser -----------------------------------------------------------------------

    if (instruction[instruction[i].chooser_index].chooser_value >= 2)   // Choose value is greater or equals to 2, select Gshare
    {
      // Gshare ------------------------------------------------------------------------

      if (instruction[instruction[i].gshare_pt_index].gshare_pt_value >= 4)
      {

    #if ENABLE_DEBUG
        printf("\tPrediction:	true\n");
    #endif

        if (instruction[i].actual_prediction == 1)
        {
          // Update Chooser value
          if (instruction[instruction[i].bimodal_pt_index].bimodal_pt_value < 4)
          {
            if(instruction[instruction[i].chooser_index].chooser_value < 3 && instruction[instruction[i].chooser_index].chooser_value >= 0)
            {
              instruction[instruction[i].chooser_index].chooser_value++;
            }
            instruction[i].chooser_value_updated = 1;
          }

          if(instruction[instruction[i].gshare_pt_index].gshare_pt_value < 7 && instruction[instruction[i].gshare_pt_index].gshare_pt_value >= 0)
          {
            instruction[instruction[i].gshare_pt_index].gshare_pt_value++;
          }
          instruction[i].gshare_pt_value_updated = 1;
          true_prediction++;
        }
        else
        {
          // Update Chooser value
          if (instruction[instruction[i].bimodal_pt_index].bimodal_pt_value < 4)
          {
            if(instruction[instruction[i].chooser_index].chooser_value <= 3 && instruction[instruction[i].chooser_index].chooser_value > 0)
            {
              instruction[instruction[i].chooser_index].chooser_value--;
            }
            instruction[i].chooser_value_updated = 1;
          }

          if(instruction[instruction[i].gshare_pt_index].gshare_pt_value <= 7 && instruction[instruction[i].gshare_pt_index].gshare_pt_value > 0)
          {
            instruction[instruction[i].gshare_pt_index].gshare_pt_value--;
          }
          instruction[i].gshare_pt_value_updated = 1;
        }
      }
      else
      {

    #if ENABLE_DEBUG
        printf("\tPrediction:	false\n");
    #endif

        if (instruction[i].actual_prediction == 1)
        {
          // Update Chooser value
          if (instruction[instruction[i].bimodal_pt_index].bimodal_pt_value >= 4)
          {
            if(instruction[instruction[i].chooser_index].chooser_value <= 3 && instruction[instruction[i].chooser_index].chooser_value > 0)
            {
              instruction[instruction[i].chooser_index].chooser_value--;
            }
            instruction[i].chooser_value_updated = 1;
          }

          if(instruction[instruction[i].gshare_pt_index].gshare_pt_value < 7 && instruction[instruction[i].gshare_pt_index].gshare_pt_value >= 0)
          {
            instruction[instruction[i].gshare_pt_index].gshare_pt_value++;
          }
          instruction[i].gshare_pt_value_updated = 1;
        }
        else
        {
          // Update Chooser value
          if (instruction[instruction[i].bimodal_pt_index].bimodal_pt_value >= 4)
          {
            if(instruction[instruction[i].chooser_index].chooser_value < 3 && instruction[instruction[i].chooser_index].chooser_value >= 0)
            {
              instruction[instruction[i].chooser_index].chooser_value++;
            }
            instruction[i].chooser_value_updated = 1;
          }

          if(instruction[instruction[i].gshare_pt_index].gshare_pt_value <= 7 && instruction[instruction[i].gshare_pt_index].gshare_pt_value > 0)
          {
            instruction[instruction[i].gshare_pt_index].gshare_pt_value--;
          }
          instruction[i].gshare_pt_value_updated = 1;
          true_prediction++;
        }
      }
  #if ENABLE_DEBUG
      printf("\tNew gshare-PT value:	%d\n", instruction[instruction[i].gshare_pt_index].gshare_pt_value);
  #endif
    }
    else        // Else Select Bimodal
    {

    // Bimodal -----------------------------------------------------------------------

    if (instruction[instruction[i].bimodal_pt_index].bimodal_pt_value >= 4)
    {

  #if ENABLE_DEBUG
      printf("\tPrediction:	true\n");
  #endif

      if (instruction[i].actual_prediction == 1)
      {
        // Update Chooser value
        if (instruction[instruction[i].gshare_pt_index].gshare_pt_value < 4)
        {
          if(instruction[instruction[i].chooser_index].chooser_value <= 3 && instruction[instruction[i].chooser_index].chooser_value > 0)
          {
            instruction[instruction[i].chooser_index].chooser_value--;
          }
          instruction[i].chooser_value_updated = 1;
        }

        if(instruction[instruction[i].bimodal_pt_index].bimodal_pt_value < 7 && instruction[instruction[i].bimodal_pt_index].bimodal_pt_value >= 0)
        {
          instruction[instruction[i].bimodal_pt_index].bimodal_pt_value++;
        }
        instruction[i].bimodal_pt_value_updated = 1;
        true_prediction++;
      }
      else
      {
        // Update Chooser value
        if (instruction[instruction[i].gshare_pt_index].gshare_pt_value < 4)
        {
          if(instruction[instruction[i].chooser_index].chooser_value < 3 && instruction[instruction[i].chooser_index].chooser_value >= 0)
          {
            instruction[instruction[i].chooser_index].chooser_value++;
          }
          instruction[i].chooser_value_updated = 1;
        }

        if(instruction[instruction[i].bimodal_pt_index].bimodal_pt_value <= 7 && instruction[instruction[i].bimodal_pt_index].bimodal_pt_value > 0)
        {
          instruction[instruction[i].bimodal_pt_index].bimodal_pt_value--;
        }
        instruction[i].bimodal_pt_value_updated = 1;
      }
    }
    else
    {

  #if ENABLE_DEBUG
      printf("\tPrediction:	false\n");
  #endif

      if (instruction[i].actual_prediction == 1)
      {
        // Update Chooser value
        if (instruction[instruction[i].gshare_pt_index].gshare_pt_value >= 4)
        {
          if(instruction[instruction[i].chooser_index].chooser_value < 3 && instruction[instruction[i].chooser_index].chooser_value >= 0)
          {
            instruction[instruction[i].chooser_index].chooser_value++;
          }
          instruction[i].chooser_value_updated = 1;
        }

        if(instruction[instruction[i].bimodal_pt_index].bimodal_pt_value < 7 && instruction[instruction[i].bimodal_pt_index].bimodal_pt_value >= 0)
        {
          instruction[instruction[i].bimodal_pt_index].bimodal_pt_value++;
        }
        instruction[i].bimodal_pt_value_updated = 1;
      }
      else
      {
        // Update Chooser value
        if (instruction[instruction[i].gshare_pt_index].gshare_pt_value >= 4)
        {
          if(instruction[instruction[i].chooser_index].chooser_value <= 3 && instruction[instruction[i].chooser_index].chooser_value > 0)
          {
            instruction[instruction[i].chooser_index].chooser_value--;
          }
          instruction[i].chooser_value_updated = 1;
        }

        if(instruction[instruction[i].bimodal_pt_index].bimodal_pt_value <= 7 && instruction[instruction[i].bimodal_pt_index].bimodal_pt_value > 0)
        {
          instruction[instruction[i].bimodal_pt_index].bimodal_pt_value--;
        }
        instruction[i].bimodal_pt_value_updated = 1;
        true_prediction++;
      }
    }

    #if ENABLE_DEBUG
      printf("\tNew bimodal-PT value:	%d\n", instruction[instruction[i].bimodal_pt_index].bimodal_pt_value);
    #endif
    }

#if ENABLE_DEBUG
    printf("\tBHR now set to:\t");
    for (unsigned int j=0; j < n_bits; j++)
    {
      printf("[%d]", global_branch_history_register[j]);
    }
    printf("\n");
    printf("\tNew CT value:	%d\n\n", instruction[instruction[i].chooser_index].chooser_value);
#endif
  }

#if 1
  printf("OUTPUT\n");
  printf("number of predictions:		%lld\n", size);
  printf("number of mispredictions:	%lld\n", (long long)(size - true_prediction));
  printf("misprediction rate:		%.2f%\n", ((round)(size - true_prediction)/size)*100);

  //Chooser
  unsigned int sizeofChooserTable = pow(2, k_bits);

  printf("FINAL CHOOSER CONTENTS\n");

  for (unsigned int i = 0; i < sizeofChooserTable; i++)
  {
    printf("%d\t%d\n",i,  instruction[i].chooser_value);
  }

  //Gshare
  unsigned int sizeofGshareTable = pow(2, m1_bits);

  printf("FINAL GSHARE CONTENTS\n");

  for (unsigned int i = 0; i < sizeofGshareTable; i++)
  {
    printf("%d\t%d\n",i,  instruction[i].gshare_pt_value);
  }

  //Bimodal
  unsigned int sizeofPredictionTable = pow(2, m2_bits);

  printf("FINAL BIMODAL CONTENTS\n");

  for (unsigned int i = 0; i < sizeofPredictionTable; i++)
  {
    printf("%d\t%d\n",i,  instruction[i].bimodal_pt_value);
  }
#endif

  fclose(inputFD);
  free(instruction);
  free(global_branch_history_register);

  return 0;
}
