/**
 * Description:
 * Copyright 2024. All Rights Reserved.
 * Author: Zhao Ming (zhaomingwork@qq.com)
 */
#include "../include/zvad.h"

#ifndef _EXTRACTDATA_H_
#define _EXTRACTDATA_H_

#include <stdint.h>
#include <stdio.h>   
typedef struct tagWAVHEADER {
    uint8_t   ChunkID[4];     // 文档标识。       大写字符串"RIFF",标明该文件为有效的 RIFF 格式文档。
    uint32_t  ChunkSize;      // 文件数据长度。   从下一个字段首地址开始到文件末尾的总字节数。该字段的数值加 8 为当前文件的实际长度。
    uint8_t   Format[4];      // 文件格式类型。   所有 WAV 格式的文件此处为字符串"WAVE",标明该文件是 WAV 格式文件。
    uint8_t   FmtChunkID[4];  // 格式块标识。     小写字符串,"fmt "。
    uint32_t  FmtChunkSize;   // 格式块长度。     其数值不确定,取决于编码格式。可以是 16、 18 、20、40 等。
    uint16_t  AudioFormat;    // 编码格式代码。   常见的 WAV 文件使用 PCM 脉冲编码调制格式,该数值通常为 1。
    uint16_t  NumChannels;    // 声道个数。       单声道为 1,立体声或双声道为 2。
    uint32_t  SampleRate;     // 采样频率。       每个声道单位时间采样次数。常用的采样频率有 11025, 22050 和 44100 kHz。
    uint32_t  ByteRate;       // 数据传输速率。   该数值为:声道数×采样频率×每样本的数据位数/8。播放软件利用此值可以估计缓冲区的大小。
    uint16_t  BlockAlign;     // 数据块对齐单位。 采样帧大小。该数值为:声道数×位数/8。播放软件需要一次处理多个该值大小的字节数据,用该数值调整缓冲区。
    uint16_t  BitsPerSample;  // 采样位数。       存储每个采样值所用的二进制数位数。常见的位数有 4、8、12、16、24、32。
    uint8_t   DataChunkID[4];
    uint32_t  DataChunkSize;
} WAVHEADER;

#endif  // #ifndef _EXTRACTDATA_H_

int main(int argc, char** argv)
{
	if(argc!=3)
	{
		printf("error in args, examples model_path example wav_path \n");
		return 0;
	}
	char * model_path=argv[1];
	char * wavfile_path=argv[2];
	printf("model_path is %s, wav file is %s\n",model_path,wavfile_path);
	FILE *wav_fp;
    WAVHEADER    file_header;    
    long file_len=0;	
    wav_fp = fopen(wavfile_path,"rb");    /* open wav file in rb mode */
    fseek(wav_fp, 0L, SEEK_END);  
    file_len = ftell(wav_fp);   
	rewind(wav_fp);   
	fread(&file_header, 1, sizeof(WAVHEADER), wav_fp);
	int sample_rate=16000;
	ZVAD_OBJ* new_vad=vad_init(model_path,sample_rate, 1,0,0.8);
	int chunk_size=1024;
	short   buff[chunk_size];               // short buffer for file 
	float buff_float[chunk_size];           // float buffer
	
	for(long i=0;i<file_len/2;i=i+chunk_size)
	{
		int read_len=fread(buff, sizeof(short), chunk_size, wav_fp);
		for(int j=0;j<read_len;j++)
		{
 
			buff_float[j]=(float)buff[j]/32768.0;  // convert from short to float
		}
		ZVAD_OBJ_STATE state=vad_feed(new_vad,(float*)buff_float,chunk_size);
		if(state==1) // when detect voice print the time and state
		{
			printf("time %0.2f, detected voice \n",(float)(new_vad->data_len)/sample_rate);
	    }
		else
		{
			printf("time %0.2f, not voice \n",(float)(new_vad->data_len)/sample_rate);
		}
	}
	fclose(wav_fp);
	vad_destroy(new_vad);
	return 0;
}