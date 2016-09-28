//
// Created by owt50 on 2016/9/28.
//

#include <log/log_wrapper.h>
#include <common/string_oprs.h>

#include "unpack.h"

#include <protocol/pbdesc/svr.container.pb.h>
#include <protocol/pbdesc/svr.const.err.pb.h>

namespace rpc {
    namespace db {
        db_reply_t::db_reply_t(const void* d, size_t len, int t): data(d), length(len), type(t) {}

        namespace detail {
            int32_t do_nothing(hello::message_container& msgc, const redisReply* data){
                return moyo_no1::err::EN_SUCCESS;
            }

            int32_t unpack_integer(hello::message_container& msgc, const redisReply* data){
                if (NULL == data) {
                    WLOGDEBUG("data mot found.");
                    // 数据找不到，直接成功结束，外层会判为无数据
                    return moyo_no1::err::EN_SUCCESS;
                }

                if (REDIS_REPLY_STRING == data->type) {
                    // 坑爹的redis的数据库回包可能回字符串类型
                    int64_t d = 0;
                    util::string::str2int(d, data->str);
                    msgc.mutable_dbmsg()->mutable_simple()->set_msg_i64(d);
                } else {
                    switch (data.begin()->length) {
                        case sizeof(int64_t): {
                            msgc.mutable_dbmsg()->mutable_simple()->set_msg_i64(data->integer);
                            break;
                        }
                        case sizeof(int32_t): {
                            msgc.mutable_dbmsg()->mutable_simple()->set_msg_i32(data->integer);
                            break;
                        }

                        default:
                            break;
                    }
                }

                return moyo_no1::err::EN_SUCCESS;
            }

            int32_t unpack_str(hello::message_container& msgc, const redisReply* data){
                if (NULL == data) {
                    WLOGDEBUG("data mot found.");
                    // 数据找不到，直接成功结束，外层会判为无数据
                    return moyo_no1::err::EN_SUCCESS;
                }

                if (REDIS_REPLY_STRING != data->type && REDIS_REPLY_STATUS != data->type && REDIS_REPLY_ERROR != data->type) {
                    WLOGERROR("data type error, type=%d", data->type);
                    return moyo_no1::err::EN_SYS_PARAM;
                }

                msgc.mutable_dbmsg()->mutable_simple()->set_msg_str(data->str, data->len);
                return moyo_no1::err::EN_SUCCESS;
            }

            int32_t unpack_arr_str(hello::message_container& msgc, const redisReply* data){
                if (NULL == data) {
                    WLOGDEBUG("data mot found.");
                    // 数据找不到，直接成功结束，外层会判为无数据
                    return moyo_no1::err::EN_SUCCESS;
                }

                if (REDIS_REPLY_ARRAY != data->type) {
                    WLOGERROR("data type error, type=%d", data->type);
                    return moyo_no1::err::EN_SYS_PARAM;
                }

                moyo_no1::table_simple_info* simple_info = msgc.mutable_dbmsg()->mutable_simple();
                for(size_t i = 0; i < data->elements; ++ i) {
                    const redisReply* subr = data->element[i];
                    if (REDIS_REPLY_STRING != subr->type && REDIS_REPLY_STATUS != subr->type && REDIS_REPLY_ERROR != subr->type) {
                        continue;
                    }

                    simple_info->add_arr_str(subr->str, subr->len);
                }

                return moyo_no1::err::EN_SUCCESS;

            }
        }
    }
}