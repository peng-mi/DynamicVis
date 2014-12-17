#ifndef _PARACOORD_KERNEL_H_
#define _PARACOORD_KERNEL_H_



__device__ unsigned int gpu_getDataValue(unsigned char *p_buffer,		unsigned int *p_item_offset,
										 unsigned int *p_item_bytes,	unsigned int n_total_bytes,
										 unsigned int n_recordId,		unsigned int n_itemId)
{
	unsigned int byteoffset = p_item_offset[n_itemId];
	unsigned int numbytes   = p_item_bytes[n_itemId];
	unsigned int values_res ;
	
	if( numbytes == 1)
	{
		unsigned char item = p_buffer[n_recordId*n_total_bytes + byteoffset];
		values_res = (unsigned int)(item);
	}
	else if( numbytes == 2)
	{
		unsigned char item_1 = p_buffer[n_recordId*n_total_bytes + byteoffset+1];
		unsigned char item_2 = p_buffer[n_recordId*n_total_bytes + byteoffset];
		unsigned short res  = item_1*256 + item_2;
		values_res = (unsigned int)(res);

	}
	else
	{
		unsigned char item_1 = p_buffer[n_recordId*n_total_bytes + byteoffset+3];
		unsigned char item_2 = p_buffer[n_recordId*n_total_bytes + byteoffset+2];
		unsigned char item_3 = p_buffer[n_recordId*n_total_bytes + byteoffset+1];
		unsigned char item_4 = p_buffer[n_recordId*n_total_bytes + byteoffset];
		unsigned int  res  = ((item_1*256 +item_2)*256+item_3)*256+item_4;
		values_res = (unsigned int)(res);
	}
	return values_res;
}

 

__global__ void gpu_dummyValues(unsigned char *p_buffer,		unsigned int *p_item_offset,
								unsigned int *p_item_bytes,		unsigned int *p_selectedItems,
								unsigned int* p_filteredData,	unsigned int *p_dummyValue,
								unsigned int n_totalBytes,		unsigned int n_selectedItemsNum,
								unsigned int n_filterSize,		unsigned int n_dimension)
{
	unsigned int data_id  = blockIdx.x*blockDim.x + threadIdx.x;
	if(data_id >= n_filterSize)
		return;

	else
	{	
		unsigned int value;
		unsigned int dummyValue = 0;
		unsigned int _index = p_filteredData[data_id];
		for (unsigned int i = 0; i < n_selectedItemsNum; i++)
		{
			value = gpu_getDataValue(p_buffer,		p_item_offset,
									 p_item_bytes,	n_totalBytes,
									 _index,		p_selectedItems[i]);
			dummyValue  = dummyValue*n_dimension + value;	 
		}
		p_dummyValue[data_id] = dummyValue;
	}
}

__global__ void gpu_minusDummyValue(unsigned int *p_dummyValue, unsigned char *p_dummyBool,
									unsigned int n_numDummy)
{
	unsigned int data_id  = blockIdx.x*blockDim.x + threadIdx.x;
	if(data_id >= n_numDummy)
		return;

	if(data_id == 0)
		p_dummyBool[data_id] = 1;
	else
	{
		if (p_dummyValue[data_id] == p_dummyValue[data_id -1])
			p_dummyBool[data_id] = 0;
		else
			p_dummyBool[data_id] = 1;
	}
}


__global__ void gpu_compactValue(unsigned char *p_buffer,		unsigned int *p_item_offset,
								 unsigned int *p_item_bytes,	unsigned int *p_dummyIndex,
								 unsigned int *p_selectedItems,	unsigned char *p_dummyBool,	
								 unsigned int *p_dummySum,		unsigned int *p_compactValue,
								 unsigned int *p_compactIndex,	unsigned int n_totalBytes,
								 unsigned int n_dummySize,		unsigned int n_selectedItemsNum,
								 unsigned int n_compactItemNum)
{
	unsigned int data_id  = blockIdx.x*blockDim.x + threadIdx.x;
	if( data_id >= n_dummySize)
		return;

	if(p_dummyBool[data_id] == 1)
	{
		unsigned int index  = p_dummySum[data_id];
		unsigned int recordId = p_dummyIndex[data_id];;
		unsigned int itemId;
		p_compactIndex[index] = recordId;

		for(unsigned int i = 0; i<n_selectedItemsNum; i++)
		{
			itemId = p_selectedItems[i];
			p_compactValue[i*n_compactItemNum + index] = gpu_getDataValue(p_buffer,		p_item_offset,
																		p_item_bytes,	n_totalBytes,
																		recordId,		itemId);
		}
	}
}

