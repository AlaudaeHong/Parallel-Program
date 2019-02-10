#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>
#include <mpi.h>


// EXAMPLE DATA STRUCTURE DESIGN AND LAYOUT FOR CLA
#define input_size 262144
#define block_size 32
//Do not touch these defines 
#define digits (input_size+1)
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
//Integer array of inputs in binary form
int* bin1=NULL;
int* bin2=NULL;
//Character array of inputs in hex form
char* hex1=NULL;
char* hex2=NULL;


//Function Prototype
void Init(void);
void DeInit(void);
int reading_text(void);
void convert(char* hex_str, int* bin_array);
void revert(int* bin_array);
void cal_gi_pi(void);
void cal_xgi_xpi(int* ygi, int* ypi, int ysize,	int *xgi, int* xpi, int xsize);
void cal_xci(int* xci, int* xgi, int* xpi, int* yci, int ysize);
void cal_sumi(void);

int main(int argc, char** argv){

	//Init MPI things
	int my_mpi_size = -1;
	int my_mpi_rank = -1;
	MPI_Init(&argc, &argv);
	MPI_Comm_size(MPI_COMM_WORLD, &my_mpi_size);
	MPI_Comm_rank(MPI_COMM_WORLD, &my_mpi_rank);

	//Check if have three arguments
	if(argc != 3){
		printf("Usage: %s [input.txt] [output.txt]", argv[0]);
		return EXIT_FAILURE;
	}

	//Allocate dynamic memory
	Init();

	//Reading input and filling hex1 and hex2
	reading_text();

	//Converting to binary
	convert(hex1, bin1);
	convert(hex2, bin2);

	//Calculate gi and pi
	cal_gi_pi();
	MPI_Barrier(MPI_COMM_WORLD);

	//Calculate g and p for groups, sections, and supersections
	cal_xgi_xpi(gi, pi, bits, ggj, gpj, ngroups);
	MPI_Barrier(MPI_COMM_WORLD);
	
	cal_xgi_xpi(ggj, gpj, ngroups, sgk, spk, nsections);
	MPI_Barrier(MPI_COMM_WORLD);
	
	cal_xgi_xpi(sgk, spk, nsections, ssgl, sspl, nsupersections);
	MPI_Barrier(MPI_COMM_WORLD);

	//Calculate ci for each supersection, section, group, and bit
	cal_xci(sscl, ssgl, sspl, NULL, 1);
	MPI_Barrier(MPI_COMM_WORLD);
	
	cal_xci(sck, sgk, spk, sscl, nsupersections);
	MPI_Barrier(MPI_COMM_WORLD);
	
	cal_xci(gcj, ggj, gpj, sck, nsections);
	MPI_Barrier(MPI_COMM_WORLD);
	
	cal_xci(ci,gi, pi, gcj, ngroups);
	MPI_Barrier(MPI_COMM_WORLD);
	
	//Calculate the sum for each bit
	cal_sumi();
	MPI_Barrier(MPI_COMM_WORLD);

	//convert the binary result into hex
	revert(sumi);

	//De-allocate dynamic memory
	DeInit();

	//MPI housekeeping
	MPI_Finalize();

	return EXIT_SUCCESS;
}

//Allocate dynamic memory
void Init(void){
	bin1 = calloc(bits, sizeof(int));
	bin2 = calloc(bits, sizeof(int));

	hex1 = calloc(digits, sizeof(int));
	hex2 = calloc(digits, sizeof(int));
}

//Free allocated memory
void DeInit(void){
	free(bin1);
	free(bin2);
	free(hex1);
	free(hex2);
}

//Parsing stdin 
int reading_text(void){

	//Read first argument of this adder
	scanf("%262144s", hex1);

	//Check if the first argument is ended with '\n'
	//And get rid of it
	if(fgetc(stdin) !=  '\n') return 0;

	//Read second argument of this adder
	scanf("%262144s", hex2);

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
void revert(int* bin_array){

	char hex_string[] = "0123456789ABCDEF";
	char* hex_result;

	//Check every four bits as a group
	for(int i = 0; i < digits - 1; i++){

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

	//Print out the result
	printf("%s\n", hex_result);

	//Free dynamic memory
	free(hex_result);
}

//Calculate gi and pi
void cal_gi_pi(void){
	for(int i = 0; i < bits; i++){
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
	int xc_1 = 0;
	
	//Run to get every upper section carry-in
	for(int i = 0; i < ysize; i++){
		
		//If is the first one
		if(i == 0) xc_1 = 0;

		//If no upper section
		else if(yci == NULL) xc_1 = 0;

		else xc_1 = yci[i - 1];

		//Calculate every c
		for(int j = 0; j < block_size; j++){
			if(j == 0){
				xci[block_size*i + j] = xgi[block_size*i + j] | (xpi[block_size*i + j] & xc_1);
			}
			else{
				xci[block_size*i + j] = 
					xgi[block_size*i + j] | (xpi[block_size*i + j] & xci[block_size*i + j - 1]);
			}

		}

	}

	//Because of the macro defination of digits, there is additional one groups need to calculate
	if(ysize == ngroups){

		int i = ngroups;

		for(int j = 0; j < block_size; j++){
             if(j == 0){
                 xci[block_size*i + j] = xgi[block_size*i + j] | (xpi[block_size*i + j] & xc_1);
             }
             else{
                 xci[block_size*i + j] =
                     xgi[block_size*i + j] | (xpi[block_size*i + j] & xci[block_size*i + j - 1]);
             }
         }
		
	}
	
}

//Calculate the sum for each bit
void cal_sumi(void){
	for(int i = 0; i < bits; i++){
		if(i == 0){
			sumi[i] = bin1[i] ^ bin2[i];
		}

		else{
			sumi[i] = bin1[i] ^ bin2[i] ^ ci[i-1];
		}
	}
}

