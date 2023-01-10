#include "Functions.h"

void encoder(BUFFER* img, DATA* file, Node* head)
{
	Send_StreamHeader(file);

	head = (Node*)malloc(sizeof(Node));

	if (head == 0)
	{
		printf("memory loss");
		exit(0);
	}

	head->next = NULL;

	//동영상 전체 frame 코딩
	while (file->frameCount < file->VideoFrame)
	{
		Node* FirstNode = createFirstNode(head);	//노드 한개 생성

		// 동영상 1 frame 코딩 Y/Cb/Cr 각각의 경우의 비트스트림을 인코딩
		for (int component = 0; component < 3; component++)
		{
			//			file->nbyte = 0;
			file->comp = component;
			Read_YCbCr(img, file, FirstNode);
		}
		// 남은 비트는 msb 쪽으로 밀기
		if (file->frameCount == file->VideoFrame - 1 && file->length % 8 != 0)
		{
			file->output = file->output << (8 - (file->length % 8));
			fwrite(&file->output, sizeof(uint8), 1, file->fp_out);
			file->nbyte++;
		}

		if (file->frameCount)
			removeLastNode(head);

		file->frameCount++;

	}
}

void Read_YCbCr(BUFFER* img, DATA* file, Node* POC)
{
	ImageLoad(img, file, POC);		// 영상 입력
	Partitioning(img, file, POC);		// 영상 분할
	ImageSave(img, file);		// 복원 영상 저장
}

void checkFirstFrame(BUFFER* img, uint32& upleft_px)
{
	uint8 p_size = img->p_size;

	/* no prediction */
	for (int j = 0; j < p_size; j++) {
		for (int i = 0; i < p_size; i++)
		{
			img->Recon[upleft_px + j + i * img->width] = img->org[upleft_px + j + i * img->width];
		}
	}

}

void Partitioning(BUFFER* img, DATA* file, Node* POC)
{
	// current picture init
	uint32 UpLeftPx = 0;

	//reference picture init
	img->UpLeftPx_ref = img->p_size + (img->width + 2 * img->p_size) * img->p_size;

	while (img->blk_cnt < img->total_blk)
	{
		img->isFirstBlk_in_row_org = !(img->blk_cnt % img->nBlk_width);		// 0이면 true가 되어, 각 행의 첫번째 블록임을 표시
		img->isLastBlk_in_row_org = !((img->blk_cnt + 1) % img->nBlk_width);// 0이면 true가 되어, 각 행의 마지막 블록임을 표시

		// 두번째 행 이상의 첫번째 블록의 경우
		if (img->blk_cnt != 0 && img->isFirstBlk_in_row_org)
		{
			img->row_order++;
			UpLeftPx = img->row_order * img->width * img->p_size;

			// inter 예측 시 사용되는 명령
//			if (file->frameCount % (file->intra_period))
			if (file->frameCount > 0)
				img->UpLeftPx_ref = img->p_size + (img->row_order + 1) * (img->width + 2 * img->p_size) * img->p_size;
		}

		//		(file->frameCount % (file->intra_period)) ? INTER_Prediction(img, file, POC->next, UpLeftPx) : INTRA_Prediction(img, file, UpLeftPx);
		(file->frameCount > 0) ? INTER_Prediction(img, file, POC->next, UpLeftPx) : checkFirstFrame(img, UpLeftPx);

		UpLeftPx += img->p_size;
		img->blk_cnt++;
	}
}