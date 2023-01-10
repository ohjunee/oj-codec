#pragma once
#include "main.h"

typedef struct Node {

	uint8* Y;
	uint8* Cb;
	uint8* Cr;

	int** mv_arr;

	Node* next;

}Node;

typedef struct imageBuffer
{
	uint8* org;
	uint8* Recon;
	uint8* Recon_;

	double* DC_Arr;
	double* DC_Recon;

	double** Ref_Pixel2D;
	double* Recon_PredErr;

	uint32	nBlk_width, nBlk_height, total_blk, blk_cnt, row_order;

	uint32  nBlk_Pad_width, nBlk_Pad_height, tot_Pad_blk;

	bool	isFirstBlk_in_row_org, isFirstBlk_in_row_pad, isLastBlk_in_row_org, isLastBlk_in_row_pad;

	uint32 imgSize, macroSize;
	uint16 height, width, height_pad, width_pad;
	uint8 p_size;

	/*inter cost*/
	uint32	cnt, cost, min_val;
	uint32	UpLeftPx_ref;
}BUFFER;

typedef struct FILE_BUFFER
{
	FILE* fp_ori;				//인코더 원본영상 버퍼(패딩된 이미지)
	FILE* fp_ori_recon;			//인코더 복원영상 버퍼
	FILE* fp_out;				//인코더 비트스트림
	FILE* fp_txt;				//결과 텍스트 파일
	FILE* fp_1;				//결과 텍스트 파일
	FILE* fp_2;				//결과 텍스트 파일

	uint8* ref_img;
	uint8* buffer;
	uint8 comp;

	double* ZigZag_arr;

	uint8* PM_Arr;
	uint8	MPM_flag;			//주변블록으로부터 예측한 방향과 같은 예측방향일 경우 1

	int mv_x, mv_y;
	int mv_arr[2];

	uint8	AC_flag;
	uint8	MVmodeflag;

	uint8	output;				//비트스트림 아웃풋
	uint32	length;
	uint32	nbyte;

	uint32  frameCount;
	uint32  VideoFrame;

	uint8	pixel_dpcm_mode;
	uint8	dc_dpcm_mode;
	uint8	intra_enable;
	uint8	intra_period;
	uint8	QP_dc;
	uint8	QP_ac;

	char* in_img;
	char* recon_img;
	char* outfile;
}DATA;