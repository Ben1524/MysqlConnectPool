/*
 * Copyright (c) 2015, 2024, Oracle and/or its affiliates.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0, as
 * published by the Free Software Foundation.
 *
 * This program is designed to work with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms, as
 * designated in a particular file or component or in included license
 * documentation. The authors of MySQL hereby grant you an additional
 * permission to link the program and your derivative works with the
 * separately licensed software that they have either included with
 * the program or referenced in the documentation.
 *
 * Without limiting anything contained in the foregoing, this file,
 * which is part of Connector/C++, is also subject to the
 * Universal FOSS Exception, version 1.0, a copy of which can be found at
 * https://oss.oracle.com/licenses/universal-foss-exception.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef MYSQL_COMMON_H
#define MYSQL_COMMON_H

#undef min
#undef max

//disable c++17 iterator warnings
#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

/*
  Common definitions and declarations that are needed by public headers.

  Note: Any common stuff that is needed only by the implementation, should be
  kept in the common/ folder, either as headers or source files.
*/

#define NOEXCEPT noexcept

#include "common_constants.h"
#include "third/mysqlx/common/api.h"
#include "third/mysqlx/common/error.h"
#include "third/mysqlx/common/util.h"
#include "third/mysqlx/common/value.h"
#include "third/mysqlx/common/settings.h"

PUSH_SYS_WARNINGS
#include <cassert>
POP_SYS_WARNINGS

/*
  On Windows, dependency on the sockets library can be handled using
  #pragma comment directive.
*/

#ifdef _WIN32
#pragma comment(lib,"ws2_32")
#endif


#endif
