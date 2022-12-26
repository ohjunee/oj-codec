#include "Functions.h"

int main()
{
	BUFFER	image;
	DATA	file;
	Node	POC;

	Check_File(&file, true);		// 동영상 압축파일 열기
	Decoder(&image, &file, &POC);	// 인코더 입력/출력
	Check_File(&file, false);		// 동영상 파일 닫기

	Check_Image(&file);
	return 0;
}