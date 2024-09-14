## Jump envelope class - part of player controller
## This can be used to determine vertical velocity.
##
## Overview of the envelope function:
##    .     .     .     .     .
##    .     .-----.     .     .      <- jump velocity
##    .    /.     .\    .     .
##    .  /  .     .  \  .     .
##  . ./. . . . . . . .\. . . . . . .<- zero velocity
##    .     .     .     .\    .
##    .     .     .     .  \  .
##    .     .     .     .    \._____ <- terminal velocity
##    |Rise |Sust.|Decay|Fall |after
##
## Vertical axis: Velocity
## Horizontal axis: Time

class_name PlayerJumpenv extends RefCounted

# Envelope start times
var t_start_rise: float:
	get:
		return 0.
var t_start_sust: float:
	get:
		return _rise_duration
var t_start_decay: float:
	get:
		return t_start_sust + _sust_duration
var t_start_fall: float:
	get:
		return t_start_decay + _decay_duration
var t_start_terminal: float:
	get:
		return t_start_fall + _fall_duration

# Velocity params
var _jump_velocity := 10.  ## Top speed upwards
var _terminal_velocity := -15  ## Fall speed limit. Notice that it's negative.

# Envelope duration params
var _rise_duration := 0.2  ## Rise: Time to reach max vel from zero
var _sust_duration := 1.0  ## Sustain: Time to hold max vel (mario jump holding his fist up state)
var _decay_duration := 0.2  ## Decay: Time to reach zero velocity from max velocity
var _fall_duration := 1.0  ## Fall: Time to reach terminal velocity from zero


## Vertical velocity envelope.
## Params: time since jump started, Returns: vertical velocity.
func get_velocity(time: float) -> float:
	## Rise
	if time < t_start_sust:
		var t = time / _rise_duration
		return _jump_velocity * t

	## Sustain
	if time < t_start_decay:
		# same as terminal velocity, but upwards.
		return _jump_velocity

	## Decay
	if time < t_start_fall:
		var t = (time - t_start_decay) / _decay_duration
		return lerpf(_jump_velocity, 0, t)

	## Fall
	if time < t_start_terminal:
		var t = (time - t_start_fall) / _fall_duration
		return lerpf(0, _terminal_velocity, t)

	## after
	return _terminal_velocity


## Reverse the above function (kind of, partially)
## Params: Vertical velocity
## Returns: Time (in decay phase) that results in this velocity.
## Useful for stopping a jump early: This gives you the exact time you need to skip to in order to
## seamlessly transition from rise/sust to decay.
func get_decay_t_from_v(v: float) -> float:
	# t_decay_left tells how much time to spend in decay to undo current velocity.
	var t_decay_left = (v / _jump_velocity) * _decay_duration
	var t = t_start_fall - t_decay_left
	return t
