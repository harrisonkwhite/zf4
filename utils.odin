package zf4

cnt_num_lines :: proc(str: string) -> int {
	cnt := 1

	for c in str {
		if c == '\n' {
			cnt += 1
		}
	}

	return cnt
}
