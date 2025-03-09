package zf4

import "core:mem"

Vec_2D :: struct {
	x: f32,
	y: f32,
}

Vec_3D :: struct {
	x: f32,
	y: f32,
	z: f32,
}

Vec_4D :: struct {
	x: f32,
	y: f32,
	z: f32,
	w: f32,
}

Size_2D :: struct {
	x: u32,
	y: u32,
}

Matrix_4x4 :: struct {
	elems: [4][4]f32
}

init_iden_matrix_4x4 :: proc(mat: ^Matrix_4x4) {
	mem.zero(mat, size_of(mat^))
	mat.elems[0][0] = 1.0
	mat.elems[1][1] = 1.0
	mat.elems[2][2] = 1.0
	mat.elems[3][3] = 1.0
}
