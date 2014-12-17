#ifndef _PARACOORD_CU_
#define _PARACOORD_CU_

#include <GL/glew.h>
#if defined(__APPLE__) || defined(MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <helper_cuda.h>
#include <helper_cuda_gl.h> 

#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/device_ptr.h>
#include <thrust/device_malloc.h>
#include <thrust/device_free.h>
#include <thrust/device_vector.h>
#include <thrust/copy.h>
#include <thrust/fill.h>
#include <thrust/sequence.h>
#include <thrust/scan.h>
#include <thrust/sort.h>




#include "datamanager.h"
#include "PerformanceTimer.h"
#include "ParaCoord_kernel.cu"

 
 

//#define DEBUG_MODE
 
using namespace VIS;

#define BLOCK_SIZE 256

//variables

unsigned char   *d_buffer_ptr;
unsigned int    dn_maxDimension;
unsigned int    dn_totalBytes;
unsigned int	dn_vertexNum;
unsigned int	dn_compactItemNum;
unsigned int	dn_edgeNum;

//desc
unsigned int *d_item_offset_ptr;	
unsigned int *d_item_bytes_ptr;	
unsigned int *d_item_values_ptr;

unsigned int *d_compactValue_ptr;
unsigned int *d_selectedItems_ptr;
unsigned int dn_selectItemsNum;

//filterdata
unsigned int *d_filterData_ptr;
unsigned int  dn_filterDataNum;

//highlightData
unsigned char *d_colorValue_ptr ;
unsigned int  dn_ColorNum;
unsigned int  dn_RecordNum;
unsigned int *d_highRecord_ptr;
unsigned char *d_highIndex_ptr;



//vertex Buffer
unsigned char *d_vertex_bool_ptr;
unsigned int  *d_vertex_prefix_ptr;
unsigned int  *d_item_prefix_ptr;
unsigned int  *d_vertex_sort_ptr;

 
//edge list   
unsigned int *d_edgeDummy_ptr;
unsigned char *d_edgeBool_ptr;
unsigned int *d_edgeItemPre_ptr;
unsigned int *d_edgePrefix_ptr;

//temporal 
//unsigned char *d_boolean_vector_ptr;
//unsigned int  *d_boolean_prefix_ptr;
unsigned int  *d_dummyIndex_ptr;
unsigned int  *d_dummyValue_ptr;
unsigned char  *d_dummyBool_ptr;
unsigned int  *d_dummySum_ptr;
unsigned int  *d_compactIndex_ptr;

unsigned int  *d_item_num_ptr;
unsigned int  *d_edgeItemNum_ptr;
unsigned int  *d_vertex_Index_ptr;
unsigned char *d_colorBool_ptr;




extern "C" 
void cuda_Init()
{
	d_compactValue_ptr	=	NULL;
	d_selectedItems_ptr	=	NULL;
	
	d_filterData_ptr	=	NULL;
	d_colorValue_ptr	=	NULL;
	d_highRecord_ptr	=	NULL;
	d_highIndex_ptr		=	NULL;
	
	d_vertex_bool_ptr	=	NULL;
	d_vertex_prefix_ptr	=	NULL;
	d_item_prefix_ptr	=	NULL;
	d_vertex_sort_ptr	=	NULL;
	
	d_edgeDummy_ptr		=	NULL;
	d_edgeBool_ptr		=	NULL;
	d_edgeItemPre_ptr	=	NULL;
	d_edgePrefix_ptr	=	NULL;

	d_compactIndex_ptr	=	NULL;

	d_item_num_ptr		=	NULL;
	d_edgeItemNum_ptr	=	NULL;
	d_vertex_Index_ptr	=	NULL;
}

extern "C"
void cuda_Clean()
{
	if(d_compactValue_ptr != NULL)
		cudaFree(d_compactValue_ptr);
	if(d_selectedItems_ptr != NULL)
		cudaFree(d_selectedItems_ptr);
	if(d_filterData_ptr != NULL)
		cudaFree(d_filterData_ptr);
	if(d_colorValue_ptr != NULL)
		cudaFree(d_colorValue_ptr);
	if(d_highRecord_ptr != NULL)
		cudaFree(d_highRecord_ptr);
	if(d_highIndex_ptr != NULL)
		cudaFree(d_highIndex_ptr);

	if(d_vertex_bool_ptr != NULL)
		cudaFree(d_vertex_bool_ptr);
	if(d_vertex_prefix_ptr != NULL)
		cudaFree(d_vertex_prefix_ptr);
	if(d_item_prefix_ptr != NULL)
		cudaFree(d_item_prefix_ptr);
	if(d_vertex_sort_ptr != NULL)
		cudaFree(d_vertex_sort_ptr);

	if(d_edgeDummy_ptr != NULL)
		cudaFree(d_edgeDummy_ptr);
	if(d_edgeBool_ptr != NULL)
		cudaFree(d_edgeBool_ptr);
	if(d_edgeItemPre_ptr != NULL)
		cudaFree(d_edgeItemPre_ptr);
	if(d_edgePrefix_ptr != NULL)
		cudaFree(d_edgePrefix_ptr);

	if(d_compactIndex_ptr != NULL)
		cudaFree(d_compactIndex_ptr);

	if(d_item_num_ptr != NULL)
		cudaFree(d_item_num_ptr);
	if(d_edgeItemNum_ptr != NULL)
		cudaFree(d_edgeItemNum_ptr);
	if(d_vertex_Index_ptr != NULL)
		cudaFree(d_vertex_Index_ptr);

	cuda_Init();
}


extern "C"
void cuda_PrepareDataConstant(unsigned int *_item_desc_offset,	unsigned int *_item_desc_bytes,
							  unsigned int *_item_desc_values,	unsigned char *_buffer_data)
{
	d_item_offset_ptr	=	_item_desc_offset;	
	d_item_bytes_ptr	=	_item_desc_bytes;	
	d_item_values_ptr	=	_item_desc_values;
	d_buffer_ptr		=	_buffer_data;
}

