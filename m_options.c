#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "m_options.h"

#define MAX_SIZE_FNAME 1024
unsigned threshold = 255;
char output_file_name[MAX_SIZE_FNAME] = "/home/pi/output";


void usage()
{
	printf ("usage: ./test_movement_detection -t $THRESHOLD -f $OUTPUTDIR/OUTPUTFILE \n");
	printf ("	example: ./test_movement_detection -t 50 -f /home/pi/myTestVideo.h264\n");
	printf ("	where:\n");
	printf ("		THRESHOLD: maximum difference between 2 pixels (0-254)\n");
	printf ("		OUTPUTDIR: directory where the video will be stored\n");
	printf ("		OUTPUTFILE: video file name\n");

	exit(0);
}

setNonDefaultOptions(int argc, char **argv)
{
  int c;

  if (argc == 1)
	  usage();

  while (1)
    {
      static struct option long_options[] =
        {
          {"threshold", required_argument, 0, 't'},
          {"filename",  required_argument, 0, 'f'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "t:f:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1) {
	      if (threshold == 0)
	      {
		      printf("0 is an invalid treshold\n\n");
		      usage();
	      }
        break;
      }

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

        case 't':
          fprintf(stderr, "User defined threshold value `%s'\n", optarg);
	  threshold = atoi(optarg);
	  if ((threshold > 254) || (threshold < 0))
		  usage();
          break;

        case 'f':
	  if (strlen(optarg) < MAX_SIZE_FNAME)
	  {
          	fprintf(stderr, "User defined path: `%s'\n", optarg);
		fprintf(stderr, "argument with size: %d\n", strlen(optarg));
		strncpy(output_file_name,optarg,strlen(optarg));
		output_file_name[strlen(optarg)]='\0';
		fprintf(stderr, "new filename: %s\n", output_file_name);
	  }
          break;

        case '?':
          break;

        default:
          usage();
        }
    }
}
