# sonic
Some Organism's Nucleotide Information Container

# Fetching and building SONIC

	git clone https://github.com/calkan/sonic.git
	make
       
	
# Standalone SONIC builder example:

	sonic --ref hg19.fasta --dups hg19.dups.bed --reps hg19_repeats.out --gaps hg19.gap.bed --make-sonic ucsc_hg19.sonic --info "UCSC_hg19"

	Duplications and gaps are expected in BED format. Repeats are in RepeatMasker .out format.

# Important things to consider when building and using SONIC files:

Some genomes (for example, human) may have different naming standards for chromosomes. The UCSC version of human reference version hg19 is equivalent to GRC Build 37 (except the mitochondria), but the naming 
schemes are different. UCSC hg19 names chromosomes as chr1, chr2, etc.; where GRC Build 37 uses 1, 2, etc. Fortunately this irregularity is solved in hg38/GRCh38. Similarly, UCSC version of C. elegans 
chromosomes are named as chrI, chrII, and Wormbase version is named as I, II, etc. Same goes for mouse genome, and many others.

When building a SONIC file, or using it with the API we provide, make sure that chromosome naming matches across all your files: reference FASTA, RepeatMasker .out, gaps, and duplications. 
Also make sure that your BAM file matches if you are using SONIC within TARDIS or VALOR, or any other tool. 

# Downloading the annotation files:

To create a .sonic file, you need to prepare the files for reference genome annotations as follows:

