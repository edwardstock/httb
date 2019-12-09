/*!
 * httb.
 * HttpClientTest.cpp
 *
 * \date 2018
 * \author Eduard Maximovich (edward.vstock@gmail.com)
 * \link   https://github.com/edwardstock
 */

#include "gtest/gtest.h"
#include <chrono>
#include <fstream>
#include <functional>
#include <httb/httb.h>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <thread>
#include <toolbox/io.h>

TEST(HttpClientTest, TestBuildRequestSimple) {
    httb::request req("http://localhost:9000/simple-server.php/get");
    ASSERT_FALSE(req.is_ssl());
    ASSERT_STREQ(req.get_proto_name().c_str(), "http");
    ASSERT_EQ(req.get_port(), (uint16_t) 9000);
    ASSERT_STREQ(req.get_port_str().c_str(), "9000");
    ASSERT_STREQ(req.get_host().c_str(), "localhost");
    ASSERT_STREQ(req.get_path().c_str(), "/simple-server.php/get");
    ASSERT_STREQ(req.get_query_string().c_str(), "");
    ASSERT_STREQ(req.get_url().c_str(), "http://localhost:9000/simple-server.php/get");
}

TEST(HttpClientTest, TestRequestSettings) {
    httb::request req("http://localhost:9000/simple-server.php/get");

    ASSERT_FALSE(req.is_ssl());
    req.use_ssl(true);
    ASSERT_TRUE(req.is_ssl());

    req.add_query({"q", "1"});
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1", req.get_url().c_str());

    req.add_query({"something[]", "1"});
    req.add_query({"something[]", "2"});
    req.add_query({"something[]", "3"});

    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=1&something[]=2&something[]=3",
                 req.get_url().c_str());

    req.remove_query_array("something[]", 0);
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=2&something[]=3", req.get_url().c_str());

    req.remove_query_array("something[]", 1);
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=2", req.get_url().c_str());

    req.remove_query("something[]");
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1", req.get_url().c_str());

    req.add_query({"something[]", "1"});
    req.add_query({"something[]", "2"});
    req.add_query({"something[]", "3"});
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=1&something[]=2&something[]=3",
                 req.get_url().c_str());

    req.remove_query_array("something[]", 1);
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=1&something[]=3", req.get_url().c_str());

    req.remove_query_array("something[]", 0);
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=3", req.get_url().c_str());

    req.remove_query_array("something[]", 10); // overflow
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=3", req.get_url().c_str());

    req.remove_query_array("something[]", -10); // underflow
    ASSERT_STREQ("http://localhost:9000/simple-server.php/get?q=1&something[]=3", req.get_url().c_str());

    req.set_host("google.com");
    ASSERT_STREQ("http://google.com:9000/simple-server.php/get?q=1&something[]=3", req.get_url().c_str());

    req.set_path("api/v1/addresses/0");
    ASSERT_STREQ("http://google.com:9000/api/v1/addresses/0?q=1&something[]=3", req.get_url().c_str());

    req.set_path("/api/v1/password");
    ASSERT_STREQ("http://google.com:9000/api/v1/password?q=1&something[]=3", req.get_url().c_str());

    req.set_port(8080u);
    ASSERT_STREQ("http://google.com:8080/api/v1/password?q=1&something[]=3", req.get_url().c_str());

    ASSERT_TRUE(req.has_query("q"));

    req.set_proto_name("ftps");
    ASSERT_STREQ("ftps://google.com:8080/api/v1/password?q=1&something[]=3", req.get_url().c_str());

    auto params = req.get_query_list();
    ASSERT_EQ(2, params.size());
    ASSERT_STREQ("q", params[0].first.c_str());
    ASSERT_STREQ("1", params[0].second.c_str());
    ASSERT_STREQ("something[]", params[1].first.c_str());
    ASSERT_STREQ("3", params[1].second.c_str());

    req.add_header("aaa", "bbb");
    req.add_header("AaA", "ccc");

    ASSERT_TRUE(req.has_header("aaa"));
    // it's the same as "aaa"
    ASSERT_TRUE(req.has_header("AaA"));
    // didn't added
    ASSERT_FALSE(req.has_header("bbb"));

    auto sr1 = req.find_header_pair("aaa");
    ASSERT_TRUE(sr1.has_value());
    // adding header is case insensitive, so, different cases in name does not mean new entry will be added
    // so because, we have ccc instead of bbb
    ASSERT_STREQ("ccc", sr1->second.c_str());

    auto sr2 = req.find_header_pair("ccc");
    ASSERT_FALSE(sr2.has_value());

    auto sr3 = req.find_header_pair("AaA");
    ASSERT_TRUE(sr3.has_value());
    ASSERT_STREQ("ccc", sr3->second.c_str());
}

