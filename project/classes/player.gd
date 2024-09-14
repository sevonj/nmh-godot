## The Player.
##
class_name Player extends BaseCharacter

enum ST {
	NORMAL,
}

# -- Public
# Nodes
var camerarig: CameraRigTP  ## Created and set by GM. We don't have to do anything locally.
var camera_target: Marker3D  ## Camerarig positions itself around this node

# Flags
var enable_input := false:  ## Public input toggle
	get = get_input_enabled,
	set = set_input_enabled

# -- Private

# Components
var _input := PlayerInput.new()  # All input is acquired through this component.
var _jumpenv := PlayerJumpenv.new()  # Determines vertical velocity unless flying.
# Components (Node)
#var _weapon_melee: WeaponBoxpunch

# Params
var _walk_speed := 5.0  ## Regular move speed

var _run_speed := 15.0
var _run_delay := 0.4

var _dodge_speed := 90.0
var _dodge_duration := 0.07

var _jump_minimum_time := .4:  ## Minimum hold time for *variable jump height*
	set = set_jump_minimum_time
var _jump_buffer_time := 0.:  ## *Jump buffering*
	set = set_jump_buffer_time
var _jump_coyote_time := .3  ## *Coyote time*



# Status vars
var _state := ST.NORMAL:
	set = _set_state

var _t_state := INF  # How long have we spent in current state
var _t_run := INF  # Time spent running, used for delay before running actually kicks in
var _t_jump := INF  # Time in jump. Used by _jumpenv
var _t_since_grounded := INF  # Used for coyote time


var _locked_direction := Vector3.ZERO  # Locked direction while dodging and other such moves.


# --- SetGet --- #
func get_input_enabled() -> bool:
	return _input.enabled


func set_input_enabled(enabled: bool) -> void:
	_input.enabled = enabled


func set_jump_minimum_time(t: float) -> void:
	_input.jump_minimum_time = t


func set_jump_buffer_time(t: float) -> void:
	_input.jump_buffer_time = t


func _set_state(state: ST):
	_state = state
	_t_state = 0.  # Reset state timer


# --- Setup --- #
func _init():
	super()
	name = "player"
	camera_target = Marker3D.new()
	camera_target.name = "camera_target"


func _ready():
	super()
	_coll = get_node_or_null("coll")
	_mdl = get_node_or_null("mdl")
	add_child(camera_target)

	# Warnings
	if !is_instance_valid(_mdl):
		push_error(name + ": Player mdl node not found.")
	if !is_instance_valid(_coll):
		push_error(name + ": Player coll node not found.")

	# -- Components
	_setup_input()
	_setup_coll()
	#_setup_weapon_melee()


# --- Updates --- #


func _process(_delta):
	_input.poll()

	# Update rotation
	if is_instance_valid(camerarig):
		global_rotation = camerarig.global_rotation

	_update_camera_target()
	_update_state()

	if _input.is_run_triggered():
		_t_run = 0.

	var grounded := is_on_floor()
	var coyotetime := _t_since_grounded < _jump_coyote_time

	match _state:
		ST.NORMAL:
			# -- Jump
			# Jump Trigger
			if _input.is_jump_triggered() or _input.is_jump_queued():
				if grounded or coyotetime:
					_input.start_jump()
					_t_jump = 0.

			# Jump Release -> skip to decay if we aren't there yet.
			if _input.is_jump_released():
				var new_t = _jumpenv.get_decay_t_from_v(velocity.y)
				_t_jump = max(new_t, _t_jump)


#	_check_interaction()


## An attempt to create sane movement code instead of fisting 5 function calls deep, setting a dozen
## status flags back and forth between different nodes on the way to wherever depths of Rube
## Goldberg's ass.
func _physics_process(delta):
	# All timers are now updated in physics process instead of frame process.
	_input.advance_timers(delta)
	_t_state += delta
	_t_run += delta
	_t_jump += delta
	_t_since_grounded += delta

	var grounded := is_on_floor()
	var input_dir := _get_input_direction()

	if grounded:
		_t_since_grounded = 0.
		_t_jump = min(_t_jump, _jumpenv.t_start_fall)  # No falling allowed on ground

	match _state:
		ST.NORMAL:
			var running := _t_run > _run_delay and _input.is_run_held()

			if running:
				velocity = input_dir * _run_speed
			else:
				velocity = input_dir * _walk_speed
			velocity.y = _jumpenv.get_velocity(_t_jump)

	move_and_slide()


# --- Private --- #
func _setup_input() -> void:
	_input.jump_minimum_time = _jump_minimum_time
	_input.jump_buffer_time = _jump_buffer_time


# Sets up world collider
func _setup_coll() -> void:
	# Layers
	#collision_layer = Globals.COL_LAYER_P
	#collision_mask = Globals.COL_MASK_P
	# Shape
	var shape := CapsuleShape3D.new()
	shape.height = 2.
	shape.radius = .3
	_coll.shape = shape
	_coll.position = Vector3.UP * shape.height / 2


# Do this once, in _ready()
#func _setup_weapon_melee() -> void:
#	_weapon_melee = get_node_or_null("weapon_melee")
#	if !is_instance_valid(_weapon_melee):
#		push_warning(name + ": Player has no melee weapon.")
#		return
#	_weapon_melee.damage = 10.
#	_weapon_melee.mask = Globals.COL_LAYER_E | Globals.COL_LAYER_I | Globals.COL_LAYER_W
#	_weapon_melee.attack_duration = .3
#	_weapon_melee.attack_time = .1


func _update_camera_target() -> void:
	camera_target.position = Vector3.UP * 1.6
	# Keep camera target slightly to the right from player from camera's perspective.
	# We can do something fancier later.
	#if is_instance_valid(camera_target) && is_instance_valid(camerarig):
	#	camera_target.position = camerarig.transform.basis * Vector3.ZERO


## This will set a new state, if appropriate.
func _update_state() -> void:
	var grounded := is_on_floor()

	match _state:
		ST.NORMAL:
			var direction := _get_input_direction()
			var is_moving := !direction.is_zero_approx()
			var is_not_running := _t_run < _run_delay



#func _check_interaction() -> void:
#	if !is_instance_valid(camerarig):
#		return
#	var target = camerarig.raycast_aim(Globals.COL_MASK_P, _interaction_distance).get("collider")
#	if target is Interactable:
#		if _input.is_interact_triggered():
#			target.interact()


## Get input direction (global)
func _get_input_direction() -> Vector3:
	var input_vector := _input.get_vector_move()
	if input_vector.is_zero_approx():
		return Vector3.ZERO

	var direction := Vector3(input_vector.x, 0, input_vector.y).normalized()
	if is_instance_valid(camerarig):
		direction = camerarig.transform.basis * direction

	return direction