extern "C"
void cuda_PrepareDataConstant2(unsigned int *_item_desc_offset,	unsigned int *_item_desc_bytes,
							   unsigned int *_item_desc_values,	unsigned char *_buffer_data,
							   unsigned int _itemNum,			unsigned int  _bufferSize)
{
#ifdef DEBUG_MODE
	RedirectIOToConsole();
#endif
	unsigned int data_size;
	unsigned int mem_size;
	data_size = _itemNum;
	mem_size = sizeof(unsigned int)*data_size;
	checkCudaErrors(cudaMalloc(&d_item_offset_ptr,mem_size));
	checkCudaErrors(cudaMalloc(&d_item_bytes_ptr,mem_size));
	checkCudaErrors(cudaMalloc(&d_item_values_ptr,mem_size));
	checkCudaErrors(cudaMemcpy( d_item_offset_ptr, _item_desc_offset, mem_size,cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy( d_item_bytes_ptr, _item_desc_bytes, mem_size,cudaMemcpyHostToDevice));
	checkCudaErrors(cudaMemcpy( d_item_values_ptr, _item_desc_values, mem_size,cudaMemcpyHostToDevice));

	data_size = _bufferSize;
	mem_size = sizeof(unsigned char)*data_size;
	checkCudaErrors(cudaMalloc(&d_buffer_ptr, mem_size));
	checkCudaErrors(cudaMemcpy(d_buffer_ptr,_buffer_data,mem_size,cudaMemcpyHostToDevice));
	cudaDeviceSynchronize();

	getLastCudaError("memlloc the gpu memory 1");
}
 

extern "C"
void cuda_PrepareData(  unsigned int* _filterData,			unsigned int _filterSize,
						unsigned int *_selected_items,		unsigned int _selectedItemsNum,
						unsigned int _max_dimension,		unsigned int _total_bytes)
{


	unsigned int mem_size = 0;
	dn_maxDimension = _max_dimension;
	dn_totalBytes = _total_bytes;

	//selected items
	dn_selectItemsNum = _selectedItemsNum;
	mem_size = sizeof(unsigned int)*dn_selectItemsNum;
	if(d_selectedItems_ptr != NULL)
		cudaFree(d_selectedItems_ptr);
	checkCudaErrors(cudaMalloc(&d_selectedItems_ptr,mem_size));
	checkCudaErrors(cudaMemcpy( d_selectedItems_ptr, _selected_items, mem_size,cudaMemcpyHostToDevice));
	//filterdata
	dn_filterDataNum = _filterSize;
	mem_size = sizeof(unsigned int)*dn_filterDataNum;
	if(d_filterData_ptr != NULL)
		cudaFree(d_filterData_ptr);
	checkCudaErrors(cudaMalloc(&d_filterData_ptr, mem_size));
	checkCudaErrors(cudaMemcpy( d_filterData_ptr, _filterData, mem_size,cudaMemcpyHostToDevice));
}

extern "C"
void cuda_CleanData()
{
	if(d_selectedItems_ptr != NULL)
		cudaFree(d_selectedItems_ptr);
	if(d_filterData_ptr != NULL)
		cudaFree(d_filterData_ptr);
	d_selectedItems_ptr =NULL;
	d_filterData_ptr = NULL;
}

extern "C" 
void cuda_GetCompactList(unsigned int* h_compactList)
{
	//copy to the CPU
	checkCudaErrors(cudaMemcpy(h_compactList,d_compactValue_ptr,sizeof(unsigned int)*dn_vertexNum,cudaMemcpyDeviceToHost));
}


extern "C"
void cuda_GetCompactListSize(unsigned int &_compactNum)
{
	if(d_dummyValue_ptr != NULL)
		cudaFree(d_dummyValue_ptr);
	if(d_dummyIndex_ptr != NULL)
		cudaFree(d_dummyIndex_ptr);
	if(d_dummyBool_ptr != NULL)
		cudaFree(d_dummyBool_ptr);
	if(d_dummySum_ptr != NULL)
		cudaFree(d_dummySum_ptr);
	cudaMalloc(&d_dummyValue_ptr,		sizeof(unsigned int)*dn_filterDataNum);
	cudaMalloc(&d_dummyIndex_ptr,		sizeof(unsigned int)*dn_filterDataNum);
	cudaMalloc(&d_dummyBool_ptr,		sizeof(unsigned char)*dn_filterDataNum);
	cudaMalloc(&d_dummySum_ptr,			sizeof(unsigned int)*dn_filterDataNum);
	thrust::device_ptr<uint>			d_dummyValue(d_dummyValue_ptr);
	thrust::device_ptr<uint>			d_dummyIndex(d_dummyIndex_ptr);
	thrust::device_ptr<uint>			d_dummySum(d_dummySum_ptr);
	thrust::device_ptr<uchar>			d_dummyBool(d_dummyBool_ptr);

	 
	cudaMemcpy(d_dummyIndex_ptr,	d_filterData_ptr,	sizeof(unsigned int)*dn_filterDataNum, cudaMemcpyDeviceToDevice);
	//step 1 get the dummy value
	unsigned int block_num  = (dn_filterDataNum + BLOCK_SIZE -1)/BLOCK_SIZE;
	gpu_dummyValues<<<block_num, BLOCK_SIZE>>>(d_buffer_ptr,		d_item_offset_ptr,
											   d_item_bytes_ptr,	d_selectedItems_ptr,
											   d_filterData_ptr,	d_dummyValue_ptr,
											   dn_totalBytes,		dn_selectItemsNum,
											   dn_filterDataNum,	dn_maxDimension);										

 
	//step 2 sort the dummy value;
	thrust::sort_by_key(d_dummyValue, d_dummyValue+dn_filterDataNum, d_dummyIndex);
	

	 //step 3 minus the dummy value
	block_num = (dn_filterDataNum +  BLOCK_SIZE -1)/BLOCK_SIZE;
	gpu_minusDummyValue<<< block_num, BLOCK_SIZE >>>(d_dummyValue_ptr, d_dummyBool_ptr, dn_filterDataNum);
	
	//step 4 scan the dummy value
	thrust::exclusive_scan(d_dummyBool, d_dummyBool+dn_filterDataNum,d_dummySum);
	
	//step 5 compact the dummy value
	dn_compactItemNum = d_dummySum[dn_filterDataNum-1];
	if (d_dummyBool[dn_filterDataNum-1] == 1)
		dn_compactItemNum = dn_compactItemNum+1;

#ifdef DEBUG_MODE
	fprintf(stdout, "compactItemNum, %d\n",dn_compactItemNum);
	fprintf(stdout, "hello world\n");
#endif

	_compactNum = dn_compactItemNum;
	if(d_compactIndex_ptr != NULL)
		cudaFree(d_compactIndex_ptr);
	if(d_compactValue_ptr)
		cudaFree(d_compactValue_ptr);
	cudaMalloc(&d_compactIndex_ptr,		sizeof(unsigned int)*dn_compactItemNum);
	cudaMalloc(&d_compactValue_ptr,		sizeof(unsigned int)*dn_compactItemNum*dn_selectItemsNum);
	thrust::device_ptr<unsigned int>	d_compactIndex(d_compactIndex_ptr);
	thrust::device_ptr<unsigned int>	d_compactValue(d_compactValue_ptr);

	block_num  = (dn_filterDataNum + BLOCK_SIZE -1)/BLOCK_SIZE;
	gpu_compactValue<<< block_num, BLOCK_SIZE >>>(d_buffer_ptr,			d_item_offset_ptr,
												  d_item_bytes_ptr,		d_dummyIndex_ptr,
												  d_selectedItems_ptr,	d_dummyBool_ptr,
												  d_dummySum_ptr,		d_compactValue_ptr,
												  d_compactIndex_ptr,	dn_totalBytes,
												  dn_filterDataNum,		dn_selectItemsNum,
												  dn_compactItemNum);
	

#ifdef DEBUG_MODE
	thrust::host_vector<uint> H3(dn_compactItemNum*dn_selectItemsNum,0);
	thrust::host_vector<uint> H4(dn_compactItemNum,0);
	thrust::copy(d_compactValue,d_compactValue+dn_compactItemNum*dn_selectItemsNum,H3.begin());
	thrust::copy(d_compactIndex,d_compactIndex+dn_compactItemNum,H4.begin());
	fprintf(stdout, "\n Compact values:\n");
	for(uint i=0; i<dn_compactItemNum*dn_selectItemsNum; i++)
		fprintf(stdout, "%d	",H3[i]);

	fprintf(stdout, "\n Compact Index:\n");
	for(uint i=0; i<dn_compactItemNum; i++)
		fprintf(stdout, "%d	",H4[i]);

	fprintf(stdout, "hello world");
#endif
}

extern "C"
void cuda_GetVertexIndexSize(unsigned int &vertexBufferSize,	unsigned int &edgeBufferSize)
{
	//step 6 vertex buffer
	if(d_vertex_bool_ptr != NULL)
		cudaFree(d_vertex_bool_ptr);
	if(d_vertex_prefix_ptr != NULL)
		cudaFree(d_vertex_prefix_ptr);
	if(d_vertex_sort_ptr != NULL)
		cudaFree(d_vertex_sort_ptr);
	if(d_item_prefix_ptr != NULL)
		cudaFree(d_item_prefix_ptr);
	if(d_item_num_ptr != NULL)
		cudaFree(d_item_num_ptr);
	dn_vertexNum  = dn_selectItemsNum*dn_compactItemNum;
	cudaMalloc(&d_vertex_bool_ptr,		sizeof(unsigned char)*dn_vertexNum);
	cudaMalloc(&d_vertex_prefix_ptr,	sizeof(unsigned int)*dn_vertexNum);
	cudaMalloc(&d_vertex_sort_ptr,		sizeof(unsigned int)*dn_vertexNum);
	cudaMalloc(&d_item_prefix_ptr,		sizeof(unsigned int)*dn_selectItemsNum);
	cudaMalloc(&d_item_num_ptr,			sizeof(unsigned int)*dn_selectItemsNum);
	thrust::device_ptr<unsigned char>	d_vertex_bool(d_vertex_bool_ptr);
	thrust::device_ptr<unsigned int>	d_vertex_prefix(d_vertex_prefix_ptr);
	thrust::device_ptr<unsigned int>	d_vertex_sort(d_vertex_sort_ptr);
	thrust::device_ptr<unsigned int>	d_item_prefix(d_item_prefix_ptr);
	thrust::device_ptr<unsigned int>	d_item_num(d_item_num_ptr);
	cudaMemcpy(d_vertex_sort_ptr, d_compactValue_ptr, sizeof(unsigned int)*dn_vertexNum, cudaMemcpyDeviceToDevice);
	
	//step 7 sort each selected items;
	for(unsigned int i =0; i< dn_selectItemsNum ; i++)
		thrust::sort(d_vertex_sort+i*dn_compactItemNum,d_vertex_sort+(i+1)*dn_compactItemNum);
	
	
	//step 8 get the bool of vertex
	block_num  = (dn_vertexNum + BLOCK_SIZE -1)/BLOCK_SIZE;
	gpu_vertexBool<<< block_num, BLOCK_SIZE >>>(d_vertex_sort_ptr,		d_vertex_bool_ptr,
												dn_compactItemNum,		dn_vertexNum);

#ifdef DEBUG_MODE
	thrust::host_vector<uchar> H5(dn_vertexNum,0);
	thrust::copy(d_vertex_bool,d_vertex_bool+dn_vertexNum,H5.begin());
	fprintf(stdout, "\nhello world\n");
	for(uint i=0; i<dn_vertexNum; i++)
		fprintf(stdout,"%d	",H5[i]);
	fprintf(stdout, "vertex buffer size %d\n",vertexBufferSize);
	fprintf(stdout, "\nhello world\n");
#endif

	//step 9 exclusive scan of the vertex
	vertexBufferSize  = 0;
	d_item_prefix[0] = 0;
	for( unsigned int i =0; i<dn_selectItemsNum; i++)
	{
		thrust::exclusive_scan(d_vertex_bool+i*dn_compactItemNum,
							   d_vertex_bool+(i+1)*dn_compactItemNum,
							   d_vertex_prefix+i*dn_compactItemNum);
		d_item_num[i] = d_vertex_prefix[(i+1)*dn_compactItemNum-1];
		if(d_vertex_bool[(i+1)*dn_compactItemNum-1] == 1)
			d_item_num[i] = d_item_num[i] +1;
		vertexBufferSize += d_item_num[i];

		if (i!=0)
			d_item_prefix[i] = d_item_prefix[i-1] +  d_item_num[i-1]; 
	}

#ifdef DEBUG_MODE
	fprintf(stdout, "vertex buffer size %d\n",vertexBufferSize);
	fprintf(stdout, "hello world");
#endif
	

	//step 10 the edge info
	dn_edgeNum  = dn_compactItemNum*(dn_selectItemsNum-1);
	if(d_edgeBool_ptr != NULL)
		cudaFree(d_edgeBool_ptr);
	if(d_edgeDummy_ptr != NULL)
		cudaFree(d_edgeDummy_ptr);
	if(d_edgePrefix_ptr != NULL)
		cudaFree(d_edgePrefix_ptr);
	if(d_edgeItemPre_ptr != NULL)
		cudaFree(d_edgeItemPre_ptr);
	if(d_edgeItemNum_ptr != NULL)
		cudaFree(d_edgeItemNum_ptr);

	cudaMalloc(&d_edgeBool_ptr,			sizeof(unsigned char)*dn_edgeNum);
	cudaMalloc(&d_edgeDummy_ptr,		sizeof(unsigned int)*dn_edgeNum);
	cudaMalloc(&d_edgePrefix_ptr,		sizeof(unsigned int)*dn_edgeNum);
	cudaMalloc(&d_edgeItemPre_ptr,		sizeof(unsigned int)*(dn_selectItemsNum-1));
	cudaMalloc(&d_edgeItemNum_ptr,		sizeof(unsigned int)*(dn_selectItemsNum-1));
	thrust::device_ptr<unsigned char>	d_edgeBool(d_edgeBool_ptr);
	thrust::device_ptr<unsigned int>	d_edgeDummy(d_edgeDummy_ptr);
	thrust::device_ptr<unsigned int>	d_edgePrefix(d_edgePrefix_ptr);
	thrust::device_ptr<unsigned int>	d_edgeItemPre(d_edgeItemPre_ptr);
	thrust::device_ptr<unsigned int>	d_edgeItemNum(d_edgeItemNum_ptr);
 
	block_num  = (dn_edgeNum + BLOCK_SIZE-1)/BLOCK_SIZE;
	gpu_edgeDummyValue<<< block_num, BLOCK_SIZE >>>(d_compactValue_ptr,		d_edgeDummy_ptr,
													dn_edgeNum,				dn_compactItemNum,
													dn_maxDimension,		dn_selectItemsNum);

	//step 11
	for ( unsigned int i = 0; i< dn_selectItemsNum-1; i++)
	{
		thrust::sort(d_edgeDummy + i*dn_compactItemNum,
					d_edgeDummy + (i+1)*dn_compactItemNum);
	}
	
	//step 12
	gpu_edgeBool<<< block_num, BLOCK_SIZE >>>(d_edgeDummy_ptr,		d_edgeBool_ptr,
											  dn_edgeNum,			dn_compactItemNum);


	edgeBufferSize = 0;
	d_edgeItemPre[0] = 0;
	for (unsigned int i =0; i < dn_selectItemsNum -1; i++)
	{
		thrust::exclusive_scan(d_edgeBool +  i*dn_compactItemNum,
								d_edgeBool + (i+1)*dn_compactItemNum,
								d_edgePrefix +i*dn_compactItemNum);

		d_edgeItemNum[i] = d_edgePrefix[(i+1)*dn_compactItemNum-1];
		if (d_edgeBool[(i+1)*dn_compactItemNum-1] == 1)
			d_edgeItemNum[i]  = d_edgeItemNum[i] +1;
		edgeBufferSize += d_edgeItemNum[i];

		if( i !=0)
			d_edgeItemPre[i] = d_edgeItemPre[i-1] + d_edgeItemNum[i-1];
	}

	
#ifdef DEBUG_MODE
	fprintf(stdout, "\n vertex buffer size %d\n",edgeBufferSize);
	fprintf(stdout, "hello world");
#endif

	

	//step 13 clean the unrelated vectors
	cudaFree(d_dummyValue_ptr);
	cudaFree(d_dummyIndex_ptr);
	cudaFree(d_dummyBool_ptr);
	cudaFree(d_dummySum_ptr);
	d_dummyValue_ptr = NULL;
	d_dummyIndex_ptr = NULL;
	d_dummyBool_ptr = NULL;
	d_dummySum_ptr = NULL;
}



extern "C"
void cuda_GenerateSize(unsigned int &vertexBufferSize,	unsigned int &edgeBufferSize,
					   unsigned int &_compactNum)
{
	if(d_dummyValue_ptr != NULL)
		cudaFree(d_dummyValue_ptr);
	if(d_dummyIndex_ptr != NULL)
		cudaFree(d_dummyIndex_ptr);
	if(d_dummyBool_ptr != NULL)
		cudaFree(d_dummyBool_ptr);
	if(d_dummySum_ptr != NULL)
		cudaFree(d_dummySum_ptr);
	cudaMalloc(&d_dummyValue_ptr,		sizeof(unsigned int)*dn_filterDataNum);
	cudaMalloc(&d_dummyIndex_ptr,		sizeof(unsigned int)*dn_filterDataNum);
	cudaMalloc(&d_dummyBool_ptr,		sizeof(unsigned char)*dn_filterDataNum);
	cudaMalloc(&d_dummySum_ptr,			sizeof(unsigned int)*dn_filterDataNum);
	thrust::device_ptr<uint>			d_dummyValue(d_dummyValue_ptr);
	thrust::device_ptr<uint>			d_dummyIndex(d_dummyIndex_ptr);
	thrust::device_ptr<uint>			d_dummySum(d_dummySum_ptr);
	thrust::device_ptr<uchar>			d_dummyBool(d_dummyBool_ptr);

	 
	cudaMemcpy(d_dummyIndex_ptr,	d_filterData_ptr,	sizeof(unsigned int)*dn_filterDataNum, cudaMemcpyDeviceToDevice);
	//step 1 get the dummy value
	unsigned int block_num  = (dn_filterDataNum + BLOCK_SIZE -1)/BLOCK_SIZE;
	gpu_dummyValues<<<block_num, BLOCK_SIZE>>>(d_buffer_ptr,		d_item_offset_ptr,
											   d_item_bytes_ptr,	d_selectedItems_ptr,
											   d_filterData_ptr,	d_dummyValue_ptr,
											   dn_totalBytes,		dn_selectItemsNum,
											   dn_filterDataNum,	dn_maxDimension);										
	

#ifdef DEBUG_MODE
	fprintf(stdout, "num of data %d , max_dimension, %d\n",dn_filterDataNum,dn_maxDimension);
	thrust::host_vector<uint> H1(dn_filterDataNum,0);
	thrust::host_vector<uint> H2(dn_filterDataNum,0);
	thrust::copy(d_dummyValue,d_dummyValue+dn_filterDataNum,H1.begin());
	thrust::copy(d_dummyIndex,d_dummyIndex+dn_filterDataNum,H2.begin());
	for(uint i=0; i< dn_filterDataNum; i++)
		fprintf(stdout,"%d \n",H1[i]);

	for(uint i=0; i< dn_filterDataNum; i++)
		fprintf(stdout,"%d \n",H2[i]);
#endif

	//step 2 sort the dummy value;
	thrust::sort_by_key(d_dummyValue, d_dummyValue+dn_filterDataNum, d_dummyIndex);

	

	 //step 3 minus the dummy value
	block_num = (dn_filterDataNum +  BLOCK_SIZE -1)/BLOCK_SIZE;
	gpu_minusDummyValue<<< block_num, BLOCK_SIZE >>>(d_dummyValue_ptr, d_dummyBool_ptr, dn_filterDataNum);
	cudaDeviceSynchronize();

	//step 4 scan the dummy value
	thrust::exclusive_scan(d_dummyBool, d_dummyBool+dn_filterDataNum,d_dummySum);
	cudaDeviceSynchronize();

	//step 5 compact the dummy value
	dn_compactItemNum = d_dummySum[dn_filterDataNum-1];
	if (d_dummyBool[dn_filterDataNum-1] == 1)
		dn_compactItemNum = dn_compactItemNum+1;

#ifdef DEBUG_MODE
	fprintf(stdout, "compactItemNum, %d\n",dn_compactItemNum);
	fprintf(stdout, "hello world\n");
#endif

	_compactNum = dn_compactItemNum;
	if(d_compactIndex_ptr != NULL)
		cudaFree(d_compactIndex_ptr);
	if(d_compactValue_ptr)
		cudaFree(d_compactValue_ptr);
	cudaMalloc(&d_compactIndex_ptr,		sizeof(unsigned int)*dn_compactItemNum);
	cudaMalloc(&d_compactValue_ptr,		sizeof(unsigned int)*dn_compactItemNum*dn_selectItemsNum);
	thrust::device_ptr<unsigned int>	d_compactIndex(d_compactIndex_ptr);
	thrust::device_ptr<unsigned int>	d_compactValue(d_compactValue_ptr);

	block_num  = (dn_filterDataNum + BLOCK_SIZE -1)/BLOCK_SIZE;
	gpu_compactValue<<< block_num, BLOCK_SIZE >>>(d_buffer_ptr,			d_item_offset_ptr,
												  d_item_bytes_ptr,		d_dummyIndex_ptr,
												  d_selectedItems_ptr,	d_dummyBool_ptr,
												  d_dummySum_ptr,		d_compactValue_ptr,
												  d_compactIndex_ptr,	dn_totalBytes,
												  dn_filterDataNum,		dn_selectItemsNum,
												  dn_compactItemNum);
	cudaDeviceSynchronize();


#ifdef DEBUG_MODE
	thrust::host_vector<uint> H3(dn_compactItemNum*dn_selectItemsNum,0);
	thrust::host_vector<uint> H4(dn_compactItemNum,0);
	thrust::copy(d_compactValue,d_compactValue+dn_compactItemNum*dn_selectItemsNum,H3.begin());
	thrust::copy(d_compactIndex,d_compactIndex+dn_compactItemNum,H4.begin());
	fprintf(stdout, "\n Compact values:\n");
	for(uint i=0; i<dn_compactItemNum*dn_selectItemsNum; i++)
		fprintf(stdout, "%d	",H3[i]);

	fprintf(stdout, "\n Compact Index:\n");
	for(uint i=0; i<dn_compactItemNum; i++)
		fprintf(stdout, "%d	",H4[i]);

	fprintf(stdout, "hello world");
#endif


	//step 6 vertex buffer
	if(d_vertex_bool_ptr != NULL)
		cudaFree(d_vertex_bool_ptr);
	if(d_vertex_prefix_ptr != NULL)
		cudaFree(d_vertex_prefix_ptr);
	if(d_vertex_sort_ptr != NULL)
		cudaFree(d_vertex_sort_ptr);
	if(d_item_prefix_ptr != NULL)
		cudaFree(d_item_prefix_ptr);
	if(d_item_num_ptr != NULL)
		cudaFree(d_item_num_ptr);
	dn_vertexNum  = dn_selectItemsNum*dn_compactItemNum;
	cudaMalloc(&d_vertex_bool_ptr,		sizeof(unsigned char)*dn_vertexNum);
	cudaMalloc(&d_vertex_prefix_ptr,	sizeof(unsigned int)*dn_vertexNum);
	cudaMalloc(&d_vertex_sort_ptr,		sizeof(unsigned int)*dn_vertexNum);
	cudaMalloc(&d_item_prefix_ptr,		sizeof(unsigned int)*dn_selectItemsNum);
	cudaMalloc(&d_item_num_ptr,			sizeof(unsigned int)*dn_selectItemsNum);
	thrust::device_ptr<unsigned char>	d_vertex_bool(d_vertex_bool_ptr);
	thrust::device_ptr<unsigned int>	d_vertex_prefix(d_vertex_prefix_ptr);
	thrust::device_ptr<unsigned int>	d_vertex_sort(d_vertex_sort_ptr);
	thrust::device_ptr<unsigned int>	d_item_prefix(d_item_prefix_ptr);
	thrust::device_ptr<unsigned int>	d_item_num(d_item_num_ptr);
	cudaMemcpy(d_vertex_sort_ptr, d_compactValue_ptr, sizeof(unsigned int)*dn_vertexNum, cudaMemcpyDeviceToDevice);
	cudaDeviceSynchronize();

	//step 7 sort each selected items;
	for(unsigned int i =0; i< dn_selectItemsNum ; i++)
	{
		thrust::sort(d_vertex_sort+i*dn_compactItemNum,d_vertex_sort+(i+1)*dn_compactItemNum);
		cudaDeviceSynchronize();
	}
	
	//step 8 get the bool of vertex
	block_num  = (dn_vertexNum + BLOCK_SIZE -1)/BLOCK_SIZE;
	gpu_vertexBool<<< block_num, BLOCK_SIZE >>>(d_vertex_sort_ptr,		d_vertex_bool_ptr,
												dn_compactItemNum,		dn_vertexNum);
	cudaDeviceSynchronize();

#ifdef DEBUG_MODE
	thrust::host_vector<uchar> H5(dn_vertexNum,0);
	thrust::copy(d_vertex_bool,d_vertex_bool+dn_vertexNum,H5.begin());
	fprintf(stdout, "\nhello world\n");
	for(uint i=0; i<dn_vertexNum; i++)
		fprintf(stdout,"%d	",H5[i]);
	fprintf(stdout, "vertex buffer size %d\n",vertexBufferSize);
	fprintf(stdout, "\nhello world\n");
#endif

	//step 9 exclusive scan of the vertex
	vertexBufferSize  = 0;
	d_item_prefix[0] = 0;
	for( unsigned int i =0; i<dn_selectItemsNum; i++)
	{
		thrust::exclusive_scan(d_vertex_bool+i*dn_compactItemNum,
							   d_vertex_bool+(i+1)*dn_compactItemNum,
							   d_vertex_prefix+i*dn_compactItemNum);
		d_item_num[i] = d_vertex_prefix[(i+1)*dn_compactItemNum-1];
		if(d_vertex_bool[(i+1)*dn_compactItemNum-1] == 1)
			d_item_num[i] = d_item_num[i] +1;
		vertexBufferSize += d_item_num[i];

		if (i!=0)
			d_item_prefix[i] = d_item_prefix[i-1] +  d_item_num[i-1]; 
		cudaDeviceSynchronize();
	}

#ifdef DEBUG_MODE
	fprintf(stdout, "vertex buffer size %d\n",vertexBufferSize);
	fprintf(stdout, "hello world");
#endif
	

	//step 10 the edge info
	dn_edgeNum  = dn_compactItemNum*(dn_selectItemsNum-1);
	if(d_edgeBool_ptr != NULL)
		cudaFree(d_edgeBool_ptr);
	if(d_edgeDummy_ptr != NULL)
		cudaFree(d_edgeDummy_ptr);
	if(d_edgePrefix_ptr != NULL)
		cudaFree(d_edgePrefix_ptr);
	if(d_edgeItemPre_ptr != NULL)
		cudaFree(d_edgeItemPre_ptr);
	if(d_edgeItemNum_ptr != NULL)
		cudaFree(d_edgeItemNum_ptr);

	cudaMalloc(&d_edgeBool_ptr,			sizeof(unsigned char)*dn_edgeNum);
	cudaMalloc(&d_edgeDummy_ptr,		sizeof(unsigned int)*dn_edgeNum);
	cudaMalloc(&d_edgePrefix_ptr,		sizeof(unsigned int)*dn_edgeNum);
	cudaMalloc(&d_edgeItemPre_ptr,		sizeof(unsigned int)*(dn_selectItemsNum-1));
	cudaMalloc(&d_edgeItemNum_ptr,		sizeof(unsigned int)*(dn_selectItemsNum-1));
	thrust::device_ptr<unsigned char>	d_edgeBool(d_edgeBool_ptr);
	thrust::device_ptr<unsigned int>	d_edgeDummy(d_edgeDummy_ptr);
	thrust::device_ptr<unsigned int>	d_edgePrefix(d_edgePrefix_ptr);
	thrust::device_ptr<unsigned int>	d_edgeItemPre(d_edgeItemPre_ptr);
	thrust::device_ptr<unsigned int>	d_edgeItemNum(d_edgeItemNum_ptr);
 
	block_num  = (dn_edgeNum + BLOCK_SIZE-1)/BLOCK_SIZE;
	gpu_edgeDummyValue<<< block_num, BLOCK_SIZE >>>(d_compactValue_ptr,		d_edgeDummy_ptr,
													dn_edgeNum,				dn_compactItemNum,
													dn_maxDimension,		dn_selectItemsNum);
	cudaDeviceSynchronize();

	//step 11
	for ( unsigned int i = 0; i< dn_selectItemsNum-1; i++)
	{
		thrust::sort(d_edgeDummy + i*dn_compactItemNum,
					d_edgeDummy + (i+1)*dn_compactItemNum);
		cudaDeviceSynchronize();
	}
	
	//step 12
	gpu_edgeBool<<< block_num, BLOCK_SIZE >>>(d_edgeDummy_ptr,		d_edgeBool_ptr,
											  dn_edgeNum,			dn_compactItemNum);
	cudaDeviceSynchronize();


	edgeBufferSize = 0;
	d_edgeItemPre[0] = 0;
	for (unsigned int i =0; i < dn_selectItemsNum -1; i++)
	{
		thrust::exclusive_scan(d_edgeBool +  i*dn_compactItemNum,
								d_edgeBool + (i+1)*dn_compactItemNum,
								d_edgePrefix +i*dn_compactItemNum);

		d_edgeItemNum[i] = d_edgePrefix[(i+1)*dn_compactItemNum-1];
		if (d_edgeBool[(i+1)*dn_compactItemNum-1] == 1)
			d_edgeItemNum[i]  = d_edgeItemNum[i] +1;
		edgeBufferSize += d_edgeItemNum[i];

		if( i !=0)
			d_edgeItemPre[i] = d_edgeItemPre[i-1] + d_edgeItemNum[i-1];
		cudaDeviceSynchronize();
	}

	
#ifdef DEBUG_MODE
	fprintf(stdout, "\n vertex buffer size %d\n",edgeBufferSize);
	fprintf(stdout, "hello world");
#endif

	

	//step 13 clean the unrelated vectors
	cudaFree(d_dummyValue_ptr);
	cudaFree(d_dummyIndex_ptr);
	cudaFree(d_dummyBool_ptr);
	cudaFree(d_dummySum_ptr);
	d_dummyValue_ptr = NULL;
	d_dummyIndex_ptr = NULL;
	d_dummyBool_ptr = NULL;
	d_dummySum_ptr = NULL;

}

 

extern "C"
void cuda_GenerateVertexBuffers(float* d_vertexBuffer_ptr,			unsigned int *d_edgeBuffer_ptr,
								//unsigned char* d_colorBuffer_ptr,	
								unsigned int vertexBufferSize, 
								unsigned int edgeBufferSize)
{

	//set the default color buffer
	//checkCudaErrors(cudaMemset(d_colorBuffer_ptr,128,sizeof(unsigned char)*vertexBufferSize*4));
	

	if(d_vertex_Index_ptr != NULL)
		cudaFree(d_vertex_Index_ptr);
	cudaMalloc(&d_vertex_Index_ptr,		sizeof(unsigned int)*dn_vertexNum);
	thrust::device_ptr<unsigned int>	d_vertex_Index(d_vertex_Index_ptr);

	unsigned int block_num  = (dn_vertexNum + BLOCK_SIZE -1)/BLOCK_SIZE;
	gpu_generateVertexBuffer<<< block_num, BLOCK_SIZE >>>(d_vertex_bool_ptr,		d_vertex_prefix_ptr,
														  d_vertex_Index_ptr,		d_vertex_sort_ptr,
														  d_item_prefix_ptr,		d_item_values_ptr,
														  d_selectedItems_ptr,		d_vertexBuffer_ptr,
														  dn_vertexNum,				dn_compactItemNum);
	cudaDeviceSynchronize();

#ifdef DEBUG_MODE
	fprintf(stdout, "\n \n Vertex Index \n\n");
	thrust::host_vector<uchar> H1(dn_vertexNum,0);
	thrust::copy(d_vertex_Index,d_vertex_Index+dn_vertexNum,H1.begin());
	fprintf(stdout, "\nhello world\n");
	for(uint i=0; i<dn_vertexNum; i++)
		fprintf(stdout,"%d	",H1[i]);
	fprintf(stdout, "\n vertex index %d\n",dn_vertexNum);
	fprintf(stdout, "hello world");
#endif

	//generate EdgeBuffer
	block_num  = (dn_edgeNum + BLOCK_SIZE-1)/BLOCK_SIZE;
	gpu_generateEdgeBuffer<<< block_num, BLOCK_SIZE >>>(d_vertex_Index_ptr,		d_edgeBuffer_ptr,
														d_edgeDummy_ptr,		d_edgeBool_ptr,
														d_edgeItemPre_ptr,		d_edgePrefix_ptr,
														d_item_prefix_ptr,		dn_vertexNum,
														vertexBufferSize,		dn_compactItemNum,
														dn_edgeNum,				dn_selectItemsNum,
														dn_maxDimension);
	cudaDeviceSynchronize();

#ifdef DEBUG_MODE
	fprintf(stdout, "\nhello world\n");
	thrust::device_ptr<unsigned int> d_edgeBuffer(d_edgeBuffer_ptr);
	thrust::host_vector<uchar> H2(dn_vertexNum*2,0);
	thrust::copy(d_edgeBuffer,d_edgeBuffer+dn_edgeNum*2,H2.begin());
	fprintf(stdout, "\nhello world\n");
	for(uint i=0; i<dn_edgeNum*2; i++)
		fprintf(stdout,"%d	",H2[i]);
	fprintf(stdout, "\n vertex index %d\n",dn_edgeNum);
	fprintf(stdout, "hello world");


	fprintf(stdout, "\n \n Vertex Index \n\n");
	thrust::copy(d_vertex_Index,d_vertex_Index+dn_vertexNum,H1.begin());
	fprintf(stdout, "\nhello world\n");
	for(uint i=0; i<dn_vertexNum; i++)
		fprintf(stdout,"%d	",H1[i]);
	fprintf(stdout, "\n vertex index %d\n",dn_vertexNum);
	fprintf(stdout, "hello world");

#endif
}

extern "C"
void cuda_PrepareColorData(unsigned char* _colorValue,	unsigned int* _highRecord,
							   unsigned char* _highIndex,	
							   unsigned int _numColor,		unsigned int  _numRecord)
{
	dn_ColorNum =  _numColor;
	dn_RecordNum = _numRecord;

	unsigned int mem_size = 0;
	mem_size = sizeof(unsigned char)*dn_ColorNum*4;
	if(d_colorValue_ptr != NULL)
		cudaFree(d_colorValue_ptr);
	checkCudaErrors(cudaMalloc(&d_colorValue_ptr,mem_size));
	checkCudaErrors(cudaMemcpy( d_colorValue_ptr, _colorValue, mem_size,cudaMemcpyHostToDevice));
	cudaDeviceSynchronize();

	mem_size = sizeof(unsigned int)*dn_RecordNum;
	if(d_highRecord_ptr != NULL)
		cudaFree(d_highRecord_ptr);
	cudaMalloc(&d_highRecord_ptr,mem_size);
	cudaMemcpy( d_highRecord_ptr, _highRecord, mem_size,cudaMemcpyHostToDevice);
	cutilDeviceSynchronize();

	mem_size = sizeof(unsigned char)*dn_RecordNum;
	if(d_highIndex_ptr != NULL)
		cudaFree(d_highIndex_ptr);
	cudaMalloc(&d_highIndex_ptr,mem_size);
	cudaMemcpy( d_highIndex_ptr, _highIndex, mem_size,cudaMemcpyHostToDevice);
	cutilDeviceSynchronize();

#ifdef DEBUG_MODE
	fprintf(stdout, "\nhello world\n");
	fprintf(stdout,"number of color %d, number of records: %d\n",dn_ColorNum,dn_RecordNum);
	thrust::host_vector<uchar> H1(dn_RecordNum,0);
	thrust::device_ptr<unsigned int> d_highRecord(d_highRecord_ptr);
	thrust::copy(d_highRecord,d_highRecord+dn_RecordNum,H1.begin());
	for(uint i=0; i<dn_RecordNum; i++)
		fprintf(stdout,"%d	",H1[i]);
#endif

}

extern "C"
void cuda_CleanColorData()
{
	if(d_colorValue_ptr != NULL)
		cudaFree(d_colorValue_ptr);
	if(d_highRecord_ptr != NULL)
		cudaFree(d_highRecord_ptr);
	if(d_highIndex_ptr != NULL)
		cudaFree(d_highIndex_ptr);
	d_colorValue_ptr = NULL;
	d_highRecord_ptr = NULL;
	d_highIndex_ptr = NULL;
}

extern "C"
void cuda_SetDefaultColorBuffer(unsigned char* d_colorBuffer_ptr, unsigned int _colorBufferSize)
{
	unsigned int mem_size = _colorBufferSize*4*sizeof(unsigned char);
	checkCudaErrors((cudaMemset(d_colorBuffer_ptr,179,mem_size)));
}


extern "C"
void cuda_GenerateColorBuffers(unsigned char* d_colorBuffer_ptr, unsigned int _colorBufferSize)
{
	uint block_num = (dn_RecordNum + BLOCK_SIZE-1)/BLOCK_SIZE;

	if(d_dummyValue_ptr != NULL)
		cudaFree(d_dummyValue_ptr);
	checkCudaErrors(cudaMalloc(&d_dummyValue_ptr,		sizeof(unsigned int)*dn_RecordNum));
	cudaMemset(d_dummyValue_ptr,0,sizeof(unsigned int)*dn_RecordNum);

	//step 1 get the dummy value
	gpu_dummyValues<<<block_num, BLOCK_SIZE>>>( d_buffer_ptr,			d_item_offset_ptr,
											   	d_item_bytes_ptr,		d_selectedItems_ptr,
												d_highRecord_ptr,		d_dummyValue_ptr,
												dn_totalBytes,			dn_selectItemsNum,
												dn_RecordNum,			dn_maxDimension);										
	cudaDeviceSynchronize();

#ifdef DEBUG_MODE
	fprintf(stdout, "\Dummy value\n");
	fprintf(stdout,"number of color %d, number of records: %d, maxdimension:%d selectedItems %d, totalBytes:%d\n",
		dn_ColorNum,dn_RecordNum,dn_maxDimension,dn_selectItemsNum,dn_totalBytes);
	thrust::host_vector<uint> H1(dn_RecordNum,0);
	thrust::device_ptr<unsigned int> d_dummyValue11(d_dummyValue_ptr);
	thrust::copy(d_dummyValue11,d_dummyValue11+dn_RecordNum,H1.begin());
	for(uint i=0; i<dn_RecordNum; i++)
		fprintf(stdout,"%d	",H1[i]);
#endif

	//step 2 sort the dummy value
	thrust::device_ptr<unsigned int> d_dummyValue(d_dummyValue_ptr);
	thrust::device_ptr<unsigned char> d_highIndex(d_highIndex_ptr);
	thrust::sort_by_key(d_dummyValue,d_dummyValue+dn_RecordNum,d_highIndex);
	cutilDeviceSynchronize();

#ifdef DEBUG_MODE
	fprintf(stdout, "\Sort Dummy value\n");
	fprintf(stdout,"number of color %d, number of records: %d\n",dn_ColorNum,dn_RecordNum);
	thrust::host_vector<uint> H2(dn_RecordNum,0);
	thrust::device_ptr<unsigned int> d_dummyValue22(d_dummyValue_ptr);
	thrust::copy(d_dummyValue22,d_dummyValue22+dn_RecordNum,H2.begin());
	for(uint i=0; i<dn_RecordNum; i++)
		fprintf(stdout,"%d	",H2[i]);
#endif

	//step 3 mark the compact item list
	 
	block_num = (dn_compactItemNum + BLOCK_SIZE-1)/BLOCK_SIZE;
	if(d_colorBool_ptr != NULL)
		cudaFree(d_colorBool_ptr);
	checkCudaErrors(cudaMalloc(&d_colorBool_ptr,		sizeof(unsigned char)*dn_compactItemNum*dn_selectItemsNum));
	
	gpuColorFilter<<<block_num, BLOCK_SIZE>>>(d_compactValue_ptr,		d_dummyValue_ptr,
											  d_highIndex_ptr,			d_colorBool_ptr,
											  dn_compactItemNum,		dn_RecordNum,
											  dn_selectItemsNum,		dn_maxDimension);
	cudaDeviceSynchronize();

#ifdef DEBUG_MODE
	fprintf(stdout, "\n Color Filter\n");
	fprintf(stdout,"number of compact values %d\n",dn_compactItemNum);
 	thrust::host_vector<uchar> H3(dn_compactItemNum,0);

 	 

	thrust::host_vector<uint> H7(dn_compactItemNum*dn_selectItemsNum,0);
	thrust::device_ptr<unsigned int> d_compactValuesd(d_compactValue_ptr);
	thrust::copy(d_compactValuesd,d_compactValuesd+dn_compactItemNum*dn_selectItemsNum,H7.begin());
	for(uint j=0; j<dn_selectItemsNum; j++)
	{
		fprintf(stdout,"\n");
		for(uint i=0; i<dn_compactItemNum; i++)
			fprintf(stdout,"%d	",H7[i+j*dn_selectItemsNum]);
	}

 

	thrust::device_ptr<unsigned char> d_colorBool(d_colorBool_ptr);
	thrust::copy(d_colorBool,d_colorBool+dn_compactItemNum,H3.begin());
	for(uint i=0; i<dn_compactItemNum; i++)
		fprintf(stdout,"%d	",H3[i]);
#endif


 
	unsigned int mem_size = _colorBufferSize*4*sizeof(unsigned char);
	checkCudaErrors((cudaMemset(d_colorBuffer_ptr,179,mem_size)));
	 
	//step 4 fill the color Buffer
	block_num = (dn_compactItemNum + BLOCK_SIZE-1)/BLOCK_SIZE;
	gpuColorBuffer<<<block_num, BLOCK_SIZE>>>(d_vertex_Index_ptr,		d_highIndex_ptr,
											  d_item_prefix_ptr,		d_compactValue_ptr,
											  d_colorValue_ptr,			d_colorBool_ptr,
											  d_colorBuffer_ptr,		dn_vertexNum,
											  dn_compactItemNum,		dn_selectItemsNum,
											  _colorBufferSize);
	cudaDeviceSynchronize();
	//checkCudaErrors((cudaMemset(d_colorBuffer_ptr,128,mem_size)));

#ifdef DEBUG_MODE
	fprintf(stdout, "\Color buffer\n");
	fprintf(stdout,"color buffer size %d\n",_colorBufferSize);



	fprintf(stdout,"\n vertex size %d\n",dn_vertexNum);
	thrust::host_vector<uint> H12(dn_vertexNum,0);
	thrust::device_ptr<unsigned int> d_vert(d_vertex_Index_ptr);
	thrust::copy(d_vert,d_vert+dn_vertexNum,H12.begin());
	for(uint i=0; i<dn_vertexNum; i++)
		fprintf(stdout,"%d	",H12[i]);

	 

#endif 
	 

	//step 5 free space
	cudaFree(d_dummyValue_ptr);
	//cudaFree(d_dummyIndex_ptr);
	cudaFree(d_colorBool_ptr);
	d_dummyValue_ptr = NULL;
	d_colorBool_ptr = NULL;

}

#endif