__global__ void gpu_vertexBool(	unsigned int *p_vertex_value,	unsigned char *p_vertex_bool,
								unsigned int n_compactItemNum,	unsigned int n_vertexNum)
{
	unsigned int data_id = blockIdx.x*blockDim.x + threadIdx.x;
	if( data_id >= n_vertexNum )
		return;

	if( data_id%n_compactItemNum == 0)
		p_vertex_bool[data_id] = 1;
	else
	{
		if ( p_vertex_value[data_id] == p_vertex_value[data_id -1] )
			p_vertex_bool[data_id] = 0;
		else
			p_vertex_bool[data_id] = 1;
	}
}

__global__ void gpu_generateVertexBuffer(unsigned char *p_vertex_bool,		unsigned int *p_vertex_prefix,
										 unsigned int *p_vertex_Index,		unsigned int *p_vertex_value,
										 unsigned int *p_item_prefix,		unsigned int *p_item_values,		
										 unsigned int *p_selected_items,	float *p_vertexBuffer,
										 unsigned int n_vertexNum,			unsigned int n_compactItemNum,
										 unsigned int n_depth)
{
	unsigned int data_id = blockIdx.x*blockDim.x +  threadIdx.x;
	if ( data_id >= n_vertexNum)
		return;
	if(p_vertex_bool[data_id] == 0)
		return;
	else
	{
		unsigned int rows = data_id/n_compactItemNum;
		unsigned int index = p_vertex_prefix[data_id] +  p_item_prefix[rows];
		if(index >= n_vertexNum)
			return;

		unsigned int itemId  = p_selected_items [rows]; 
		float stepX = (0.8f*n_compactItemNum)/(n_vertexNum - n_compactItemNum);
		float stepY	= 0.9f/(p_item_values[itemId] -1);

		float valueX = 0.1f + stepX*rows;
		float valueY = 0.05f + stepY*p_vertex_value[data_id];
		
		//if(index %2 ==0 )
		//{
		//	p_vertexBuffer[index*3]		= 0.1f;//valueX;
		//	p_vertexBuffer[index*3+1]	= 0.5f;//valueY;
		//	p_vertexBuffer[index*3+2]	= 0.1f;//0.0f;//n_depth*0.01f;
		//}
		//else
		//{
		//	p_vertexBuffer[index*3]		= 0.9f;//valueX;
		//	p_vertexBuffer[index*3+1]	= 0.5f;//valueY;
		//	p_vertexBuffer[index*3+2]	= 0.2f;//0.0f;//n_depth*0.01f;

		//}

		p_vertexBuffer[index*3]		= valueX;
		p_vertexBuffer[index*3+1]	= valueY;
		p_vertexBuffer[index*3+2]	= n_depth*0.01f;

	
		p_vertex_Index[index] = p_vertex_value[data_id];
	}
}

__global__ void gpu_edgeDummyValue(unsigned int *p_vertex_value,		unsigned int *p_edgeDummy,
								   unsigned int n_edgeNum,				unsigned int n_compactItemNum, 
								   unsigned int n_max_dimension,		unsigned int n_selectedItemNum)
{
	unsigned int data_id  = blockIdx.x*blockDim.x  +  threadIdx.x;
	if( data_id >= n_edgeNum)
		return;

	unsigned int column = data_id%n_compactItemNum;
	unsigned int row = data_id/n_compactItemNum;
	unsigned int index  = row*n_compactItemNum + column;
	unsigned int valueX = p_vertex_value[index];
	unsigned int valueY = p_vertex_value[index+n_compactItemNum];
	p_edgeDummy[data_id] = valueX*n_max_dimension +  valueY;
}

__global__ void gpu_edgeBool(unsigned int *p_edgeDummy,			unsigned char *p_edgeBool,
							 unsigned int n_edgeNum,			unsigned int n_compactItemNum)
{
	unsigned int data_id = blockIdx.x*blockDim.x +  threadIdx.x;
	if( data_id >= n_edgeNum)
		return;
	if( data_id%n_compactItemNum == 0)
		p_edgeBool[data_id] = 1;
	else
	{
		if (p_edgeDummy[data_id] == p_edgeDummy[data_id -1])
			p_edgeBool[data_id] = 0;
		else
			p_edgeBool[data_id] = 1;
	}
}


__device__ unsigned int gpu_searchVertexIndex(unsigned int *p_vertex_index,			unsigned int *p_item_prefix,
											  unsigned int n_vertexNum,				unsigned int n_selectedItemNum,
											  unsigned int n_vertexBufferSize,		unsigned int n_key,
											  unsigned int n_itemId)
{
	unsigned int begin = p_item_prefix[n_itemId];
	unsigned int end, start, finish;
	if(n_itemId >= (n_selectedItemNum-1))
		end  = n_vertexBufferSize/3;
	else
		end = p_item_prefix[n_itemId+1];
	end = end -1;
	start = begin;
	finish = end;
	unsigned int mid;
	while ((begin <= end) && (end < 2147483647))
	{
		 mid  = (begin + end)/2;
		
		if( n_key > p_vertex_index[mid])
		{
			if(mid == finish)
				return 0;
			begin = mid +1;
		}
		else if ( n_key < p_vertex_index[mid])	
		{
			if(mid == start)
				return 0;
			end =  mid -1;
		}
		else
			return mid;
	}
	return 0;
}
 
