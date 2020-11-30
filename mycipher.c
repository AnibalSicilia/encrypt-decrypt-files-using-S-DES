#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

void syserr(char *msg) {
	perror(msg);
	exit(-1);
}

// Converts back 'char array number' to 'char number'
unsigned char binBack(unsigned char* d)
{
	int x;
	unsigned char ch;
	unsigned char t_arr[8];
	for(x=0; x<8; x++)
		t_arr[x] = d[x];
	// Revert First
  	int i = 0, j = 7;
  	while(i < j) { 
    	unsigned char c = t_arr[j];
    	t_arr[j--] = t_arr[i];
    	t_arr[i++] = c;
  	}
	// Now do the conversion
	ch=0;
	for(x=0; x<8; x++) {
		if(t_arr[x] == 0) {
			continue;
		}
		else if(x == 0)
			ch +=1;
		else if(x == 1)
			ch += 2;
		else
			ch += (2 << (x-1));		
	}
	return ch;
}


// ip Function - Initial Permutation
void perform_initial_permutation(unsigned char * od, unsigned char * ip) 
{
	int x;

	ip[7] = od[6];
	ip[6] = od[4];
	ip[5] = od[7];
	ip[4] = od[3];
	ip[3] = od[0];
	ip[2] = od[2];
	ip[1] = od[5];
	ip[0] = od[1];

}

//The FK Function
void perform_FK(int w, unsigned char *p8a, unsigned char *p8b, unsigned char * ip, unsigned char * R, unsigned char * L) {
	int j, x, y;
	unsigned int S0 [4][4]= {{1,0,3,2}, {3,2,1,0}, {0,2,1,3}, {3,1,3,2}};
	unsigned int S1 [4][4]= {{0,1,2,3}, {2,0,1,3}, {3,0,1,0}, {2,1,0,3}};

	// performing expansion/permutation operation

	unsigned char e[8]; // for expansion operation
	x=0;
	e[x++] = R[3];
	e[x++] = R[0];
	e[x++] = R[1];
	e[x++] = R[2];
	e[x++] = R[1];
	e[x++] = R[2];
	e[x++] = R[3];
	e[x++] = R[0];

	// Do the XOR with key 1
	if(w == 1) {
		for(x=0; x<8; x++) {
			e[x] = e[x] ^ p8a[x];
		}
	}
	else { // Do XOR with Key2
			for(x=0; x<8; x++) {
			e[x] = e[x] ^ p8b[x];
		}
	}


	// place result into a 2x4 matrix
	unsigned char arr_p[2][4];
	for(x=0, y=0; x<2; x++) {
		for(j=0; j<4; j++) {
			arr_p[x][j] = e[y++];
		}
	}
	unsigned char row_s01 = 2*arr_p[0][0] + arr_p[0][3];
	unsigned char column_s01 = 2*arr_p[0][1] + arr_p[0][2];
	unsigned char row_s02 = 2*arr_p[1][0] + arr_p[1][3];
	unsigned char column_s02 = 2*arr_p[1][1] + arr_p[1][2];
	unsigned char f = S0[row_s01][column_s01];
	unsigned char g = S1[row_s02][column_s02];

	unsigned char s0[8];
	unsigned char s1[8];
	unsigned char p4[4];	//holds actual permutation
	unsigned char p4a[4];	//holds values to perform permutation

	// adds found values from S0, S1
	for(x=0; x<8; x++) {
		s0[x] = f & 0x80 ? 1 : 0;
		f <<= 1;
		s1[x] = g & 0x80 ? 1 : 0;
		g <<= 1;
	}

	// put the 4 bits together
	p4a[0] = s1[7];
	p4a[1] = s1[6];
	p4a[2] = s0[7];
	p4a[3] = s0[6];

	// Now, apply P4 permutation
	p4[3] = p4a[0];
	p4[2] = p4a[2];
	p4[1] = p4a[3];
	p4[0] = p4a[1];

	//now, after obtaining F Xor Left-Side with result

	for(x=0; x<4; x++) {
		L[x] = L[x] ^ p4[x];
	}

	// Assemble both halves into a unique array

	for(x=0; x<4; x++)
		ip[x] = L[x];
	for(x=0, j=4; x<4; x++, j++)
		ip[j] = R[x];
}


// Switch Function
void do_the_switch(unsigned char * ip, unsigned char * R, unsigned char * L){

	int j, x;
	//now divide input data after ip into L and R halves
	// The right Array
	for(x=0; x<4; x++) {
		L[x] = ip[x];
	}

	// The left Array
	for(x=0; x<4; x++) {
			R[x] = ip[x+4];				
	}

	unsigned char temp;
	for(j=0; j<4; j++) {
		temp = R[j];
		R[j] = L[j];
		L[j] = temp;
	}
}

