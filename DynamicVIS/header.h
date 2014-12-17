#ifndef _HEADER_H_
#define _HEADER_H_

/*************************************************************************************************/
/**************** Head Files ********************************************************************/
/*************************************************************************************************/

//#include <time.h>

#include <vector>
#include <map>
#include <set>
#include <string>
#include <algorithm>
using namespace std;

//cuda
//#include <cutil_inline.h>    
//#include <cutil_gl_inline.h>  
//#include <cuda.h>
//#include <cuda_runtime_api.h>


/*************************************************************************************************/
/**************** constant variles definition ****************************************************/
/*************************************************************************************************/
#define TIMESTEP 1
#define RANGESTEP 1
#define RANGERATIO 0.05f

#define CONFIGURATION ".\\configuration.xml"

#define MAX_LOADING_TIME 100
#define HISTOGRAM_ID 3000

/*************************************************************************************************/
/**************** enum      definition ****************************************************/
/*************************************************************************************************/
typedef enum
{
	SE_UPDATE = 0,
	EX_UPDATE,
	NEX_UPDATE,
	NONE,
}t_FilterType;

typedef enum
{
	WORKER_EVENT =0,
}t_event;


typedef enum
{
	Brush_One = 0,
	Brush_And,
	Brush_OR,
	Brush_One_EX,
	Brush_And_EX,
	Brush_OR_EX,
	Brush_Neg_One_EX,
	Brush_Neg_And_EX,
	Brush_Neg_OR_EX,
} t_BrushType;

typedef enum
{
	UPDATE_STYLE = 0,
	INCREMENTAL_STYLE,
}t_DataUpdateStyle;




/*************************************************************************************************/
/**************** data structure definition ****************************************************/
/*************************************************************************************************/
typedef unsigned int uint;
typedef unsigned char uchar;
typedef unsigned short ushort;

typedef vector<uint> t_uintvector;
typedef map<t_uintvector, uint> t_recordset;
typedef set<pair<uint, uint>> t_AndFilterSet;
typedef set<pair<uint, uint>> t_EdgeSet;
typedef map<pair<uint,uint>,uint> t_EdgeInfo;
typedef vector<t_AndFilterSet> t_FilterSet;
typedef pair<uint, uint> t_intpair;
typedef map<t_intpair,uint> t_edge;
typedef map<uint, uint> t_ref;
typedef map<uint, t_ref> t_group;
typedef vector<t_intpair> t_uintpairvector;
typedef vector<t_recordset::iterator> t_FilteredRecords;
typedef vector<time_t> t_TimeStamps;

typedef map<uint,vector<t_recordset::iterator>> t_ParaFRecords;

typedef struct{
	uint numTSs;
	uint numpoints;
	float *data;
	t_uintvector color;
	bool need_rebuild_renderingdata;
} t_TimeSeriesData;

typedef struct{
	float *data;
	uchar *colordata;
	uint numpoints;
	float xspan[2];
	float yspan[2];
} t_ScatterPlotData;

typedef struct
{
	string name;
	uint pos[2];
	uint size[2];
}t_FrameInfo;



typedef struct
{
	uint pos[2];
	uint size[2];
	uint time_step;
	uint range_step;
	char* data_folder;
	string title;
	vector<t_FrameInfo> subframes;
}t_configPara;


typedef map<uint, t_FilterSet> t_SelectedFilterSet;
typedef map<uint, set<uint>> t_highlightData;

/****************  Graph   ****************************/

typedef struct
{ 
	float	pos[2];
	uint	begin;
	uint	numEdge;
	bool	stable;
	float	weight;
}t_GraphNode;

typedef struct
{
	int		node;
	float	weight;
}t_GraphEdge;

typedef struct
{
	float	attExponent;
	float	graFactor;
	float	graScale;
	float	repFactor;
	float	repExponent;
	uint	numNode;
	uint	numNode_1;
	uint	numNode_2;
	uint	numEdge;
	float	nodeSize;
	bool	graphReady;
	float	range;
	float	baryCenter[2];
	float	boundary[3];
}t_LayoutParamter;



typedef struct
{
	bool	is_merge;
	bool	is_total;
	uint		items[2];
}t_GraphInfo;


typedef struct
{
	t_GraphNode* node;
	t_GraphEdge* edge;
	t_LayoutParamter graphPara;
	map<uint,string> nodeInfo;
	map<uint,string> edgeInfo;
	t_GraphInfo	graphInfo;
	t_recordset	records;
	vector<t_ref> vertices;
	t_EdgeInfo edges;
	map<uint,uint> vertex_IdToValue;
	t_recordset	extra_edges;
	map<uint,uint> vertex_ValueToId;
	map<uint,uint> duplicatedD_S;
	map<uint,uint> duplicatedS_D;
	unsigned int offset;
	bool is_layout;
}t_Graph;

typedef struct
{
	map<uint,pair<uint,uint>> node;
	map<uint,map<pair<uint,uint>,uint>> edge;
}t_HighlightGraph;


typedef struct
{
	unsigned char red;
	unsigned char green;
	unsigned char blue;
	unsigned char alpha;
}t_color;

#endif