/* ========================================================================
   $File: $
   $Date: $
   $Revision: $
   $Creator: Casey Muratori $
   $Notice: (C) Copyright 2014 by Molly Rocket, Inc. All Rights Reserved. $
   ======================================================================== */

#include "handmade.h"

internal void
GameOutputSound(game_state *GameState, game_sound_output_buffer *SoundBuffer, int ToneHz)
{
	int16 ToneVolume = 3000;
	int WavePeriod = SoundBuffer->SamplesPerSecond / ToneHz;

	int16 *SampleOut = SoundBuffer->Samples;
	for (int SampleIndex = 0; SampleIndex < SoundBuffer->SampleCount; ++SampleIndex)
	{
		// TODO(casey): Draw this out for people
#if 0
		real32 SineValue = sinf(GameState->tSine);
		int16 SampleValue = (int16)(SineValue * ToneVolume);
#else
		int16 SampleValue = 0;
#endif
		*SampleOut++ = SampleValue;
		*SampleOut++ = SampleValue;

#if 0
		GameState->tSine += 2.0f * Pi32 * 1.0f / (real32)WavePeriod;
		if (GameState->tSine > 2.0f * Pi32)
		{
			GameState->tSine -= 2.0f * Pi32;
		}
#endif
	}
}

inline int32 RoundReal32ToInt32(real32 Real32)
{
	int32 Result = (int32)(Real32 + 0.5f);
	return Result;
}

inline uint32 RoundReal32ToUInt32(real32 Real32)
{
	uint32 Result = (uint32)(Real32 + 0.5f);
	// TODO(casey): Intrinsic????
	return Result;
}

#include "math.h"
inline int32 FloorReal32ToInt32(real32 Real32)
{
	int32 Result = (int32)floorf(Real32);
	return Result;
}

inline int32 TruncateReal32ToInt32(real32 Real32)
{
	int32 Result = (int32)Real32;
	return Result;
}

internal void DrawRectangle(game_offscreen_buffer *Buffer,
							real32 RealMinX,
							real32 RealMinY,
							real32 RealMaxX,
							real32 RealMaxY,
							real32 R,
							real32 G,
							real32 B)
{
	int32 MinX = RoundReal32ToInt32(RealMinX);
	int32 MinY = RoundReal32ToInt32(RealMinY);
	int32 MaxX = RoundReal32ToInt32(RealMaxX);
	int32 MaxY = RoundReal32ToInt32(RealMaxY);

	if (MinX < 0)
	{
		MinX = 0;
	}

	if (MinY < 0)
	{
		MinY = 0;
	}

	if (MaxX > Buffer->Width)
	{
		MaxX = Buffer->Width;
	}

	if (MaxY > Buffer->Height)
	{
		MaxY = Buffer->Height;
	}

	uint32 Color =
		((RoundReal32ToUInt32(R * 255.0f) << 16) | (RoundReal32ToUInt32(G * 255.0f) << 8) |
		 (RoundReal32ToUInt32(B * 255.0f) << 0));

	uint8 *Row = ((uint8 *)Buffer->Memory + MinX * Buffer->BytesPerPixel + MinY * Buffer->Pitch);
	for (int Y = MinY; Y < MaxY; ++Y)
	{
		uint32 *Pixel = (uint32 *)Row;
		for (int X = MinX; X < MaxX; ++X)
		{
			*Pixel++ = Color;
		}

		Row += Buffer->Pitch;
	}
}

inline tile_map *GetTileMap(world *World, int32 TileMapX, int32 TileMapY)
{
	tile_map *TileMap = 0;

	if ((TileMapX >= 0) && (TileMapX < World->TileMapCountX) && (TileMapY >= 0) &&
	(TileMapY < World->TileMapCountY))
	{
		TileMap = &World->TileMaps[TileMapY * World->TileMapCountX + TileMapX];
	}

	return TileMap;
}

inline uint32 GetTileValueUnchecked(world *World, tile_map *TileMap, int32 TileX, int32 TileY)
{
	Assert(TileMap);
	Assert((TileX >= 0) && (TileX < World->CountX) && (TileY >= 0) && (TileY < World->CountY));

	uint32 TileMapValue = TileMap->Tiles[TileY * World->CountX + TileX];
	return TileMapValue;
}

