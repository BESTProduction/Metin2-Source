#include "stdafx.h"
#include "start_position.h"

char g_nation_name[4][32] =
{
	"",
	"�ż���",
	"õ����",
	"���뱹",
};

long g_start_map[4] =
{
	0,	// reserved
	1,
	21,
	41
};

DWORD g_start_position[4][2] =
{
	{      0,      0 },	// reserved
	{ 469300, 964200 },
	{  55700, 157900 },
	{ 969600, 278400 }
};

DWORD arena_return_position[4][2] =
{
	{       0,  0       },
	{   347600, 882700  },
	{   138600, 236600  },
	{   857200, 251800  }
};

DWORD g_create_position[4][2] =
{
	{		0,		0 },
	{ 474800, 966000 },		// Kirmizi Bayrak
	{ 60000, 155700 },		// Sari Bayrak
	{ 963700, 278400 },		// Mavi Bayrak
};

DWORD g_create_position_canada[4][2] =
{
	{		0,		0 },
	{ 457100, 946900 },
	{ 45700, 166500 },
	{ 966300, 288300 },
};