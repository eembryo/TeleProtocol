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

#ifndef _SocketUtils_H_
#define _SocketUtils_H_

#include <glib.h>

G_BEGIN_DECLS

char *GetIpv4AddressForNetInterface(int sockfd, const char *ifcname);

G_END_DECLS
#endif
