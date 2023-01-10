#include "Functions.h"

int main(int argc, char* argv[])
{
	BUFFER	image;
	DATA	file;
	Node	POC;

	checkFile(&file, argc, argv, true);				// 동영상 파일 열기
	Decoder(&image, &file, &POC);	// 인코더 입력/출력
	checkFile(&file, argc, argv, false);			// 동영상 파일 닫기

	Check_Image(&file, argv);
	return 0;
}