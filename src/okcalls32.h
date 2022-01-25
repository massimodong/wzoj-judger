/*
 * okcalls32.h
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
int LANG_CV[256] = { 85, 8,140,146, SYS_time, SYS_read, SYS_uname, SYS_write, SYS_open,
		SYS_close, SYS_execve, SYS_access, SYS_brk, SYS_munmap, SYS_mprotect,
		SYS_mmap2, SYS_fstat64, SYS_set_thread_area, 252, 0 };

int LANG_PV[256] = { 0,9, 59, 97, 13, 16, 89, 140, 91, 175, 195, 13, SYS_open, SYS_set_thread_area,
		SYS_brk, SYS_read, SYS_uname, SYS_write, SYS_execve, SYS_ioctl,
		SYS_readlink, SYS_mmap, SYS_rt_sigaction, SYS_getrlimit, 252, 191, 0 };

int LANG_YV[256] = {3,4,5,6,11,33,45,54,85,91,122,125,140,174,175,183,191,192,195,
		    196,197,199,200,201,202,220,240,243,252,258,295,311, 146, 
		    SYS_mremap, 158, 117, 60, 39, 102, SYS_access,
		SYS_brk, SYS_close, SYS_execve, SYS_exit_group, SYS_fcntl64,
		SYS_fstat64, SYS_futex, SYS_getcwd, SYS_getdents64, SYS_getegid32,
		SYS_geteuid32, SYS_getgid32, SYS_getrlimit, SYS_getuid32, SYS_ioctl,
		SYS__llseek, SYS_lstat64, SYS_mmap2, SYS_mprotect, SYS_munmap, SYS_open,
		SYS_read, SYS_readlink, SYS_rt_sigaction, SYS_rt_sigprocmask,
		SYS_set_robust_list, SYS_set_thread_area, SYS_set_tid_address,
		SYS_stat64, SYS_uname, SYS_write,0 };