#pragma once
#include <server/request.h>
#include <server/response.h>

namespace controller {

using namespace ic::server;

/**
 * @brief 测试控制器.
 */
class TestController {
public:
    /**
     * @brief 空请求，用于压力测试.
     *
     * @route  /api/Test/TestEmpty
     * @method GET
     * @config Authorization(0)
     */
    static void TestEmpty(Request& req, Response& res);


    /*********************************** CHECK_PARAM_xxx ******************************************/

    /**
     * @brief 测试宏-CHECK_URL_PARAM_xxx.
     * 
     * @route  /api/Test/TestMacro_CHECK_URL_PARAM
     * @method GET
     */
    static void TestMacro_CHECK_URL_PARAM(Request& req, Response& res);

    /**
     * @brief 测试宏-CHECK_BODY_PARAM_xxx.
     * 
     * @route  /api/Test/TestMacro_CHECK_BODY_PARAM
     * @method POST
     */
    static void TestMacro_CHECK_BODY_PARAM(Request& req, Response& res);

    /**
     * @brief 测试宏-CHECK_JSON_PARAM_xxx.
     * 
     * @route  /api/Test/TestMacro_CHECK_JSON_PARAM
     * @method POST
     */
    static void TestMacro_CHECK_JSON_PARAM(Request& req, Response& res);

    /**
     * @brief 测试宏-CHECK_JSON_PARAM_xxx_ARRAY.
     * 
     * @route  /api/Test/TestMacro_CHECK_JSON_PARAM_ARRAY
     * @method POST
     */
    static void TestMacro_CHECK_JSON_PARAM_ARRAY(Request& req, Response& res);

    /**
     * @brief 测试宏-CHECK_BODY_PARAM_xxx_EX.
     * 
     * @route  /api/Test/TestMacro_CHECK_BODY_PARAM_EX
     * @method POST
     */
    static void TestMacro_CHECK_BODY_PARAM_EX(Request& req, Response& res);


    /************************************* GET_PARAM_xxx ******************************************/

    /**
     * @brief 测试宏-GET_URL_PARAM_xxx.
     * 
     * @route  /api/Test/TestMacro_GET_URL_PARAM
     * @method GET
     */
    static void TestMacro_GET_URL_PARAM(Request& req, Response& res);

    /**
     * @brief 测试宏-GET_BODY_PARAM_xxx.
     * 
     * @route  /api/Test/TestMacro_GET_BODY_PARAM
     * @method POST
     */
    static void TestMacro_GET_BODY_PARAM(Request& req, Response& res);

    /**
     * @brief 测试宏-GET_JSON_PARAM_xxx.
     * 
     * @route  /api/Test/TestMacro_GET_JSON_PARAM
     * @method POST
     */
    static void TestMacro_GET_JSON_PARAM(Request& req, Response& res);

    /**
     * @brief 测试宏-GET_JSON_PARAM_xxx_ARRAY.
     * 
     * @route  /api/Test/TestMacro_GET_JSON_PARAM_ARRAY
     * @method POST
     */
    static void TestMacro_GET_JSON_PARAM_ARRAY(Request& req, Response& res);

    /**
     * @brief 测试宏-GET_BODY_PARAM_xxx_EX.
     * 
     * @route  /api/Test/TestMacro_GET_BODY_PARAM_EX
     * @method POST
     */
    static void TestMacro_GET_BODY_PARAM_EX(Request& req, Response& res);
};

} // namespace controller
