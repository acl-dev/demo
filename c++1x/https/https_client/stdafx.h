// stdafx.h : 标准系统包含文件的包含文件，
// 或是常用但不常更改的项目特定的包含文件
//

#pragma once


//#include <iostream>
//#include <tchar.h>

// TODO: 在此处引用程序要求的附加头文件

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <acl-lib/acl_cpp/lib_acl.hpp>
#include <acl-lib/fiber/lib_fiber.hpp>
#include <acl-lib/fiber/go_fiber.hpp>

#ifdef	WIN32
#define	snprintf _snprintf
#endif

