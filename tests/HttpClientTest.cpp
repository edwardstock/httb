/*!
 * httb.
 * HttpClientTest.cpp
 *
 * \date 2018
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include <chrono>
#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <httb/httb.h>
#include <fstream>
#include <toolboxpp.hpp>
#include <functional>
#include <thread>
#include "gtest/gtest.h"

TEST(HttpClientTest, TestBuildRequestSimple) {
    httb::request req("http://localhost:9000/simple-server.php/get");
    ASSERT_FALSE(req.isSSL());
    ASSERT_STREQ(req.getProtocolName().c_str(), "http");
    ASSERT_EQ(req.getPort(), (uint16_t) 9000);
    ASSERT_STREQ(req.getPortString().c_str(), "9000");
    ASSERT_STREQ(req.getHost().c_str(), "localhost");
    ASSERT_STREQ(req.getPath().c_str(), "/simple-server.php/get");
    ASSERT_STREQ(req.getQueryString().c_str(), "");
    ASSERT_STREQ(req.getUrl().c_str(), "http://localhost:9000/simple-server.php/get");
}

TEST(HttpClientTest, TestRequestSettings) {
    httb::request req("http://localhost:9000/simple-server.php/get");

    ASSERT_FALSE(req.isSSL());
    req.useSSL(true);
    ASSERT_TRUE(req.isSSL());

    req.addQuery({"q", "1"});
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1", req.getUrl().c_str());

    req.addQuery({"something[]", "1"});
    req.addQuery({"something[]", "2"});
    req.addQuery({"something[]", "3"});

    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=1&something[]=2&something[]=3",
                 req.getUrl().c_str());

    req.removeQueryArray("something[]", 0);
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=2&something[]=3", req.getUrl().c_str());

    req.removeQueryArray("something[]", 1);
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=2", req.getUrl().c_str());

    req.removeQuery("something[]");
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1", req.getUrl().c_str());

    req.addQuery({"something[]", "1"});
    req.addQuery({"something[]", "2"});
    req.addQuery({"something[]", "3"});
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=1&something[]=2&something[]=3",
                 req.getUrl().c_str());

    req.removeQueryArray("something[]", 1);
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=1&something[]=3", req.getUrl().c_str());

    req.removeQueryArray("something[]", 0);
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=3", req.getUrl().c_str());

    req.removeQueryArray("something[]", 10); // overflow
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=3", req.getUrl().c_str());

    req.removeQueryArray("something[]", -10); // underflow
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=3", req.getUrl().c_str());

    req.setHost("google.com");
    ASSERT_STREQ("http://google.com:9000/simple-server.php/get?q=1&something[]=3", req.getUrl().c_str());

    req.setPath("api/v1/addresses/0");
    ASSERT_STREQ("http://google.com:9000/api/v1/addresses/0?q=1&something[]=3", req.getUrl().c_str());

    req.setPath("/api/v1/password");
    ASSERT_STREQ("http://google.com:9000/api/v1/password?q=1&something[]=3", req.getUrl().c_str());

    req.setPort(8080u);
    ASSERT_STREQ("http://google.com:8080/api/v1/password?q=1&something[]=3", req.getUrl().c_str());

    ASSERT_TRUE(req.hasQuery("q"));

    req.setProtocolName("ftps");
    ASSERT_STREQ("ftps://google.com:8080/api/v1/password?q=1&something[]=3", req.getUrl().c_str());

    auto params = req.getQueryList();
    ASSERT_EQ(2, params.size());
    ASSERT_STREQ("q", params[0].first.c_str());
    ASSERT_STREQ("1", params[0].second.c_str());
    ASSERT_STREQ("something[]", params[1].first.c_str());
    ASSERT_STREQ("3", params[1].second.c_str());
}

TEST(HttpRequestTest, TestPathAdding) {
    httb::request req("http://localhost:9000");
    req.setPath("search");
    ASSERT_STREQ("http://localhost:9000/search", req.getUrl().c_str());

    req.setPath("/search");
    ASSERT_STREQ("http://localhost:9000/search", req.getUrl().c_str());

    req.setPath("/api/v1");
    ASSERT_STREQ("http://localhost:9000/api/v1", req.getUrl().c_str());

    req.addPath("user/create");
    ASSERT_STREQ("http://localhost:9000/api/v1/user/create", req.getUrl().c_str());

    req.setPath("api/v1/");
    ASSERT_STREQ("http://localhost:9000/api/v1/", req.getUrl().c_str());

    req.addPath("user/create");
    ASSERT_STREQ("http://localhost:9000/api/v1/user/create", req.getUrl().c_str());

    req.setPath("/api/v1/");
    req.addPath("user/create/");
    ASSERT_STREQ("http://localhost:9000/api/v1/user/create/", req.getUrl().c_str());
}

TEST(HttpClientTest, TestBuildRequestGoogleQuery) {
    const std::string src =
        "https://www.google.com/search?q=boost+beast&oq=boost+beast&aqs=chrome.0.69i59l3j69i60l3.2684j1j9&sourceid=chrome&ie=UTF-8";
    httb::request req(src);
    ASSERT_TRUE(req.isSSL());
    ASSERT_STREQ(req.getProtocolName().c_str(), "https");
    ASSERT_EQ(req.getPort(), (uint16_t) 443);
    ASSERT_STREQ(req.getPortString().c_str(), "443");
    ASSERT_STREQ(req.getHost().c_str(), "www.google.com");
    ASSERT_STREQ(req.getPath().c_str(), "/search");
    ASSERT_STREQ(req.getQueryString().c_str(),
                 "?q=boost+beast&oq=boost+beast&aqs=chrome.0.69i59l3j69i60l3.2684j1j9&sourceid=chrome&ie=UTF-8");
    ASSERT_STREQ(req.getQuery("q").c_str(), "boost+beast");
    ASSERT_STREQ(req.getQuery("oq").c_str(), "boost+beast");
    ASSERT_STREQ(req.getQuery("aqs").c_str(), "chrome.0.69i59l3j69i60l3.2684j1j9");
    ASSERT_STREQ(req.getQuery("sourceid").c_str(), "chrome");
    ASSERT_STREQ(req.getQuery("ie").c_str(), "UTF-8");

    ASSERT_STREQ(req.getUrl().c_str(), src.c_str());
}

TEST(HttpClientTest, TestResponseError) {
    httb::request req("http://wtf");
    httb::client client;
    client.setVerbose(true);
    httb::response resp = client.executeBlocking(req);

    const std::string body = resp.getBody();
    if (!resp.isSuccess()) {
        std::cout << "Error response:" << std::endl;
        std::cout << body << std::endl;
    }
    ASSERT_FALSE(resp.isSuccess());
}

TEST(HttpClientTest, TestGet) {
    httb::request req("http://localhost:9000/simple-server.php/get");
    httb::client client;
    client.setVerbose(true);
    httb::response resp = client.executeBlocking(req);

    const std::string body = resp.getBody();
    if (!resp.isSuccess()) {
        std::cout << "Error response:" << std::endl;
        std::cout << body << std::endl;
    }
    ASSERT_TRUE(resp.isSuccess());
    ASSERT_STREQ(body.c_str(), "This is GET method response!");
}

TEST(HttpClientTest, TestGetWithParams) {
    httb::request req("http://localhost:9000/simple-server.php/get");
    req.addQuery({"a", "1"});
    req.addQuery({"b[]", "2"});
    req.addQuery({"c", "three"});
    req.addQuery(httb::kvd{"double_value", 105.3851});
    req.addQuery(httb::kvd{"int_value", 500});
    httb::client client;
    client.setVerbose(true);
    client.execute(req, [](httb::response resp) {
      const std::string body = resp.getBody();
      if (!resp.isSuccess()) {
          std::cout << "Error response:" << std::endl;
          std::cout << body << std::endl;
      }
      ASSERT_TRUE(resp.isSuccess());
      ASSERT_STREQ("This is GET method response! Input: a=1;b[0=2;];c=three;double_value=105;int_value=500;",
                   body.c_str());
    });

}

TEST(HttpClientTest, TestSimplePost) {
    httb::request req("http://localhost:9000/simple-server.php/post");
    req.setMethod(httb::request::method::post);
    req.setBody(httb::body_form_urlencoded("aaa=1&bbb=2&ccc=3"));
    req.setHeader({"content-type", "application/x-www-form-urlencoded"});

    httb::client client;
    client.setVerbose(true);
    httb::response resp = client.executeBlocking(req);

    if (!resp.isSuccess()) {
        std::cout << "Error response:" << std::endl;
        std::cout << resp.getBody() << std::endl;
    }
    ASSERT_TRUE(resp.isSuccess());
    ASSERT_STREQ(resp.getBodyC(), "This is POST method response!");
}

TEST(HttpClientTest, DownloadFile) {
    httb::client client;
    httb::context ctx(2);

    client.setVerbose(true);
    httb::request
        req("https://raw.githubusercontent.com/MinterTeam/minter-go-node/dev/mainnet/minter-mainnet-1/genesis.json");

    req.addHeader({"Connection", "keep-alive"});
    req.addHeader({"Cache-Control", "max-age=0"});
    req.addHeader({"Accept", "*/*"});

    client.executeInContext(ctx, req, [](httb::response resp) {
      std::cout << "Resp 1 size: " << resp.getBodySize() << std::endl;
    });

    ctx.run();
}

