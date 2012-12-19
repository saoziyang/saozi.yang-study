#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/videodev2.h>
#define CLEAR(x) memset (&(x), 0, sizeof (x))

#define PUTBITS  \
{	\
	bits_in_next_word = (short) (jpeg_encoder_structure->bitindex + numbits - 32);	\
	if (bits_in_next_word < 0)	\
	{	\
		jpeg_encoder_structure->lcode = (jpeg_encoder_structure->lcode << numbits) | data;	\
		jpeg_encoder_structure->bitindex += numbits;	\
	}	\
	else	\
	{	\
		jpeg_encoder_structure->lcode = (jpeg_encoder_structure->lcode << (32 - jpeg_encoder_structure->bitindex)) | (data >> bits_in_next_word);	\
		if ((*output_ptr++ = (unsigned char)(jpeg_encoder_structure->lcode >> 24)) == 0xff)	\
			*output_ptr++ = 0;	\
		if ((*output_ptr++ = (unsigned char)(jpeg_encoder_structure->lcode >> 16)) == 0xff)	\
			*output_ptr++ = 0;	\
		if ((*output_ptr++ = (unsigned char)(jpeg_encoder_structure->lcode >> 8)) == 0xff)	\
			*output_ptr++ = 0;	\
		if ((*output_ptr++ = (unsigned char) jpeg_encoder_structure->lcode) == 0xff)	\
			*output_ptr++ = 0;	\
		jpeg_encoder_structure->lcode = data;	\
		jpeg_encoder_structure->bitindex = bits_in_next_word;	\
	}	\
}

    struct buffer {
	void *start;
	size_t length;
};

static char *dev_name = "/dev/video0";	//摄像头设备名
static char *fb_name = "/dev/fb0";	//framebuffer
static int fd = -1;
struct buffer *buffers = NULL;
static unsigned int n_buffers = 0;

FILE *file_fd;
static unsigned long file_length;
static unsigned char *file_name;

static unsigned char* jpeg_buf = NULL;
//////////////////////////////////////////////////////
//获取一帧数据
//////////////////////////////////////////////////////

struct JPEG_ENCODER_STRUCTURE 
{
	unsigned short	mcu_width;
	unsigned short	mcu_height;
	unsigned short	horizontal_mcus;
	unsigned short	vertical_mcus;

	unsigned short	rows;
	unsigned short	cols;

	unsigned short	length_minus_mcu_width;
	unsigned short	length_minus_width;
	unsigned short	incr;
	unsigned short	mcu_width_size;
	unsigned short	offset;

	signed short	ldc1;
	signed short	ldc2;
	signed short	ldc3;
	
	unsigned long int	lcode;
	unsigned short	bitindex;
	/* MCUs */
	signed short	Y1 [64];
	signed short	Y2 [64];
	signed short	Temp [64];
	signed short	CB [64];
	signed short	CR [64];
	/* Quantization Tables */
	unsigned char	Lqt [64];
	unsigned char	Cqt [64];
	unsigned short	ILqt [64];
	unsigned short	ICqt [64];

};
void 
initialization (struct JPEG_ENCODER_STRUCTURE * jpeg, int image_width, int image_height)
{
	unsigned short mcu_width, mcu_height, bytes_per_pixel;

	jpeg->mcu_width = mcu_width = 16;
	jpeg->horizontal_mcus = (unsigned short) (image_width >> 4);/* width/16 */
	
	jpeg->mcu_height = mcu_height = 8;
	jpeg->vertical_mcus = (unsigned short) (image_height >> 3); /* height/8 */ 
	
	bytes_per_pixel = 2;
		
	jpeg->length_minus_mcu_width = (unsigned short) ((image_width - mcu_width) * bytes_per_pixel);
	jpeg->length_minus_width = (unsigned short) (image_width * bytes_per_pixel);
	
	jpeg->mcu_width_size = (unsigned short) (mcu_width * bytes_per_pixel);

	jpeg->rows = jpeg->mcu_height;
	jpeg->cols = jpeg->mcu_width;
	jpeg->incr = jpeg->length_minus_mcu_width;
	jpeg->offset = (unsigned short) ((image_width * mcu_height) * bytes_per_pixel);
	
	jpeg->ldc1 = 0;
	jpeg->ldc2 = 0;
	jpeg->ldc3 = 0;
	

	jpeg->lcode = 0;
	jpeg->bitindex = 0;
}

unsigned char zigzag_table [] =
{
	0,  1,   5,  6, 14, 15, 27, 28,
	2,  4,   7, 13, 16, 26, 29, 42,
	3,  8,  12, 17, 25, 30, 41, 43,
	9,  11, 18, 24, 31, 40, 44, 53,
	10, 19, 23, 32, 39, 45, 52, 54,
	20, 22, 33, 38, 46, 51, 55, 60,
	21, 34, 37, 47, 50, 56, 59, 61,
	35, 36, 48, 49, 57, 58, 62, 63
};


/*	This function implements 16 Step division for Q.15 format data */
unsigned short DSP_Division (unsigned long int numer, unsigned long int denom)
{
	unsigned short i;

	denom <<= 15;

	for (i=16; i>0; i--)
	{
		if (numer > denom)
		{
			numer -= denom;
			numer <<= 1;
			numer++;
		}
		else numer <<= 1;
	}

	return (unsigned short) numer;
}