// Inverse Permutation Function
inverse_permutation(unsigned char * ip, unsigned char * ip_inv)
{
	int x=0;
	ip_inv[x++] = ip[3];
	ip_inv[x++] = ip[0];
	ip_inv[x++] = ip[2];
	ip_inv[x++] = ip[4];
	ip_inv[x++] = ip[6];
	ip_inv[x++] = ip[1];
	ip_inv[x++] = ip[7];
	ip_inv[x++] = ip[5];
}



// The MAIN Function
int main(int argc, char* argv[])
{
	unsigned char *d_option;
	int is_decryption;	// used as a boolean with -d argument
	unsigned char *init_key;
	unsigned char *init_vector;

	unsigned char *original_file;
	unsigned char *result_file;
	int x, i, j;
	unsigned char bin[8];
	unsigned char ip[8];
	unsigned char ip_inv[8];
	unsigned char R[4];	// 4 rightmost bit
	unsigned char L[4]; // 4 leftmost bit


	int encryption; // determines whether to encrypt/decrypt
	int which_key;	// used to determine either key1 or key2
	unsigned char p8a[8]; // stores key1
	unsigned char p8b[8]; //stores key2

    // if encryption
	if(argc == 5)
	{
		init_key = argv[1];
		for(j=0; j<10; j++) {
		// now convert char d into bits

			if(*(init_key + j) == '0' )
				*(init_key + j) = 0;
			else if(*(init_key + j) == '1') 
				*(init_key + j) = 1;
			else {
				fprintf(stderr, "Not a valid key. key values are either 1 or 0.\n"
						"and this value is %c. Program will now exit\n"
						, *(init_key + j));
				return 1;
			}
		}

		init_vector = argv[2];
		for(j=0; j<8; j++) {
		// now convert char d into bits

			if(*(init_vector + j) == '0' )
				*(init_vector + j) = 0;
			else if(*(init_vector + j) == '1') 
				*(init_vector + j) = 1;
			else {
				fprintf(stderr, "Not a valid key. key values are either 1 or 0.\n"
						"and this value is %c. Program will now exit\n"
						, *(init_vector + j));
				return 1;
			}
		}

		original_file = argv[3];
		result_file = argv[4];
		encryption = 1;
	}
	else if(argc == 6)
	{
		d_option = argv[1];
		is_decryption = strcmp(d_option, "-d");
		if(is_decryption != 0) {
			printf("You entered %s. \n", d_option);
			syserr("This value is not recognized. The comand is '-d'.\n"
					"Program now will exit\n");
		}
		init_key = argv[2];
		for(j=0; j<10; j++) {
		// now convert char d into bits

			if(*(init_key + j) == '0' )
				*(init_key + j) = 0;
			else if(*(init_key + j) == '1') 
				*(init_key + j) = 1;
			else {
				fprintf(stderr, "Not a valid key. key values are either 1 or 0.\n"
						"and this value is %c. Program will now exit\n"
						, *(init_key + j));
				return 1;
			}
		}

		init_vector = argv[3];
		for(j=0; j<8; j++) {
		// now convert char d into bits

			if(*(init_vector + j) == '0' )
				*(init_vector + j) = 0;
			else if(*(init_vector + j) == '1') 
				*(init_vector + j) = 1;
			else {
				fprintf(stderr, "Not a valid key. key values are either 1 or 0.\n"
						"and this value is %c. Program will now exit\n"
						, *(init_vector + j));
				return 1;
			}
		}

		original_file = argv[4];
		result_file = argv[5];
		encryption = 0;
	}
	else {
		fprintf(stderr, "Usage: %s [-d] (if you want to decript file, "
						"skip if not) \n<init_key> <init_vector> <original_file> "
						"<result_file>\n", argv[0]);
		return 1;
	} // End of 'Introduction'

	// permute the 10-bit key (initial_permutation)
	unsigned char p10[10];
	i=0;
	p10[i++] = *(init_key + 2);
	p10[i++] = *(init_key + 4);
	p10[i++] = *(init_key + 1);
	p10[i++] = *(init_key + 6);
	p10[i++] = *(init_key + 3);
	p10[i++] = *(init_key + 9);
	p10[i++] = *(init_key + 0);
	p10[i++] = *(init_key + 8);
	p10[i++] = *(init_key + 7);
	p10[i++] = *(init_key + 5);

	// Creating Subkey-K1
	// First half
	unsigned char tmp[5];
	for(x=0; x<5; x++)
		tmp[x] = p10[x];

	// now do a circular left shift of 1
	//start with the first five bits
	unsigned char p5a[5];
	for(j=0; j<5-1; j++)
		p5a[j] = tmp[j+1];
	p5a[j] = tmp[0];

	//Then, on the second five bits
	// Second half
	for(j=0,x=5; j<5; x++, j++)
		tmp[j] = p10[x];

	// now do a circular left shift on second five bits
	unsigned char p5b[5];
	for(x=0; x<5-1; x++)
		p5b[x] = tmp[x+1];
	p5b[x] = tmp[0];

	for(j=0; j<sizeof(p5a); j++)
		p10[j] = p5a[j];
	for(i=0 ; j<sizeof(p10); j++, i++)
		p10[j]= p5b[i];

	// Apply P8 to finalize Subkey-K1 creation
	i=0;
	p8a[i++] = p10[5];
	p8a[i++] = p10[2];
	p8a[i++] = p10[6];
	p8a[i++] = p10[3];
	p8a[i++] = p10[7];
	p8a[i++] = p10[4];
	p8a[i++] = p10[9];
	p8a[i++] = p10[8];

	// Creating Subkey-Key2
	unsigned char p5c[5];
	unsigned char p5d[5];
	for(j=0; j<5-2;j++) {
		p5c[j] = p5a[j+2];
		p5d[j] = p5b[j+2];
	}
	p5c[j] = p5a[0];
	p5c[j+1] = p5a[1];
	p5d[j] = p5b[0];
	p5d[j+1] = p5b[1];

	for(j=0; j<5; j++)
		p10[j] = p5c[j];
	for(i=0, j=5; i<5; j++, i++)
		p10[j]= p5d[i];

	//Apply p8 to finalize Subkey-K2 creation
	i=0;
	p8b[i++] = p10[5];
	p8b[i++] = p10[2];
	p8b[i++] = p10[6];
	p8b[i++] = p10[3];
	p8b[i++] = p10[7];
	p8b[i++] = p10[4];
	p8b[i++] = p10[9];
	p8b[i++] = p10[8];

	// end of keys' creation

	// S-DES Encryption
	FILE *fpr; // file to be read
	FILE *fpw; // file to be written
	int file_size;
	fpr = fopen(original_file, "r");
	if(fpr == NULL) {	// if there is no file...
		syserr("Error Opening the file, the system will now exit");
	}                      
	// if file exists...
	fseek(fpr, 0L, SEEK_END); // goto end of file
	file_size = ftell(fpr); // then, obtain file's length
	fseek(fpr, 0L, SEEK_SET);

	// This will read the entire file into memory. OK for XXI Century 
	unsigned char *data = NULL;
	data = (unsigned char *) malloc(sizeof(unsigned char *) * (file_size));
	int rd;
	if((rd = fread(data, sizeof(unsigned char), file_size, fpr)) == 0)
		printf("Error reading file\n");
	fpw = fopen(result_file, "w");
	fclose(fpr);

	int cbc = 1; // used this to determine if use cbc or ebc

	unsigned char bin2[8]; // used in decryption to save data - 1
	unsigned char d_vector[8];
	unsigned char d;
	int boolean = 1;
	// save initial vector in vector array
	for(x=0; x<8; x++)
		d_vector[x] = *(init_vector + x);
	//THE LOOP
	for(i=0; i<rd; i++){
		d = *(data + i); // get character to be encrypted

		// now convert char d into bits
		for(x=0; x<8; x++) {
			bin[x] = d & 0x80 ? 1 : 0;
			d <<= 1;
		}

//---------------------------------------------------------------------------------------
//CBC

		if(encryption == 1) {
			for(x=0; x<8; x++)
			bin[x] = bin[x] ^ d_vector[x];
		}
		else {
			for(x=0; x<8;x++)
				bin2[x] = bin[x]; // save original data
		}

//END CBC
//---------------------------------------------------------------------------------------

		// Apply ip function (initial permutation)
		perform_initial_permutation(bin, ip);
		//now divide input data after ip into L and R halves
		// The right Array
		for(x=0; x<4; x++) {
			L[x] = ip[x];
		}

		// The left Array
		for(x=0; x<4; x++) {
				R[x] = ip[x+4];				
		}
		// Now comes the FK function (first_time)
		if(encryption == 1)
			which_key = 1;
		else
			which_key = 0;
		perform_FK(which_key, p8a, p8b, ip, R, L);
		// Do the switch
		do_the_switch(ip, R, L);

		// Now, apply the FK function again
		if(encryption == 1)
			which_key = 0;
		else
			which_key = 1;
		perform_FK(which_key, p8a, p8b, ip, R, L);

		// Apply inverse ip function
		inverse_permutation(ip, ip_inv);

		unsigned char ch;
//---------------------------------------------------------------------------------------
//CBC
		unsigned char vtbw[8]; // value to be written
		if(encryption == 1) {
			for(x=0; x<8; x++) {
				d_vector[x] = ip_inv[x];
			}
			ch = binBack(d_vector);	// obtain char value 	
			fwrite(&ch, 1, 1, fpw);
		}
		else {
			for(x=0; x<8; x++) {
				vtbw[x] = d_vector[x] ^ ip_inv[x];
				d_vector[x] = bin2[x];
			}

			ch = binBack(vtbw);				
			fwrite(&ch, 1, 1, fpw);
		}
//End CBC
//---------------------------------------------------------------------------------------
	}

	free(data);
	fclose(fpw);
	return 0;
}

