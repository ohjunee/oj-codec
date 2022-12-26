#include "Buffer.h"
#include "Node.h"
#include "input.h"

int main()
{
	BUFFER	image;
	Data	file;

	checkFile	(&file, true);	// 동영상 파일 열기
	encoder		(&image, &file);// 인코더 입력/출력
	checkFile	(&file, false);	// 동영상 파일 닫기

	return 0;
}