TEST(HttpClientTest, TestSimplePostSmallFile) {
    httb::request req("http://localhost:9000/simple-server.php/file");
    req.setMethod(httb::request::method::post);

    httb::body_multipart body;

    httb::file_path_entry fpEntry = {
        "myfile.txt",
        "text/plain",
        std::string(TEST_ROOT) + "/mock/test.txt"
    };
    body.addEntry({"myfile", fpEntry});
    body.addEntry({"somekey", "somevalue"});

    req.setBody(body);

    httb::client client;
    client.setVerbose(true);
    httb::response resp = client.executeBlocking(req);

    if (!resp.isSuccess()) {
        std::cout << "Error response:" << std::endl;
        std::cout << resp.getBody() << std::endl;
    }
    ASSERT_TRUE(resp.isSuccess());
    ASSERT_STREQ(resp.getBodyC(), "This is POST method response!");
}

TEST(HttpClientTest, TestSimplePostMediumFile) {
    httb::request req("http://localhost:9000/simple-server.php/file");
    req.setMethod(httb::request::method::post);

    httb::body_multipart body;

    httb::file_body_entry fbEntry = {"my-medium-file.txt", "application/binary",
                                     toolboxpp::fs::readFile(std::string(TEST_ROOT) + "/mock/test_medium.bin")};
    body.addEntry({"myfile", fbEntry});
    body.addEntry({"somekey", "somevalue"});

    req.setBody(body);

    httb::client client;
    client.setVerbose(true);
    httb::response resp = client.executeBlocking(req);

    if (!resp.isSuccess()) {
        std::cout << "Error response:" << std::endl;
        std::cout << resp.getBody() << std::endl;
    }
    ASSERT_TRUE(resp.isSuccess());
    ASSERT_STREQ(resp.getBodyC(), "This is POST method response!");
}