TEST(HttpRequestTest, TestPathAdding) {
    httb::request req("http://localhost:9000");
    req.set_path("search");
    ASSERT_STREQ("http://localhost:9000/search", req.get_url().c_str());

    req.set_path("/search");
    ASSERT_STREQ("http://localhost:9000/search", req.get_url().c_str());

    req.set_path("/api/v1");
    ASSERT_STREQ("http://localhost:9000/api/v1", req.get_url().c_str());

    req.add_path("user/create");
    ASSERT_STREQ("http://localhost:9000/api/v1/user/create", req.get_url().c_str());

    req.set_path("api/v1/");
    ASSERT_STREQ("http://localhost:9000/api/v1/", req.get_url().c_str());

    req.add_path("user/create");
    ASSERT_STREQ("http://localhost:9000/api/v1/user/create", req.get_url().c_str());

    req.set_path("/api/v1/");
    req.add_path("user/create/");
    ASSERT_STREQ("http://localhost:9000/api/v1/user/create/", req.get_url().c_str());
}

TEST(HttpClientTest, TestBuildRequestGoogleQuery) {
    const std::string src =
        "https://www.google.com/search?q=boost+beast&oq=boost+beast&aqs=chrome.0.69i59l3j69i60l3.2684j1j9&sourceid=chrome&ie=UTF-8";
    httb::request req(src);
    ASSERT_TRUE(req.is_ssl());
    ASSERT_STREQ(req.get_proto_name().c_str(), "https");
    ASSERT_EQ(req.get_port(), (uint16_t) 443);
    ASSERT_STREQ(req.get_port_str().c_str(), "443");
    ASSERT_STREQ(req.get_host().c_str(), "www.google.com");
    ASSERT_STREQ(req.get_path().c_str(), "/search");
    ASSERT_STREQ(req.get_query_string().c_str(),
                 "?q=boost+beast&oq=boost+beast&aqs=chrome.0.69i59l3j69i60l3.2684j1j9&sourceid=chrome&ie=UTF-8");
    ASSERT_STREQ(req.get_query_value("q").c_str(), "boost+beast");
    ASSERT_STREQ(req.get_query_value("oq").c_str(), "boost+beast");
    ASSERT_STREQ(req.get_query_value("aqs").c_str(), "chrome.0.69i59l3j69i60l3.2684j1j9");
    ASSERT_STREQ(req.get_query_value("sourceid").c_str(), "chrome");
    ASSERT_STREQ(req.get_query_value("ie").c_str(), "UTF-8");

    ASSERT_STREQ(req.get_url().c_str(), src.c_str());
}

TEST(HttpClientTest, TestResponseError) {
    httb::request req("http://wtf");
    httb::client client;
    client.set_verbose(true);
    httb::response resp = client.execute_blocking(req);

    const std::string body = resp.get_body();
    if (!resp.success()) {
        std::cout << "Error response:" << std::endl;
        std::cout << body << std::endl;
    }
    ASSERT_FALSE(resp.success());

    req.add_query({"param", "value"});
    req.add_query({"array[]", "val1"});
    req.add_query({"array[]", "val2"});
}

TEST(HttpClientTest, TestGet) {
    httb::request req("http://localhost:9000/simple-server.php/get");
    httb::client client;
    client.set_verbose(true);
    httb::response resp = client.execute_blocking(req);

    const std::string body = resp.get_body();
    if (!resp.success()) {
        std::cout << "Error response:" << std::endl;
        std::cout << body << std::endl;
    }
    ASSERT_TRUE(resp.success());
    ASSERT_STREQ(body.c_str(), "This is GET method response!");
}

TEST(HttpClientTest, TestGetWithParams) {
    httb::request req("http://localhost:9000/simple-server.php/get");
    req.add_query({"a", "1"});
    req.add_query({"b[]", "2"});
    req.add_query({"c", "three"});
    req.add_query(httb::kvd{"double_value", 105.3851});
    req.add_query(httb::kvd{"int_value", 500});
    httb::client client;
    client.set_verbose(true);
    client.execute(req, [](httb::response resp) {
        const std::string body = resp.get_body();
        if (!resp.success()) {
            std::cout << "Error response:" << std::endl;
            std::cout << body << std::endl;
        }
        ASSERT_TRUE(resp.success());
        ASSERT_STREQ("This is GET method response! Input: a=1;b[0=2;];c=three;double_value=105;int_value=500;",
                     body.c_str());
    });
}

#include "httb/body_string.h"

