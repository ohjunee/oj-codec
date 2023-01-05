#include "Functions.h"

int main(int argc, char* argv[])
{
	BUFFER	image;
	DATA	file;
	Node	ref_image;

	checkFile(&file, argc, argv, true);				// 동영상 파일 열기
	encoder(&image, &file, &ref_image);				// 인코더 입력/출력
	checkFile(&file, argc, argv, false);			// 동영상 파일 닫기

	PSNR(&file, argv);

	return 0;
}