__global__ void gpu_generateEdgeBuffer(unsigned int *p_vertex_Index,		unsigned int *p_edgeBuffer,
									   unsigned int *p_edgeDummy,			unsigned char *p_edgeBool,
									   unsigned int *p_edgeItemPre,			unsigned int *p_edgePrefix,
									   unsigned int *p_item_prefix,			unsigned int n_vertexNum,
									   unsigned int n_vertexBufferSize,		unsigned int n_compactItemNum,
									   unsigned int n_edgeNum,				unsigned int n_selectItemNum,
									   unsigned int n_max_dimension)
{
	unsigned int data_id = blockIdx.x*blockDim.x + threadIdx.x;
	if( data_id >= n_edgeNum)
		return;

	if (p_edgeBool[data_id] == 0)
		return;
	else
	{
		unsigned int rows = data_id/n_compactItemNum;
		unsigned int index = p_edgePrefix[data_id] + p_edgeItemPre[rows];
		if(index >= n_edgeNum)
			return;
		unsigned int key = p_edgeDummy[data_id]/n_max_dimension;

		 

		p_edgeBuffer[index*2] = gpu_searchVertexIndex(p_vertex_Index,		p_item_prefix,
													  n_vertexNum,			n_selectItemNum,
													  n_vertexBufferSize,	key,
													  rows);

		 


		key  = p_edgeDummy[data_id]%n_max_dimension;

		p_edgeBuffer[index*2+1]  = gpu_searchVertexIndex(p_vertex_Index,		p_item_prefix,
														n_vertexNum,		n_selectItemNum,
														n_vertexBufferSize, key,
														rows+1);
	}
}

 
__global__ void gpuColorFilter(unsigned int *p_compactValues,		unsigned int *p_dummyValue,
							   unsigned char *p_colorIndex,			unsigned char *p_colorBool,
							   unsigned int n_compactItemNum,		unsigned int n_dummyValueNum,
							   unsigned int n_selectedItem,			unsigned int n_maxDimension)
{
	unsigned int data_id = threadIdx.x + blockIdx.x*blockDim.x;
	if(data_id >= n_compactItemNum)
		return;
	p_colorBool[data_id] = 0;
	
//	bool satisfied = true;
	unsigned int begin, end, mid,value;
	begin = 0;
	end = n_dummyValueNum-1 ;
	mid = 0;//end +1;
	value =0;
	for(begin =0; begin < n_selectedItem; begin++)
	{
		value = n_maxDimension*value + p_compactValues[begin*n_compactItemNum + data_id];
	}
	

	if(n_dummyValueNum <= 1)
	{
		if(value == p_dummyValue[0])
			p_colorBool[data_id] = p_colorIndex[0];
		return;
	}


	begin =0;
	unsigned int pos =2147483647;
	while ((begin <= end)&&(end < 2147483647))
	{
		mid = (begin+end)/2;

		if(value > p_dummyValue[mid])
		{
			if(mid == n_dummyValueNum-1)
			{
				begin  = n_dummyValueNum;
				break;
			}
			begin = mid +1;
		}
		else if(value < p_dummyValue[mid])
		{
			if(mid ==0)
			{
				begin  = n_dummyValueNum;
				break;
			}
			end = mid -1;
		}
		else
		{
			pos = mid;
			break;
		}
	}
	
	if(begin <= end)
	{
		if(pos <2147483647)
		{
			if(value == p_dummyValue[mid])
				p_colorBool[data_id] = p_colorIndex[mid];
		}
	}
	else
	{
		if(mid>= 0 && mid <n_dummyValueNum)
		{
			if(value == p_dummyValue[mid])
				p_colorBool[data_id] = p_colorIndex[mid];
		}
	}
}

