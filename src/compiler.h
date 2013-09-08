/*
 * Copyright 2013 Julian Maurice
 *
 * This file is part of libio
 *
 * libio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libio.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef libio_compiler_h_included
#define libio_compiler_h_included

char *
io_compile(
	const char *template,
	const char *start_tag,
	const char *end_tag
);

char *
io_compile_filep(
	FILE *filep,
	const char *start_tag,
	const char *end_tag
);

char *
io_compile_file(
	const char *filename,
	const char *start_tag,
	const char *end_tag
);

#endif /* ! libio_compiler_h_included */

