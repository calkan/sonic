#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include "sonic.h"
sonic *test_sonic;

char bedfile[1024];

int print_help(char *prog_name){

  fprintf(stderr, "\nSONIC: Some Organism's Nucleotide Information Container.\nVersion: %s\n\n", SONIC_VERSION);
  fprintf(stderr, "Usage: %s [arguments]\n\n", prog_name);
  fprintf(stderr, "COMPULSORY PARAMETERS FOR CREATING SONIC:\n");
  fprintf(stderr, "\t--ref        [in.fasta]        : Reference genome FASTA file.\n");
  fprintf(stderr, "\t--gaps       [gap.bed]         : Reference genome assembly gap coodinates (BED).\n");
  fprintf(stderr, "\t--reps       [repeatmasker.out]: RepeatMasker output files (.out).\n");
  fprintf(stderr, "\t--dups       [dups.bed]        : Segmental duplication coordinates in the reference (BED).\n");
  fprintf(stderr, "\t--make-sonic [ref.sonic]       : SONIC file to be created.\n");
  fprintf(stderr, "VALIDATING EXISTING SONIC FILE:\n");
  fprintf(stderr, "\t--test-sonic [ref.sonic]       : SONIC file to be checked for validity.\n");  
  fprintf(stderr, "OPTIONAL PARAMETERS:\n");
  fprintf(stderr, "\t--mei        [Alu:L1:SVA]      : Colon-seperated obile element class names in RepeatMasker file. Default: Alu:L1:SVA.\n");
  fprintf(stderr, "\t--info       [info_string]     : Information string to annotate the SONIC file with, such as version.\n");
  fprintf(stderr, "\t--help                         : Print this help information and exit.\n\n");
  fprintf(stderr, "\t--version                      : Print version information and exit.\n\n");
  return 0;
}

int parse_command_line( int argc, char** argv)
{
        int index;
	int o;

	static int do_make_sonic = 1;
	static int do_test_sonic = 0;
	static int do_load_sonic = 0; // this is for testing purposes only
	char ref_genome[1024];
	char gaps[1024];
	char dups[1024];
	char sonic[1024];
	char reps[1024];
	char mei[1024];
	char info[1024];
	
	ref_genome[0] = 0;
	gaps[0] = 0;
	dups[0] = 0;
	reps[0] = 0;
	mei[0] = 0;
	sonic[0] = 0;
	bedfile[0] = 0;
	
	static struct option long_options[] = 
	{
			{"ref"    , required_argument,   0, 'f'},
			{"gaps"   , required_argument,   0, 'g'},
			{"dups"   , required_argument,   0, 'd'},
			{"reps"   , required_argument,   0, 'r'},
			{"mei"    , required_argument,   0, 'm'},
			{"info"    , required_argument,   0, 'i'},
			{"make-sonic"    , required_argument,	 0, 'c'},
			{"test-sonic"    , required_argument,	 0, 't'},
			{"help"    , no_argument,	 0, 'h'},
			{"version"    , no_argument,	 0, 'v'},
			{"sonic"    , required_argument,	 0, 's'}, /* testing purposes only */
			{"bed"    , required_argument,	 0, 'b'}, /* testing purposes only */
			{0        , 0,                   0,  0 }
	};

	if( argc == 1)
	{
	  return print_help(argv[0]);
	}

	while( ( o = getopt_long( argc, argv, "f:i:g:d:r:m:c:h:s:b:t:v:", long_options, &index)) != -1)
	{
		switch( o)
		{

		case 'f':
			strcpy(  ref_genome, optarg);
			break;

		case 'g':
			strcpy(  gaps, optarg);
			break;

		case 's':
			strcpy(  sonic, optarg);
			do_load_sonic = 1;
			do_make_sonic = 0;
			break;

		case 't':
			strcpy(  sonic, optarg);
			do_test_sonic = 1;
			do_make_sonic = 0;
			break;

		case 'c':
			strcpy(  sonic, optarg);
			do_make_sonic = 1;
			break;

		case 'd':
			strcpy(  dups, optarg);
			break;

		case 'r':
			strcpy(  reps, optarg);
			break;

		case 'm':
			strcpy(  mei, optarg);
			break;

		case 'i':
			strcpy(  info, optarg);
			break;

		case 'b':
			strcpy(  bedfile, optarg);
			break;
		case 'h':
 		        return print_help(argv[0]);
			break;
		case 'v':
		        fprintf(stderr, "\nSONIC: Some Organism's Nucleotide Information Container.\nVersion: %s\n\n", SONIC_VERSION);
		        return 0;
			break;
		}
	}


	
	/* check if --ref   is invoked */
	if( ref_genome [0] == 0 && do_make_sonic)
	{
		fprintf( stderr, "[SONIC CMDLINE ERROR] Please enter reference genome file (FASTA) using the --ref option.\n");
		return 0;
	}

	/* check if --gaps  is invoked */
	if( gaps [0] == 0 && do_make_sonic)
	{
		fprintf( stderr, "[SONIC CMDLINE ERROR] Please enter the assembly gaps file (BED) using the --gaps option.\n");
		return 0;
	}

	/* check if --reps  is invoked */
	if( reps [0] == 0 && do_make_sonic)
	{
		fprintf( stderr, "[SONIC CMDLINE ERROR] Please enter the repeats file (RepeaMasker) using the --reps option.\n");
		return 0;
	}

	/* check if --dups  is invoked */
	if( dups [0] == 0 && do_make_sonic)
	{
		fprintf( stderr, "[SONIC CMDLINE ERROR] Please enter the segmental duplications file (BED) using the --dups option.\n");
		return 0;
	}

	/* check if --mei is invoked. If not, set Alu:L1Hs:SVA as default */
	if( mei [0] == 0 && do_make_sonic)
	{
		strcpy(  mei, "Alu:L1:SVA");
	}

	if (do_load_sonic){
	  test_sonic = sonic_load(sonic);
	  return 1;
	} 

	if (do_test_sonic){
	  test_sonic = sonic_load(sonic);
	  if (test_sonic == NULL){
	    fprintf( stderr, "[SONIC] The SONIC file %s is invalid.\n", sonic);
	    exit (-1);
	  }
	  fprintf( stderr, "[SONIC] Number of chromosomes: %d\n", test_sonic->number_of_chromosomes);
	  fprintf( stderr, "[SONIC] Genome length: %ld\n", test_sonic->genome_length);
	  fprintf( stderr, "[SONIC] The SONIC file %s seems to be valid.\n", sonic);
	  fprintf( stdout, "Memory usage: %0.2f MB.\n", sonic_get_mem_usage());	  
	  return 1;
	} 

	else if (do_make_sonic)
	{
	  sonic_build(ref_genome, gaps, reps, dups, info, sonic);	 
	}	
	
	return 0;

}

