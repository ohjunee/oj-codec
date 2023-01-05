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
	double* Recon_DCT_blk;

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
	FILE* fp_in;		// 디코더 입력파일
	FILE* fp_out;		// 디코더 복원영상

	FILE* fp_ori;		// 원본 영상
	FILE* fp_recon_en;	// 인코더 복원영상 
	FILE* fp_txt;		// psnr, 압축률 결과값 텍스트파일

	uint8* ref_img;
	uint8* buffer;
	uint8  comp;

	double* ZigZag_arr;
	int* recon_bit;

	uint16	height, width;
	uint8	QP_AC, QP_DC, pxDPCM_MODE, DC_DPCM_MODE, IntraPeriod, IntraEnable, MV_PREDMODE;

	uint8* PM_Arr;
	uint8	MPM_flag;			//주변블록으로부터 예측한 방향과 같은 예측방향일 경우 1
	uint8	PredMode;

	int		mv_arr[2];

	uint8	AC_flag;
	uint8	MVmodeflag;

	uint8	output;				//비트스트림 아웃풋
	uint32	length;
	uint32	inFileSize;

	uint32  frameCount;
	uint32	VideoFrame;

	char* recon_en;
	char* recon_de;
	char* infile;
	char* ori;
}DATA;