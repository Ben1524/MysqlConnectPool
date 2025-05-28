//
// Created by cxk_zjq on 25-5-27.
//

#ifndef RESULTIMPL_H
#define RESULTIMPL_H

#include "NonCopyable.h"
#include "Result.h"

namespace cxk
{
    class ResultImpl : public cxk::NonCopyable
    {
    public:
        ResultImpl() = default;
        using SizeType = Result::SizeType;
        using RowSizeType = Result::RowSizeType;
        using FieldSizeType = Result::FieldSizeType;
        virtual SizeType size() const noexcept = 0;
        virtual RowSizeType columns() const noexcept = 0;
        virtual const char *columnName(RowSizeType Number) const = 0;
        virtual SizeType affectedRows() const noexcept = 0;
        virtual RowSizeType columnNumber(const char colName[]) const = 0;
        virtual const char *getValue(SizeType row, RowSizeType column) const = 0;
        virtual bool isNull(SizeType row, RowSizeType column) const = 0;
        virtual FieldSizeType getLength(SizeType row, RowSizeType column) const = 0;

        virtual unsigned long long insertId() const noexcept
        {
            return 0;
        }

        virtual int oid(RowSizeType column) const
        {
            (void)column;
            return 0;
        }

        virtual ~ResultImpl()
        {
        }
    };

}  // namespace orm

#endif //RESULTIMPL_H