void initialize_quantization_tables (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure)
{
	unsigned short i, index;
	unsigned long int value;

	/* quickcam 5000pro tables (very good quality) */
	static unsigned char luminance_quant_table [] = 
	{
	 0x04, 0x02, 0x03, 0x03, 0x03, 0x02, 0x04, 0x03,
	 0x03, 0x03, 0x04, 0x04, 0x04, 0x04, 0x06, 0x0a,
	 0x06, 0x06, 0x05, 0x05, 0x06, 0x0c, 0x08, 0x09,
	 0x07, 0x0a, 0x0e, 0x0c, 0x0f, 0x0f, 0x0e, 0x0c,
	 0x0e, 0x0f, 0x10, 0x12, 0x17, 0x13, 0x10, 0x11,
	 0x15, 0x11, 0x0d, 0x0e, 0x14, 0x1a, 0x14, 0x15,
	 0x17, 0x18, 0x19, 0x1a, 0x19, 0x0f, 0x13, 0x1c,
	 0x1e, 0x1c, 0x19, 0x1e, 0x17, 0x19, 0x19, 0x18 
	};
	
	static unsigned char chrominance_quant_table [] =
	{
	 0x04, 0x04, 0x04, 0x06, 0x05, 0x06, 0x0b, 0x06, 
	 0x06, 0x0b, 0x18, 0x10, 0x0e, 0x10, 0x18, 0x18, 
	 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 
	 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18,
	 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18	
	};
	
	for (i=0; i<64; i++)
	{
		index = zigzag_table [i];
		
		value= luminance_quant_table [i];

		jpeg_encoder_structure->Lqt [index] = (unsigned char) value;
		jpeg_encoder_structure->ILqt [i] = DSP_Division (0x8000, value);


		value = chrominance_quant_table [i];
				
		jpeg_encoder_structure->Cqt[index] = (unsigned char) value;
		jpeg_encoder_structure->ICqt [i] = DSP_Division (0x8000, value);
	}
}

#define JPG_HUFFMAN_TABLE_LENGTH 0x01A0
static const unsigned char JPEGHuffmanTable[JPG_HUFFMAN_TABLE_LENGTH] = 
{
	// luminance dc - length bits	
	0x00, 
	0x00, 0x01, 0x05, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x00,
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
	// luminance dc - code
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0A, 0x0B, 
	// chrominance dc - length bits	
	0x01, 
	0x00, 0x03, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 
	0x01, 0x00, 0x00, 0x00, 0x00, 0x00,
	// chrominance dc - code
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
	0x0A, 0x0B, 
	// luminance ac - number of codes with # bits (ordered by code length 1-16)
	0x10,
	0x00, 0x02, 0x01, 0x03, 0x03, 0x02, 0x04, 0x03, 0x05, 0x05, 
	0x04, 0x04, 0x00, 0x00, 0x01, 0x7D,
	// luminance ac - run size (ordered by code length)	
	0x01, 0x02, 0x03, 0x00, 0x04, 0x11, 0x05, 0x12, 0x21, 0x31,
	0x41, 0x06, 0x13, 0x51, 0x61, 0x07, 0x22, 0x71, 0x14, 0x32,
	0x81, 0x91, 0xA1, 0x08, 0x23, 0x42, 0xB1, 0xC1, 0x15, 0x52,
	0xD1, 0xF0, 0x24, 0x33, 0x62, 0x72, 0x82, 0x09, 0x0A, 0x16,
	0x17, 0x18, 0x19, 0x1A, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44, 0x45,
	0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56, 0x57,
	0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
	0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x83,
	0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92, 0x93, 0x94,
	0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3, 0xA4, 0xA5,
	0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6,
	0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
	0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7, 0xD8,
	0xD9, 0xDA, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7, 0xE8,
	0xE9, 0xEA, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
	0xF9, 0xFA, 
	// chrominance ac -number of codes with # bits (ordered by code length 1-16)
	0x11, 
	0x00, 0x02, 0x01, 0x02, 0x04, 0x04, 0x03, 0x04, 0x07, 0x05,
	0x04, 0x04, 0x00, 0x01, 0x02, 0x77,
	// chrominance ac - run size (ordered by code length)
	0x00, 0x01, 0x02, 0x03, 0x11, 0x04, 0x05, 0x21, 0x31, 0x06,
	0x12, 0x41, 0x51, 0x07, 0x61, 0x71, 0x13, 0x22, 0x32, 0x81,
	0x08, 0x14, 0x42, 0x91, 0xA1, 0xB1, 0xC1, 0x09, 0x23, 0x33,
	0x52, 0xF0, 0x15, 0x62, 0x72, 0xD1, 0x0A, 0x16, 0x24, 0x34,
	0xE1, 0x25, 0xF1, 0x17, 0x18, 0x19, 0x1A, 0x26, 0x27, 0x28,
	0x29, 0x2A, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x43, 0x44,
	0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x53, 0x54, 0x55, 0x56,
	0x57, 0x58, 0x59, 0x5A, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68,
	0x69, 0x6A, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A,
	0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89, 0x8A, 0x92,
	0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99, 0x9A, 0xA2, 0xA3,
	0xA4, 0xA5, 0xA6, 0xA7, 0xA8, 0xA9, 0xAA, 0xB2, 0xB3, 0xB4,
	0xB5, 0xB6, 0xB7, 0xB8, 0xB9, 0xBA, 0xC2, 0xC3, 0xC4, 0xC5,
	0xC6, 0xC7, 0xC8, 0xC9, 0xCA, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6,
	0xD7, 0xD8, 0xD9, 0xDA, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
	0xE8, 0xE9, 0xEA, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7, 0xF8,
	0xF9, 0xFA
};

