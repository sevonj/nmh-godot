## Third person camera controller.
class_name CameraRigTP extends Node3D

const _PROFILE_PATH = "res://config/cameraprofile/"
const _PROFILE_VER = "1"

const _DEFAULT_RAYAIM_MASK = -1 #Globals.COL_LAYER_W | Globals.COL_LAYER_E
const _CAMERA_COLL_MASK = -1 #Globals.COL_LAYER_W  # Only collide against world.

# Nodes
var target: Marker3D

# Flags
var enable_input := false

# -- Camera profile
# See CameraProfile class at the bottom.
var _profile: CameraProfile  # The default profile
var _profile_override: CameraProfile  # Scripts can set a temporary override.
var _profile_returning := false  # If returning from override to default, interpolate backwards.
var _profile_time := INF  # Time since profile change
var _profile_interpolate_duration := 5.

# These are derived from profiles. SEtting them directly is futile.
var _max_pitch_angle := 90.
var _min_pitch_angle := -90.
var _camera_distance := 2.5
var _camera_offset := Vector3.ZERO

# Children
@onready var _pivot := Node3D.new()
@onready var _camera := Camera3D.new()


# --- Virtual Methods --- #
func _init():
	name = "camera_rig_tp"
	_profile = CameraProfile.new()  # in case nothing gets loaded


func _ready():
	_setup_camera_arm()
	# _save_profile(CameraProfile.new(), "latest")


func _process(delta):
	_profile_time += delta
	_update_profile_params()
	_update_camera_arm()


func _input(event):
	if !enable_input:
		return

	if event is InputEventMouseMotion:
		rotate_y(-event.relative.x * .01 )#* Globals.input_mouse_sens)
		_pivot.rotate_x(-event.relative.y * .01 )#* Globals.input_mouse_sens)

		# Limit pitch
		_pivot.rotation.x = clamp(
			_pivot.rotation.x, deg_to_rad(_min_pitch_angle), deg_to_rad(_max_pitch_angle)
		)

		return


# --- Public --- #


## Camera rotation
func get_camera_rotation() -> Vector3:
	return _camera.global_rotation


## Set default camera profile by name.
func load_profile(profilename: String) -> void:
	print("set profile called with: ", profilename)
	var json = _get_profile_json(profilename)
	_profile = _json_to_profile(json)


func set_profile_override(profilename: String) -> void:
	var json = _get_profile_json(profilename)
	_profile_override = _json_to_profile(json)
	_profile_returning = false
	_profile_time = 0.


func remove_profile_override() -> void:
	_profile_returning = true
	_profile_time = 0.


## Raycast from camera to mousepos (screen center on captured). This can be used by weapons.
## Return dictionary is guaranteed to contain a 'position'. Check for 'collider' to learn whether
## the ray actually hit something.
func raycast_aim(mask := _DEFAULT_RAYAIM_MASK, distance: float = 4096) -> Dictionary:
	var mousepos := get_viewport().get_mouse_position()
	var origin := _camera.project_ray_origin(mousepos)
	var end := origin + _camera.project_ray_normal(mousepos) * distance
	var query := PhysicsRayQueryParameters3D.create(origin, end, mask)
	var result = get_world_3d().direct_space_state.intersect_ray(query)
	if !result.has("position"):
		result.position = end
	return result


# -- Debug


## Visualizes a 3d point in world.
## Overlay with no depth test, which can be both a feature and a limitation.
#func debug_draw_point3d(
#	point3d: Vector3, allow_behind := false, duration := 3., color := Color.OLIVE
#) -> void:
#	var ui_debugdraw := GameUI.get_ovl_debugdraw()
#	if !is_instance_valid(ui_debugdraw):
#		push_error("Cannot draw debug line! No valid overlay!")
#		return
#	if _camera.is_position_behind(point3d) and not allow_behind:
#		return
#	var point := _camera.unproject_position(point3d)
#	if is_nan(point.x) or is_nan(point.y):
#		point = Vector2.ZERO
#	ui_debugdraw.add_point(point, duration, color)