__global__ void gpuColorBuffer(unsigned int *p_vertex_Index,		unsigned char *p_highIndex,
							   unsigned int *p_item_prefix,			unsigned int *p_compactValues,
							   unsigned char *p_colorValue,			unsigned char *p_colorBool,
							   unsigned char *p_colorBuffer,		unsigned int n_vertexNum,
							   unsigned int n_compactItemNum,		unsigned int n_selectItemNum,
							   unsigned int n_vertexBufferSize)
{
	unsigned int data_id  = threadIdx.x + blockIdx.x*blockDim.x;
	if(data_id >= n_compactItemNum)
		return;
	if(p_colorBool[data_id] > 0)
	{
		unsigned int i;
		unsigned int key,index;
		unsigned char colorIndex;
		colorIndex = p_colorBool[data_id]-1;//0;//p_highIndex[p_colorBool[data_id]]-1;
		
		for(i =0;i<n_selectItemNum; i++)
		{
			key =  p_compactValues[i*n_compactItemNum + data_id];
			index = gpu_searchVertexIndex(p_vertex_Index,		p_item_prefix,
										  n_vertexNum,			n_selectItemNum,
										  n_vertexBufferSize,	key,
										  i);
			//if(index<n_vertexNum)
			{
				p_colorBuffer[4*index  ] = p_colorValue[4*colorIndex];
				p_colorBuffer[4*index+1] = p_colorValue[4*colorIndex+1];
				p_colorBuffer[4*index+2] = p_colorValue[4*colorIndex+2];
				p_colorBuffer[4*index+3] = p_colorValue[4*colorIndex+3];
			}
		}
	}
}

/*************************************************************************************************************
**************** Previous cuda codes, please do not delete!!!!!!!!!*******************************************
*************************************************************************************************************/

//__device__ unsigned int gpu_getDataValue(unsigned char *d_buffer,		unsigned int recordId, 
//										 unsigned int itemId,			unsigned int item_num,
//										 unsigned int *d_item_offset,	unsigned int *d_item_bytes,
//										 unsigned int total_bytes)
//{
//	unsigned int byteoffset = d_item_offset[itemId];
//	unsigned int numbytes   = d_item_bytes[itemId];
//
//	unsigned int values ;
//
//
//	if( numbytes == 1)
//	{
//		unsigned char item = d_buffer[recordId*total_bytes + byteoffset];
//		values = (unsigned int)(item);
//	}
//	else if( numbytes == 2)
//	{
//		unsigned char item_1 = d_buffer[recordId*total_bytes + byteoffset+1];
//		unsigned char item_2 = d_buffer[recordId*total_bytes + byteoffset];
//		unsigned short res  = item_1*256 + item_2;
//		values = (unsigned int)(res);
//
//	}
//	else
//	{
//		unsigned char item_1 = d_buffer[recordId*total_bytes + byteoffset+3];
//		unsigned char item_2 = d_buffer[recordId*total_bytes + byteoffset+2];
//		unsigned char item_3 = d_buffer[recordId*total_bytes + byteoffset+1];
//		unsigned char item_4 = d_buffer[recordId*total_bytes + byteoffset];
//		unsigned int  res  = ((item_1*256 +item_2)*256+item_3)*256+item_4;
//		values = (unsigned int)(res);
//	}
//	return values;
//}

//this function is used for the filtering of the excludedset
/*
__global__ void gpu_filter(unsigned char *d_buffer,unsigned int startId, unsigned int endId,
						   unsigned int *d_excludedset_begin, unsigned int *d_excludedset_size,
						   unsigned int *d_excludedset, unsigned char *d_boolean_vector,
						   unsigned int excludedset_begin_len, unsigned int excludedset_len,
						   unsigned int num_items, unsigned int* d_item_offset, 
						   unsigned int *d_item_bytes, unsigned int total_bytes)
{
	unsigned int data_id  = blockIdx.x*blockDim.x + threadIdx.x;
	unsigned int data_size  = endId - startId +1;
	if(data_id >= data_size)
		return;
	data_id = data_id + startId;

	unsigned int i,j;
	unsigned int index,refer_id, value,itemId;
	bool satified = true;
	for( i = 0; i< excludedset_begin_len; ++i)
	{
		index = d_excludedset_begin[i];
		satified = true;
		for( j = 0; j < d_excludedset_size[i]; ++j )
		{
			refer_id = index + j;
			itemId   = d_excludedset[refer_id*2]; 
			value = gpu_getDataValue(d_buffer,data_id,itemId, 
									 num_items,d_item_offset,d_item_bytes,total_bytes);
			if( value != d_excludedset[refer_id*2+1])
				satified = false;
			
			if(satified == false)
				break;

		}
		if( satified == true)
		{
			d_boolean_vector[data_id-startId] = 0;
			return;
		}
	}
	if(satified == false)
		d_boolean_vector[data_id-startId] = 1;
}
*/


