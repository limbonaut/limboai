## Physical cover object for GOAP demo
## Provides collision for LoS blocking and cover positions
extends StaticBody2D
class_name CoverObject

## Collision layer for LoS blocking
const LOS_COLLISION_LAYER := 16  # Bit 5 (1 << 4)

## Returns the cover position relative to a threat position
## The agent should position themselves on the opposite side of cover from the threat
## Uses raycasting to verify the position actually breaks line-of-sight
func get_cover_position_against(threat_pos: Vector2) -> Vector2:
	var to_threat := threat_pos - global_position
	var away_from_threat := -to_threat.normalized()

	# Start with a position behind cover (opposite side from threat)
	# Using 100 units for clearer visual separation from the cover object
	var base_offset := 100.0
	var test_pos := global_position + away_from_threat * base_offset

	# Verify this position breaks LoS using raycast
	var space_state := get_world_2d().direct_space_state
	var query := PhysicsRayQueryParameters2D.create(threat_pos, test_pos, LOS_COLLISION_LAYER)
	var result := space_state.intersect_ray(query)

	# If raycast hits cover, position is good (cover blocks LoS)
	if not result.is_empty() and result.collider == self:
		return test_pos

	# If LoS isn't blocked, try moving further back
	for extra_distance in [20.0, 40.0, 60.0]:
		test_pos = global_position + away_from_threat * (base_offset + extra_distance)
		query = PhysicsRayQueryParameters2D.create(threat_pos, test_pos, LOS_COLLISION_LAYER)
		result = space_state.intersect_ray(query)
		if not result.is_empty() and result.collider == self:
			return test_pos

	# Fallback to geometric calculation if raycasting fails
	return global_position + away_from_threat * base_offset


## Returns true if the cover blocks LoS between two points
func blocks_los_between(from_pos: Vector2, to_pos: Vector2) -> bool:
	var space_state := get_world_2d().direct_space_state
	var query := PhysicsRayQueryParameters2D.create(from_pos, to_pos, LOS_COLLISION_LAYER)
	var result := space_state.intersect_ray(query)
	return result.size() > 0 and result.collider == self
