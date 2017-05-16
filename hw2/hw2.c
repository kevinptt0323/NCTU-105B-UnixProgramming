#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>

#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>

#define declare(FUNC_TYPE, FUNC_NAME, ...) \
	typedef FUNC_TYPE(*T_##FUNC_NAME)(__VA_ARGS__); \
	T_##FUNC_NAME old_##FUNC_NAME


#define load(FUNC_NAME) old_##FUNC_NAME = (T_##FUNC_NAME)dlsym(RTLD_NEXT, #FUNC_NAME)

#define wrap(FUNC_TYPE, FUNC_NAME, FMT1, FMT2, ...) \
	if (old_##FUNC_NAME==NULL) hijack_init(); \
	FUNC_TYPE ret = old_##FUNC_NAME(__VA_ARGS__); \
	fprintf(debug_output, "[monitor] " #FUNC_NAME "(" FMT1 ") = " FMT2 "\n", __VA_ARGS__, ret); \
	return ret

#define wrap_noarg(FUNC_TYPE, FUNC_NAME, FMT2) \
	if (old_##FUNC_NAME==NULL) hijack_init(); \
	FUNC_TYPE ret = old_##FUNC_NAME(); \
	fprintf(debug_output, "[monitor] " #FUNC_NAME "() = " FMT2 "\n", ret); \
	return ret

#define wrap_void(FUNC_NAME, FMT1, ...) \
	if (old_##FUNC_NAME==NULL) hijack_init(); \
	old_##FUNC_NAME(__VA_ARGS__); \
	fprintf(debug_output, "[monitor] " #FUNC_NAME "(" FMT1 ")\n", ##__VA_ARGS__)

#define wrap_noprint(FUNC_TYPE, FUNC_NAME, ...) \
	if (old_##FUNC_NAME==NULL) hijack_init(); \
	FUNC_TYPE ret = old_##FUNC_NAME(__VA_ARGS__)

__attribute__((constructor)) void hijack_init();

FILE* debug_output;

char __buf__[1024];
char __buf2__[1024];
void get_fd_name(int, char*);
void get_file_name(FILE*, char*);
void get_dir_name(DIR*, char*);

declare(int, closedir, DIR*);
int closedir(DIR *dirp) {
	wrap_noprint(int, closedir, dirp);
	get_dir_name(dirp, __buf__);
	fprintf(debug_output, "[monitor] closedir(%d(%s)) = %p\n", dirp, __buf__, ret);
	return ret;
}

declare(DIR*, fdopendir, int);
DIR *fdopendir(int fd) {
	wrap_noprint(DIR*, fdopendir, fd);
	get_fd_name(fd, __buf__);
	fprintf(debug_output, "[monitor] fdopendir(%d(%s)) = %p\n", fd, __buf__, ret);
	return ret;
}

declare(DIR*, opendir, const char*);
DIR *opendir(const char *name) {
	wrap(DIR*, opendir, "'%s'", "%p", name);
}

declare(struct dirent*, readdir, DIR*);
struct dirent *readdir (DIR *__dirp) {
	wrap_noprint(struct dirent*, readdir, __dirp);
	get_dir_name(__dirp, __buf__);
	fprintf(debug_output, "[monitor] readdir(%d(%s)) = %p\n", __dirp, __buf__, ret);
	return ret;
}

declare(int, readdir_r, DIR*, struct dirent*, struct dirent**);
int readdir_r(DIR *dirp, struct dirent *entry, struct dirent **result) {
	wrap_noprint(int, readdir_r, dirp, entry, result);
	get_dir_name(dirp, __buf__);
	fprintf(debug_output, "[monitor] readdir_r(%d(%s), %p, %p) = %d\n", dirp, __buf__, entry, result, ret);
	return ret;
}

declare(void, rewinddir, DIR*);
void rewinddir(DIR *dirp) {
	old_rewinddir(dirp);
	get_dir_name(dirp, __buf__);
	fprintf(debug_output, "[monitor] rewinddir(%d(%s))\n", dirp, __buf__);
}

declare(void, seekdir, DIR*, long);
void seekdir(DIR *dirp, long loc) {
	seekdir(dirp, loc);
	get_dir_name(dirp, __buf__);
	fprintf(debug_output, "[monitor] seekdir(%d(%s), %ld)\n", dirp, __buf__, loc);
}

declare(long, telldir, DIR*);
long telldir(DIR *dirp) {
	wrap_noprint(long, telldir, dirp);
	get_dir_name(dirp, __buf__);
	fprintf(debug_output, "[monitor] telldir(%d(%s)) = %ld\n", dirp, __buf__, ret);
	return ret;
}

declare(int, creat, const char*, mode_t);
int creat(const char *pathname, mode_t mode) {
	wrap(int, creat, "'%s', 0x%x", "%d", pathname, mode);
}

declare(int, open, const char*, int, mode_t);
int open(const char *pathname, int flags, mode_t mode) {
	wrap(int, open, "'%s', 0x%x, 0x%x", "%d", pathname, flags, mode);
}

declare(int, remove, const char*);
int remove(const char *pathname) {
	wrap(int, remove, "'%s'", "%d", pathname);
}

declare(int, rename, const char*, const char*);
int rename(const char *old, const char *new) {
	wrap(int, rename, "'%s', '%s'", "%d", old, new);
}

declare(void, setbuf, FILE*, char*);
void setbuf(FILE *stream, char *buf) {
	wrap_void(setbuf, "%p, %p", stream, buf);
}

declare(int, setvbuf, FILE*, char*, int, size_t);
int setvbuf(FILE *stream, char *buf, int mode, size_t size) {
	wrap(int, setvbuf, "%p, %p, %d, %lu", "%d", stream, buf, mode, size);
}

declare(char*, tempnam, const char*, const char*);
char *tempnam(const char *dir, const char *pfx) {
	wrap(char*, tempnam, "%s, %s", "%s", dir, pfx);
}

declare(FILE*, tmpfile, void);
FILE *tmpfile(void) {
	wrap_noarg(FILE*, tmpfile, "%p");
}

declare(char*, tmpnam, char*);
char *tmpnam(char *s) {
	wrap(char*, tmpnam, "'%s'", "'%s'", s);
}

declare(__attribute__ ((noreturn)) void, exit, int);
__attribute__ ((noreturn)) void exit(int status) {
	wrap_void(exit, "%d", status);
}

declare(char*, getenv, const char*);
char *getenv(const char *name) {
	wrap(char*, getenv, "'%s'", "'%s'", name);
}

declare(char*, mkdtemp, char*);
char *mkdtemp(char *template) {
	wrap(char*, mkdtemp, "'%s'", "'%s'", template);
}

declare(int, mkstemp, char*);
int mkstemp(char *template) {
	wrap(int, mkstemp, "'%s'", "%d", template);
}

declare(int, putenv, char*);
int putenv(char *string) {
	wrap(int, putenv, "'%s'", "%d", string);
}

declare(int, rand, void);
int rand(void) {
	wrap_noarg(int, rand, "%d");
}

declare(int, rand_r, unsigned int*);
int rand_r(unsigned int *seedp) {
	wrap(int, rand_r, "%p", "%d", seedp);
}

declare(int, setenv, const char*, const char*, int);
int setenv(const char *name, const char *value, int overwrite) {
	wrap(int, setenv, "'%s', '%s', %d", "%d", name, value, overwrite);
}

declare(void, srand, unsigned int);
void srand(unsigned int seed) {
	wrap_void(srand, "%u", seed);
}

declare(int, system, const char*);
int system(const char *command) {
	wrap(int, system, "'%s'", "%d", command);
}

declare(int, chdir, const char*);
int chdir(const char *path) {
	wrap(int, chdir, "'%s'", "%d", path);
}

declare(int, chown, const char*, uid_t, gid_t);
int chown(const char *path, uid_t owner, gid_t group) {
	wrap(int, chown, "'%s', %d, %d", "%d", path, owner, group);
}

declare(int, close, int);
int close(int fildes) {
	wrap(int, close, "%d", "%d", fildes);
}

declare(int, dup, int);
int dup(int oldfd) {
	wrap_noprint(int, dup, oldfd);
	get_fd_name(oldfd, __buf__);
	fprintf(debug_output, "[monitor] dup(%d(%s)) = %d\n", oldfd, __buf__, ret);
	return ret;
}

declare(int, dup2, int, int);
int dup2(int oldfd, int newfd) {
	wrap_noprint(int, dup2, oldfd, newfd);
	get_fd_name(oldfd, __buf__);
	get_fd_name(newfd, __buf2__);
	fprintf(debug_output, "[monitor] dup(%d(%s), %d(%s)) = %d\n", oldfd, __buf__, newfd, __buf2__, ret);
	return ret;
}

declare(__attribute__((noreturn)) void, _exit, int);
__attribute__((noreturn)) void _exit(int status) {
	wrap_void(_exit, "%d", status);
}

declare(int, execl, const char*, const char*, ...);
int execl(const char *path, const char *arg, ...) {
	wrap(int, execl, "'%s', '%s'", "%d", path, arg);
}

declare(int, execle, const char*, const char*, ...);
int execle(const char *path, const char *arg, ...) {
	wrap(int, execle, "'%s', '%s'", "%d", path, arg);
}

declare(int, execlp, const char*, const char*, ...);
int execlp(const char *file, const char *arg, ...) {
	wrap(int, execlp, "'%s', '%s'", "%d", file, arg);
}

declare(int, execv, const char*, char* const*);
int execv(const char *path, char *const argv[]) {
	wrap(int, execv, "'%s', %p", "%d", path, argv);
}

declare(int, execve, const char*, char* const*, char* const*);
int execve(const char *filename, char *const argv[], char *const envp[]) {
	wrap(int, execve, "'%s', %p, %p", "%d", filename, argv, envp);
}

declare(int, execvp, const char*, char* const*);
int execvp(const char *file, char *const argv[]) {
	wrap(int, execvp, "'%s', %p", "%d", file, argv);
}

declare(int, fchdir, int);
int fchdir(int fd) {
	wrap_noprint(int, fchdir, fd);
	get_fd_name(fd, __buf__);
	fprintf(debug_output, "[monitor] fchdir(%d(%s)) = %d\n", fd, __buf__, ret);
	return ret;
}

declare(int, fchown, int, uid_t, gid_t);
int fchown(int fd, uid_t owner, gid_t group) {
	wrap_noprint(int, fchown, fd, owner, group);
	get_fd_name(fd, __buf__);
	struct passwd* pw = getpwuid(owner);
	struct group* gr = getgrgid(group);
	fprintf(debug_output, "[monitor] fchdir(%d(%s), %d(%s), %d(%s)) = %d\n", fd, __buf__, owner, pw->pw_name, group, gr->gr_name, ret);
	return ret;
}

declare(pid_t, fork, void);
pid_t fork(void) {
	wrap_noarg(pid_t, fork, "%d");
}

declare(int, fsync, int);
int fsync(int fd) {
	wrap_noprint(int, fsync, fd);
	get_fd_name(fd, __buf__);
	fprintf(debug_output, "[monitor] fsync(%d(%s)) = %d\n", fd, __buf__, ret);
	return ret;
}

declare(int, ftruncate, int, off_t);
int ftruncate(int fd, off_t length) {
	wrap_noprint(int, ftruncate, fd, length);
	get_fd_name(fd, __buf__);
	fprintf(debug_output, "[monitor] ftruncate(%d(%s), %ld) = %d\n", fd, __buf__, length, ret);
	return ret;
}

declare(char*, getcwd, char*, size_t);
char *getcwd(char *buf, size_t size) {
	wrap(char*, getcwd, "%p, %lu", "'%s'", buf, size);
}

declare(gid_t, getegid, void);
gid_t getegid(void) {
	wrap_noprint(gid_t, getegid);
	struct group* gr = getgrgid(ret);
	fprintf(debug_output, "[monitor] getegid() = %d(%s)\n", ret, gr->gr_name); \
	return ret;
}

declare(uid_t, geteuid, void);
uid_t geteuid(void) {
	wrap_noprint(uid_t, geteuid);
	struct passwd* pw = getpwuid(ret);
	fprintf(debug_output, "[monitor] geteuid() = %d(%s)\n", ret, pw->pw_name); \
	return ret;
}

declare(gid_t, getgid, void);
gid_t getgid(void) {
	wrap_noprint(gid_t, getgid);
	struct group* gr = getgrgid(ret);
	fprintf(debug_output, "[monitor] getgid() = %d(%s)\n", ret, gr->gr_name); \
	return ret;
}

declare(uid_t, getuid, void);
uid_t getuid(void) {
	wrap_noprint(uid_t, getuid);
	struct passwd* user = getpwuid(ret);
	fprintf(debug_output, "[monitor] getuid() = %d(%s)\n", ret, user->pw_name); \
	return ret;
}

declare(int, link, const char*, const char*);
int link(const char *path1, const char *path2) {
	wrap(int, link, "'%s', '%s'", "%d", path1, path2);
}

declare(int, pipe, int[2]);
int pipe(int fd[2]) {
	wrap(int, pipe, "%p", "%d", fd);
}

declare(ssize_t, pread, int, void*, size_t, off_t);
ssize_t pread(int fd, void *buf, size_t nbyte, off_t offset) {
	wrap_noprint(ssize_t, pread, fd, buf, nbyte, offset);
	get_fd_name(fd, __buf__);
	fprintf(debug_output, "[monitor] pread(%d(%s), %p, %lu, %ld) = %ld\n", fd, __buf__, buf, nbyte, offset, ret);
	return ret;
}

declare(ssize_t, pwrite, int, const void*, size_t, off_t);
ssize_t pwrite(int fd, const void *buf, size_t nbyte, off_t offset) {
	wrap_noprint(ssize_t, pwrite, fd, buf, nbyte, offset);
	get_fd_name(fd, __buf__);
	fprintf(debug_output, "[monitor] pwrite(%d(%s), %p, %lu, %ld) = %ld\n", fd, __buf__, buf, nbyte, offset, ret);
	return ret;
}

declare(ssize_t, read, int, void*, size_t);
ssize_t read(int fd, void *buf, size_t nbyte) {
	wrap_noprint(ssize_t, read, fd, buf, nbyte);
	get_fd_name(fd, __buf__);
	fprintf(debug_output, "[monitor] read(%d(%s), %p, %lu) = %ld\n", fd, __buf__, buf, nbyte, ret);
	return ret;
}

declare(ssize_t, readlink, const char* restrict, char *restrict, size_t);
ssize_t readlink(const char *restrict path, char *restrict buf, size_t bufsize) {
	wrap(ssize_t, readlink, "'%s', %p, %lu", "%ld", path, buf, bufsize);
}

declare(int, rmdir, const char*);
int rmdir(const char *path) {
	wrap(int, rmdir, "'%s'", "%d", path);
}

declare(int, setegid, gid_t);
int setegid(gid_t egid) {
	wrap(int, setegid, "%d", "%d", egid);
}

declare(int, seteuid, uid_t);
int seteuid(uid_t euid) {
	wrap(int, seteuid, "%d", "%d", euid);
}

declare(int, setgid, gid_t);
int setgid(gid_t gid) {
	wrap(int, setgid, "%d", "%d", gid);
}

declare(int, setuid, uid_t);
int setuid(uid_t uid) {
	wrap(int, setuid, "%d", "%d", uid);
}

declare(unsigned int, sleep, unsigned int);
unsigned int sleep(unsigned int seconds) {
	wrap(unsigned int, sleep, "%u", "%u", seconds);
}

declare(int, symlink, const char*, const char*);
int symlink(const char *path1, const char *path2) {
	wrap(int, symlink, "'%s', '%s'", "%d", path1, path2);
}

declare(int, unlink, const char*);
int unlink(const char *path) {
	wrap(int, unlink, "'%s'", "%d", path);
}

declare(ssize_t, write, int, const void*, size_t);
ssize_t write(int fd, const void *buf, size_t nbyte) {
	wrap_noprint(ssize_t, write, fd, buf, nbyte);
	get_fd_name(fd, __buf__);
	fprintf(debug_output, "[monitor] write(%d(%s), %p, %lu) = %ld\n", fd, __buf__, buf, nbyte, ret);
	return ret;
}

declare(int, chmod, const char*, mode_t);
int chmod(const char *path, mode_t mode) {
	wrap(int, chmod, "'%s', %d", "%d", path, mode);
}

declare(int, fchmod, int, mode_t);
int fchmod(int fd, mode_t mode) {
	wrap_noprint(int, fchmod, fd, mode);
	get_fd_name(fd, __buf__);
	fprintf(debug_output, "[monitor] fchmod(%d(%s), 0x%x) = %d\n", fd, __buf__, mode, ret);
	return ret;
}

declare(int, fstat, int, struct stat*);
int fstat(int fd, struct stat *buf) {
	wrap_noprint(int, fstat, fd, buf);
	get_fd_name(fd, __buf__);
	fprintf(debug_output, "[monitor] fstat(%d(%s), %p) = %d\n", fd, __buf__, buf, ret);
	return ret;
}

declare(int, __fxstat, int, int, struct stat*);
int __fxstat(int ver, int fd, struct stat *buf) {
	wrap_noprint(int, __fxstat, ver, fd, buf);
	get_fd_name(fd, __buf__);
	fprintf(debug_output, "[monitor] __fxstat(%d, %d(%s), %p) = %d\n", ver, fd, __buf__, buf, ret);
	return ret;
}

declare(int, lstat, const char*, struct stat*);
int lstat(const char *path, struct stat *buf) {
	wrap(int, lstat, "'%s', %p", "%d", path, buf);
}

declare(int, __lxstat, int, const char*, struct stat*);
int __lxstat (int ver, const char *path, struct stat *buf) {
	wrap(int, __lxstat, "%d, '%s', %p", "%d", ver, path, buf);
}

declare(int, mkdir, const char*, mode_t);
int mkdir(const char *path, mode_t mode) {
	wrap(int, mkdir, "'%s', %d", "%d", path, mode);
}

declare(int, mkfifo, const char*, mode_t);
int mkfifo(const char *pathname, mode_t mode) {
	wrap(int, mkfifo, "'%s', %d", "%d", pathname, mode);
}

declare(int, stat, const char* restrict, struct stat* restrict);
int stat(const char *restrict path, struct stat *restrict buf) {
	wrap(int, stat, "'%s', %p", "%d", path, buf);
}

declare(int, __xstat, int, const char*, struct stat*);
int __xstat (int vers, const char *path, struct stat *buf) {
	wrap(int, __xstat, "%d, '%s', %p", "%d", vers, path, buf);
}

declare(mode_t, umask, mode_t);
mode_t umask(mode_t cmask) {
	wrap(mode_t, umask, "%d", "%d", cmask);
}

declare(ssize_t, recv, int, void*, size_t, int);
ssize_t recv(int sockfd, void *buf, size_t len, int flags) {
	wrap(ssize_t, recv, "%d, %p, %lu, 0x%x", "%ld", sockfd, buf, len, flags);
}

declare(ssize_t, recvfrom, int, void*, size_t, int, struct sockaddr*, socklen_t*);
ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
	wrap(ssize_t, recvfrom, "%d, %p, %lu, 0x%x, %p, %p", "%ld", sockfd, buf, len, flags, src_addr, addrlen);
}

declare(ssize_t, recvmsg, int, struct msghdr*, int);
ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags) {
	wrap(ssize_t, recvmsg, "%d, %p, 0x%x", "%ld", sockfd, msg, flags);
}

declare(ssize_t, send, int, const void*, size_t, int);
ssize_t send(int sockfd, const void *buf, size_t len, int flags) {
	wrap(ssize_t, send, "%d, %p, %lu, 0x%x", "%ld", sockfd, buf, len, flags);
}

declare(ssize_t, sendto, int, const void*, size_t, int, const struct sockaddr*, socklen_t);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {
	wrap(ssize_t, sendto, "%d, %p, %lu, 0x%x, %p, %d", "%ld", sockfd, buf, len, flags, dest_addr, addrlen);
}

declare(ssize_t, sendmsg, int, const struct msghdr*, int);
ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
	wrap(ssize_t, sendmsg, "%d, %p, 0x%x", "%ld", sockfd, msg, flags);
}

void get_fd_name(int fd, char *buf) {
	static char path[64];
	sprintf(path, "/proc/self/fd/%d", fd);
	ssize_t len = old_readlink(path, buf, 1024);
	buf[len] = '\0';
}
void get_file_name(FILE* f, char *buf) {
	get_fd_name(fileno(f), buf);
}

void get_dir_name(DIR* d, char *buf) {
	get_fd_name(dirfd(d), buf);
}

__attribute__((constructor)) void hijack_init() {
	static int called = 0;
	if (called) return;
	called = 1;
	load(closedir);
	load(fdopendir);
	load(opendir);
	load(readdir);
	load(readdir_r);
	load(rewinddir);
	load(seekdir);
	load(telldir);

	load(creat);
	load(open);
	load(remove);
	load(rename);
	load(setbuf);
	load(setvbuf);
	load(tempnam);
	load(tmpfile);

	load(tmpnam);
	load(exit);
	load(getenv);
	load(mkdtemp);
	load(mkstemp);
	load(putenv);
	load(rand);
	load(rand_r);

	load(setenv);
	load(srand);
	load(system);
	load(chdir);
	load(chown);
	load(close);
	load(dup);
	load(dup2);

	load(_exit);
	load(execl);
	load(execle);
	load(execlp);
	load(execv);
	load(execve);
	load(execvp);
	load(fchdir);

	load(fchown);
	load(fork);
	load(fsync);
	load(ftruncate);
	load(getcwd);
	load(getegid);
	load(geteuid);
	load(getgid);

	load(getuid);
	load(link);
	load(pipe);
	load(pread);
	load(pwrite);
	load(read);
	load(readlink);
	load(rmdir);

	load(setegid);
	load(seteuid);
	load(setgid);
	load(setuid);
	load(sleep);
	load(symlink);
	load(unlink);
	load(write);

	load(chmod);
	load(fchmod);
	load(fstat); load(__fxstat);
	load(lstat); load(__lxstat);
	load(mkdir);
	load(mkfifo);
	load(stat); load(__xstat);
	load(umask);

	load(recv);
	load(recvfrom);
	load(recvmsg);
	load(send);
	load(sendto);
	load(sendmsg);

	char* filename = old_getenv("MONITOR_OUTPUT");
	if (filename==NULL) debug_output = fopen("monitor.out", "w");
	else if (!strcmp(filename, "stdout")) debug_output = stdout;
	else if (!strcmp(filename, "stderr")) debug_output = stderr;
	else debug_output = fopen(filename, "w");
}
