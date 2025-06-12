/**
*@ClassName DbTypes
*@Author cxk
*@Data 25-6-10 下午7:33
*/
//

#ifndef DBTYPES_H
#define DBTYPES_H

namespace cxk
{
namespace type
{
    enum FieldType
    {
        MySqlTiny,
        MySqlShort,
        MySqlLong,
        MySqlLongLong,
        MySqlNull,
        MySqlString,
        DrogonDefaultValue,
    };
}
}

#endif //DBTYPES_H
