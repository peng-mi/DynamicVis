#ifndef _LAYOUT_KERNEL_CU_
#define _LAYOUT_KERNEL_CU_

#include <GL/glew.h>
#if defined(__APPLE__) || defined(MACOSX)
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif

//#include <cutil_inline.h>
#include "datamanager.h"
#include <math.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <cuda_runtime.h>
#include <cuda_gl_interop.h>
#include <helper_cuda.h>
#include <helper_cuda_gl.h> 

using namespace VIS;
#define BLOCK_SIZE 512

__device__ float addGraDir(int& id, float* dir, float* baryCenter,float& graFactor,float& repFactor,float& attExponent,t_GraphNode* node)
{
	float dist =0.0;
	float diff =0.0;
	diff  = node[id].pos[0]-baryCenter[0];
	dist += diff*diff;
	diff  = node[id].pos[1]-baryCenter[1];
	dist += diff*diff;
	dist = sqrtf(dist);

	float tmp  = graFactor*repFactor*node[id].weight*pow(dist,attExponent-2);
	dir[0] += (baryCenter[0] - node[id].pos[0])*tmp;
	dir[1] += (baryCenter[1] - node[id].pos[1])*tmp;
	return tmp*fabs(attExponent-1.0);
}

__device__ float  getDistance(int& id1,int& id2,t_GraphNode *node)
{
	float dist =0.0;
	float diff;
	diff = node[id1].pos[0] - node[id2].pos[0];
	dist +=diff*diff;
	diff = node[id1].pos[1] - node[id2].pos[1];
	dist +=diff*diff;
	return sqrtf(dist);
}




__device__ float addAttDir(int &id,int& numNode, float* dir, t_GraphNode* node, t_GraphEdge* edge, float& attExponent)
{
	float dir2 =0.0;
	int index = node[id].begin; 

	for( int i=0;i<node[id].numEdge;i++)
	{
		float dist = getDistance(id, edge[index].node,node);
		if(dist ==0.0)
			continue;

		float tmp = edge[index].weight*pow(dist,attExponent-2);

		dir2 += tmp* fabs(attExponent-1);

		dir[0] += (node[edge[index].node].pos[0] - node[id].pos[0])*tmp ;
		dir[1] += (node[edge[index].node].pos[1] - node[id].pos[1])*tmp ;
		index++;
	}
	return dir2;
}



__device__ float addRepDir(int& id, int& numNode, float& repFactor, float& repExponent,float* dir, t_GraphNode* node)
{
	if(node[id].weight ==0.0)
		return 0.0;

	float dir2=0.0;
	for(int i=0;i< numNode;i++)
	{
		if(i == id || node[i].weight ==0.0)
			continue;

		float dist = getDistance(i,id,node);
		if(dist ==0.0)
			continue;

		float tmp = repFactor*node[i].weight*node[id].weight*pow(dist,repExponent-2);

		dir2 += tmp* fabs(repExponent-1);
		dir[0] -= (node[i].pos[0] - node[id].pos[0])*tmp;
		dir[1] -= (node[i].pos[1] - node[id].pos[1])*tmp;
	}
	return dir2;
}


__device__ void getDirection(int id,float* dir, int& numNode,float& repFactor,float& repExponent, float& graFactor, float& attExponent, float* baryCenter, t_GraphNode* node,t_GraphEdge* edge)
{
	dir[0] = dir[1]=0.0;
	float dir2=0;

	dir2 = addRepDir(id, numNode, repFactor, repExponent, dir,node);
	dir2 += addAttDir(id,numNode, dir,  node,edge,  attExponent);
	dir2 += addGraDir(id, dir, baryCenter, graFactor, repFactor, attExponent,node);

	if(fabs(dir2)<0.00001)
	{
		if(dir2>0)
			dir2 = 0.00001;
		else
			dir2 = -0.00001;
	}

	dir[0] /= dir2;
	dir[1] /= dir2;
}



__global__ void ForceDirect(t_LayoutParamter* parmeter,t_GraphNode* graph_node,t_GraphEdge* graph_edge)
{
	float attExponent = parmeter[0].attExponent;
	float graFactor = parmeter[0].graFactor;
	float repFactor = parmeter[0].repFactor;
	float repExponent = parmeter[0].repExponent;
	int numNode = parmeter[0].numNode;



	int id  = blockIdx.x*blockDim.x + threadIdx.x;
	if(id>= numNode)
		return;

	if(graph_node[id].stable==true)
		return;


	float bestDir[2];
	float oldPos[2];
	bestDir[0] = bestDir[1] = 0.0;
	getDirection(id,bestDir,numNode,repFactor,repExponent,graFactor,attExponent,parmeter[0].baryCenter,graph_node,graph_edge);

	oldPos[0] = graph_node[id].pos[0];
	oldPos[1] = graph_node[id].pos[1];

	//int bestMultiple =0;

	graph_node[id].pos[0] = oldPos[0] + bestDir[0];//*multiple;
	graph_node[id].pos[1] = oldPos[1] + bestDir[1];//*multiple;

	if(fabs(graph_node[id].pos[0])>=parmeter[0].range)
	{ 
		if(graph_node[id].pos[0]>0)
			graph_node[id].pos[0] =parmeter[0].range;
		else
			graph_node[id].pos[0] =0-parmeter[0].range;

	}
	if (fabs(graph_node[id].pos[1])>=parmeter[0].range)
	{
		if(graph_node[id].pos[1]>0)
			graph_node[id].pos[1] =parmeter[0].range;
		else
			graph_node[id].pos[1] =0-parmeter[0].range;
	}


	__syncthreads(); 
}



extern "C"
void kernel_function(t_LayoutParamter* layoutParameter,t_GraphNode* graph_node,t_GraphEdge* graph_edge)
{
	t_LayoutParamter*	d_layoutParameter;
	t_GraphNode*		d_graphNode;
	t_GraphEdge*		d_graphEdge;

	cudaMalloc((void**)&d_layoutParameter, sizeof(t_LayoutParamter));
	cudaMalloc((void**)&d_graphNode, sizeof( t_GraphNode )*(layoutParameter[0].numNode));
	cudaMalloc((void**)&d_graphEdge, sizeof( t_GraphEdge )*(layoutParameter[0].numEdge*2));

	int blocks = layoutParameter[0].numNode/BLOCK_SIZE;
	if(layoutParameter[0].numNode%BLOCK_SIZE!=0)
		blocks++;


	cudaMemcpy(d_layoutParameter,layoutParameter,sizeof(t_LayoutParamter),cudaMemcpyHostToDevice);
	cudaMemcpy(d_graphNode, graph_node,sizeof(t_GraphNode)*layoutParameter[0].numNode,cudaMemcpyHostToDevice);
	cudaMemcpy(d_graphEdge, graph_edge,sizeof(t_GraphEdge)*layoutParameter[0].numEdge*2,cudaMemcpyHostToDevice);
	ForceDirect<<<blocks,BLOCK_SIZE>>>(d_layoutParameter, d_graphNode, d_graphEdge);
	cudaMemcpy(graph_node,d_graphNode,sizeof(t_GraphNode)*layoutParameter[0].numNode,cudaMemcpyDeviceToHost);


	cudaFree(d_layoutParameter);
	cudaFree(d_graphNode);
	cudaFree(d_graphEdge);


}
#endif