inline bool32 IsTileMapPointEmpty(world *World, tile_map *TileMap, int32 TestTileX, int32 TestTileY)
{
	bool32 Empty = false;

	if (TileMap)
	{
		if ((TestTileX >= 0) && (TestTileX < World->CountX) && (TestTileY >= 0) &&
			(TestTileY < World->CountY))
		{
			uint32 TileMapValue = GetTileValueUnchecked(World, TileMap, TestTileX, TestTileY);
			Empty = (TileMapValue == 0);
		}
	}

	return Empty;
}

// inline canonical_position GetCanonicalPosition(world *World, raw_position Pos)
// {
// 	canonical_position Result;
	
// 	Result.TileMapX = Pos.TileMapX;
// 	Result.TileMapY = Pos.TileMapY;

// 	real32 X = Pos.TileRelX - World->UpperLeftX;
// 	real32 Y = Pos.TileRelY - World->UpperLeftY;
// 	Result.TileX = FloorReal32ToInt32(X / World->TileWidth);
// 	Result.TileY = FloorReal32ToInt32(Y / World->TileHeight);

// 	Result.TileRelX = X - Result.TileX * World->TileWidth;
// 	Result.TileRelY = Y - Result.TileY * World->TileHeight;

// 	Assert(Result.TileRelX >= 0);
// 	Assert(Result.TileRelY >= 0);
// 	Assert(Result.TileRelX < World->TileWidth);
// 	Assert(Result.TileRelY < World->TileHeight);

// 	if (Result.TileX < 0)
// 	{
// 		Result.TileX = World->CountX + Result.TileX;
// 		--Result.TileMapX;
// 	}

// 	if (Result.TileY < 0)
// 	{
// 		Result.TileY = World->CountY + Result.TileY;
// 		--Result.TileMapY;
// 	}

// 	if (Result.TileX >= World->CountX)
// 	{
// 		Result.TileX = Result.TileX - World->CountX;
// 		++Result.TileMapX;
// 	}

// 	if (Result.TileY >= World->CountY)
// 	{
// 		Result.TileY = Result.TileY - World->CountY;
// 		++Result.TileMapY;
// 	}

// 	return Result;
// }

internal position GetWorldPosition(world *World, position Pos, int32 DirectionX, int32 DirectionY)
{
	position Result = Pos;

	if (DirectionY < 0) // Moving up
	{
		if (Result.TileY == 0)
		{
			Result.TileY = World->CountY - 1;
			--Result.TileMapY;
		}
		else
		{
			--Result.TileY;
		}
	}
	else if (DirectionY > 0) // Moving down
	{
		if (Result.TileY == World->CountY - 1)
		{
			Result.TileY = 0;
			++Result.TileMapY;
		}
		else
		{
			++Result.TileY;
		}
	}
	else if (DirectionX < 0) // Moving left
	{
		if (Result.TileX == 0)
		{
			Result.TileX = World->CountX - 1;
			--Result.TileMapX;
		}
		else
		{
			--Result.TileX;
		}
	}
	else if (DirectionX > 0) // Moving right
	{
		if (Result.TileX == World->CountX - 1)
		{
			Result.TileX = 0;
			++Result.TileMapX;
		}
		else
		{
			++Result.TileX;
		}
	}

	return Result;
}

internal bool32
IsWorldPointEmpty(world *World, position Pos)
{
	bool32 Empty = false;

	tile_map *TileMap = GetTileMap(World, Pos.TileMapX, Pos.TileMapY);
	Empty = IsTileMapPointEmpty(World, TileMap, Pos.TileX, Pos.TileY);

	return Empty;
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
	Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0]) ==
		   (ArrayCount(Input->Controllers[0].Buttons)));
	Assert(sizeof(game_state) <= Memory->PermanentStorageSize);

