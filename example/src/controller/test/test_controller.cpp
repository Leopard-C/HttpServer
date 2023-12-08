#include "test_controller.h"
#include <server/helper/helper.h>

namespace controller {

/**
 * @brief 空请求，用于压力测试.
 *
 * @route  /api/Test/TestEmpty
 * @method GET
 * @config Authorization(0)
 */
void TestController::TestEmpty(Request& req, Response& res) {
    res.SetStringBody("Ok", "text/plain");
}

} // namespace controller
