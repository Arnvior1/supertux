//  SuperTux
//  Copyright (C) 2010 Ingo Ruhnke <grumbel@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "badguy/snowman.hpp"

#include "audio/sound_manager.hpp"
#include "badguy/snowball.hpp"
#include "object/bullet.hpp"
#include "object/player.hpp"
#include "supertux/sector.hpp"

Snowman::Snowman(const Reader& reader) :
  WalkingBadguy(reader, "images/creatures/snowman/snowman.sprite", "walk-left", "walk-right")
{
  walk_speed = 40;
  SoundManager::current()->preload("sounds/pop.ogg");
}

void
Snowman::loose_head()
{
  // replace with Snowball
  Vector snowball_pos = get_pos();
  // Hard-coded values from sprites
  snowball_pos.x += 5;
  snowball_pos.y += 1;

  /* Create death animation for the (now headless) snowman. */
  set_action (dir == LEFT ? "headless-left" : "headless-right", /* loops = */ -1);
  set_pos (get_pos () + Vector (-4.0, 19.0)); /* difference in the sprite offsets */
  physic.set_velocity_y(0);
  physic.set_acceleration_y(0);
  physic.enable_gravity(true);
  set_state (STATE_FALLING);

  /* Create a new snowball where the snowman's head was */
  auto snowball = std::make_shared<SnowBall>(snowball_pos, dir, dead_script);
  Sector::current()->add_object(snowball);
}

HitResponse
Snowman::collision_bullet(Bullet& bullet, const CollisionHit& hit)
{
  if(bullet.get_type() == FIRE_BONUS) {
    // fire bullets destroy snowman's body
    loose_head();

    SoundManager::current()->play("sounds/pop.ogg", get_pos()); // this could be a different sound
    bullet.remove_me();

    return ABORT_MOVE;
  }
  else {
    // in all other cases, bullets ricochet
    bullet.ricochet(*this, hit);
    return FORCE_MOVE;
  }
}

bool
Snowman::collision_squished(GameObject& object)
{
  // bounce
  Player* player = dynamic_cast<Player*>(&object);
  if (player)
    player->bounce(*this);

  SoundManager::current()->play("sounds/pop.ogg", get_pos());

  loose_head();

  return true;
}

/* EOF */
