## This base class provides common features for characters other "active" entities.
## Anything that uses health or intends to receive damage should inherit this.
##
## Setting up a character requires a few steps: [br]
##
## Node structure:
## [codeblock]
## Absolutely necessary nodes:
##     CharacterRoot: extends BaseCharacter - Your character script
##     ├── coll: CollisionShape3D           - World collision for movement.
##     ├── mdl:  Node3D                     - Visual model and detailed hit collisions.
##
## Optional nodes:
##     └── focus_point: Marker3D            - Offsets focus_point
## [/codeblock]
##
## Collisions: BaseCharacter doesn't know or care about collision layers or masks. Some inheriting
## classes, like [Enemy] do. [br]
##
## init and ready:

@icon("res://editor/icons/icon_base_character.png")
class_name BaseCharacter extends CharacterBody3D

## Emitted when the characted dies.
signal sig_died
## Emitted when hurt. Death itself (calling [method kill], etc.) skips this.
signal sig_health_changed(old: float, new: float)

# -- Public
# Flags
var god := false  ## Godmode: Ignore hurt. Can still die if asked to.
var buddha := false  ## Hurt hurts, but heath, can't go below 1. Can still die if asked to.
var instadie := false  ## Die immediately from hurt() regardless of how little damage taken.
# Variables
var max_health := 100.:
	set = _set_max_health
var health := 100.:
	set = _set_health
## Focus point is a global coordinate that represents the center of this character (not origin,
## which usually should be at the bottom). For example, NPCs shall aim their shots at the focus
## point. Set the offset by adding "focus_point" node.
var focus_point: Vector3:
	get:
		if is_instance_valid(_focus_point):
			return _focus_point.global_position
		return global_position

# -- Private
# Flags
var _dead := false
# Children
## Optional child. Focus point offset marker. Add a child named "focus_point" to set.
var _focus_point: Marker3D
## Required child. World collider for movement. Add a child named "coll" to set.
@onready var _coll: CollisionShape3D = $coll
## Required child. Graphics model & hitbox container. Add a child named "mdl" to set.
@onready var _mdl: Node3D = $mdl


# --- Setters --- #
func _set_max_health(value: float):
	max_health = clamp(value, 0, INF)
	health = clamp(health, 0, max_health)


func _set_health(value: float):
	health = clamp(value, 0, max_health)


# --- Setup --- #
func _init():
	pass


# Remember to call super() at the end of your _ready().
func _ready():
	if Engine.is_editor_hint():
		return
	if not is_instance_valid(_coll):
		push_error(name + ": coll node not found. (CollisionShape3D, world collider)")

	if not is_instance_valid(_mdl):
		push_error(name + ": mdl node not found. (Node3D, model container)")

	_focus_point = get_node_or_null("focus_point")


# --- Character Commands --- #


## Apply pain to this character (health damage)
#func hurt(damage: DamageInfo) -> void:
#	if god:
#		return
#	if instadie:
#		damage.amount = INF
#
#	var new_health = clamp(health - damage.amount, 0., max_health)
#	sig_health_changed.emit(health, new_health)
#	health = new_health
#
#	if Globals.debug_hitnumbers:
#		_debug_hitnumber(damage.amount)
#
#	if buddha:
#		health = max(health, 1)
#
#	if is_zero_approx(health):
#		kill()


## Tell this character to die.
func kill() -> void:
	_dead = true
	sig_died.emit()
	queue_free()


##  Perform gory gib explosion (die).
func explode() -> void:
	push_warning(name, ": explode() not implemented.")
	kill()


## Become ragdoll (die).
func ragdoll() -> void:
	push_warning(name, ": ragdoll() not implemented.")
	kill()


## Push to a direction. Used by explosions and such.
## For damage knockback, use [member DamageInfo.force] instead.
func knockback(_force: Vector3) -> void:
	push_warning(name, ": knockback() not implemented.")


# --- Miscellaneous --- #


# Debug hit effect that shows damage number
#func _debug_hitnumber(value):
#	var entitycont = GM.get_world_entities()
#	if !is_instance_valid(entitycont):
#		return
#
#	var hitfx := FXHitnumber.new(-value)
#	entitycont.add_child(hitfx)
#	hitfx.global_position = global_position


# --- listeners --- "
func _on_death():
	queue_free()