static unsigned short luminance_dc_code_table[12] =
{
	0x0000, 0x0002, 0x0003, 0x0004, 0x0005, 0x0006,
	0x000E, 0x001E, 0x003E, 0x007E, 0x00FE, 0x01FE
};

static unsigned short luminance_dc_size_table [12] =
{
	0x0002, 0x0003, 0x0003, 0x0003, 0x0003, 0x0003,
	0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009
};

static unsigned short chrominance_dc_code_table [12] =
{	
	0x0000, 0x0001, 0x0002, 0x0006, 0x000E, 0x001E,
	0x003E, 0x007E, 0x00FE, 0x01FE, 0x03FE, 0x07FE
};

static unsigned short chrominance_dc_size_table [12] =
{
	0x0002, 0x0002, 0x0002, 0x0003, 0x0004, 0x0005,
	0x0006, 0x0007, 0x0008, 0x0009, 0x000A, 0x000B
};

unsigned char* 
write_markers (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure, unsigned char *output_ptr,
	int huff, unsigned long int image_width, unsigned long int image_height)
{
	unsigned short i, header_length;
	unsigned char number_of_components;

	// Start of image marker
	*output_ptr++ = 0xFF;
	*output_ptr++ = 0xD8;
	//added from here 
	// Start of APP0 marker
	*output_ptr++ = 0xFF;
	*output_ptr++ = 0xE0;
	//header length
	*output_ptr++= 0x00;
	*output_ptr++= 0x10;//16 bytes
	
	//type
	if(huff) 
	{	//JFIF0 0x4A46494600
		*output_ptr++= 0x4A;
		*output_ptr++= 0x46;
		*output_ptr++= 0x49;
		*output_ptr++= 0x46;
		*output_ptr++= 0x00;
	} 
	else
	{	// AVI10 0x4156493100
		*output_ptr++= 0x41;
		*output_ptr++= 0x56;
		*output_ptr++= 0x49;
		*output_ptr++= 0x31;
		*output_ptr++= 0x00;
	}
	// version
	*output_ptr++= 0x01;
	*output_ptr++= 0x02;
	// density 0- no units 1- pix per inch 2- pix per mm
	*output_ptr++= 0x01;
	// xdensity - 120
	*output_ptr++= 0x00;
	*output_ptr++= 0x78;
	// ydensity - 120
	*output_ptr++= 0x00;
	*output_ptr++= 0x78;
	
	//thumb x y
	*output_ptr++= 0x00;
	*output_ptr++= 0x00;
	//to here
	
	// Quantization table marker
	*output_ptr++ = 0xFF;
	*output_ptr++ = 0xDB;

	// Quantization table length
	*output_ptr++ = 0x00;
	*output_ptr++ = 0x43;

	// Pq, Tq
	*output_ptr++ = 0x00;

	// Lqt table
	for (i=0; i<64; i++)
		*output_ptr++ = jpeg_encoder_structure->Lqt [i];

	// Quantization table marker
	*output_ptr++ = 0xFF;
	*output_ptr++ = 0xDB;
	
	// Quantization table length
	*output_ptr++ = 0x00;
	*output_ptr++ = 0x43;
	
	// Pq, Tq
	*output_ptr++ = 0x01;

	// Cqt table
	for (i=0; i<64; i++)
		*output_ptr++ = jpeg_encoder_structure->Cqt [i];

	if (huff) 
	{
		// huffman table(DHT)
		
		*output_ptr++=0xff;
		*output_ptr++=0xc4;
		*output_ptr++=0x01;
		*output_ptr++=0xa2;
		memmove(output_ptr,&JPEGHuffmanTable,JPG_HUFFMAN_TABLE_LENGTH);/*0x01a0*/
		output_ptr+=JPG_HUFFMAN_TABLE_LENGTH;
		
	}

	number_of_components = 3;

	// Frame header(SOF)

	// Start of frame marker
	*output_ptr++ = 0xFF;
	*output_ptr++ = 0xC0;

	header_length = (unsigned short) (8 + 3 * number_of_components);

	// Frame header length	
	*output_ptr++ = (unsigned char) (header_length >> 8);
	*output_ptr++ = (unsigned char) header_length;

	// Precision (P)
	*output_ptr++ = 0x08;/*8 bits*/

	// image height
	*output_ptr++ = (unsigned char) (image_height >> 8);
	*output_ptr++ = (unsigned char) image_height;

	// image width
	*output_ptr++ = (unsigned char) (image_width >> 8);
	*output_ptr++ = (unsigned char) image_width;

	// Nf
	*output_ptr++ = number_of_components;

	/* type 422 */
	*output_ptr++ = 0x01; /*id (y)*/
	*output_ptr++ = 0x21; /*horiz|vertical */
	*output_ptr++ = 0x00; /*quantization table used*/
	
	*output_ptr++ = 0x02; /*id (u)*/
	*output_ptr++ = 0x11; /*horiz|vertical*/
	*output_ptr++ = 0x01; /*quantization table used*/

	*output_ptr++ = 0x03; /*id (v)*/
	*output_ptr++ = 0x11; /*horiz|vertical*/
	*output_ptr++ = 0x01; /*quantization table used*/


	// Scan header(SOF)

	// Start of scan marker
	*output_ptr++ = 0xFF;
	*output_ptr++ = 0xDA;
	
	header_length = (unsigned short) (6 + (number_of_components << 1));

	// Scan header length
	*output_ptr++ = (unsigned char) (header_length >> 8);
	*output_ptr++ = (unsigned char) header_length;

	// Ns = number of scans
	*output_ptr++ = number_of_components;

	/* type 422*/
	*output_ptr++ = 0x01; /*component id (y)*/
	*output_ptr++ = 0x00; /*dc|ac tables*/

	*output_ptr++ = 0x02; /*component id (u)*/
	*output_ptr++ = 0x11; /*dc|ac tables*/

	*output_ptr++ = 0x03; /*component id (v)*/
	*output_ptr++ = 0x11; /*dc|ac tables*/
	
	*output_ptr++ = 0x00; /*0 */
	*output_ptr++ = 0x3F; /*63*/
	*output_ptr++ = 0x00; /*0 */
	
	return output_ptr;
}