TEST(HttpClientTest, TestSimplePost) {
    httb::request req("http://localhost:9000/simple-server.php/post");
    req.set_method(httb::request::method::post);

    httb::body_string b1("aaa=1&bbb=2&ccc=3");
    httb::body_form_urlencoded b2("aaa=1&bbb=2&ccc=3");

    ASSERT_STREQ(b1.build(&req).c_str(), b2.build(&req).c_str());

    req.set_body(b1);
    req.set_header({"content-type", "application/x-www-form-urlencoded"});

    httb::client client;
    client.set_verbose(true);
    httb::response resp = client.execute_blocking(req);

    if (!resp.success()) {
        std::cout << "Error response:" << std::endl;
        std::cout << resp.get_body() << std::endl;
    }
    ASSERT_TRUE(resp.success());
    ASSERT_STREQ(resp.get_body_c(), "This is POST method response!");
}

TEST(HttpClientTest, DownloadFile) {
    httb::client client;
    httb::context ctx(2);

    client.set_verbose(true);
    httb::request
        req("https://raw.githubusercontent.com/MinterTeam/minter-go-node/dev/mainnet/minter-mainnet-1/genesis.json");

    req.add_header({"Connection", "keep-alive"});
    req.add_header({"Cache-Control", "max-age=0"});
    req.add_header({"Accept", "*/*"});

    client.execute_in_context(ctx, req, [](httb::response resp) {
        std::cout << "Resp 1 size: " << resp.get_body_size() << std::endl;
    });

    ctx.run();
}

TEST(HttpClientTest, TestSimplePostSmallFile) {
    httb::request req("http://localhost:9000/simple-server.php/file");
    req.set_method(httb::request::method::post);

    httb::body_multipart body;

    httb::file_path_entry fpEntry = {
        "myfile.txt",
        "text/plain",
        std::string(TEST_ROOT) + "/mock/test.txt"};
    body.add_entry({"myfile", fpEntry});
    body.add_entry({"somekey", "somevalue"});

    req.set_body(body);

    httb::client client;
    client.set_verbose(true);
    httb::response resp = client.execute_blocking(req);

    if (!resp.success()) {
        std::cout << "Error response:" << std::endl;
        std::cout << resp.get_body() << std::endl;
    }
    ASSERT_TRUE(resp.success());
    ASSERT_STREQ(resp.get_body_c(), "This is POST method response!");
}

TEST(HttpClientTest, TestSimplePostMediumFile) {
    httb::request req("http://localhost:9000/simple-server.php/file");
    req.set_method(httb::request::method::post);

    httb::body_multipart body;

    httb::file_body_entry fbEntry = {"my-medium-file.txt", "application/binary",
                                     toolbox::io::file_read_full(std::string(TEST_ROOT) + "/mock/test_medium.bin")};
    body.add_entry({"myfile", fbEntry});
    body.add_entry({"somekey", "somevalue"});

    req.set_body(body);

    httb::client client;
    client.set_verbose(true);
    httb::response resp = client.execute_blocking(req);

    if (!resp.success()) {
        std::cout << "Error response:" << std::endl;
        std::cout << resp.get_body() << std::endl;
    }
    ASSERT_TRUE(resp.success());
    ASSERT_STREQ(resp.get_body_c(), "This is POST method response!");
}

TEST(HttpClientTest, TestSimplePostBigFile) {
    httb::request req("http://localhost:9000/simple-server.php/file");
    req.set_method(httb::request::method::post);

    httb::body_multipart body;
    httb::file_body_entry fbEntry = {
        "my-big-file.txt",
        "application/binary",
        toolbox::io::file_read_full(std::string(TEST_ROOT) + "/mock/test_big.bin")};
    body.add_entry({"myfile", fbEntry});
    body.add_entry({"somekey", "somevalue"});

    req.set_body(body);

    httb::client client;
    client.set_verbose(true);
    httb::response resp = client.execute_blocking(req);

    if (!resp.success()) {
        std::cout << "Error response:" << std::endl;
        std::cout << resp.get_body() << std::endl;
    }
    ASSERT_TRUE(resp.success());
    ASSERT_STREQ(resp.get_body_c(), "This is POST method response!");
}

TEST(HttpClientTest, TestSimplePut) {
    httb::request req("http://localhost:9000/simple-server.php/put");
    req.set_method(httb::request::method::put);
    req.set_body("aaa=1&bbb=2&ccc=3");
    req.set_header({"content-type", "application/x-www-form-urlencoded"});

    httb::client client;
    client.set_verbose(true);
    httb::response resp = client.execute_blocking(req);

    if (!resp.success()) {
        std::cout << "Error response:" << std::endl;
        std::cout << resp.get_body() << std::endl;
    }
    ASSERT_TRUE(resp.success());
    ASSERT_STREQ(resp.get_body_c(), "This is PUT method response!");
}

