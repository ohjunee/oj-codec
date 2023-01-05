#pragma once
#include "main.h"

typedef struct Node {

	uint8*	ref_img;
	Node*	next;

}Node;

typedef struct imageBuffer
{
	uint8* org;
	uint8* Recon;

	double* Recon_DCT_blk;
	double* DC_Recon;
	double* ZigZag_arr;

	double** PredErr_buf2D;

	uint32	nBlk_width, nBlk_height, total_blk, blk_cnt, row_order;

	bool	isFirstBlk_in_row, isLastBlk_in_row;

}BUFFER;

typedef struct FILE_BUFFER
{
	FILE* fp_in;				//디코더 입력영상 파일포인터
	FILE* fp_out;				//디코더 출력영상 파일포인터

	FILE* fp;					//텍스트 파일

	FILE* fp_1;
	FILE* fp_2;

	double* buffer;

	uint8*	PM_Arr;
	uint8	MPM_flag;			//주변블록으로부터 예측한 방향과 같은 예측방향일 경우 1
	uint8	PredMode;

	uint8*	symbol;
	uint8	n_symbol;
	
	int mv_x, mv_y;

	uint8	AC_flag;
	uint8	MVmodeflag;

	uint8	output;				//비트스트림 아웃풋
	uint32	length;

	uint32  frameCount;

}DATA;