int main(int argc, char **argv){

  FILE *bed;

  char chrom[1024]; int s, e;

  sonic_interval *this_interval;
  sonic_repeat *this_repeat;
  
  float gc;

  int loaded = 0;
  
  loaded = parse_command_line(argc, argv);

  if (test_sonic!= NULL && bedfile[0] != 0){
    bed = fopen(bedfile, "r");
    if (bed == NULL)
      return -1;

    
    
    while (fscanf(bed, "%s\t%d\t%d\n", chrom, &s, &e) > 0){
      
      fprintf(stdout, "Search gaps %s-%d-%d\n", chrom, s, e);
      this_interval = sonic_intersect(test_sonic, chrom, s, e, SONIC_GAP);
      sonic_print_interval(this_interval);
      fprintf(stdout, "Search dups %s-%d-%d\n", chrom, s, e);
      this_interval = sonic_intersect(test_sonic, chrom, s, e, SONIC_DUP);
      sonic_print_interval(this_interval); 
      fprintf(stdout, "Search res %s-%d-%d\n", chrom, s, e);
      this_interval = sonic_intersect(test_sonic, chrom, s, e, SONIC_REP);
      sonic_print_interval(this_interval);
      
      gc = sonic_get_gc_content(test_sonic, chrom, s, e);
      fprintf(stdout, "GC %s-%d-%d \t\t %f\n", chrom, s, e, gc);
      

      this_repeat = sonic_is_mobile_element(test_sonic, chrom, s, e, "Alu:L1");
      if (this_repeat == NULL)
	fprintf(stdout, "NOT %s-%d-%d\n", chrom, s, e);
      else
	fprintf(stdout, "MEI %s-%d-%d\t type: %s\n", chrom, s, e, this_repeat->repeat_type);
    } 

    
  }


  if (loaded)
    free_sonic(test_sonic);

  return 0;
}