TEST(HttpClientTest, TestSimpleDelete) {
    httb::request req("http://localhost:9000/simple-server.php/delete");
    req.set_method(httb::request::method::delete_);

    httb::client client;
    client.set_verbose(true);
    httb::response resp = client.execute_blocking(req);

    if (!resp.success()) {
        std::cout << "Error response:" << std::endl;
        std::cout << resp.get_body() << std::endl;
    }
    ASSERT_TRUE(resp.success());
    ASSERT_STREQ(resp.get_body_c(), "This is DELETE method response!");
}

TEST(HttpClientTest, TestGetWithRedirectAndDisabledFollow) {
    httb::request
        req("http://google.com");
    httb::client client;
    client.set_follow_redirects(false);
    httb::response resp = client.execute_blocking(req);

    ASSERT_TRUE(resp.success());
    ASSERT_EQ(resp.code, 301);
}

TEST(HttpClientTest, TestAsyncGetWithRedirectAndDisabledFollow) {
    httb::request
        req("http://google.com");
    httb::client client;
    client.set_follow_redirects(false);
    httb::response resp;
    client.execute(req, [&resp](httb::response result) {
        resp = result;
    });

    ASSERT_TRUE(resp.success());
    ASSERT_EQ(resp.code, 301);
}

TEST(HttpClientTest, TestGetWithRedirectAndEnabledFollow) {
    httb::request
        req("http://www.boost.org/doc/libs/develop/libs/beast/doc/html/beast/using_http/message_containers.html");
    httb::client client;
    client.set_verbose(true);
    client.set_follow_redirects(true);
    httb::response resp = client.execute_blocking(req);

    ASSERT_TRUE(resp.success());
    ASSERT_EQ(resp.code, 200);
}

TEST(HttpClientBatchTest, TestRunEach) {
    httb::request req("http://google.com");
    httb::batch_request batch;

    int n = 10;
    for (int i = 0; i < n; i++) {
        batch.add(req);
    }

    std::atomic_int respN(0);
    batch.run_all([&respN](const httb::response& res) {
        respN++;
        std::cout << res.status_message << std::endl;
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
    batch.run_all([&respN](const std::vector<httb::response>& res) {
        respN = res.size();
        std::cout << "Result count: " << respN << std::endl;
    });

    ASSERT_EQ(n, respN);
}

TEST(HttpClientTest, TestAsyncGetWithRedirectAndEnabledFollow) {
    httb::request
        req("http://www.boost.org/doc/libs/develop/libs/beast/doc/html/beast/using_http/message_containers.html");
    httb::client client;
    client.set_follow_redirects(true);
    httb::response resp;
    client.execute(req, [&resp](httb::response result) {
        resp = result;
        std::cout << "Resulting" << std::endl;
    });

    std::cout << "Asserting" << std::endl;
    ASSERT_TRUE(resp.success());
    ASSERT_EQ(resp.code, 200);
}

TEST(HttpClientTest, TestReusingClientInstance) {
    httb::request
        req("https://www.boost.org/doc/libs/develop/libs/beast/doc/html/beast/using_http/message_containers.html");
    httb::client client;
    client.set_follow_redirects(true);
    httb::response resp1 = client.execute_blocking(req);
    ASSERT_TRUE(resp1.success());
    ASSERT_EQ(resp1.code, 200);

    httb::response resp2 = client.execute_blocking(req);
    ASSERT_TRUE(resp2.success());
    ASSERT_EQ(resp2.code, 200);
}

TEST(HttpClientTest, TestSimpleAsyncGet) {
    httb::request req("http://localhost:9000/simple-server.php/get");
    httb::request req2("http://localhost:9000/simple-server.php/get");
    httb::client client;
    client.set_verbose(true);
    bool executed1, executed2 = false;
    bool responseIsSuccess1, responseIsSuccess2 = false;
    client.execute(req, [&executed1, &responseIsSuccess1](httb::response response) {
        executed1 = true;
        responseIsSuccess1 = response.success();
        ASSERT_STREQ(response.get_body_c(), "This is GET method response!");
    });
    client.execute(req2, [&executed2, &responseIsSuccess2](httb::response response) {
        executed2 = true;
        responseIsSuccess2 = response.success();
        ASSERT_STREQ(response.get_body_c(), "This is GET method response!");
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

std::string exec(const char* cmd) {
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

int main(int argc, char** argv) {
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
        ss2 << "$(which curl) -vvv "
            << "http://127.0.0.1:9000/simple-server.php/get";
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