#define TILE_MAP_COUNT_X 16
#define TILE_MAP_COUNT_Y 9
	uint32 Tiles00[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = {
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
		{1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
		{1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
		{1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1},
		{1, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1},
		{1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1},
	};

	uint32 Tiles01[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = {
		{1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	};

	uint32 Tiles10[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = {
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1},
	};

	uint32 Tiles11[TILE_MAP_COUNT_Y][TILE_MAP_COUNT_X] = {
		{1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1},
		{1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
	};

	tile_map TileMaps[2][2];

	TileMaps[0][0].Tiles = (uint32 *)Tiles00;
	TileMaps[0][1].Tiles = (uint32 *)Tiles10;
	TileMaps[1][0].Tiles = (uint32 *)Tiles01;
	TileMaps[1][1].Tiles = (uint32 *)Tiles11;

	world World;
	World.TileMapCountX = 2;
	World.TileMapCountY = 2;
	World.CountX = TILE_MAP_COUNT_X;
	World.CountY = TILE_MAP_COUNT_Y;
	World.UpperLeftX = 0;
	World.UpperLeftY = 0;
	World.TileWidth = 60;
	World.TileHeight = 60;

	real32 PlayerWidth = World.TileWidth;
	real32 PlayerHeight = World.TileHeight;

	World.TileMaps = (tile_map *)TileMaps;

	position PlayerPos;
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	if (!Memory->IsInitialized)
	{
		GameState->PlayerTileMapX = 0;
		GameState->PlayerTileMapY = 0;

		PlayerPos.TileX = 2;
		PlayerPos.TileY = 2;

		GameState->PlayerX = PlayerPos.TileX*World.TileWidth + World.TileWidth/2;
		GameState->PlayerY = PlayerPos.TileY*World.TileHeight + World.TileHeight/2;

		GameState->Movable = true;

		Memory->IsInitialized = true;
	}

	PlayerPos.TileMapX = GameState->PlayerTileMapX;
	PlayerPos.TileMapY = GameState->PlayerTileMapY;
	PlayerPos.TileX = FloorReal32ToInt32(GameState->PlayerX/World.TileWidth);
	PlayerPos.TileY = FloorReal32ToInt32(GameState->PlayerY/World.TileHeight);
	tile_map *TileMap = GetTileMap(&World, GameState->PlayerTileMapX, GameState->PlayerTileMapY);
	Assert(TileMap);

	for (int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers);
		 ++ControllerIndex)
	{
		game_controller_input *Controller = GetController(Input, ControllerIndex);
		if (GameState->Movable)
		{
			if (Controller->IsAnalog)
			{
				// NOTE(casey): Use analog movement tuning
			}
			else
			{
				// NOTE(casey): Use digital movement tuning
				int32 dPlayerX = 0;
				int32 dPlayerY = 0;

				if (Controller->MoveUp.EndedDown)
				{
					dPlayerY = -1;
				}
				if (Controller->MoveDown.EndedDown)
				{
					dPlayerY = 1;
				}
				if (Controller->MoveLeft.EndedDown)
				{
					dPlayerX = -1;
				}
				if (Controller->MoveRight.EndedDown)
				{
					dPlayerX = 1;
				}

				position TestPos = GetWorldPosition(&World, PlayerPos, dPlayerX, dPlayerY);
				if (IsWorldPointEmpty(&World, TestPos))
				{
					PlayerPos.TileMapX = TestPos.TileMapX;
					PlayerPos.TileMapY = TestPos.TileMapY;
					PlayerPos.TileX = TestPos.TileX;
					PlayerPos.TileY = TestPos.TileY;
					GameState->PlayerTileMapX = PlayerPos.TileMapX;
					GameState->PlayerTileMapY = PlayerPos.TileMapY;
					GameState->PlayerX = PlayerPos.TileX*World.TileWidth + World.TileWidth/2;
					GameState->PlayerY = PlayerPos.TileY*World.TileHeight + World.TileHeight/2;
					// GameState->Movable = false;
				}
			}
		}
	}

	DrawRectangle(
		Buffer, 0.0f, 0.0f, (real32)Buffer->Width, (real32)Buffer->Height, 1.0f, 0.0f, 0.1f);
	for (int Row = 0; Row < 9; ++Row)
	{
		for (int Column = 0; Column < 16; ++Column)
		{
			uint32 TileID = GetTileValueUnchecked(&World, TileMap, Column, Row);
			real32 Gray = 0.5f;
			if (TileID == 1)
			{
				Gray = 1.0f;
			}

			real32 MinX = World.UpperLeftX + ((real32)Column) * World.TileWidth;
			real32 MinY = World.UpperLeftY + ((real32)Row) * World.TileHeight;
			real32 MaxX = MinX + World.TileWidth;
			real32 MaxY = MinY + World.TileHeight;
			DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);
		}
	}

	real32 PlayerR = 1.0f;
	real32 PlayerG = 1.0f;
	real32 PlayerB = 0.0f;
	real32 PlayerLeft = GameState->PlayerX - PlayerWidth*0.5f;
	real32 PlayerTop = GameState->PlayerY - PlayerHeight*0.5f;
	DrawRectangle(Buffer,
				  PlayerLeft,
				  PlayerTop,
				  PlayerLeft + PlayerWidth,
				  PlayerTop + PlayerHeight,
				  PlayerR,
				  PlayerG,
				  PlayerB);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
	game_state *GameState = (game_state *)Memory->PermanentStorage;
	GameOutputSound(GameState, SoundBuffer, 400);
}