Using UCSC sequence and annotations [download page](http://hgdownload.cse.ucsc.edu/downloads.html);


1 - For reference genome, navigate to the related genome's "Full data set" page and download the chromFa FASTA files (i.e., http://hgdownload.cse.ucsc.edu/goldenPath/mm10/bigZips/chromFa.tar.gz is for mouse genome GRCm38/mm10)

Extract the files into a folder and merge them into a single .fasta file:
	
	cat * >ref.fasta

Sonic also requires the index file in the same folder:
	
	samtools faidx ref.fasta


2 - For repeat annotations, navigate to the related genome's "Full data set" page and download the RepeatMasker .out files (i.e., http://hgdownload.cse.ucsc.edu/goldenPath/mm10/bigZips/chromOut.tar.gz is for mouse genome GRCm38/mm10)

Extract the files into a folder and merge them into a single .out file:
	
	cat * >reps.out
	
* Alternatively, if the files are inside directories for each chromosome, then use:

	cat */* >reps.out


3 - For gap annotations, navigate to the related genome's "Full data set" page and download the description of how the assembly was generated (i.e., http://hgdownload.cse.ucsc.edu/goldenPath/mm10/bigZips/chromAgp.tar.gz is for mouse genome GRCm38/mm10)

Extract the files into a folder, merge them and grep the ones with component type U or N, then convert it to bed format:
	
	cat *|awk '{ if($5=="N" || $5=="U") print $1"\t"$2"\t"$3}' > gaps.bed

* Alternatively, if the files are inside directories for each chromosome, then use:

	cat */*|awk '{ if($5=="N" || $5=="U") print $1"\t"$2"\t"$3}' > gaps.bed

* If the genome you are using has no assembly gaps, you can use an empty file. The gaps file is used to filter out calls that span assembly gaps, 
certainly an issue in mammalian genomes since those gaps are surrounded by repeats, causing mapping ambiguity. Create one as:
	
	touch gaps.bed

4 - For segmental duplication annotations, navigate to the related genome's "Annotation Database" page and download genomicsSuperDups file (i.e., http://hgdownload.soe.ucsc.edu/goldenPath/mm10/database/genomicSuperDups.txt.gz is for mouse genome GRCm38/mm10)

Extract the file into a folder and convert it to BED format and sort and merge using [BEDtools](http://bedtools.readthedocs.io/en/latest/):
	
	 cat genomicSuperDups.txt | awk '{OFS="\t"; print $2,$3,$4"\n"$8,$9,$10}'| sortBed | mergeBed > dups.bed

* If the genome you are using has no segmental duplications, you can use an empty file. The duplications file is used to filter out calls that may have originated
due to ambiguous mapping within segmental duplications. Create one as:
	
	touch dups.bed


* You can copy ref.fasta, ref.fasta.fai, reps.out, gaps.bed and dups.bed into the folder where you want to run sonic and remove the others. Make sure all the annotations and the reference are of the same genome and version.


# API Documentation

## Building a SONIC file

	int sonic_build(char *ref_genome, char *gaps, char *reps, char *dups, char *info, char *sonic);

	ref_genome: path to the reference genome file (FASTA) [input]. Also requires the ref_genome.fai file in the same directory (samtools faidx).
	gaps: path to the gaps file (BED) [input].
	reps: path to the repeat annotation file (UCSC/RepeatMasker .out) [input].
	dups: path to the segmental duplications annotation file (BED) [input].
	info: information string to annotate the SONIC file [input].
	sonic: path to the SONIC file [output].
	
	RETURN VALUES:
		1: success
		5: file open error
		7: annotation files error
		

## Loading a SONIC file

	sonic *sonic_load(char *sonic_file_name);

	sonic_file_name: path to the SONIC file [input].

	RETURN VALUE:
		a SONIC.

## General-purpose interval search

	sonic_interval *sonic_intersect(sonic *sonic, char *chromosome, int start, int end, sonic_interval_type interval_type);

	sonic: Loaded SONIC.
	chromosome: name of the chromosome of the interval of interest.
	start: start coordinate of the interval of interest (inclusive -- BED-like).
	end: end coordinate of the interval of interest (exclusive -- BED-like).
	interval_type:
		SONIC_GAP: search gap annotation.
		SONIC_DUP: search segmental duplication annotation.
		SONIC_REP: search repeats annotation.

	RETURN VALUE:
		pointer to a SONIC interval data structure on success.
		NULL if not found.

## Check if interval hits a satellite region

	float sonic_is_satellite(sonic *sonic, char *chromosome, int start, int end);

	sonic: Loaded SONIC.
	chromosome: name of the chromosome of the interval of interest.
	start: start coordinate of the interval of interest (inclusive -- BED-like).
	end: end coordinate of the interval of interest (exclusive -- BED-like).

	RETURN VALUE:
		>0 if the interval is in a satellite region,  it will return the fraction of the interval that is covered by satellites.
		0 if it is not.

## Check if interval hits a gap

	int sonic_is_gap(sonic *sonic, char *chromosome, int start, int end);

	sonic: Loaded SONIC.
	chromosome: name of the chromosome of the interval of interest.
	start: start coordinate of the interval of interest (inclusive -- BED-like).
	end: end coordinate of the interval of interest (exclusive -- BED-like).

	RETURN VALUE:
		1 if the interval is in a gap region.
		0 if it is not.

## Check if interval hits a segmental duplication region

	float sonic_is_segmental_duplication(sonic *sonic, char *chromosome, int start, int end);

	sonic: Loaded SONIC.
	chromosome: name of the chromosome of the interval of interest.
	start: start coordinate of the interval of interest (inclusive -- BED-like).
	end: end coordinate of the interval of interest (exclusive -- BED-like).

	RETURN VALUE:
		>0 if the interval is in a segmental duplication region, it will return the fraction of the interval that is covered by segmental duplication.
		0 if it is not.

## Check if interval hits a mobile element

	sonic_repeat *sonic_is_mobile_element(sonic *sonic, char *chromosome, int start, int end, char *mei_string);

	sonic: Loaded SONIC.
	chromosome: name of the chromosome of the interval of interest.
	start: start coordinate of the interval of interest (inclusive -- BED-like).
	end: end coordinate of the interval of interest (exclusive -- BED-like).
	mei_string: a colon-seperated of MEI keywords, i.e. "Alu:L1:HERV".

	RETURN VALUE:
		pointer to a SONIC repeat data structure if the interval hits a mobile element.
		NULL if it is not.

## Get GC content over a region (as pre-calculated in non-overlapping 100 bp intervals).

	float sonic_get_gc_content(sonic *sonic, char *chromosome, int start, int end);

	sonic: Loaded SONIC.
	chromosome: name of the chromosome of the interval of interest.
	start: start coordinate of the interval of interest (inclusive -- BED-like).
	end: end coordinate of the interval of interest (exclusive -- BED-like).

	RETURN VALUE:
		GC% over the region, returned in [0-100] interval.
	
