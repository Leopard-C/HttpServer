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
};

} // namespace controller