void 
jpeg_restart (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure)
{
	jpeg_encoder_structure->ldc1 = 0;
	jpeg_encoder_structure->ldc2 = 0;
	jpeg_encoder_structure->ldc3 = 0;

	jpeg_encoder_structure->lcode = 0;
	jpeg_encoder_structure->bitindex = 0;
}


/*YUYV*/
unsigned char* read_422_format (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure, unsigned char *input_ptr)
{
	int i, j;
	
	signed short *Y1_Ptr = jpeg_encoder_structure->Y1; /*64 signed short block*/ 
	signed short *Y2_Ptr = jpeg_encoder_structure->Y2;
	signed short *CB_Ptr = jpeg_encoder_structure->CB;
	signed short *CR_Ptr = jpeg_encoder_structure->CR;

	unsigned short incr = jpeg_encoder_structure->incr;
	
	unsigned char *tmp_ptr=NULL;
	tmp_ptr=input_ptr;
	
	for (i=8; i>0; i--) /*8 rows*/
	{
		for (j=4; j>0; j--) /* 8 cols*/
		{
			*Y1_Ptr++ = *tmp_ptr++;
			*CB_Ptr++ = *tmp_ptr++;
			*Y1_Ptr++ = *tmp_ptr++;
			*CR_Ptr++ = *tmp_ptr++;
		}

		for (j=4; j>0; j--) /* 8 cols*/
		{
			*Y2_Ptr++ = *tmp_ptr++;
			*CB_Ptr++ = *tmp_ptr++;
			*Y2_Ptr++ = *tmp_ptr++;
			*CR_Ptr++ = *tmp_ptr++;
		}

		tmp_ptr += incr; /* next row (width - mcu_width)*/
	}
	tmp_ptr=NULL;/*clean*/
	return (input_ptr);
}

/* Level shifting to get 8 bit SIGNED values for the data  */
void levelshift (signed short* const data)
{
	signed short i;

	for (i=63; i>=0; i--)
		data [i] -= 128;
}

/* DCT for One block(8x8) */
void DCT (signed short *data)
{
	signed short i;
	int x0, x1, x2, x3, x4, x5, x6, x7, x8;
	signed short *tmp_ptr;
	tmp_ptr=data;
	/*  All values are shifted left by 10   */
	/*  and rounded off to nearest integer  */

	/* scale[0] = 1
	 * scale[k] = cos(k*PI/16)*root(2)
	 */
	static const unsigned short c1=1420;    /* cos PI/16 * root(2)  */
	static const unsigned short c2=1338;    /* cos PI/8 * root(2)   */
	static const unsigned short c3=1204;    /* cos 3PI/16 * root(2) */
	static const unsigned short c5=805;     /* cos 5PI/16 * root(2) */
	static const unsigned short c6=554;     /* cos 3PI/8 * root(2)  */
	static const unsigned short c7=283;     /* cos 7PI/16 * root(2) */

	static const unsigned short s1=3;
	static const unsigned short s2=10;
	static const unsigned short s3=13;


	/* row pass */
	for (i=8; i>0; i--)
	{
		x8 = data [0] + data [7];
		x0 = data [0] - data [7];

		x7 = data [1] + data [6];
		x1 = data [1] - data [6];

		x6 = data [2] + data [5];
		x2 = data [2] - data [5];

		x5 = data [3] + data [4];
		x3 = data [3] - data [4];

		x4 = x8 + x5;
		x8 -= x5;

		x5 = x7 + x6;
		x7 -= x6;

		data [0] = (signed short) (x4 + x5);
		data [4] = (signed short) (x4 - x5);

		data [2] = (signed short) ((x8*c2 + x7*c6) >> s2);
		data [6] = (signed short) ((x8*c6 - x7*c2) >> s2);

		data [7] = (signed short) ((x0*c7 - x1*c5 + x2*c3 - x3*c1) >> s2);
		data [5] = (signed short) ((x0*c5 - x1*c1 + x2*c7 + x3*c3) >> s2);
		data [3] = (signed short) ((x0*c3 - x1*c7 - x2*c1 - x3*c5) >> s2);
		data [1] = (signed short) ((x0*c1 + x1*c3 + x2*c5 + x3*c7) >> s2);

		data += 8;
	}

	data = tmp_ptr;/* return to start of mcu */
	
	/* column pass */
	for (i=8; i>0; i--)
	{
		x8 = data [0] + data [56];
		x0 = data [0] - data [56];

		x7 = data [8] + data [48];
		x1 = data [8] - data [48];

		x6 = data [16] + data [40];
		x2 = data [16] - data [40];

		x5 = data [24] + data [32];
		x3 = data [24] - data [32];

		x4 = x8 + x5;
		x8 -= x5;

		x5 = x7 + x6;
		x7 -= x6;

		data [0] = (signed short) ((x4 + x5) >> s1);
		data [32] = (signed short) ((x4 - x5) >> s1);

		data [16] = (signed short) ((x8*c2 + x7*c6) >> s3);
		data [48] = (signed short) ((x8*c6 - x7*c2) >> s3);

		data [56] = (signed short) ((x0*c7 - x1*c5 + x2*c3 - x3*c1) >> s3);
		data [40] = (signed short) ((x0*c5 - x1*c1 + x2*c7 + x3*c3) >> s3);
		data [24] = (signed short) ((x0*c3 - x1*c7 - x2*c1 - x3*c5) >> s3);
		data [8] = (signed short) ((x0*c1 + x1*c3 + x2*c5 + x3*c7) >> s3);

		data++;
	}
	data=tmp_ptr; /* return to start of mcu */
}

