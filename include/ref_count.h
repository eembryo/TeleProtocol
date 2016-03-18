// @@@LICENSE
//
// Copyright (C) 2015, LG Electronics, All Right Reserved.
//
// No part of this source code may be communicated, distributed, reproduced
// or transmitted in any form or by any means, electronic or mechanical or
// otherwise, for any purpose, without the prior written permission of
// LG Electronics.
//
//
// design/author : hyobeom1.lee@lge.com
// date   : 12/30/2015
// Desc   :
//
// LICENSE@@@

#ifndef __ref_count_h__
#define __ref_count_h__


#include <glib.h>
#include <stdio.h>

G_BEGIN_DECLS

#ifndef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif
#endif

/**
 * container_of - cast a member of a structure out to the containing structure
 *
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({                      \
		typeof( ((type *)0)->member ) *__mptr = (ptr);		\
		(type *)( (char *)__mptr - offsetof(type,member) );})

/**
 * The parent structure, which include this "struct ref", should implement free() callback function.
 * The free() callback function obtains the address of parent structure from pointer of ref strucutre.
 *
 * For example,
 * struct pstr {
 *	struct ref;
 *	int ....;
 * };
 *
 * void free_callback(const struct ref *r) {
 *	struct pstr parent = container_of(r, struct pstr, ref);
 *
 * int main() { struct pstr parent; parent.ref.free = free_callback; ...}
 */
struct ref {
	void (*free)(struct ref *);
	int count;
};

static inline void
ref_inc(struct ref *ref)
{
	((struct ref *)ref)->count++;
}

static inline void
ref_dec(struct ref *ref)
{
	if (--((struct ref *)ref)->count == 0)
		ref->free(ref);
}

static inline int
get_ref_count(struct ref *ref)
{
	return ref->count;
}

static inline void
ref_init(struct ref *ref, void (*f)(struct ref *))
{
	((struct ref *)ref)->free = f;
	((struct ref *)ref)->count = 0;
}

G_END_DECLS

#endif //__ref_count_h__
