//
// Created by cxk_zjq on 25-5-27.
//

#include "Exception.h"

using namespace cxk;

Failure::Failure(const std::string &whatarg) : std::runtime_error(whatarg)
{
}

BrokenConnection::BrokenConnection() : Failure("Connection to database failed")
{
}

BrokenConnection::BrokenConnection(const std::string &whatarg)
    : Failure(whatarg)
{
}

SqlError::SqlError(const std::string &whatarg,
                   const std::string &Q,
                   const char sqlstate[])
    : Failure(whatarg),
      query_(Q),
      sqlState_(sqlstate ? sqlstate : ""),
      errcode_(0),
      extendedErrcode_(0)
{
}

SqlError::SqlError(const std::string &whatarg,
                   const std::string &Q,
                   const int errcode,
                   const int extendedErrcode)
    : Failure(whatarg),
      query_(Q),
      sqlState_(""),
      errcode_(errcode),
      extendedErrcode_(extendedErrcode)
{
}

SqlError::~SqlError() noexcept
{
}

const std::string &SqlError::query() const noexcept
{
    return query_;
}

const std::string &SqlError::sqlState() const noexcept
{
    return sqlState_;
}

int SqlError::errcode() const noexcept
{
    return errcode_;
}

int SqlError::extendedErrcode() const noexcept
{
    return extendedErrcode_;
}

InDoubtError::InDoubtError(const std::string &whatarg) : Failure(whatarg)
{
}

TransactionRollback::TransactionRollback(const std::string &whatarg)
    : Failure(whatarg)
{
}

SerializationFailure::SerializationFailure(const std::string &whatarg)
    : TransactionRollback(whatarg)
{
}

StatementCompletionUnknown::StatementCompletionUnknown(
    const std::string &whatarg)
    : TransactionRollback(whatarg)
{
}

DeadlockDetected::DeadlockDetected(const std::string &whatarg)
    : TransactionRollback(whatarg)
{
}

InternalError::InternalError(const std::string &whatarg)
    : logic_error("drogon database internal error: " + whatarg)
{
}

TimeoutError::TimeoutError(const std::string &whatarg) : logic_error(whatarg)
{
}

UsageError::UsageError(const std::string &whatarg) : logic_error(whatarg)
{
}

ArgumentError::ArgumentError(const std::string &whatarg)
    : invalid_argument(whatarg)
{
}

ConversionError::ConversionError(const std::string &whatarg)
    : domain_error(whatarg)
{
}

RangeError::RangeError(const std::string &whatarg) : out_of_range(whatarg)
{
}
