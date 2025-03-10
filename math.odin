package zf4

import "core:math"
import "core:mem"

Vec_2D :: [2]f32
Vec_3D :: [3]f32
Vec_4D :: [4]f32

Size_2D :: struct {
	x: u32,
	y: u32,
}

Rect :: struct {
	x:      f32,
	y:      f32,
	width:  f32,
	height: f32,
}

Rect_I :: struct {
	x:      i32,
	y:      i32,
	width:  i32,
	height: i32,
}

Rect_Edges :: struct {
	left:   f32,
	top:    f32,
	right:  f32,
	bottom: f32,
}

Matrix_4x4 :: struct {
	elems: [4][4]f32,
}

calc_rect_right :: proc(rect: Rect) -> f32 {
	return rect.x + rect.width
}

calc_rect_i_right :: proc(rect: Rect_I) -> i32 {
	return rect.x + rect.width
}

calc_rect_bottom :: proc(rect: Rect) -> f32 {
	return rect.y + rect.height
}

calc_rect_i_bottom :: proc(rect: Rect_I) -> i32 {
	return rect.y + rect.height
}

init_iden_matrix_4x4 :: proc(mat: ^Matrix_4x4) {
	mem.zero(mat, size_of(mat^))
	mat.elems[0][0] = 1.0
	mat.elems[1][1] = 1.0
	mat.elems[2][2] = 1.0
	mat.elems[3][3] = 1.0
}

init_ortho_matrix_4x4 :: proc(
	mat: ^Matrix_4x4,
	left: f32,
	right: f32,
	bottom: f32,
	top: f32,
	near: f32,
	far: f32,
) {
	mem.zero(mat, size_of(mat^))
	mat.elems[0][0] = 2.0 / (right - left)
	mat.elems[1][1] = 2.0 / (top - bottom)
	mat.elems[2][2] = -2.0 / (far - near)
	mat.elems[3][0] = -(right + left) / (right - left)
	mat.elems[3][1] = -(top + bottom) / (top - bottom)
	mat.elems[3][2] = -(far + near) / (far - near)
	mat.elems[3][3] = 1.0
}

calc_mag :: proc(vec: Vec_2D) -> f32 {
	return math.sqrt((vec.x * vec.x) + (vec.y * vec.y))
}

calc_normal_or_zero :: proc(vec: Vec_2D) -> Vec_2D {
	mag := calc_mag(vec)

	if mag == 0.0 {
		return {}
	}

	return {vec.x / mag, vec.y / mag}
}

calc_dist :: proc(a: Vec_2D, b: Vec_2D) -> f32 {
	return calc_mag(a - b)
}

calc_dir :: proc(vec: Vec_2D) -> f32 {
	return math.atan2(vec.y, vec.x)
}

