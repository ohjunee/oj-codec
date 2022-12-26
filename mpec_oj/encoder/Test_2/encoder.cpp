#include "Functions.h"

void encoder(BUFFER* img, DATA* file, Node* head)
{

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
			file->comp = component;
			Read_YCbCr(img, file, FirstNode);
		}

		if(file->frameCount)
			removeLastNode(head);
		
		file->frameCount++;
	}
}

void Read_YCbCr(BUFFER* img, DATA* file, Node* POC)
{
	ImageLoad			(img, file, POC);		// 영상 입력
	Partitioning			(img, file, POC);		// 영상 분할
	ImageSave 			(img, file);				// 복원 영상 저장
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

			/* intra 일때 안씀 */
			if (file->frameCount > 0)
				img->UpLeftPx_ref = img->p_size + (img->row_order + 1) * (img->width + 2 * img->p_size) * img->p_size; 
		}
		
		(file->frameCount > 0) ? INTER_Prediction(img, file, POC->next, UpLeftPx) : checkFirstFrame(img, UpLeftPx);

		UpLeftPx += img->p_size;
		img->blk_cnt++;
	}
}