/*
__global__ void gpu_CompactList(unsigned char *d_dummyBool, unsigned int *d_dummyIndex,
								 unsigned int *d_dummySum, unsigned int *d_compactValue,
								 unsigned int *d_selectedItems,unsigned int selectedItemsNum,
								 unsigned int *d_compactIndex, unsigned int dummySize)
{
	unsigned int data_id  = blockIdx.x*BLOCK_SIZE + threadIdx.x;
	if( data_id >= dummySize)
		return;

	if(d_dummyBool[data_id] == 1)
	{
		unsigned int index  = d_dummySum[data_id];
		unsigned int recordId = d_dummyIndex[data_id];;
		unsigned int itemId;
		d_compactIndex[index] = recordId;

		for(unsigned int i = 0; i<selectedItemsNum; i++)
		{
			itemId = d_selectedItems[i];
			d_compactValue[index*selectedItemsNum + i] = gpu_getDataValue(d_buffer,recordId,itemId,
															num_items,d_item_offset,d_item_bytes,total_bytes);
		}
	}

}
*/

/*
__global__ void gpu_dummyValues(unsigned char *d_buffer, unsigned int* d_filteredData, unsigned int filter_size,
								unsigned int dimension, unsigned int num_items,
								unsigned int *d_dummyIndex, unsigned int *d_dummyValue,
								unsigned int *d_selectedItems,  
								unsigned int selectedItemsNum, unsigned int* d_item_offset, 
								unsigned int *d_item_bytes, unsigned int total_bytes)
{
	unsigned int data_id  = blockIdx.x*blockDim.x + threadIdx.x;
	unsigned int data_size =  filter_size;
	if(data_id >= data_size)
		return;

	else
	{	
		unsigned int value;
		unsigned int dummyValue = 0;
		unsigned int _index = d_filteredData[data_id];
		for (unsigned int i = 0; i < selectedItemsNum; i++)
		{
			value = gpu_getDataValue(d_buffer,_index,d_selectedItems[i], num_items,
									 d_item_offset,d_item_bytes,total_bytes);
			dummyValue  = dummyValue*dimension + value;	 
		}
		d_dummyValue[data_id] = dummyValue;
		d_dummyIndex[data_id] = _index;
	}
}
*/

/*
__global__ void gpu_dummyValues(unsigned char *d_buffer, unsigned int startId, unsigned int endId,
								unsigned int dimension, unsigned int num_items,
								unsigned char *d_boolean_vector,unsigned int *d_boolean_prefix, 
								unsigned int *d_dummyIndex, unsigned int *d_dummyValue,
								unsigned int *d_selectedItems,  
								unsigned int selectedItemsNum, unsigned int* d_item_offset, 
								unsigned int *d_item_bytes, unsigned int total_bytes)
{
	unsigned int data_id  = blockIdx.x*blockDim.x + threadIdx.x;
	unsigned int data_size =  endId - startId +1;
	if(data_id >= data_size)
		return;
	if (d_boolean_vector[data_id] == 0)
		return;

	else
	{	
		unsigned int value;
		unsigned int dummyValue = 0;
		data_id = data_id + startId;
		
		for (unsigned int i = 0; i < selectedItemsNum; i++)
		{
			value = gpu_getDataValue(d_buffer,data_id,d_selectedItems[i], num_items,
									 d_item_offset,d_item_bytes,total_bytes);
			dummyValue  = dummyValue*dimension + value;	 
		}
		
		int index  = d_boolean_prefix[data_id-startId];
		d_dummyValue[index] = dummyValue;
		d_dummyIndex[index] = data_id;
	}
}
*/

/*
__global__ void gpu_compactItemNum_Step1(unsigned char *d_dummyBool, unsigned int *d_dummySum, 
								   unsigned int *d_item_times_tmp, unsigned int sizeDummy)
{
	unsigned int data_id  =  blockIdx.x*blockDim.x + threadIdx.x;
	if (data_id >= sizeDummy)
		return;
	if( d_dummyBool[data_id] == 0)
		return;
	else
	{
		unsigned int index  = d_dummySum[data_id];
		d_item_times_tmp[index] = data_id;
	}
}

__global__ void gpu_compactItemNum_Step2(unsigned int *d_item_times_tmp, unsigned int *d_item_times,
										 unsigned int compactItemNum, unsigned int sizeDummy)
{
	unsigned int data_id = blockIdx.x*blockDim.x + threadIdx.x;
	if( data_id >= compactItemNum )
		return;
	if( data_id == compactItemNum -1)
		d_item_times[data_id] = sizeDummy - d_item_times_tmp[data_id];
	else
		d_item_times[data_id] = d_item_times_tmp[data_id +1] - d_item_times_tmp[data_id];
}

__global__ void gpu_compactIndex(unsigned char *d_dummyBool, unsigned int *d_dummyIndex,
								 unsigned int *d_dummySum,
								 unsigned int *d_compactIndex, unsigned int dummySize)
{
	unsigned int data_id  = blockIdx.x*blockDim.x + threadIdx.x;
	if( data_id >= dummySize)
		return;

	if(d_dummyBool[data_id] == 1)
	{
		unsigned int index  = d_dummySum[data_id];
		d_compactIndex[index] = d_dummyIndex[data_id];
	}
}
*/

