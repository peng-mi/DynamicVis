//thrust
#include <thrust/host_vector.h>
#include <thrust/device_vector.h>
#include <thrust/sort.h>
#include <thrust/functional.h>

extern "C"
void cuda_HistogramOrder(unsigned int* _histogramData, unsigned int* _reference, unsigned int* _order, unsigned int _size)
{
	unsigned int* d_histogramData_ptr;
	unsigned int* d_tmp_ptr;
	unsigned int* d_reference_ptr;
	unsigned int* d_order_ptr;
	unsigned int mem_size = sizeof(unsigned int)*_size;

	cudaMalloc(&d_histogramData_ptr, mem_size);
	cudaMalloc(&d_tmp_ptr, mem_size);
	cudaMalloc(&d_reference_ptr, mem_size);
	cudaMalloc(&d_order_ptr,mem_size);

	cudaMemcpy(d_histogramData_ptr, _histogramData, mem_size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_reference_ptr, _reference, mem_size, cudaMemcpyHostToDevice);
	cudaMemcpy(d_order_ptr, _reference, mem_size, cudaMemcpyHostToDevice);

	thrust::device_ptr<unsigned int> d_histogramData(d_histogramData_ptr);
	thrust::device_ptr<unsigned int> d_reference(d_reference_ptr);
	thrust::device_ptr<unsigned int> d_order(d_order_ptr);
	thrust::device_ptr<unsigned int> d_tmp(d_tmp_ptr);

	thrust::sort_by_key(d_histogramData, d_histogramData + _size, d_reference, thrust::greater<unsigned int>());

	cudaMemcpy(d_tmp_ptr, d_reference_ptr, mem_size, cudaMemcpyDeviceToDevice);
	thrust::sort_by_key(d_tmp, d_tmp + _size, d_order);

	cudaMemcpy(_histogramData, d_histogramData_ptr, mem_size, cudaMemcpyDeviceToHost);
	cudaMemcpy(_reference, d_reference_ptr, mem_size, cudaMemcpyDeviceToHost);
	cudaMemcpy(_order, d_order_ptr, mem_size, cudaMemcpyDeviceToHost);


	cudaFree(d_histogramData_ptr);
	cudaFree(d_reference_ptr);
	cudaFree(d_tmp_ptr);
}

extern "C"
unsigned int binarySearch(unsigned int* _data, unsigned int _value, unsigned int _size)
{
	unsigned int begin = 0, end = _size-1;
	unsigned int middle;
	
	while(begin <= end)
	{
		middle = (begin + end)/2;
		if(_data[middle] == _value)
			return middle;
		else
		{
			if(_value > _data[middle])
				begin = middle +1;
			else
				end = middle -1;
		}
	}

	return 0;

}