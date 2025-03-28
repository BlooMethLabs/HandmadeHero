#if !defined(HANDMADE_H)
/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

/*
  NOTE(casey):

  HANDMADE_INTERNAL:
    0 - Build for public release
    1 - Build for developer only

  HANDMADE_SLOW:
    0 - Not slow code allowed!
    1 - Slow code welcome.
*/

#include "handmade_platform.h"

#define internal static
#define local_persist static
#define global_variable static

#define Pi32 3.14159265359f

#if HANDMADE_SLOW
// TODO(casey): Complete assertion macro - don't worry everyone!
#define Assert(Expression)                                                                         \
	if (!(Expression))                                                                             \
	{                                                                                              \
		*(int *)0 = 0;                                                                             \
	}
#else
#define Assert(Expression)
#endif

#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
// TODO(casey): swap, min, max ... macros???

inline uint32 SafeTruncateUInt64(uint64 Value)
{
	// TODO(casey): Defines for maximum values
	Assert(Value <= 0xFFFFFFFF);
	uint32 Result = (uint32)Value;
	return Result;
}

inline game_controller_input *GetController(game_input *Input, int unsigned ControllerIndex)
{
	Assert(ControllerIndex < ArrayCount(Input->Controllers));

	game_controller_input *Result = &Input->Controllers[ControllerIndex];
	return Result;
}

//
//
//

struct position
{
	int32 TileMapX;
	int32 TileMapY;

	int32 TileX;
	int32 TileY;
};

struct tile_map
{
	uint32 *Tiles;
};

struct world
{
	int32 CountX;
	int32 CountY;

	real32 UpperLeftX;
	real32 UpperLeftY;
	real32 TileWidth;
	real32 TileHeight;

	int32 TileMapCountX;
	int32 TileMapCountY;

	tile_map *TileMaps;
};

struct game_state
{
	int32 PlayerTileMapX;
	int32 PlayerTileMapY;

	real32 PlayerX;
	real32 PlayerY;
	
	// bool32 Movable;
	real32 DistanceToMoveX;
	real32 DistanceToMoveY;
};

#define HANDMADE_H
#endif
