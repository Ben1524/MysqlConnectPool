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

#ifndef MYSQLX_DETAIL_ERROR_H
#define MYSQLX_DETAIL_ERROR_H

/**
  @file
  Classes used to access query and command execution results.
*/

#include "third/mysqlx/devapi/common.h"

//#include <memory>


namespace mysqlx {
MYSQLX_ABI_BEGIN(2,0)

namespace internal {

class Result_detail;

class Warning_detail
  : public virtual common::Printable
{
protected:

  byte     m_level;
  uint16_t m_code;
  string   m_msg;

  Warning_detail(Warning_detail&&) = default;
  Warning_detail(const Warning_detail&) = default;

  Warning_detail(byte level, uint16_t code, const std::string &msg)
    : m_level(level), m_code(code), m_msg(msg)
  {}

  void print(std::ostream &) const override;

  friend Result_detail;
};


}  // internal namespace
MYSQLX_ABI_END(2,0)
}  // mysqlx

#endif
