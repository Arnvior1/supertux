//  $Id$
//
//  Pingus - A free Lemmings clone
//  Copyright (C) 2002 Ingo Ruhnke <grumbel@gmx.de>
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

#include <iostream>
#include <vector>
#include <assert.h>
#include "texture.h"
#include "screen.h"
#include "lispreader.h"
#include "gameloop.h"
#include "worldmap.h"

namespace WorldMapNS {

TileManager* TileManager::instance_  = 0;

TileManager::TileManager()
{
  lisp_stream_t stream;
  FILE* in = fopen(DATA_PREFIX "images/worldmap/antarctica.scm", "r");
  assert(in);
  lisp_stream_init_file (&stream, in);
  lisp_object_t* root_obj = lisp_read (&stream);
  
  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-worldmap-tiles") == 0)
    {
      lisp_object_t* cur = lisp_cdr(root_obj);

      while(!lisp_nil_p(cur))
        {
          lisp_object_t* element = lisp_car(cur);

          if (strcmp(lisp_symbol(lisp_car(element)), "tile") == 0)
            {
              int id = 0;
              std::string filename = "<invalid>";

              Tile* tile = new Tile;             
              tile->north = true;
              tile->east  = true;
              tile->south = true;
              tile->west  = true;
              tile->stop  = true;
  
              LispReader reader(lisp_cdr(element));
              reader.read_int("id",  &id);
              reader.read_bool("north", &tile->north);
              reader.read_bool("south", &tile->south);
              reader.read_bool("west",  &tile->west);
              reader.read_bool("east",  &tile->east);
              reader.read_bool("stop",  &tile->stop);
              reader.read_string("image",  &filename);

              texture_load(&tile->sprite, 
                           const_cast<char*>((std::string(DATA_PREFIX "/images/worldmap/") + filename).c_str()), 
                           USE_ALPHA);

              if (id >= int(tiles.size()))
                tiles.resize(id+1);

              tiles[id] = tile;
            }
          else
            {
              puts("Unhandled symbol");
            }

          cur = lisp_cdr(cur);
        }
    }
  else
    {
      assert(0);
    }
}

Tile*
TileManager::get(int i)
{
  assert(i >=0 && i < int(tiles.size()));
  return tiles[i];
}

WorldMap::WorldMap()
{
  quit = false;
  width  = 20;
  height = 15;

  texture_load(&tux_sprite, DATA_PREFIX "/images/worldmap/tux.png", USE_ALPHA);
  texture_load(&level_sprite, DATA_PREFIX "/images/worldmap/levelmarker.png", USE_ALPHA);

  tux_offset = 0;
  tux_moving = false;
  tux_tile_pos.x = 0;
  tux_tile_pos.y = 0;
  tux_direction = NONE;

  input_direction = NONE;
  enter_level = false;

  name = "<no name>";
  music = "SALCON.MOD";
  song = 0;

  load_map();
}

WorldMap::~WorldMap()
{
}

void
WorldMap::load_map()
{
  lisp_stream_t stream;
  FILE* in = fopen(DATA_PREFIX "levels/default/worldmap.scm", "r");
  assert(in);
  lisp_stream_init_file (&stream, in);
  lisp_object_t* root_obj = lisp_read (&stream);
  
  if (strcmp(lisp_symbol(lisp_car(root_obj)), "supertux-worldmap") == 0)
    {
      lisp_object_t* cur = lisp_cdr(root_obj);

      while(!lisp_nil_p(cur))
        {
          lisp_object_t* element = lisp_car(cur);

          if (strcmp(lisp_symbol(lisp_car(element)), "tilemap") == 0)
            {
              LispReader reader(lisp_cdr(element));
              reader.read_int("width",  &width);
              reader.read_int("height", &height);
              reader.read_int_vector("data", &tilemap);
            }
          else if (strcmp(lisp_symbol(lisp_car(element)), "properties") == 0)
            {
              LispReader reader(lisp_cdr(element));
              reader.read_string("name",  &name);
              reader.read_string("music", &music);
            }
          else if (strcmp(lisp_symbol(lisp_car(element)), "levels") == 0)
            {
              lisp_object_t* cur = lisp_cdr(element);
              
              while(!lisp_nil_p(cur))
                {
                  lisp_object_t* element = lisp_car(cur);
                  
                  if (strcmp(lisp_symbol(lisp_car(element)), "level") == 0)
                    {
                      Level level;
                      LispReader reader(lisp_cdr(element));
                      reader.read_string("name",  &level.name);
                      reader.read_int("x-pos", &level.x);
                      reader.read_int("y-pos", &level.y);
                      levels.push_back(level);
                    }
                  
                  cur = lisp_cdr(cur);      
                }
            }
          else
            {
              
            }
          
          cur = lisp_cdr(cur);
        }
    }
}

void
WorldMap::get_input()
{
  SDL_Event event;

  enter_level = false;

  while (SDL_PollEvent(&event))
    {
      switch(event.type)
        {
        case SDL_QUIT:
          quit = true;
          break;
          
        case SDL_KEYDOWN:
          switch(event.key.keysym.sym)
            {
            case SDLK_ESCAPE:
              quit = true;
              break;
            case SDLK_LCTRL:
            case SDLK_RETURN:
              enter_level = true;
              break;
            default:
              break;
            }
          break;
        }
    }

  Uint8 *keystate = SDL_GetKeyState(NULL);
  
  input_direction = NONE;
  
  if (keystate[SDLK_LEFT])
    input_direction = WEST;
  else if (keystate[SDLK_RIGHT])
    input_direction = EAST;
  else if (keystate[SDLK_UP])
    input_direction = NORTH;
  else if (keystate[SDLK_DOWN])
    input_direction = SOUTH;
}

Point
WorldMap::get_next_tile(Point pos, Direction direction)
{
  switch(direction)
    {
    case WEST:
      pos.x -= 1;
      break;
    case EAST:
      pos.x += 1;
      break;
    case NORTH:
      pos.y -= 1;
      break;
    case SOUTH:
      pos.y += 1;
      break;
    case NONE:
      break;
    }
  return pos;
}

bool
WorldMap::path_ok(Direction direction, Point old_pos, Point* new_pos)
{
  *new_pos = get_next_tile(old_pos, direction);

  if (!(new_pos->x >= 0 && new_pos->x < width
        && new_pos->y >= 0 && new_pos->y < height))
    { // New position is outsite the tilemap
      return false;
    }
  else
    { // Check if we the tile allows us to go to new_pos
      switch(direction)
        {
        case WEST:
          return (at(old_pos)->west && at(*new_pos)->east);

        case EAST:
          return (at(old_pos)->east && at(*new_pos)->west);

        case NORTH:
          return (at(old_pos)->north && at(*new_pos)->south);

        case SOUTH:
          return (at(old_pos)->south && at(*new_pos)->north);

        case NONE:
          assert(!"path_ok() can't work if direction is NONE");
        }
      return false;
    }
}

void
WorldMap::update()
{
  float speed = 4.5;

  if (enter_level && !tux_moving)
    {
      for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
        {
          if (i->x == tux_tile_pos.x && 
              i->y == tux_tile_pos.y)
            {
              std::cout << "Enter the current level: " << i->name << std::endl;;
              halt_music();
              gameloop(const_cast<char*>((DATA_PREFIX "levels/default/" + i->name).c_str()),
                       1, ST_GL_LOAD_LEVEL_FILE);
              play_music(song, 1);
              break;
            }
        }
    }
  else
    {
      if (!tux_moving)
        {
          if (input_direction == NONE)
            {
              tux_offset = 0;
              tux_direction = NONE;
            }
          else
            {
              Point next_tile;
              if (path_ok(input_direction, tux_tile_pos, &next_tile))
                {
                  tux_tile_pos = next_tile;
                  tux_moving = true;
                  tux_direction = input_direction;
                }
              else
                { // Stop
                  tux_offset = 0;
                  tux_direction = NONE;
                }
            }
        }
      else
        {
          tux_offset += speed;

          if (tux_offset > 32)
            {
              tux_offset -= 32;

              if (at(tux_tile_pos)->stop)
                {
                  tux_direction = NONE;
                  tux_moving = false;
                }
              else
                {
                  Point next_tile;
                  if (path_ok(tux_direction, tux_tile_pos, &next_tile))
                    {
                      tux_tile_pos = next_tile;
                    }
                  else
                    {
                      puts("Tilemap data is buggy");
                      tux_direction = NONE;
                      tux_moving = false;
                      tux_offset = 0;
                    }
                }
            }
        }
    }
}

Tile*
WorldMap::at(Point p)
{
  assert(p.x >= 0 
         && p.x < width
         && p.y >= 0
         && p.y < height);
  return TileManager::instance()->get(tilemap[width * p.y + p.x]);
}

void
WorldMap::draw()
{
  for(int y = 0; y < height; ++y)
    for(int x = 0; x < width; ++x)
      {
        Tile* tile = at(Point(x, y));
        texture_draw(&tile->sprite, x*32, y*32, NO_UPDATE);
      }
  
  float x = tux_tile_pos.x * 32;
  float y = tux_tile_pos.y * 32;

  switch(tux_direction)
    {
    case WEST:
      x -= tux_offset - 32;
      break;
    case EAST:
      x += tux_offset - 32;
      break;
    case NORTH:
      y -= tux_offset - 32;
      break;
    case SOUTH:
      y += tux_offset - 32;
      break;
    case NONE:
      break;
    }

  for(Levels::iterator i = levels.begin(); i != levels.end(); ++i)
    {
      texture_draw(&level_sprite, i->x*32, i->y*32, NO_UPDATE);
    }

  texture_draw(&tux_sprite, (int)x, (int)y, NO_UPDATE);
  flipscreen();
}

void
WorldMap::display()
{
  quit = false;

  song = load_song(const_cast<char*>((DATA_PREFIX "/music/" + music).c_str()));
  play_music(song, 1);

  while(!quit) {
    draw();
    get_input();
    update();
    SDL_Delay(20);
  }

  free_music(song);
}

} // namespace WorldMapNS

void worldmap_run()
{
  WorldMapNS::WorldMap worldmap;
  
  worldmap.display();
}

/* EOF */
