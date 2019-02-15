//Included library
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <unistd.h>
#include <mpi.h>
// EXAMPLE DATA STRUCTURE DESIGN AND LAYOUT FOR CLA
#define input_size 262144
#define block_size 32
//Do not touch these defines 
#define digits (input_size)					//Changed for better portability
#define bits digits*4
#define ngroups bits/block_size
#define nsections ngroups/block_size
#define nsupersections nsections/block_size
//Global definitions of the various arrays used in steps for easy access
int gi[bits] = {0};
int pi[bits] = {0};
int ci[bits] = {0};
int ggj[ngroups] = {0};
int gpj[ngroups] = {0};
int gcj[ngroups] = {0};
int sgk[nsections] = {0};
int spk[nsections] = {0};
int sck[nsections] = {0};
int ssgl[nsupersections] = {0} ;
int sspl[nsupersections] = {0} ;
int sscl[nsupersections] = {0} ;
int sumi[bits] = {0};
int temp_sumi[bits] = {0};					//Because MPI_Gather need saperate buffer
//Integer array of inputs in binary form
int* bin1=NULL;
int* temp_bin1=NULL;						//Because MPI_Scatter need saperate buffer
int* bin2=NULL;
int* temp_bin2=NULL;						//Because MPI_Scatter need saperate buffer
//Character array of inputs in hex form
char* hex1=NULL;
char* hex2=NULL;
//MPI characteristics
int my_mpi_size = -1;
int my_mpi_rank = -1;
MPI_Request mpi_request;
MPI_Status mpi_status;
//Process info
int CIN = 0;
int COUT = 0;
int div_bits = 0;							//# of bits processed by this thread
int div_ngroups = 0;						//# of groups processed by this thread
int div_nsections = 0;						//# of sections processed by this thread
int div_nsupersections = 0;					//# of supersection processed by this thread

//Function Prototype
void Init(void);
void DeInit(void);
int reading_text(char* input);
void convert(char* hex_str, int* bin_array);
void revert(int* bin_array, char* output);
void cal_gi_pi(void);
void cal_xgi_xpi(int* ygi, int* ypi, int ysize,	int *xgi, int* xpi, int xsize);
void cal_xci(int* xci, int* xgi, int* xpi, int* yci, int ysize);
void cal_sumi(void);

int main(int argc, char** argv){

	//Init MPI things
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &my_mpi_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_mpi_rank);

	//Check if have three arguments
	if(argc != 3){
		printf("Usage: %s [input.txt] [output.txt]\n", argv[0]);
		return EXIT_FAILURE;
	}
	
	//Allocate dynamic memory
	Init();

	//Reading input and filling hex1 and hex2 by rank 0
	if(my_mpi_rank == 0){ reading_text(argv[1]); }

	//Record starting time with I/O effect
	double start_t, end_t;
	start_t = MPI_Wtime();

	//Calculate gi and pi
	MPI_Scatter(temp_bin1, div_bits, MPI_INT, bin1, div_bits, MPI_INT, 0, MPI_COMM_WORLD);	

	MPI_Scatter(temp_bin2, div_bits, MPI_INT, bin2, div_bits, MPI_INT, 0, MPI_COMM_WORLD);	

	cal_gi_pi();
#ifdef Barrier
	MPI_Barrier(MPI_COMM_WORLD);
#endif

	//Calculate g and p for groups, sections, and supersections
	//Calculate groups
	cal_xgi_xpi(gi, pi, div_bits, ggj, gpj, div_ngroups);
#ifdef Barrier
	MPI_Barrier(MPI_COMM_WORLD);
#endif

	//Calculate sections
	cal_xgi_xpi(ggj, gpj, div_ngroups, sgk, spk, div_nsections);
#ifdef Barrier
	MPI_Barrier(MPI_COMM_WORLD);
#endif

	
	cal_xgi_xpi(sgk, spk, div_nsections, ssgl, sspl, div_nsupersections);
#ifdef Barrier
	MPI_Barrier(MPI_COMM_WORLD);
#endif

	//Calculate ci for each supersection, section, group, and bit
	//Calculate supersections
	cal_xci(sscl, ssgl, sspl, NULL, 1);
#ifdef Barrier
	MPI_Barrier(MPI_COMM_WORLD);
#endif
	
	//Calculate sections
	cal_xci(sck, sgk, spk, sscl, div_nsupersections);
