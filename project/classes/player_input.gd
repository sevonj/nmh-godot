## PlayerInput - Player controller component
## Adds a layer of abstraction to simplify the actual player code.
##
## You should only get actions in _process(), after calling update.
class_name PlayerInput extends RefCounted

# -- Public
var enabled := true  ## Input enabled

var jump_minimum_time := .4  ## Variable jump height - Minimum hold time
var jump_buffer_time := 0.15  ## *Jump buffering*


# -- Private

# Status
var _jump_t := INF  # Time since jump started. Used to filter events in jump release.
var _jump_held := false  # Jump has been held down since start of jump
var _jump_buffer_t := INF  # Jump buffer timer; time since last jump press.

# --- Public --- #


## Call this __per frame__ aka in _process().
func poll() -> void:
	if !enabled:
		return

	# Update Jump status
	if not Input.is_action_pressed("move_jump"):
		_jump_held = false
	if Input.is_action_just_pressed("move_jump"):
		_jump_buffer_t = 0.


func advance_timers(delta) -> void:
	if !enabled:
		return

	_jump_t += delta
	_jump_buffer_t += delta


# --- Get Actions --- #
# You should only get actions in _process(), after calling update.

# -- Movement


func is_jump_triggered() -> bool:
	if !enabled:
		return false
	return Input.is_action_just_pressed("move_jump")


func is_jump_released() -> bool:
	if !enabled:
		return false
	if _jump_held:  # Still holding
		return false
	if _jump_t < jump_minimum_time:  # Not holding, but gotta wait
		return false
	return true  # Not holding _and_ time's up


func is_jump_queued() -> bool:
	if !enabled:
		return false
	return _jump_buffer_t <= jump_buffer_time


func is_run_triggered() -> bool:
	if !enabled:
		return false
	return Input.is_action_just_pressed("move_sprint")


func is_run_held() -> bool:
	if !enabled:
		return false
	return Input.is_action_pressed("move_sprint")


func is_atk_melee_triggered() -> bool:
	if !enabled:
		return false
	return Input.is_action_just_pressed("atk_melee")


func is_atk_ranged_triggered() -> bool:
	if !enabled:
		return false
	return Input.is_action_just_pressed("atk_ranged")



# --- Other

# --- Get Axes & Vectors --- #


## Horizontal movement vector
func get_vector_move() -> Vector2:
	if !enabled:
		return Vector2.ZERO
	return Input.get_vector("move_left", "move_right", "move_forw", "move_back")


# --- Change State --- #


func start_jump() -> void:
	_jump_buffer_t = INF
	_jump_t = 0
	_jump_held = true