/*
__global__ void gpu_compactValue(unsigned char *d_dummyBool, unsigned int *d_dummyIndex,
								 unsigned int *d_dummySum, unsigned int *d_compactValue,
								 unsigned int *d_selectedItems,unsigned int selectedItemsNum,
								 unsigned char *d_buffer, unsigned int num_items,
								 unsigned int *d_item_offset, unsigned int compactItemNum,
								 unsigned int *d_item_bytes, unsigned int total_bytes,
								 unsigned int *d_compactIndex, unsigned int dummySize)
{
	unsigned int data_id  = blockIdx.x*blockDim.x + threadIdx.x;
	if( data_id >= dummySize)
		return;

	if(d_dummyBool[data_id] == 1)
	{
		unsigned int index  = d_dummySum[data_id];
		unsigned int recordId = d_dummyIndex[data_id];;
		unsigned int itemId;
		d_compactIndex[index] = recordId;

		for(unsigned int i = 0; i<selectedItemsNum; i++)
		{
			itemId = d_selectedItems[i];
			d_compactValue[i*compactItemNum + index] = gpu_getDataValue(d_buffer,recordId,itemId,
															num_items,d_item_offset,d_item_bytes,total_bytes);
		}
	}
}
*/

/*


__global__ void gpu_vertexIndex(unsigned char *d_buffer, unsigned int compactItemNum,
								unsigned int selectedItemsNum, unsigned int *d_selected_items,
								unsigned int *d_vertex_value, unsigned int *d_vertex_value_sorted,
								unsigned int *d_compactIndex, unsigned int item_num,
								unsigned int *d_item_offset, unsigned int *d_item_bytes,
								unsigned int total_bytes)
{
	unsigned int data_id  = blockIdx.x*blockDim.x + threadIdx.x;
	if (data_id >= compactItemNum*selectedItemsNum)
		return;
	
	unsigned int recordId =  d_compactIndex[data_id%compactItemNum];
	unsigned int itemId =  d_selected_items[data_id/compactItemNum];
	unsigned int vertexValue = gpu_getDataValue(d_buffer, recordId, itemId, item_num,
												d_item_offset, d_item_bytes, total_bytes);

	d_vertex_value[data_id] = vertexValue;
	d_vertex_value_sorted[data_id] = vertexValue;
}

*/