/* multiply DCT Coefficients with Quantization table and store in ZigZag location */
void quantization (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure, signed short* const data, unsigned short* const quant_table_ptr)
{
	signed short i;
	int value;

	for (i=63; i>=0; i--)
	{
		value = data [i] * quant_table_ptr [i];
		value = (value + 0x4000) >> 15;

		jpeg_encoder_structure->Temp [zigzag_table [i]] = (signed short) value;
	}
}

static unsigned short luminance_ac_code_table [162] =
{	
	0x000A,
	0x0000, 0x0001, 0x0004, 0x000B, 0x001A, 0x0078, 0x00F8, 0x03F6, 0xFF82, 0xFF83,
	0x000C, 0x001B, 0x0079, 0x01F6, 0x07F6, 0xFF84, 0xFF85, 0xFF86, 0xFF87, 0xFF88,
	0x001C, 0x00F9, 0x03F7, 0x0FF4, 0xFF89, 0xFF8A, 0xFF8b, 0xFF8C, 0xFF8D, 0xFF8E,
	0x003A, 0x01F7, 0x0FF5, 0xFF8F, 0xFF90, 0xFF91, 0xFF92, 0xFF93, 0xFF94, 0xFF95,
	0x003B, 0x03F8, 0xFF96, 0xFF97, 0xFF98, 0xFF99, 0xFF9A, 0xFF9B, 0xFF9C, 0xFF9D,
	0x007A, 0x07F7, 0xFF9E, 0xFF9F, 0xFFA0, 0xFFA1, 0xFFA2, 0xFFA3, 0xFFA4, 0xFFA5,
	0x007B, 0x0FF6, 0xFFA6, 0xFFA7, 0xFFA8, 0xFFA9, 0xFFAA, 0xFFAB, 0xFFAC, 0xFFAD,
	0x00FA, 0x0FF7, 0xFFAE, 0xFFAF, 0xFFB0, 0xFFB1, 0xFFB2, 0xFFB3, 0xFFB4, 0xFFB5,
	0x01F8, 0x7FC0, 0xFFB6, 0xFFB7, 0xFFB8, 0xFFB9, 0xFFBA, 0xFFBB, 0xFFBC, 0xFFBD,
	0x01F9, 0xFFBE, 0xFFBF, 0xFFC0, 0xFFC1, 0xFFC2, 0xFFC3, 0xFFC4, 0xFFC5, 0xFFC6,
	0x01FA, 0xFFC7, 0xFFC8, 0xFFC9, 0xFFCA, 0xFFCB, 0xFFCC, 0xFFCD, 0xFFCE, 0xFFCF,
	0x03F9, 0xFFD0, 0xFFD1, 0xFFD2, 0xFFD3, 0xFFD4, 0xFFD5, 0xFFD6, 0xFFD7, 0xFFD8,
	0x03FA, 0xFFD9, 0xFFDA, 0xFFDB, 0xFFDC, 0xFFDD, 0xFFDE, 0xFFDF, 0xFFE0, 0xFFE1,
	0x07F8, 0xFFE2, 0xFFE3, 0xFFE4, 0xFFE5, 0xFFE6, 0xFFE7, 0xFFE8, 0xFFE9, 0xFFEA,
	0xFFEB, 0xFFEC, 0xFFED, 0xFFEE, 0xFFEF, 0xFFF0, 0xFFF1, 0xFFF2, 0xFFF3, 0xFFF4,
	0xFFF5, 0xFFF6, 0xFFF7, 0xFFF8, 0xFFF9, 0xFFFA, 0xFFFB, 0xFFFC, 0xFFFD, 0xFFFE,
	0x07F9
};

static unsigned short luminance_ac_size_table [162] =
{	
	0x0004,
	0x0002, 0x0002, 0x0003, 0x0004, 0x0005, 0x0007, 0x0008, 0x000A, 0x0010, 0x0010,
	0x0004, 0x0005, 0x0007, 0x0009, 0x000B, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0005, 0x0008, 0x000A, 0x000C, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0006, 0x0009, 0x000C, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0006, 0x000A, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0007, 0x000B, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0007, 0x000C, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0008, 0x000C, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0009, 0x000F, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000A, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000A, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000B, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000B
};