TEST(HttpClientTest, TestSimplePostBigFile) {
    httb::request req("http://localhost:9000/simple-server.php/file");
    req.setMethod(httb::request::method::post);

    httb::body_multipart body;
    httb::file_body_entry fbEntry = {
        "my-big-file.txt",
        "application/binary",
        toolboxpp::fs::readFile(std::string(TEST_ROOT) + "/mock/test_big.bin")
    };
    body.addEntry({"myfile", fbEntry});
    body.addEntry({"somekey", "somevalue"});

    req.setBody(body);

    httb::client client;
    client.setVerbose(true);
    httb::response resp = client.executeBlocking(req);

    if (!resp.isSuccess()) {
        std::cout << "Error response:" << std::endl;
        std::cout << resp.getBody() << std::endl;
    }
    ASSERT_TRUE(resp.isSuccess());
    ASSERT_STREQ(resp.getBodyC(), "This is POST method response!");
}

TEST(HttpClientTest, TestSimplePut) {
    httb::request req("http://localhost:9000/simple-server.php/put");
    req.setMethod(httb::request::method::put);
    req.setBody("aaa=1&bbb=2&ccc=3");
    req.setHeader({"content-type", "application/x-www-form-urlencoded"});

    httb::client client;
    client.setVerbose(true);
    httb::response resp = client.executeBlocking(req);

    if (!resp.isSuccess()) {
        std::cout << "Error response:" << std::endl;
        std::cout << resp.getBody() << std::endl;
    }
    ASSERT_TRUE(resp.isSuccess());
    ASSERT_STREQ(resp.getBodyC(), "This is PUT method response!");
}

