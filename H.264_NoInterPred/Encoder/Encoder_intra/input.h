#pragma once
#include "main.h"

#define VIDEO_FRAME			90

#define WIDTH				352
#define HEIGHT				288
#define PSIZE				8				// macro-block size (default)
 
#define INPUT_FILENAME		"C:\\images\\football_cif(352X288)_90f.yuv"
#define OUTPUT_FILENAME_1	"C:\\images\\football_cif(352X288)_90f_bitstream.raw"
#define OUTPUT_FILENAME_2	"C:\\images\\football_cif(352X288)_90f_recon_encoder.yuv"

#define OUTPUT_FILENAME_3	"C:\\images\\text_encoder.txt"

#define DCT_MODE			2	
#define PRED_MODE			4

#define FORWARD				1
#define INVERSE				0

#define PADDING_VAL			pow(2, 8) /2	// PADDING_VAL = pow(2 , Pixel_range) / 2
#define log(x,base)			log(x)/log(base)

#define TEST_HUFFMAN		0				// 허프만 부호화 적용	: 1
											// 지수골룸 부호화 적용 : 0
#define TEST				0

#define SEND_SYMBOL_VAL		1
#define SEND_SYMBOL_VAL_	0

#define SEND_SYMBOL_TYPE	1
#define SEND_FREQ			0