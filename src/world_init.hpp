#pragma once

#include "common.hpp"
#include "render_system.hpp"
#include <optional>

// invaders
entt::entity createPlayer(RenderSystem* renderer, vec2 position);

// towers
entt::entity createSniperEnemy(RenderSystem* renderer, vec2 position, Direction direction);

entt::entity createSniperBullet(RenderSystem* renderer, vec2 pos, vec2 size, vec2 velocity, int damage);

entt::entity createPortalBullet(RenderSystem* renderer, vec2 pos, vec2 size, vec2 velocity);

entt::entity createBoomerang(RenderSystem* renderer, vec2 pos, vec2 size, vec2 velocity, int damage, vec2 start, vec2 end, vec2 control_start, vec2 control_end);

entt::entity createWeaponIndicator(vec2 pos, float rotation);

entt::entity createPortal(RenderSystem* renderer, vec2 position, vec2 scale, int direction, std::optional<entt::entity> previous_portal = std::nullopt);

entt::entity createText(std::string text, vec2 pos, vec2 scale);

entt::entity createSkipButton(vec2 pos, vec2 scale);

entt::entity createExplosion(vec2 pos);

entt::entity createNWEWall(RenderSystem* renderer, vec2 position);

entt::entity createSouthWall(RenderSystem* renderer, vec2 position);

entt::entity createFloorTile(vec2 position);

entt::entity createBlankTile(vec2 position);

entt::entity createDoor(RenderSystem* renderer, vec2 position);

entt::entity createExit(RenderSystem* renderer, vec2 position);

entt::entity createKey(RenderSystem* renderer, vec2 position);

entt::entity createFloorIce(RenderSystem* renderer, vec2 position);

entt::entity createCheese(RenderSystem* renderer, vec2 position);

entt::entity createPatrolEnemy(RenderSystem* renderer, std::vector<vec2> positions);

entt::entity createPatrolRangeIndicator(entt::entity owner, vec2 position);

entt::entity createMousetrap(RenderSystem* renderer, vec2 position);

entt::entity createCutscene(int cutscene_index);

entt::entity createUI(RenderSystem* renderer, TEXTURE_ASSET_ID texture, int z);

entt::entity createStartScreen();

// legacy
// the player
entt::entity createChicken(RenderSystem* renderer, vec2 position);

bool is_level_saved();