#ifdef Barrier
	MPI_Barrier(MPI_COMM_WORLD);
#endif
    
	//Calculate groups
	cal_xci(gcj, ggj, gpj, sck, div_nsections);
    
#ifdef Barrier
	MPI_Barrier(MPI_COMM_WORLD);
#endif
    
	//Calcualte bits
	cal_xci(ci,gi, pi, gcj, div_ngroups);
    
#ifdef Barrier
	MPI_Barrier(MPI_COMM_WORLD);
#endif
    
	//Calculate the sum for each bit
	cal_sumi();

    //Gather data from each rank
	if(my_mpi_rank == 0)
		MPI_Gather(temp_sumi, div_bits, MPI_INT, sumi, div_bits, MPI_INT, 0, MPI_COMM_WORLD);
	else
		MPI_Gather(temp_sumi, div_bits, MPI_INT, NULL, div_bits, MPI_INT, 0, MPI_COMM_WORLD);

    //Count time
	end_t  = MPI_Wtime();
	if(my_mpi_rank == 0) printf("%f ", end_t - start_t);
	
    //convert the binary result into hex
	if(my_mpi_rank == 0) revert(sumi, argv[2]);

	//De-allocate dynamic memory
	DeInit();


	//MPI housekeeping
	MPI_Finalize();

	return EXIT_SUCCESS;
}

//Allocate dynamic memory
void Init(void){

	hex1 = calloc(digits + 1, sizeof(int));
	hex2 = calloc(digits + 1, sizeof(int));
	
	bin1		= calloc(bits, sizeof(int));
	temp_bin1	= calloc(bits, sizeof(int));
	bin2		= calloc(bits, sizeof(int));
	temp_bin2	= calloc(bits, sizeof(int));
	
    //Calculate the workload for each rank
	div_bits = bits / my_mpi_size;
	div_ngroups = div_bits / block_size;
	div_nsections = div_ngroups / block_size;
	div_nsupersections = div_nsections / block_size;
}

//Free allocated memory
void DeInit(void){
	free(temp_bin1);
	free(bin1);
	free(temp_bin2);
	free(bin2);
	free(hex1);
	free(hex2);
}

//Parsing stdin 
int reading_text(char* input){

	FILE *pinput = NULL;

	if( (pinput = fopen( input, "r")) == NULL ){
		perror("fopen");
	}

	//Read first argument of this adder
	fscanf(pinput, "%262144s", hex1);

	//Check if the first argument is ended with '\n'
	//And get rid of it
	if(fgetc(pinput) !=  '\n') return 0;

	//Read second argument of this adder
	fscanf(pinput, "%262144s", hex2);

	//Converting to binary, store in temp array to save I/O
	convert(hex1, temp_bin1);
	convert(hex2, temp_bin2);
	
	return 1;
}

//Converting a hex string into binary array
void convert(char* hex_str, int* bin_array){

	//Converting hex to binary digit by digit
	for(int i = 0; i < digits; i++){
		
		int value = 0;

		//Because of some unknown error caused by strtol()
		//Use switch to check
		switch(hex_str[i]){
			case '0': value = 0; break;

			case '1': value = 1; break;

			case '2': value = 2; break;

			case '3': value = 3; break;
		
			case '4': value = 4; break;

			case '5': value = 5; break;
		
			case '6': value = 6; break;

			case '7': value = 7; break;
		
			case '8': value = 8; break;

			case '9': value = 9; break;
		
			case 'A': value = 10; break;

			case 'B': value = 11; break;
		
			case 'C': value = 12; break;

			case 'D': value = 13; break;
		
			case 'E': value = 14; break;

			case 'F': value = 15; break;
		}

		//Put it into binary array
		if(value >= 8){
			bin_array[bits - 4*i - 1] = 1;
			value -= 8;
		}
			
		if(value >= 4){
			bin_array[bits - 4*i - 2] = 1;
			value -= 4;
		}

		if(value >= 2){
			bin_array[bits - 4*i - 3] = 1;
			value -= 2;
		}

		if(value >= 1){
			bin_array[bits - 4*i - 4] = 1;
			value -= 1;
		}
	}
}