TEST(HttpClientTest, TestSimpleDelete) {
    httb::request req("http://localhost:9000/simple-server.php/delete");
    req.setMethod(httb::request::method::delete_);

    httb::client client;
    client.setVerbose(true);
    httb::response resp = client.executeBlocking(req);

    if (!resp.isSuccess()) {
        std::cout << "Error response:" << std::endl;
        std::cout << resp.getBody() << std::endl;
    }
    ASSERT_TRUE(resp.isSuccess());
    ASSERT_STREQ(resp.getBodyC(), "This is DELETE method response!");
}

TEST(HttpClientTest, TestGetWithRedirectAndDisabledFollow) {
    httb::request
        req("http://google.com");
    httb::client client;
    client.setFollowRedirects(false);
    httb::response resp = client.executeBlocking(req);

    ASSERT_TRUE(resp.isSuccess());
    ASSERT_EQ(resp.statusCode, 301);
}

TEST(HttpClientTest, TestAsyncGetWithRedirectAndDisabledFollow) {
    httb::request
        req("http://google.com");
    httb::client client;
    client.setFollowRedirects(false);
    httb::response resp;
    client.execute(req, [&resp](httb::response result) {
      resp = result;
    });

    ASSERT_TRUE(resp.isSuccess());
    ASSERT_EQ(resp.statusCode, 301);
}

TEST(HttpClientTest, TestGetWithRedirectAndEnabledFollow) {
    httb::request
        req("http://www.boost.org/doc/libs/develop/libs/beast/doc/html/beast/using_http/message_containers.html");
    httb::client client;
    client.setVerbose(true);
    client.setFollowRedirects(true);
    httb::response resp = client.executeBlocking(req);

    ASSERT_TRUE(resp.isSuccess());
    ASSERT_EQ(resp.statusCode, 200);
}

TEST(HttpClientBatchTest, TestRunEach) {
    httb::request req("http://google.com");
    httb::batch_request batch;

    int n = 10;
    for (int i = 0; i < n; i++) {
        batch.add(req);
    }

    std::atomic_int respN(0);
    batch.runEach([&respN](const httb::response &res) {
      respN++;
      std::cout << res.statusMessage << std::endl;
    });

    ASSERT_EQ(n, respN);
}

TEST(HttpClientBatchTest, TestRunAll) {
    httb::request req("http://google.com");
    httb::batch_request batch;

    int n = 10;
    for (int i = 0; i < n; i++) {
        batch.add(req);
    }

    std::atomic_int respN(0);
    batch.runAll([&respN](const std::vector<httb::response> &res) {
      respN = res.size();
      std::cout << "Result count: " << respN << std::endl;
    });

    ASSERT_EQ(n, respN);
}

