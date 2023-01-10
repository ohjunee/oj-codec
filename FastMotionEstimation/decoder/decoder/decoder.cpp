#include "Functions.h"

void Decoder(BUFFER* img, DATA* file, Node* head)
{
	// 맨 처음 데이터 읽어오기
	fread(&file->output, sizeof(uint8), 1, file->fp_in);

	head = (Node*)malloc(sizeof(Node));
	head->next = NULL;
	
	//동영상 전체 frame 디코딩
	while (file->frameCount < VIDEO_FRAME)
	{
		Node* FirstNode = createFirstNode(head);	//노드 한개 생성

		// 동영상 1 frame 코딩 Y/Cb/Cr 각각의 경우의 비트스트림을 디코딩
		for (int component = 0; component < 1; component++)
			component == 0 ? Read_Y(img, file, FirstNode, HEIGHT, WIDTH) : Read_CbCr(img, file, HEIGHT / 2, WIDTH / 2);

		if (file->frameCount)
			removeLastNode(head);

		file->frameCount++;
	}
}

// luma coding
void Read_Y(BUFFER* img, DATA* file, Node* POC, uint32 height, uint32 width)
{
	Setup_Memory(img, file, POC, height, width);	// 영상 입력
	Partitioning(img, file, POC, height, width);	// 영상 분할
	ImageSave(img, file, POC, height, width);		// 복원 영상 저장
}

// chroma coding
void Read_CbCr(BUFFER* img, DATA* file, uint32 height, uint32 width)
{
	uint32 img_size = HEIGHT / 2 * WIDTH / 2;
	uint8* CbCr_img = (uint8*)calloc(img_size, sizeof(uint8));
	fread(CbCr_img, sizeof(uint8), img_size, file->fp_in);
	free(CbCr_img);
}

void Partitioning(BUFFER* img, DATA* file, Node* POC, uint32 height, uint32 width)
{
	uint32 UpLeftPx = 0;

	while (img->blk_cnt < img->total_blk)
	{
		img->isFirstBlk_in_row = !(img->blk_cnt % img->nBlk_width);	// 0이면 true가 되어, 각 행의 첫번째 블록임을 표시
		img->isLastBlk_in_row = !((img->blk_cnt + 1) % img->nBlk_width);// 0이면 true가 되어, 각 행의 마지막 블록임을 표시

		// 두번째 행 이상의 첫번째 블록의 경우
		if (img->blk_cnt != 0 && img->isFirstBlk_in_row)
		{
			img->row_order++;
			UpLeftPx = img->row_order * width * PSIZE;
		}
		
		/* Read_INTER_INFO, Read_INTRA_INFO */
		(file->frameCount % INTRA_PERIOD) ? Read_Inter(img, file, POC->next, width, UpLeftPx) : Read_Intra(img, file, width, UpLeftPx);
		UpLeftPx += PSIZE;
		img->blk_cnt++;
	}
}

void Read_Intra(BUFFER* img, DATA* file, uint32 width, uint32 upleft_px)
{
	uint8* Pred_blk = (uint8*)calloc(PSIZE * PSIZE, sizeof(uint8));
	file->buffer = (double*)calloc(PSIZE * PSIZE, sizeof(double));	//복원된 예측에러블록

	// A0. 예측모드 읽기
	Read_PRED_INFO(file, &file->MPM_flag, 2);

	// A1. MPM_flag가 0인 경우 디코더에서 보낸 예측모드정보를 읽어야함, 
	if (!file->MPM_flag)
		Read_PRED_INFO(file, &file->PredMode, 4);					// 예측모드정보, 표현부호어 4가지(0, 1, 2, 3번 모드)

	// A2. 1인 경우 Find_MPM으로 모드예측, bit수 절약
	else
		Find_MPM(img, file, width);

	// A3. 예측모드 배열에 복호화된 예측모드 정보 저장
	file->PM_Arr[img->blk_cnt] = file->PredMode;

	/* B. 예측 모드에 대한 예측 블록 생성 */
	getPredBlock(img, Pred_blk, file->PredMode, upleft_px); //nWidth + 1

	/* C2 전달받은 예측에러블록 정보 복호화 */
	for (int i = 0; i < PSIZE * PSIZE; i++)
	{
		int cnt = 0;

		while (cnt < 12)
		{
			uint8 result = 0, buffer = 0;

			buffer = file->output << (file->length % 8) & 128;

			result = result << 1 | buffer;

			file->length++;

			if (file->length % 8 == 0)
			{
				fread(&file->output, sizeof(uint8), 1, file->fp_in);
				file->length = 0;
			}

			if (cnt)
			{
				switch (result)
				{
				case 0:
					file->buffer[i] = 0;
					break;
				case 2:
					Read_Symbol(file, i, 0);
					break;
				case 3:
					Read_Symbol(file, i, 1);
					break;
				case 4:
					Read_Symbol(file, i ,2);
					break;
				case 5:
					Read_Symbol(file, i, 3);
					break;
				case 6:
					Read_Symbol(file, i, 4);
					break;
				case 14:
					Read_Symbol(file, i, 5);
					break;
				case 30:
					Read_Symbol(file, i, 6);
					break;
				case 62:
					Read_Symbol(file, i, 7);
					break;
				case 126:
					Read_Symbol(file, i, 8);
					break;
				case 254:
					Read_Symbol(file, i, 9);
					break;
				case 510:
					Read_Symbol(file, i, 10);
					break;
				case 1022:
					Read_Symbol(file, i, 11);
					break;
				case 2046:
					Read_Symbol(file, i, 12);
					break;
				}
			}
		
		}
	}

	/* Reordering (Decoder에서 복호화된 양자화 계수값들 재정렬) */
	Recon_ZigZagScan(img, file);										// file->buffer -> file->ZigZag_blk
	
	/* 양자화 변환계수블록의 DC값을 DPCM 복호화 */
	Inverse_DC_DPCM	(img, file);
	Inverse_Q		(img, file);
	IDCT			(img, file);										// file->ZigZag_blk -> img->Recon_DCT_blk

	/* D. 예측블록 + 복원된 예측에러블록 = 복원블록 -> 복원이미지 */
	for (uint8 j = 0; j < PSIZE; j++) {
		for (uint8 i = 0; i < PSIZE; i++)
		{
			int recon_px = Pred_blk[j + i * PSIZE] + (int)round(img->Recon_DCT_blk[j + i * PSIZE]);

			// 클리핑
			if (recon_px > 255)
				img->Recon[upleft_px + j + i * width] = 255;
			else if (recon_px < 0)
				img->Recon[upleft_px + j + i * width] = 0;
			else
				img->Recon[upleft_px + j + i * width] = recon_px;
		}
	}
	free(file->buffer);
	free(Pred_blk);
}

