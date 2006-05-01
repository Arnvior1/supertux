//  $Id: rainsplash.cpp 3327 2006-04-13 15:02:40Z ravu_al_hemio $
//
//  SuperTux
//  Copyright (C) 2006 Matthias Braun <matze@braunis.de>
//  Copyright (C) 2006 Christoph Sommer <christoph.sommer@2006.expires.deltadevelopment.de>
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

#include "sprite_particle.hpp"
#include "sector.hpp"
#include "camera.hpp"
#include "main.hpp"

SpriteParticle::SpriteParticle(std::string sprite_name, Vector position, Vector velocity, Vector acceleration, int drawing_layer) 
	: position(position), velocity(velocity), acceleration(acceleration), drawing_layer(drawing_layer)
{
  sprite = sprite_manager->create(sprite_name);
  if (!sprite) throw std::runtime_error("Could not load sprite "+sprite_name);
  sprite->set_animation_loops(1);
}
  
SpriteParticle::~SpriteParticle() 
{
  remove_me();
}

void
SpriteParticle::hit(Player& )
{
}

void
SpriteParticle::update(float elapsed_time) 
{
  // die when animation is complete
  if (sprite->animation_done()) {
    remove_me();
    return;
  }

  // calculate new position and velocity
  position.x += velocity.x * elapsed_time;
  position.y += velocity.y * elapsed_time;
  velocity.x += acceleration.x * elapsed_time;
  velocity.y += acceleration.y * elapsed_time;

  // die when too far offscreen
  Vector camera = Sector::current()->camera->get_translation();
  if ((position.x < camera.x - 128) || (position.x > SCREEN_WIDTH + camera.x + 128) || 
      (position.y < camera.y - 128) || (position.y > SCREEN_HEIGHT + camera.y + 128)) {
    remove_me();
    return;
  }
}

void
SpriteParticle::draw(DrawingContext& context) 
{
   sprite->draw(context, position, drawing_layer);
}