static unsigned short chrominance_ac_code_table [162] =
{	
	0x0000,
	0x0001, 0x0004, 0x000A, 0x0018, 0x0019, 0x0038, 0x0078, 0x01F4, 0x03F6, 0x0FF4,
	0x000B, 0x0039, 0x00F6, 0x01F5, 0x07F6, 0x0FF5, 0xFF88, 0xFF89, 0xFF8A, 0xFF8B,
	0x001A, 0x00F7, 0x03F7, 0x0FF6, 0x7FC2, 0xFF8C, 0xFF8D, 0xFF8E, 0xFF8F, 0xFF90,
	0x001B, 0x00F8, 0x03F8, 0x0FF7, 0xFF91, 0xFF92, 0xFF93, 0xFF94, 0xFF95, 0xFF96,
	0x003A, 0x01F6, 0xFF97, 0xFF98, 0xFF99, 0xFF9A, 0xFF9B, 0xFF9C, 0xFF9D, 0xFF9E,
	0x003B, 0x03F9, 0xFF9F, 0xFFA0, 0xFFA1, 0xFFA2, 0xFFA3, 0xFFA4, 0xFFA5, 0xFFA6,
	0x0079, 0x07F7, 0xFFA7, 0xFFA8, 0xFFA9, 0xFFAA, 0xFFAB, 0xFFAC, 0xFFAD, 0xFFAE,
	0x007A, 0x07F8, 0xFFAF, 0xFFB0, 0xFFB1, 0xFFB2, 0xFFB3, 0xFFB4, 0xFFB5, 0xFFB6,
	0x00F9, 0xFFB7, 0xFFB8, 0xFFB9, 0xFFBA, 0xFFBB, 0xFFBC, 0xFFBD, 0xFFBE, 0xFFBF,
	0x01F7, 0xFFC0, 0xFFC1, 0xFFC2, 0xFFC3, 0xFFC4, 0xFFC5, 0xFFC6, 0xFFC7, 0xFFC8,
	0x01F8, 0xFFC9, 0xFFCA, 0xFFCB, 0xFFCC, 0xFFCD, 0xFFCE, 0xFFCF, 0xFFD0, 0xFFD1,
	0x01F9, 0xFFD2, 0xFFD3, 0xFFD4, 0xFFD5, 0xFFD6, 0xFFD7, 0xFFD8, 0xFFD9, 0xFFDA,
	0x01FA, 0xFFDB, 0xFFDC, 0xFFDD, 0xFFDE, 0xFFDF, 0xFFE0, 0xFFE1, 0xFFE2, 0xFFE3,
	0x07F9, 0xFFE4, 0xFFE5, 0xFFE6, 0xFFE7, 0xFFE8, 0xFFE9, 0xFFEA, 0xFFEb, 0xFFEC,
	0x3FE0, 0xFFED, 0xFFEE, 0xFFEF, 0xFFF0, 0xFFF1, 0xFFF2, 0xFFF3, 0xFFF4, 0xFFF5,
	0x7FC3, 0xFFF6, 0xFFF7, 0xFFF8, 0xFFF9, 0xFFFA, 0xFFFB, 0xFFFC, 0xFFFD, 0xFFFE, 
	0x03FA
};


static unsigned short chrominance_ac_size_table [162] =
{
	0x0002,
	0x0002, 0x0003, 0x0004, 0x0005, 0x0005, 0x0006, 0x0007, 0x0009, 0x000A, 0x000C,
	0x0004, 0x0006, 0x0008, 0x0009, 0x000B, 0x000C, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0005, 0x0008, 0x000A, 0x000C, 0x000F, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0005, 0x0008, 0x000A, 0x000C, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0006, 0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0006, 0x000A, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0007, 0x000B, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0007, 0x000B, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0008, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x0009, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000B, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000E, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000F, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010, 0x0010,
	0x000A
};
 
static unsigned char bitsize [256] =/* bit size from 0 to 255 */
{
	0, 1, 2, 2, 3, 3, 3, 3,
	4, 4, 4, 4, 4, 4, 4, 4,
	5, 5, 5, 5, 5, 5, 5, 5,
	5, 5, 5, 5, 5, 5, 5, 5,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	6, 6, 6, 6, 6, 6, 6, 6,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	7, 7, 7, 7, 7, 7, 7, 7,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8,
	8, 8, 8, 8, 8, 8, 8, 8
};



unsigned char* huffman (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure, 
		unsigned short component, unsigned char *output_ptr)
{
	unsigned short i;
	unsigned short *DcCodeTable, *DcSizeTable, *AcCodeTable, *AcSizeTable;

	signed short *Temp_Ptr, Coeff, LastDc;
	unsigned short AbsCoeff, HuffCode, HuffSize, RunLength=0, DataSize=0, index;

	signed short bits_in_next_word;
	unsigned short numbits;
	unsigned long int data;

	Temp_Ptr = jpeg_encoder_structure->Temp;
	Coeff = *Temp_Ptr++;/* Coeff = DC */
    
	/* code DC - Temp[0] */
	if (component == 1)/* luminance - Y */
	{
		DcCodeTable = luminance_dc_code_table;
		DcSizeTable = luminance_dc_size_table;
		AcCodeTable = luminance_ac_code_table;
		AcSizeTable = luminance_ac_size_table;

		LastDc = jpeg_encoder_structure->ldc1;
		jpeg_encoder_structure->ldc1 = Coeff;
	}
	else /* Chrominance - U V */
	{
		DcCodeTable = chrominance_dc_code_table;
		DcSizeTable = chrominance_dc_size_table;
		AcCodeTable = chrominance_ac_code_table;
		AcSizeTable = chrominance_ac_size_table;

		if (component == 2) /* Chrominance - U */
		{
			LastDc = jpeg_encoder_structure->ldc2;
			jpeg_encoder_structure->ldc2 = Coeff;
		}
		else/* Chrominance - V */
		{
			LastDc = jpeg_encoder_structure->ldc3;
			jpeg_encoder_structure->ldc3 = Coeff;
		}
	}

	Coeff = Coeff - LastDc; /* DC - LastDC */

	AbsCoeff = (Coeff < 0) ? -(Coeff--) : Coeff;

	/*calculate data size*/
	while (AbsCoeff != 0)
	{
		AbsCoeff >>= 1;
		DataSize++;
	}

	HuffCode = DcCodeTable [DataSize];
	HuffSize = DcSizeTable [DataSize];

	Coeff &= (1 << DataSize) - 1;
	data = (HuffCode << DataSize) | Coeff;
	numbits = HuffSize + DataSize;

	PUTBITS
		
    /* code AC */
	for (i=63; i>0; i--)
	{
		
		if ((Coeff = *Temp_Ptr++) != 0)
		{
			while (RunLength > 15)
			{
				RunLength -= 16;
				data = AcCodeTable [161];   /* ZRL 0xF0 ( 16 - 0) */
				numbits = AcSizeTable [161];/* ZRL                */
				PUTBITS
			}

			AbsCoeff = (Coeff < 0) ? -(Coeff--) : Coeff;

			if (AbsCoeff >> 8 == 0) /* Size <= 8 bits */
				DataSize = bitsize [AbsCoeff];
			else /* 16 => Size => 8 */
				DataSize = bitsize [AbsCoeff >> 8] + 8;

			index = RunLength * 10 + DataSize;
			
			
			HuffCode = AcCodeTable [index];
			HuffSize = AcSizeTable [index];

			Coeff &= (1 << DataSize) - 1;
			data = (HuffCode << DataSize) | Coeff;
			numbits = HuffSize + DataSize;
			
			PUTBITS
			RunLength = 0;
		}
		else
			RunLength++;/* Add while Zero */
	}

	if (RunLength != 0)
	{
		data = AcCodeTable [0];   /* EOB - 0x00 end of block */
		numbits = AcSizeTable [0];/* EOB                     */ 
		PUTBITS
	}
	return output_ptr;
}



