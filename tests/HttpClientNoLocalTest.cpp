/*!
 * httb.
 * HttpClientNoLocalTest.cpp
 *
 * \date 2019
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include <gtest/gtest.h>
#include <httb/httb.h>

TEST(HttpClient, ExecSession) {
    httb::client client;
    httb::request req("http://google.com");

    client.execute((req), [](httb::response resp) {
      std::cout << "Resp body:    " << resp.getBody() << std::endl;
      std::cout << "Resp message: " << resp.statusMessage << std::endl;
    });
}