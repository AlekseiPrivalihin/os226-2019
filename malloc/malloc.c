#define _GNU_SOURCE

#include <stdlib.h>
#include <stdio.h>
#include <dlfcn.h>
#include<string.h>

#define CONCAT(...) __VA_ARGS__

#if 1
#define FNX(_ret, _name, _decl, _call, ...) \
	static _ret (*_name ## _p)(_decl); \
	void _name ## _init(void) __attribute__((constructor)); \
	void _name ## _init(void) { \
		_name ## _p = dlsym(RTLD_NEXT, # _name); \
		fprintf(stderr, "init " # _name " %p\n", _name ## _p); \
	} \
        _ret _name (_decl) { \
		__VA_ARGS__ \
        }

char mem[1000000];
char *memPtr = mem;
void init(void) __attribute__((constructor));

FNX(void *, malloc,
		size_t size,
		size,
{
	fprintf(stderr, "%s size %lu\n", __func__, size);
	char *oldPtr = memPtr;
	memPtr += size;
	return oldPtr;
})

FNX(void, free,
		void *ptr,
		ptr,
{
	fprintf(stderr, "%s ptr %p\n", __func__, ptr);
})

FNX(void *, calloc,
		CONCAT(size_t nmemb, size_t size),
		CONCAT(nmemb, size),
{
	fprintf(stderr, "%s nmemb %lu size %lu\n", __func__, nmemb, size);
	char *oldPtr = memPtr;
	memPtr += size * nmemb;
	return oldPtr;
})

FNX(void *, realloc,
		CONCAT(void *ptr, size_t size),
		CONCAT(ptr, size),
{
	fprintf(stderr, "%s ptr %p size %lu\n", __func__, ptr, size);
	if (size == 0)
		return NULL;
	if (ptr == NULL) 
		return malloc(size);
	memcpy(memPtr, ptr, size);
	char *oldPtr = memPtr;
	memPtr += size;
	return oldPtr; 
})

FNX(void *, reallocarray,
		CONCAT(void *ptr, size_t nmemb, size_t size),
		CONCAT(ptr, nmemb, size),
{
	fprintf(stderr, "%s ptr %p nmemb %lu size %lu\n", __func__, ptr, nmemb, size);
	if (size == 0)
		return NULL;
	if (ptr == NULL) 
		return malloc(size * nmemb);
	memcpy(memPtr, ptr, size * nmemb);
	char *oldPtr = memPtr;
	memPtr += size * nmemb;
	return oldPtr; 
})

#else

#define MALLOCFN_X(x) \
        x(void *, malloc, 1, size_t, size, "%u") \
	x(void, free, 1, void *, ptr, "%p") \
	x(void *, calloc, 2, size_t, nmemb, "%u", size_t, size, "%u") \
	x(void *, realloc, 2, void *, ptr, "%p", size_t, size, "%p") \
	x(void *, reallocarray, 3, void *, ptr, "%p", size_t, nmemb, "%u", size_t, size, "%u")

#define DEFINE_(ret, name, dec) \
	ret (*name ## _p)(dec);
#define DEFINE1(ret, name, type1, name1, fmt1) \
	DEFINE_(ret, name, CONCAT(type1 name1))
#define DEFINE2(ret, name, type1, name1, fmt1, type2, name2, fmt2) \
	DEFINE_(ret, name, CONCAT(type1 name1, type2 name2))
#define DEFINE3(ret, name, type1, name1, fmt1, type2, name2, fmt2, type3, name3, fmt3) \
	DEFINE_(ret, name, CONCAT(type1 name1, type2 name2, type3 name3))
#define DEFINE(ret, name, n, ...) \
        DEFINE ## n (ret, name, ## __VA_ARGS__)
MALLOCFN_X(DEFINE)
#undef DEFINE_
#undef DEFINE1
#undef DEFINE2
#undef DEFINE3
#undef DEFINE

#define DEFINE_(ret, name, dec, call, fmt) \
	ret name (dec) { \
		fprintf(stderr, "%s " fmt "\n", __func__, call); \
		return (ret)0; /* FIXME implement me */ \
		return name ## _p(call); \
	}
#define DEFINE1(ret, name, type1, name1, fmt1) \
	DEFINE_(ret, name, CONCAT(type1 name1), CONCAT(name1), #name1 " " fmt1)
#define DEFINE2(ret, name, type1, name1, fmt1, type2, name2, fmt2) \
	DEFINE_(ret, name, CONCAT(type1 name1, type2 name2), CONCAT(name1, name2), #name1 " " fmt1 " " #name2 " " fmt2)
#define DEFINE3(ret, name, type1, name1, fmt1, type2, name2, fmt2, type3, name3, fmt3) \
	DEFINE_(ret, name, CONCAT(type1 name1, type2 name2, type3 name3), CONCAT(name1, name2, name3), #name1 " " fmt1 #name2 " " fmt2 " " #name3 " " fmt3)
#define DEFINE(ret, name, n, ...) \
        DEFINE ## n (ret, name, ## __VA_ARGS__)
MALLOCFN_X(DEFINE)
#undef DEFINE_
#undef DEFINE1
#undef DEFINE2
#undef DEFINE3
#undef DEFINE

void init(void) __attribute__((constructor));
void init(void) {
#define INIT(ret, name, n, ...) \
	name ## _p = dlsym(RTLD_NEXT, # name);
MALLOCFN_X(INIT)
#undef INIT
}

#endif