## Visualizes a 3d line in world.
## Overlay with no depth test, which can be both a feature and a limitation.
#func debug_draw_line3d(
#	start3d: Vector3, end3d: Vector3, allow_behind := false, duration := 3., color := Color.OLIVE
#) -> void:
#	var ui_debugdraw := GameUI.get_ovl_debugdraw()
#	if !is_instance_valid(ui_debugdraw):
#		push_error("Cannot draw debug line! No valid overlay!")
#		return
#	if (
#		(_camera.is_position_behind(start3d) or _camera.is_position_behind(end3d))
#		and not allow_behind
#	):
#		return
#	var start := _camera.unproject_position(start3d)
#	var end := _camera.unproject_position(end3d)
#	if is_nan(start.x) or is_nan(start.y):
#		start = Vector2.ZERO
#	if is_nan(end.x) or is_nan(end.y):
#		end = Vector2.ZERO
#	ui_debugdraw.add_line(start, end, duration, color)


# --- Private --- #


## Updates variables based on profile & profile override.
func _update_profile_params():
	if _profile == null:
		return

	# No override, use profile as is.
	if !is_instance_valid(_profile_override):
		_max_pitch_angle = _profile.max_pitch_angle
		_min_pitch_angle = _profile.min_pitch_angle
		_camera_distance = _profile.camera_distance
		_camera_offset = _profile.offset
		return

	# Yes override, interpolate
	var t := smoothstep(0, 1, _profile_time / _profile_interpolate_duration)
	t = clamp(t, 0, 1)

	if _profile_returning:
		t = 1 - t

	_max_pitch_angle = lerp(_profile.max_pitch_angle, _profile_override.max_pitch_angle, t)
	_min_pitch_angle = lerp(_profile.min_pitch_angle, _profile_override.min_pitch_angle, t)
	_camera_distance = lerp(_profile.camera_distance, _profile_override.camera_distance, t)
	_camera_offset = _profile.offset.lerp(_profile_override.offset, t)


func _setup_camera_arm():
	_pivot.name = "pivot"
	_camera.name = "camera"
	add_child(_pivot)
	_pivot.add_child(_camera)
	_camera.current = true  # This may or may not be the wanted behaviour.


func _update_camera_arm():
	if is_instance_valid(target):
		var off := transform.basis * _camera_offset
		global_position = target.global_position + off

	# Raycast backwards to check if there's a wall in the way of the imaginary camera arm and
	# update camera distance accordingly.
	var end := global_position + transform.basis.z * _camera_distance
	var query := PhysicsRayQueryParameters3D.create(global_position, end, _CAMERA_COLL_MASK)
	var result := get_world_3d().direct_space_state.intersect_ray(query)
	var distance := _camera_distance

	# Limit distance if raycast hit a wall
	if result:
		distance = global_position.distance_to(result.position)

	_camera.position = Vector3(0, 0, distance)


## This will pull a camera profile json from config files.
## Empty indicates error.
func _get_profile_json(profilename: String) -> Dictionary:
	var path := _PROFILE_PATH + profilename + ".json"
	if not FileAccess.file_exists(path):
		push_error("Can't load camera profile: File doesn't exist:\n'", path, "'")
		return {}

	var file := FileAccess.open(path, FileAccess.READ)
	var json = JSON.parse_string(file.get_as_text())
	file.close()

	if json is Dictionary:
		return json

	push_error("Can't load camera profile: File was found, but could not be parsed:\n'", path, "'")
	return {}


## This will convert camera profile json to actual profile.
## These functions have been split because this part ot deserialization desperately needed tests.
func _json_to_profile(json: Dictionary) -> CameraProfile:
	if json.version != _PROFILE_VER:
		push_error(
			"Camera profile version mismatch! ",
			"Expected '",
			_PROFILE_VER,
			"', but got '",
			json.version,
			"'"
		)
		return null

	var profile = dict_to_inst(json)
	# Gotta convert String to Vector3 because dict_to_inst() is shallow.
	profile.offset = str_to_var("Vector3" + profile.offset)
	return profile


func _save_profile(profile: CameraProfile, profilename: String):
	var file = FileAccess.open(_PROFILE_PATH + profilename + ".json", FileAccess.WRITE)
	file.store_string(JSON.stringify(inst_to_dict(profile)))


# --- Classes --- #


## Camera profiles can be used to smoothly change between views.
## Example use: Far centered profile for outdoors, and closer, over-the-shoulder view for indoors.
class CameraProfile:
	extends Resource
	# This class might see some changes. This tells us if serialized files are out of date.
	var version := "1"
	var max_pitch_angle := 90.
	var min_pitch_angle := -90.
	var camera_distance := 10.
	var offset := Vector3.ZERO