//Convert binary form back to hex
void revert(int* bin_array, char* output){

	char hex_string[] = "0123456789ABCDEF";
	char* hex_result;

	//Check every four bits as a group
	for(int i = 0; i < digits; i++){

		int value = 0;
		
		value += (bin_array[bits - 4*i - 1] * 8);
		value += (bin_array[bits - 4*i - 2] * 4);
		value += (bin_array[bits - 4*i - 3] * 2);
		value += (bin_array[bits - 4*i - 4] * 1);

		//Allocate dynamic memory
		if(i == 0){
			//hex_result = calloc(digits + 1, sizeof(char));
			hex_result = calloc(digits, sizeof(char));
			//hex_result[0] = '0';
		}

		//Give a corresponding hex value
		hex_result[i] = hex_string[value];

	}

	FILE* pfile = NULL;
	if( (pfile = fopen(output, "w"))  == NULL){
		perror("fopen");
	}

	//Print out the result
	fprintf(pfile, "%s\n", hex_result);

	fclose(pfile);

	//Free dynamic memory
	free(hex_result);
}

//Calculate gi and pi
void cal_gi_pi(void){
	for(int i = 0; i < div_bits; i++){
		gi[i] = bin1[i] & bin2[i];
		pi[i] = bin1[i] | bin2[i];
	}
}

//Calculate ggj and gpj, or sgk and spk, or ssgl and sspl
void cal_xgi_xpi(int* ygi, int* ypi, int ysize,	int *xgi, int* xpi, int xsize){

	//Run for every g and p needed
	for(int j = 0; j < xsize; j++){

		int xg = 0;
		int xp = 0;

		for(int i = 0; i < block_size; i++){

			//Define basic case
			if(i == 0){
				xp = ypi[block_size*j + i];
				xg = ygi[block_size*j + i];
			}

			//Define recursive behavior
			else{
				xp = ypi[block_size*j + i] & xp;
				xg = ygi[block_size*j + i] | (ypi[block_size*j + i] & xg);
			}
		}

		//put the final result
		xgi[j] = xg;
		xpi[j] = xp;
	}
}

//Calculate sscl or sck or gcj
void cal_xci(int* xci, int* xgi, int* xpi, int* yci, int ysize){

	int group_size = ysize * block_size;
	int xc_1 = 0;

	//Prepare to recieve from it right neighbor
	//And use current group size as tag
	if(my_mpi_rank == 0) CIN = 0;
	else if(yci == NULL) {
		MPI_Irecv(&CIN, 1, MPI_INT, my_mpi_rank - 1,
			group_size, MPI_COMM_WORLD, &mpi_request);
	}

	//Make sure that all recv is setup
	MPI_Barrier(MPI_COMM_WORLD);

	//Wait for CIN to be received
	if(my_mpi_rank != 0) { MPI_Wait(&mpi_request, &mpi_status); }
	
	
	//Run to get every upper section carry-in
	for(int i = 0; i < ysize; i++){
		
		//If is the first one
		if(i == 0) xc_1 = CIN;

		//If no upper section
		else if(yci == NULL) xc_1 = 0;

		else xc_1 = yci[i - 1];

		//Calculate every c
		int c_size = ysize;
		
		if (c_size == 1) c_size = div_nsupersections;
		else c_size = block_size;
		
		for(int j = 0; j < c_size; j++){
			if(j == 0){
				xci[block_size*i + j] = xgi[block_size*i + j] | (xpi[block_size*i + j] & xc_1);
			}
			else{
				xci[block_size*i + j] = 
					xgi[block_size*i + j] | (xpi[block_size*i + j] & xci[block_size*i + j - 1]);
			}
			
		//	if(ysize == div_nsupersections) printf("//Rank %d: Carryout %d\n", my_mpi_rank, xci[block_size*i + j]);
		}
	}

	//For the supersections, have to wait before send any carryout
	if(yci == NULL) {
		COUT = xci[ div_nsupersections - 1 ];
	
		if(my_mpi_rank != my_mpi_size - 1){

			MPI_Isend(&COUT, 1, MPI_INT, my_mpi_rank + 1,
				group_size, MPI_COMM_WORLD, &mpi_request);

	
		}
	}
}

//Calculate the sum for each bit
void cal_sumi(void){

    //Store in temp array to save I/O
	for(int i = 0; i < div_bits; i++){
		if(i == 0){
			temp_sumi[i] = bin1[i] ^ bin2[i] ^ CIN;
		}

		else{
			temp_sumi[i] = bin1[i] ^ bin2[i] ^ ci[i-1];
		}
	}
}