#if TEST == 1
void Read_Inter(BUFFER* img, DATA* file, Node* POC, uint32 width, uint32 upleft_px)
{
	uint8* Pred_Block = (uint8*)calloc(PSIZE * PSIZE, sizeof(uint8));
	file->buffer = (double*)calloc(PSIZE * PSIZE, sizeof(double));	//복원된 예측에러블록

	// A0. 예측모드 읽기
	Read_PRED_INFO(file, &file->PredMode, 4);

	/* B. 예측 모드에 대한 예측 블록 생성 */
	getPredBlock(img, Pred_Block, file->PredMode, upleft_px); //nWidth + 1

	/* C2 전달받은 예측에러블록 정보 복호화 -> 역양자화, IDCT 수행 */

	/* Reordering (Decoder에서 복호화된 양자화 계수값들 재정렬) */
	Recon_ZigZagScan(img, file);										// file->buffer -> file->ZigZag_blk

	/* 양자화 변환계수블록의 DC값을 DPCM 복호화 */
	Inverse_DC_DPCM(img, file);
	Inverse_Q(img, file);
	IDCT(img, file);										// file->ZigZag_blk -> img->Recon_DCT_blk

	/* D. 예측블록 + 복원된 예측에러블록 = 복원블록 -> 복원이미지 */
	for (uint8 j = 0; j < PSIZE; j++) {
		for (uint8 i = 0; i < PSIZE; i++)
		{
			int recon_px = Pred_Block[j + i * PSIZE] + (int)round(file->Recon_DCT_blk[j + i * PSIZE]);

			// 클리핑
			if (recon_px > 255)
				img->Recon[upleft_px + j + i * width] = 255;
			else if (recon_px < 0)
				img->Recon[upleft_px + j + i * width] = 0;
			else
				img->Recon[upleft_px + j + i * width] = recon_px;
		}
	}

	free(file->buffer);
	free(Pred_Block);
}
#endif


void Read_Symbol(DATA* file, uint8 idx, uint8 bitLen)
{
	/* read sign bit */
	uint8	sign = file->output << (file->length % 8) & 128, buffer = 0;
	uint32	result = 0;

	file->length++;

	if (file->length % 8 == 0)
	{
		fread(&file->output, sizeof(uint8), 1, file->fp_in);
		file->length = 0;
	}

	/* FLC */
	for (int i = 0; i < bitLen; i++)
	{
		buffer = file->output << (file->length % 8) & 128;

		result = result << 1 | buffer;

		file->length++;

		if (file->length % 8 == 0)
		{
			fread(&file->output, sizeof(uint8), 1, file->fp_in);
			file->length = 0;
		}
	}

	file->buffer[idx] = pow(-1, sign) * (result + pow(2, bitLen));
}


void Read_PRED_INFO(DATA* file, uint8* code, uint8 level)
{
	uint8 buffer = 0;
	uint8 bitLen = round(log(level, 2));		//표현부호어 갯수에 따라 비트의 길이 결정

	*code = 0;

	for (int cnt = 0; cnt < bitLen; cnt++)
	{
		buffer = (file->output << (file->length % 8) & 128) ? 1 : 0;
		*code = *code << 1 | buffer;

		file->length++;

		if (file->length % 8 == 0)
		{
			fread(&file->output, sizeof(uint8), 1, file->fp_in);
			file->length = 0;
		}
	}
}