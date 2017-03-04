/*
 * okcalls64.h
 *
 * Copyright (C) 2016 - Unknown
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

int LANG_CV[256] = {0,1,2,4,5,9,11,12,21,59,63,89,158,231,240, 8,
	 SYS_time, SYS_read, SYS_uname, SYS_write, SYS_open,
		SYS_close, SYS_execve, SYS_access, SYS_brk, SYS_munmap, SYS_mprotect,
		SYS_mmap, SYS_fstat, SYS_set_thread_area, 252, SYS_arch_prctl, 231, 0 };

int LANG_PV[256] = {0,1,2,4,9,11,13,16,59,89,97,201,231,
	SYS_open, SYS_set_thread_area, SYS_brk, SYS_read,
		SYS_uname, SYS_write, SYS_execve, SYS_ioctl, SYS_readlink, SYS_mmap,
		SYS_rt_sigaction, SYS_getrlimit, 252, 191, 158, 231, SYS_close,
		SYS_exit_group, SYS_munmap, SYS_time, 4, 0 };

int LANG_YV[256] = { 0,1,2,3,4,5,6,8,9,10,11,12,13,14,16,21,32,39,41,42,59,
	72,78,79,89,97,99,102,104,107,108,131,158,202,218,231,257,273
		, 146, SYS_mremap, 158, 117, 60, 102, 191,318,
		SYS_access, SYS_arch_prctl, SYS_brk, SYS_close, SYS_execve,
		SYS_exit_group, SYS_fcntl, SYS_fstat, SYS_futex, SYS_getcwd,
		SYS_getdents, SYS_getegid, SYS_geteuid, SYS_getgid, SYS_getrlimit,
		SYS_getuid, SYS_ioctl, SYS_lseek, SYS_lstat, SYS_mmap, SYS_mprotect,
		SYS_munmap, SYS_open, SYS_read, SYS_readlink, SYS_rt_sigaction,
		SYS_rt_sigprocmask, SYS_set_robust_list, SYS_set_tid_address, SYS_stat,
		SYS_write, 0 };