unsigned char* 
encodeMCU (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure, unsigned char *output_ptr)
{
	levelshift (jpeg_encoder_structure->Y1);
	DCT (jpeg_encoder_structure->Y1);
	quantization (jpeg_encoder_structure, jpeg_encoder_structure->Y1, 
		jpeg_encoder_structure->ILqt);
	
	output_ptr = huffman (jpeg_encoder_structure, 1, output_ptr);

	levelshift (jpeg_encoder_structure->Y2);
	DCT (jpeg_encoder_structure->Y2);

	quantization (jpeg_encoder_structure, jpeg_encoder_structure->Y2, 
		jpeg_encoder_structure->ILqt);
	
	output_ptr = huffman (jpeg_encoder_structure, 1, output_ptr);
 
	levelshift (jpeg_encoder_structure->CB);
	DCT (jpeg_encoder_structure->CB);
	
	quantization (jpeg_encoder_structure, jpeg_encoder_structure->CB, 
		jpeg_encoder_structure->ICqt);
	
	output_ptr = huffman (jpeg_encoder_structure, 2, output_ptr);

	levelshift (jpeg_encoder_structure->CR);
	DCT (jpeg_encoder_structure->CR);
	
	quantization (jpeg_encoder_structure, jpeg_encoder_structure->CR, 
		jpeg_encoder_structure->ICqt);
	
	output_ptr = huffman (jpeg_encoder_structure, 3, output_ptr);

	return output_ptr;
}


/* For bit Stuffing and EOI marker */
unsigned char* close_bitstream (struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure, 
		unsigned char *output_ptr)
{
	unsigned short i, count;
	unsigned char *ptr;
	
	

	if (jpeg_encoder_structure->bitindex > 0)
	{
		jpeg_encoder_structure->lcode <<= (32 - jpeg_encoder_structure->bitindex);
		count = (jpeg_encoder_structure->bitindex + 7) >> 3;

		ptr = (unsigned char*) &jpeg_encoder_structure->lcode + 3;

		for (i=count; i>0; i--)
		{
			if ((*output_ptr++ = *ptr--) == 0xff)
				*output_ptr++ = 0;
		}
	}

	/* End of image marker (EOI) */
	*output_ptr++ = 0xFF;
	*output_ptr++ = 0xD9;
	return output_ptr;
}

int encode_image (unsigned char *input_ptr, unsigned char *output_ptr, 
	struct JPEG_ENCODER_STRUCTURE * jpeg_encoder_structure,
	int huff, unsigned long int image_width, unsigned long int image_height)
{
	int size;
	unsigned short i, j;
	unsigned char *tmp_ptr=NULL;
	unsigned char *tmp_iptr=NULL;
	unsigned char *tmp_optr=NULL;
	tmp_iptr=input_ptr;
	tmp_optr=output_ptr;
	
	/* clean jpeg parameters*/
	jpeg_restart(jpeg_encoder_structure);
	
	/* Writing Marker Data */
	tmp_optr = write_markers (jpeg_encoder_structure, tmp_optr, huff, 
							         image_width, image_height);

	for (i=0; i<jpeg_encoder_structure->vertical_mcus; i++) /* height /8 */
	{
		tmp_ptr=tmp_iptr;
		for (j=0; j<jpeg_encoder_structure->horizontal_mcus; j++) /* width /16 */
		{	
			/*reads a block*/
			read_422_format (jpeg_encoder_structure, tmp_iptr); /*YUYV*/
	
			/* Encode the data in MCU */
			tmp_optr = encodeMCU (jpeg_encoder_structure, tmp_optr);
			
			if(j<(jpeg_encoder_structure->horizontal_mcus -1)) 
			{
				tmp_iptr += jpeg_encoder_structure->mcu_width_size;
			}
			else 
			{
				tmp_iptr=tmp_ptr;
			}
		}
		tmp_iptr += jpeg_encoder_structure->offset;
		
	}

	/* Close Routine */
	tmp_optr=close_bitstream (jpeg_encoder_structure, tmp_optr);
	size=tmp_optr-output_ptr;
	tmp_iptr=NULL;
	tmp_optr=NULL;

	return (size);
}


