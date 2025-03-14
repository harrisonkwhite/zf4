package zf4

import "core:math"
import "core:mem"

Vec_2D :: [2]f32
Vec_2D_I :: [2]int
Vec_3D :: [3]f32
Vec_4D :: [4]f32

Rect :: struct {
	x:      f32,
	y:      f32,
	width:  f32,
	height: f32,
}

Rect_I :: struct {
	x:      int,
	y:      int,
	width:  int,
	height: int,
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

is_size :: proc(vec: Vec_2D) -> bool {
	return vec.x >= 0.0 && vec.y >= 0.0
}

is_size_i :: proc(vec: Vec_2D_I) -> bool {
	return vec.x >= 0 && vec.y >= 0
}

calc_rect_pos :: proc(rect: Rect) -> Vec_2D {
	return {rect.x, rect.y}
}

calc_rect_i_pos :: proc(rect: Rect_I) -> Vec_2D_I {
	return {rect.x, rect.y}
}

calc_rect_size :: proc(rect: Rect) -> Vec_2D {
	return {rect.width, rect.height}
}

calc_rect_i_size :: proc(rect: Rect_I) -> Vec_2D_I {
	return {rect.width, rect.height}
}

calc_rect_right :: proc(rect: Rect) -> f32 {
	return rect.x + rect.width
}

calc_rect_i_right :: proc(rect: Rect_I) -> int {
	return rect.x + rect.width
}

calc_rect_bottom :: proc(rect: Rect) -> f32 {
	return rect.y + rect.height
}

calc_rect_i_bottom :: proc(rect: Rect_I) -> int {
	return rect.y + rect.height
}

translate_rect :: proc(rect: ^Rect, trans: Vec_2D) {
	assert(rect != nil)
	rect.x += trans.x
	rect.y += trans.y
}

translate_rect_i :: proc(rect: ^Rect_I, trans: Vec_2D_I) {
	assert(rect != nil)
	rect.x += trans.x
	rect.y += trans.y
}

is_point_in_rect :: proc(pt: Vec_2D, rect: Rect) -> bool {
	return(
		pt.x >= rect.x &&
		pt.y >= rect.y &&
		pt.x < calc_rect_right(rect) &&
		pt.y < calc_rect_bottom(rect) \
	)
}

gen_spanning_rect :: proc(rects: []Rect) -> Rect {
	assert(len(rects) > 0)

	span_edges: Rect_Edges

	for r in rects {
		span_edges.left = min(r.x, span_edges.left)
		span_edges.top = min(r.y, span_edges.top)
		span_edges.right = max(r.x + r.width, span_edges.right)
		span_edges.bottom = max(r.y + r.height, span_edges.bottom)
	}

	return {
		span_edges.left,
		span_edges.top,
		span_edges.right - span_edges.left,
		span_edges.bottom - span_edges.top,
	}
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