TEST(HttpClientTest, TestAsyncGetWithRedirectAndEnabledFollow) {
    httb::request
        req("http://www.boost.org/doc/libs/develop/libs/beast/doc/html/beast/using_http/message_containers.html");
    httb::client client;
    client.setFollowRedirects(true);
    httb::response resp;
    client.execute(req, [&resp](httb::response result) {
      resp = result;
      std::cout << "Resulting" << std::endl;
    });

    std::cout << "Asserting" << std::endl;
    ASSERT_TRUE(resp.isSuccess());
    ASSERT_EQ(resp.statusCode, 200);
}

TEST(HttpClientTest, TestReusingClientInstance) {
    httb::request
        req("https://www.boost.org/doc/libs/develop/libs/beast/doc/html/beast/using_http/message_containers.html");
    httb::client client;
    client.setFollowRedirects(true);
    httb::response resp1 = client.executeBlocking(req);
    ASSERT_TRUE(resp1.isSuccess());
    ASSERT_EQ(resp1.statusCode, 200);

    httb::response resp2 = client.executeBlocking(req);
    ASSERT_TRUE(resp2.isSuccess());
    ASSERT_EQ(resp2.statusCode, 200);
}

TEST(HttpClientTest, TestSimpleAsyncGet) {
    httb::request req("http://localhost:9000/simple-server.php/get");
    httb::request req2("http://localhost:9000/simple-server.php/get");
    httb::client client;
    client.setVerbose(true);
    bool executed1, executed2 = false;
    bool responseIsSuccess1, responseIsSuccess2 = false;
    client.execute(req, [&executed1, &responseIsSuccess1](httb::response response) {
      executed1 = true;
      responseIsSuccess1 = response.isSuccess();
      ASSERT_STREQ(response.getBodyC(), "This is GET method response!");
    });
    client.execute(req2, [&executed2, &responseIsSuccess2](httb::response response) {
      executed2 = true;
      responseIsSuccess2 = response.isSuccess();
      ASSERT_STREQ(response.getBodyC(), "This is GET method response!");
    });

    int tries = 0;
    while (!executed1) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        tries++;
        if (tries >= 3) {
            executed1 = false;
            break;
        }
    }

    tries = 0;
    while (!executed2) {
        std::this_thread::sleep_for(std::chrono::seconds(3));
        tries++;
        if (tries >= 3) {
            executed2 = false;
            break;
        }
    }

    ASSERT_TRUE(executed1);
    ASSERT_TRUE(executed2);
    ASSERT_TRUE(responseIsSuccess1);
    ASSERT_TRUE(responseIsSuccess2);
}

std::string exec(const char *cmd) {
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    std::array<char, 128> buffer;
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

int main(int argc, char **argv) {
    bool runLocalServer = true;
    std::string pid;

    if (argc >= 2 && std::string(argv[1]) == "nolocal") {
        runLocalServer = false;
    }

    if (runLocalServer) {
        std::stringstream ss1;
        ss1 << "$(which bash) ";
        ss1 << TEST_ROOT << "/mock/run-server.sh " << TEST_ROOT << "/mock";

        std::cout << ss1.str() << std::endl;
        pid = exec(ss1.str().c_str());
    }

    ::testing::InitGoogleTest(&argc, argv);

    std::this_thread::sleep_for(std::chrono::seconds(2));
    int ret = RUN_ALL_TESTS();

    if (runLocalServer) {
        std::stringstream ss2;
        ss2 << "$(which curl) -vvv " << "http://127.0.0.1:9000/simple-server.php/get";
        std::cout << ss2.str() << std::endl;
        std::cout << system(ss2.str().c_str()) << std::endl;

        ss2.str("");
        ss2.clear();

        ss2 << "cat " << TEST_ROOT << "/mock/run.log";
        std::cout << ss2.str() << std::endl;
        std::cout << system(ss2.str().c_str()) << std::endl;

        std::stringstream ss;
        ss << "kill -9 " << pid;
        system(ss.str().c_str());
    }

    return ret;
}


 