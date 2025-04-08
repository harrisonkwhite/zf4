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

Poly :: struct {
	pts: []Vec_2D,
}

to_vec_2d :: proc(vec: Vec_2D_I) -> Vec_2D {
	return {f32(vec.x), f32(vec.y)}
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

calc_rect_top_center :: proc(rect: Rect) -> Vec_2D {
	return {rect.x + (rect.width / 2.0), rect.y}
}

calc_rect_i_top_center :: proc(rect: Rect_I) -> Vec_2D_I {
	return {rect.x + (rect.width / 2), rect.y}
}

calc_rect_top_right :: proc(rect: Rect) -> Vec_2D {
	return {rect.x + rect.width, rect.y}
}

calc_rect_i_top_right :: proc(rect: Rect_I) -> Vec_2D_I {
	return {rect.x + rect.width, rect.y}
}

calc_rect_center_left :: proc(rect: Rect) -> Vec_2D {
	return {rect.x, rect.y + (rect.height / 2.0)}
}

calc_rect_i_center_left :: proc(rect: Rect_I) -> Vec_2D_I {
	return {rect.x, rect.y + (rect.height / 2)}
}

calc_rect_center :: proc(rect: Rect) -> Vec_2D {
	return {rect.x + (rect.width / 2.0), rect.y + (rect.height / 2.0)}
}

calc_rect_i_center :: proc(rect: Rect_I) -> Vec_2D_I {
	return {rect.x + (rect.width / 2), rect.y + (rect.height / 2)}
}

calc_rect_center_right :: proc(rect: Rect) -> Vec_2D {
	return {rect.x + rect.width, rect.y + (rect.height / 2.0)}
}

calc_rect_i_center_right :: proc(rect: Rect_I) -> Vec_2D_I {
	return {rect.x + rect.width, rect.y + (rect.height / 2)}
}

calc_rect_bottom_left :: proc(rect: Rect) -> Vec_2D {
	return {rect.x, rect.y + rect.height}
}

calc_rect_i_bottom_left :: proc(rect: Rect_I) -> Vec_2D_I {
	return {rect.x, rect.y + rect.height}
}

calc_rect_bottom_center :: proc(rect: Rect) -> Vec_2D {
	return {rect.x + (rect.width / 2.0), rect.y + rect.height}
}

calc_rect_i_bottom_center :: proc(rect: Rect_I) -> Vec_2D_I {
	return {rect.x + (rect.width / 2), rect.y + rect.height}
}

calc_rect_bottom_right :: proc(rect: Rect) -> Vec_2D {
	return {rect.x + rect.width, rect.y + rect.height}
}

calc_rect_i_bottom_right :: proc(rect: Rect_I) -> Vec_2D_I {
	return {rect.x + rect.width, rect.y + rect.height}
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

do_rects_inters :: proc(a: Rect, b: Rect) -> bool {
	return(
		a.x < b.x + b.width &&
		a.y < b.y + b.height &&
		a.x + a.width > b.x &&
		a.y + a.height > b.y \
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

calc_dot :: proc(a: Vec_2D, b: Vec_2D) -> f32 {
	return (a.x * b.x) + (a.y * b.y)
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
	return math.atan2(-vec.y, vec.x)
}

calc_len_dir :: proc(len: f32, dir: f32) -> Vec_2D {
	return {math.cos(dir), -math.sin(dir)} * len
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

alloc_quad_poly :: proc(
	pos: Vec_2D,
	size: Vec_2D,
	origin: Vec_2D,
	allocator := context.allocator,
) -> (
	Poly,
	bool,
) {
	poly := Poly {
		pts = make([]Vec_2D, 4, allocator = allocator),
	}

	if poly.pts == nil {
		return poly, false
	}

	pos_base := pos - {size.x * origin.x, size.y * origin.y}
	poly.pts[0] = pos_base
	poly.pts[1] = pos_base + {size.x, 0.0}
	poly.pts[2] = pos_base + {size.x, size.y}
	poly.pts[3] = pos_base + {0.0, size.y}

	return poly, true
}

alloc_quad_poly_rotated :: proc(
	pos: Vec_2D,
	size: Vec_2D,
	origin: Vec_2D,
	rot: f32,
	allocator := context.allocator,
) -> (
	Poly,
	bool,
) {
	poly := Poly {
		pts = make([]Vec_2D, 4, allocator = allocator),
	}

	if poly.pts == nil {
		return poly, false
	}

	left_offs := calc_len_dir(size.x * origin.x, rot + math.PI)
	up_offs := calc_len_dir(size.y * origin.y, rot + (math.PI / 2.0))
	right_offs := calc_len_dir(size.x * (1.0 - origin.x), rot)
	down_offs := calc_len_dir(size.y * (1.0 - origin.y), rot - (math.PI / 2.0))

	poly.pts[0] = pos + left_offs + up_offs
	poly.pts[1] = pos + right_offs + up_offs
	poly.pts[2] = pos + right_offs + down_offs
	poly.pts[3] = pos + left_offs + down_offs

	return poly, true
}

do_polys_inters :: proc(a: Poly, b: Poly) -> bool {
	return check_poly_sep(a, b) && check_poly_sep(b, a)
}

does_poly_inters_with_rect :: proc(poly: Poly, rect: Rect) -> bool {
	rect_poly_pts := [4]Vec_2D {
		{rect.x, rect.y},
		{rect.x + rect.width, rect.y},
		{rect.x + rect.width, rect.y + rect.height},
		{rect.x, rect.y + rect.height},
	}

	rect_poly := Poly {
		pts = rect_poly_pts[:],
	}

	return do_polys_inters(poly, rect_poly)
}

check_poly_sep :: proc(poly: Poly, other: Poly) -> bool {
	for i in 0 ..< len(poly.pts) {
		a := poly.pts[i]
		b := poly.pts[(i + 1) % len(poly.pts)]

		normal := Vec_2D{b.y - a.y, -(b.x - a.x)}

		poly_a_range_min, poly_a_range_max := project_pts(poly.pts, normal)
		poly_b_range_min, poly_b_range_max := project_pts(other.pts, normal)

		if (poly_a_range_max <= poly_b_range_min || poly_b_range_max <= poly_a_range_min) {
			return false
		}
	}

	return true
}

project_pts :: proc(pts: []Vec_2D, edge: Vec_2D) -> (f32, f32) {
	min := f32(math.F32_MAX)
	max := f32(math.F32_MIN)

	for i in 0 ..< len(pts) {
		dot := calc_dot(pts[i], edge)

		if (dot < min) {
			min = dot
		}

		if (dot > max) {
			max = dot
		}
	}

	return min, max
}

/*
poly_left :: proc(poly: Poly) -> f32 {

}

poly_right :: proc(poly: Poly) -> f32 {

}

poly_top :: proc(poly: Poly) -> f32 {

}

poly_bottom :: proc(poly: Poly) -> f32 {

}

is_poly_quad :: proc(poly: Poly) -> bool {

}

is_poly_quad_non_rot :: proc(poly: Poly) -> bool {

}*/