/*
__global__ void gpu_vertexBool(unsigned int *d_vertex_value, unsigned char *d_vertex_bool,
								 unsigned int compactItemNum, unsigned int vertexNum)
{
	unsigned int data_id = blockIdx.x*blockDim.x + threadIdx.x;
	if( data_id >= vertexNum )
		return;

	if( data_id%compactItemNum == 0)
		d_vertex_bool[data_id] = 1;
	else
	{
		if ( d_vertex_value[data_id] == d_vertex_value[data_id -1] )
			d_vertex_bool[data_id] = 0;
		else
			d_vertex_bool[data_id] = 1;
	}
}

__global__ void gpu_generateVertexBuffer(unsigned char *d_vertex_bool, unsigned int *d_vertex_prefix,
										 unsigned int *d_vertex_Index, unsigned int vertexNum,
										 unsigned int compactItemNum, unsigned int *d_item_prefix,
										 float *d_vertexBuffer, unsigned int *d_selected_items,
										 unsigned int *d_item_values, unsigned int *d_vertex_value)
{
	unsigned int data_id = blockIdx.x*blockDim.x +  threadIdx.x;
	if ( data_id >= vertexNum)
		return;
	if(d_vertex_bool[data_id] == 0)
		return;
	else
	{
		unsigned int rows = data_id/compactItemNum;
		unsigned int index = d_vertex_prefix[data_id] +  d_item_prefix[rows];
		if(index >= vertexNum)
			return;

		unsigned int itemId  = d_selected_items [rows]; 
		float stepX = (0.8f*compactItemNum)/(vertexNum - compactItemNum);
		float stepY	= 0.9f/(d_item_values[itemId] -1);

		float valueX = 0.1f + stepX*rows;
		float valueY = 0.05f + stepY*d_vertex_value[data_id];
		 
		d_vertexBuffer[index*3]		= valueX;
		d_vertexBuffer[index*3+1]	= valueY;
		d_vertexBuffer[index*3+2]	= 0.0f;
		d_vertex_Index[index] = d_vertex_value[data_id];
	}
}


__global__ void gpu_edgeDummyValue(unsigned int *d_vertex_value, unsigned int edgeNum,
								   unsigned int compactItemNum,  unsigned int *d_edgeDummy, 
								   unsigned int max_dimension, unsigned int selectedItemNum)
{
	unsigned int data_id  = blockIdx.x*blockDim.x  +  threadIdx.x;
	if( data_id >= edgeNum)
		return;
	//unsigned int row  = data_id%compactItemNum;
	//unsigned int column = data_id/compactItemNum;
	//unsigned int index;
	//index = row*selectedItemNum + column;
	//unsigned int valueX  = d_vertex_value[index];
	//unsigned int valueY  = d_vertex_value[index + 1]; 
	unsigned int column = data_id%compactItemNum;
	unsigned int row = data_id/compactItemNum;
	unsigned int index  = row*compactItemNum + column;
	unsigned int valueX = d_vertex_value[index];
	unsigned int valueY = d_vertex_value[index+compactItemNum];
	d_edgeDummy[data_id] = valueX*max_dimension +  valueY;
}

__global__ void gpu_edgeBool(unsigned int *d_edgeDummy, unsigned int edgeNum,
							 unsigned char *d_edgeBool, unsigned int compactItemNum)
{
	unsigned int data_id = blockIdx.x*blockDim.x +  threadIdx.x;
	if( data_id >= edgeNum)
		return;
	if( data_id%compactItemNum == 0)
		d_edgeBool[data_id] = 1;
	else
	{
		if (d_edgeDummy[data_id] == d_edgeDummy[data_id -1])
			d_edgeBool[data_id] = 0;
		else
			d_edgeBool[data_id] = 1;
	}
}

__device__ unsigned int gpu_searchVertexIndex(unsigned int key,unsigned int vertexNum, 
											  unsigned int itemId, unsigned int *d_vertex_index, 
											  unsigned int *d_item_prefix, unsigned int vertexBufferSize,
											  unsigned int selectedItemNum)
{
	unsigned int begin = d_item_prefix[itemId];
	unsigned int end;
	if(itemId == (selectedItemNum-1))
		end  = vertexBufferSize;
	else
		end = d_item_prefix[itemId+1];

	while (begin <= end)
	{
		unsigned int mid  = (begin + end)/2;
		
		if( key > d_vertex_index[mid])
			begin = mid +1;
		else if ( key < d_vertex_index[mid])
			end =  mid -1;
		else
			return mid;
	}
	

	return vertexNum+1000;
}

__global__ void gpu_generateEdgeBuffer(unsigned int *d_edgeBuffer, unsigned int edgeNum,
									   unsigned int *d_vertex_Index,unsigned int compactItemNum,
									   unsigned int *d_edgeDummy, unsigned char *d_edgeBool,
									   unsigned int *d_edgeItemPre, unsigned int *d_edgePrefix,
									   unsigned int max_dimension, unsigned int vertexNum,
									   unsigned int *d_item_prefix, unsigned int vertexBufferSize,
									   unsigned int selectItemNum)
{
	unsigned int data_id = blockIdx.x*blockDim.x + threadIdx.x;
	if( data_id >= edgeNum)
		return;

	if (d_edgeBool[data_id] == 0)
		return;
	else
	{
		unsigned int rows = data_id/compactItemNum;
		unsigned int index = d_edgePrefix[data_id] + d_edgeItemPre[rows];
		if(index >= edgeNum)
			return;
		unsigned int key = d_edgeDummy[data_id]/max_dimension;

		d_edgeBuffer[index*2] = gpu_searchVertexIndex(key,vertexNum,rows,d_vertex_Index,d_item_prefix,vertexBufferSize,selectItemNum);
		key  = d_edgeDummy[data_id]%max_dimension;
		d_edgeBuffer[index*2+1] = gpu_searchVertexIndex(key,vertexNum,rows+1,d_vertex_Index,d_item_prefix,vertexBufferSize,selectItemNum);
	}
}





__global__ void gpuColorFilter_V2(unsigned int *d_compactValues,	unsigned int compactItemNum,
								  unsigned int *d_dummyValue,		unsigned int dummyValueNum,
								  unsigned char *d_colorIndex,		unsigned char *d_colorBool)
{
	unsigned int data_id = threadIdx.x + blockIdx.x*blockDim.x;
	if(data_id >= compactItemNum)
		return;
	d_colorBool[data_id] = 0;
	bool satisfied = true;
	unsigned int begin, end, value;
	begin = 0;
	end = dummyValueNum;
	value = d_compactValues[data_id];
	unsigned int mid = end +1;
	while(begin <= end)
	{
		mid = (begin+end)/2;
		if(value > d_dummyValue[mid])
			begin = mid -1;
		else if(value < d_dummyValue[mid])
			end = mid -1;
		else
			break;
	}

	if(begin <= end)
		d_colorBool[data_id] = (unsigned char)d_colorIndex[mid];
}

 
__global__ void gpuColorFilter(unsigned int *d_compactValues, unsigned int compactItemNum,
							   unsigned int *d_selectedItem, unsigned int selectItemNum,
							   unsigned int *d_selectset, unsigned int* d_selectset_begin,
							   unsigned int *d_selectset_size, unsigned int length_selectset,
							   unsigned int num_selectedset, unsigned char *d_colorBool)
{
	unsigned int data_id  = threadIdx.x + blockIdx.x*blockDim.x;
	if (data_id >= compactItemNum)
		return;

	d_colorBool[data_id] = 0;
	bool satisfied = true;
	unsigned int i,j,k;
	unsigned int refer_id, itemId, value;
	for (i=0; i<num_selectedset; i++)
	{
		unsigned int begin  = d_selectset_begin[i];
		satisfied = true;
		for (j=0; j<d_selectset_size[i];j++)
		{
			refer_id  = begin + j;
			itemId = d_selectset[2*refer_id];
			for(k=0;k<selectItemNum;k++)
			{
				if(itemId == d_selectedItem[k])
					break;
			}
			value = d_compactValues[data_id + k*compactItemNum];
			if(value != d_selectset[refer_id*2+1])
				satisfied = false;
			if(satisfied == false)
				break;
		}
		if (satisfied == true)
		{
			d_colorBool[data_id] = 1;
			return;
		}
	}
}

__global__ void gpuColorBuffer(unsigned int *d_compactValues, unsigned char *d_colorBuffer,
							   unsigned char *d_colorBool, unsigned int compactItemNum,
							   unsigned int selectItemNum, unsigned int vertexNum,
							   unsigned int *d_vertex_Index, unsigned int *d_item_prefix,
								unsigned int vertexBufferSize)
{
	unsigned int data_id  = threadIdx.x + blockIdx.x*blockDim.x;
	if(data_id >= compactItemNum)
		return;
	if(d_colorBool[data_id] ==1)
	{
		unsigned int i;
		unsigned int key,index;
		for(i =0;i<selectItemNum; i++)
		{
			key =  d_compactValues[i*compactItemNum + data_id];
			index = gpu_searchVertexIndex(key,vertexNum,i,d_vertex_Index,d_item_prefix,vertexBufferSize,selectItemNum);
			d_colorBuffer[3*index] = 255;
			d_colorBuffer[3*index+1] = 0;
			d_colorBuffer[3*index+2] = 0;
		}
	}
}


__global__ void gpuColorBuffer_V2(unsigned int *d_compactValues, unsigned char *d_colorBuffer,
							   unsigned char *d_colorBool, unsigned int compactItemNum,
							   unsigned int selectItemNum, unsigned int vertexNum,
							   unsigned int *d_vertex_Index, unsigned int *d_item_prefix,
								unsigned int vertexBufferSize, unsigned int *d_highIndex,
								unsigned int d_colorValue)
{
	unsigned int data_id  = threadIdx.x + blockIdx.x*blockDim.x;
	if(data_id >= compactItemNum)
		return;
	if(d_colorBool[data_id] > 0)
	{
		unsigned int i;
		unsigned int key,index, colorIndex;
		colorIndex = d_highIndex[d_colorBool[data_id]]-1;
		for(i =0;i<selectItemNum; i++)
		{
			key =  d_compactValues[i*compactItemNum + data_id];
			index = gpu_searchVertexIndex(key,vertexNum,i,d_vertex_Index,d_item_prefix,vertexBufferSize,selectItemNum);
			d_colorBuffer[4*index  ] = d_colorValue[4*colorIndex];
			d_colorBuffer[4*index+1] = d_colorValue[4*colorIndex+1];
			d_colorBuffer[4*index+2] = d_colorValue[4*colorIndex+2];
			d_colorBuffer[4*index+4] = d_colorValue[4*colorIndex+3];
		}
	}
}

 

__global__ void gpu_VertexBufferDummy(float* d_vertexBuffer_ptr, unsigned int vertexNum)
{
	unsigned int data_id = threadIdx.x + blockIdx.x*blockDim.x;
	if(data_id >=vertexNum)
		return;
	float tmp = 1.0/vertexNum;
	d_vertexBuffer_ptr[3*data_id] = data_id*tmp;
	d_vertexBuffer_ptr[3*data_id+1] = data_id*tmp;
	d_vertexBuffer_ptr[3*data_id+2] = 0.0f;

}

__global__ void gpu_EdgeBufferDummy(unsigned int *d_edgeBuffer_ptr,unsigned int edgeNum, unsigned int vertexBufferSize)
{
	unsigned int data_id = threadIdx.x + blockIdx.x*blockDim.x;
	if(data_id >=edgeNum)
		return;
	float tmp = (vertexBufferSize-1)/edgeNum;
	d_edgeBuffer_ptr[data_id*2] = (unsigned int)(data_id*tmp);
	d_edgeBuffer_ptr[data_id*2] = vertexBufferSize-1 -(unsigned int)(data_id*tmp);


}
*/

#endif



