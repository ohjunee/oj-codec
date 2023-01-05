#pragma once
#include "structure.h"
#include "input.h"

void		ImageSave		(BUFFER* img, DATA* file, Node* POC, uint32 height, uint32 width);
void		Setup_Memory	(BUFFER* img, DATA* file, Node* POC, uint32 height, uint32 width);
void		Check_Image		(DATA* file);

uint8**		uint8_calloc2D	(uint8 row, uint8 col);
uint32**	uint32_calloc2D	(uint8 row, uint8 col);
double**	double_calloc2D	(uint8 row, uint8 col);
int**		int_calloc2D	(uint8 row, uint8 col);

void		Check_File	(DATA* file, bool Bool);
void		Open_File	(DATA* file);
void		Close_File	(DATA* file);

double		DCT_Basis_Vector(uint8 k, uint8 n);

void		Inverse_Q	(BUFFER* img, DATA* file);
void		IDCT		(BUFFER* img, DATA* file);

void		Recon_ZigZagScan	(BUFFER* img, DATA* file);
void		Inverse_DC_DPCM		(BUFFER* img, DATA* file);

void		Decoder				(BUFFER* img, DATA* file, Node* head);
void		getPredBlock		(BUFFER* img, uint8* Pred_b, uint8 mode, uint32 upleft_px);

void		Read_PRED_INFO		(DATA* file, uint8* code, uint8 level);
void		Read_Symbol			(DATA* file, uint8 idx, uint8 bitLen);

Node*		createFirstNode		(Node* head);
void		removeLastNode		(Node* head);

void		Read_Inter		(BUFFER* img, DATA* file, Node* POC, uint32 width, uint32 upleft_px);
void		Read_Intra		(BUFFER* img, DATA* file, uint32 width, uint32 upleft_px);

void		Find_MPM		(BUFFER* img, DATA* file, uint32 width);

void		QuickSort(int* ref_val, int left, int right);
void		QuickSort(double* ref_val, int left, int right);