static int read_frame(void)
{
	struct v4l2_buffer buf;
	unsigned int i, jpeg_size;
	struct JPEG_ENCODER_STRUCTURE *jpeg_struct=NULL;

	CLEAR(buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//buf.type = V4L2_BUF_TYPE_PRIVATE;
	buf.memory = V4L2_MEMORY_MMAP;

	ioctl(fd, VIDIOC_DQBUF, &buf);	//出列采集的帧缓冲

	assert(buf.index < n_buffers);
	printf("buf.index dq is %d,\n", buf.index);


	//编码jpeg图片
	jpeg_buf = malloc((sizeof(char)*640*480)/2);	
	printf("----%d-----\n", sizeof(struct JPEG_ENCODER_STRUCTURE));
	jpeg_struct = malloc(sizeof(struct JPEG_ENCODER_STRUCTURE));
	initialization (jpeg_struct, 640, 480);
	initialize_quantization_tables (jpeg_struct);
	jpeg_size = encode_image(buffers[buf.index].start, jpeg_buf, 
							jpeg_struct,1, 640, 480);
							

	//fwrite(buffers[buf.index].start, buffers[buf.index].length, 1, file_fd);	//将其写入文件中
	fwrite(jpeg_buf, jpeg_size, 1, file_fd);	//将其写入文件中
	ioctl(fd, VIDIOC_QBUF, &buf);	//再将其入列
    printf("------------------------#########################\n");

	return 1;
}

int main(int argc, char **argv)
{
	struct v4l2_capability cap;
	struct v4l2_format fmt;
	unsigned int i;
	enum v4l2_buf_type type;

	file_fd = fopen("test-mmap.jpg", "w");	//图片文件名

	fd = open(dev_name, O_RDWR /* required */  | O_NONBLOCK, 0);	//打开设备

	//fb_fd = open(dev_name, O_RDWR | O_NONBLOCK, 0);

	ioctl(fd, VIDIOC_QUERYCAP, &cap);	//获取摄像头参数

	CLEAR(fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//fmt.type = V4L2_BUF_TYPE_PRIVATE;
	fmt.fmt.pix.width = 640;
	fmt.fmt.pix.height = 480;
	//fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
	fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUV420;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;
	ioctl(fd, VIDIOC_S_FMT, &fmt);	//设置图像格式

	file_length = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;	//计算图片大小

	struct v4l2_requestbuffers req;
	CLEAR(req);
	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//req.type = V4L2_BUF_TYPE_PRIVATE;
	req.memory = V4L2_MEMORY_MMAP;

	ioctl(fd, VIDIOC_REQBUFS, &req);	//申请缓冲，count是申请的数量

	if (req.count < 2)
		printf("Insufficient buffer memory\n");

	buffers = calloc(req.count, sizeof(*buffers));	//内存中建立对应空间

	for (n_buffers = 0; n_buffers < req.count; n_buffers++) {
		struct v4l2_buffer buf;	//驱动中的一帧
		CLEAR(buf);
		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		//buf.type = V4L2_BUF_TYPE_PRIVATE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;

		if (-1 == ioctl(fd, VIDIOC_QUERYBUF, &buf))	//映射用户空间
			printf("VIDIOC_QUERYBUF error\n");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = mmap(NULL /* start anywhere */ ,	//通过mmap建立映射关系
						buf.length,
						PROT_READ | PROT_WRITE
						/* required */ ,
						MAP_SHARED
						/* recommended */ ,
						fd, buf.m.offset);

        printf("start:%x\n", buffers[n_buffers].start);
		if (MAP_FAILED == buffers[n_buffers].start)
			printf("mmap failed\n");
	}

	for (i = 0; i < n_buffers; ++i) {
		struct v4l2_buffer buf;
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		//buf.type = V4L2_BUF_TYPE_PRIVATE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = i;

		if (-1 == ioctl(fd, VIDIOC_QBUF, &buf))	//申请到的缓冲进入列队
			printf("VIDIOC_QBUF failed\n");
	}

	type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	//type = V4L2_BUF_TYPE_PRIVATE;

	if (-1 == ioctl(fd, VIDIOC_STREAMON, &type))	//开始捕捉图像数据
		printf("VIDIOC_STREAMON failed\n");

	for (;;)		//这一段涉及到异步IO
	{
		fd_set fds;
		struct timeval tv;
		int r;

		FD_ZERO(&fds);	//将指定的文件描述符集清空
		FD_SET(fd, &fds);	//在文件描述符集合中增加一个新的文件描述符

		/* Timeout. */
		tv.tv_sec = 2;
		tv.tv_usec = 0;

		r = select(fd + 1, &fds, NULL, NULL, &tv);	//判断是否可读（即摄像头是否准备好），tv是定时

		if (-1 == r) {
			if (EINTR == errno)
				continue;
			printf("select err/n");
		}
		if (0 == r) {
			fprintf(stderr, "select timeout\n");
			exit(EXIT_FAILURE);
		}

		if (read_frame())	//如果可读，执行read_frame ()函数，并跳出循环
			break;
	}

unmap:
	for (i = 0; i < n_buffers; i++)
		if (-1 == munmap(buffers[i].start, buffers[i].length))
			printf("munmap error");
    printf("--------\n");
	close(fd);
	fclose(file_fd);
	return 0;
}
