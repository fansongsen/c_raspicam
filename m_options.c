#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "m_options.h"

#define MAX_SIZE_FNAME 1024
unsigned timeout=5000;
unsigned threshold=50;
char output_file_name[MAX_SIZE_FNAME] = "/home/pi/output";

setNonDefaultOptions(int argc, char **argv)
{
  int c;

  while (1)
    {
      static struct option long_options[] =
        {
          {"timeout",   required_argument, 0, 't'},
          {"threshold", required_argument, 0, 'd'},
          {"filename",  required_argument, 0, 'f'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "d:t:f:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            	break;
          printf ("option %s", long_options[option_index].name);
          if (optarg)
            printf (" with arg %s", optarg);
          printf ("\n");
          break;

        case 'd':
          fprintf(stderr, "User defined threshold value `%s'\n", optarg);
	  threshold = atoi(optarg);
          break;

        case 'f':
	  if (strlen(optarg) < MAX_SIZE_FNAME)
	  {
          	fprintf(stderr, "User defined filename value `%s'\n", optarg);
		fprintf(stderr, "argument with size: %d\n", strlen(optarg));
		strncpy(output_file_name,optarg,strlen(optarg));
		output_file_name[strlen(optarg)]='\0';
		fprintf(stderr, "new filename: %s\n", output_file_name);
	  }
          break;

        case 't':
          fprintf(stderr, "User defined timeout value `%s'\n", optarg);
	  timeout = atoi(optarg);
          break;

        case '?':
          /* getopt_long already printed an error message. */
          break;

        default:
          abort ();
        }
    }
}
