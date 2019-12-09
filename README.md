# httb - HyperText Transfer Beast
Lightweight C++ HTTP Client based on Boost.Beast

| Bintray | Windows | Linux & macOS |
|:--------:|:---------:|:-----------------:|
|[ ![Download](https://api.bintray.com/packages/edwardstock/edwardstock/httb%3Aedwardstock/images/download.svg) ](https://bintray.com/edwardstock/edwardstock/httb%3Aedwardstock/_latestVersion)|unavailable|[![CircleCI](https://circleci.com/gh/edwardstock/httb/tree/master.svg?style=svg)](https://circleci.com/gh/edwardstock/httb/tree/master)|

## Features
 * Any request type
 * Bundled SSL support (OpenSSL 1.1.1b)
 * Request builder
 * Follow redirects
 * Multipart body
 * File downloading/uploading
 * Progress listener
 * Parser for `x-www-form-urlencoded` `POST`/`PUT` body
 
 
## Examples:

#### `GET` request
```cpp
#include <iostream>
#include <httb/httb.h>

int main() {

    httb::request request("http://localhost:9000/simple-server.php/get");
    httb::client client;
    client.set_verbose(true, std::cout);

    // this is a blocking async method, wait for response
    client.execute(request, [](httb::response resp) {
        std::cout << "Resp body:    " << resp.body() << std::endl;
        std::cout << "Resp message: " << resp.status_message << std::endl;
    });


    // this is 2 simultaneous requests
    // httb::context is an alias for boost::asio::io_context
    httb::context ioctx(2);

    client.execute_in_context(ioctx, request, []{httb::response resp} {
        //...
    });

    client.execute_in_context(ioctx, request, []{httb::response resp} {
        //...
    });

    // blocks this thread and wait until requests are complete
    ioctx.run();

    // Read more about boost asio here
    // https://www.boost.org/doc/libs/1_69_0/doc/html/boost_asio.html
    
    return 0;
}
```

#### `POST` request
```cpp
#include <iostream>
#include <httb/httb.h>

int main() {

    httb::request req("http://localhost:9000/simple-server.php/post");
    req.set_method(httb::request::method::post);
    req.set_body("aaa=1&bbb=2&ccc=3");
    req.set_header({"content-type", "application/x-www-form-urlencoded"});
    
    httb::client client;
    client.set_verbose(true);
    client.execute(req, [](httb::response resp) {
        if(!resp.success()) {
                std::cout << "Error response:" << std::endl;
                std::cout << resp.body() << std::endl;
        }
    });
    
    return 0;
}
```

#### `POST` file upload
```cpp
#include <iostream>
#include <httb/httb.h>

int main() {

    httb::request req("http://localhost:9000/simple-server.php/post");
    req.set_method(httb::request::method::post);
    req.set_header({"content-type", "application/x-www-form-urlencoded"});
    
    httb::body_multipart body;

    // you can load in memory file body by yourself
    // TODO: use io streams
    httb::file_body_entry fb = {"myfile.txt", "text/plain", load_my_file_to_string()};
    body.add_entry({"myfile", fb});

    // or use lazy function
    httb::file_path_entry filePathEntry = {"myfile.txt", "text/plain", "/path/to/file.txt"}
    body.add_entry({"myfile", filePathEntry});

    body.add_entry({"my_post_key", "post_value"});
    
    req.set_body(body);
    
    httb::client client;
    client.set_verbose(true);
    client.execute_blocking(req, [](httb::response resp){
        if(!resp.success()) {
            std::cout << "Error response:" << std::endl;
            std::cout << resp.body() << std::endl;
        }
    });
    
    return 0;
}
```

#### Fully customizable request
```cpp
const std::string src = "https://www.google.com/search?q=boost+beast&oq=boost+beast&aqs=chrome.0.69i59l3j69i60l3.2684j1j9&sourceid=chrome&ie=UTF-8";
httb::request req(src);

req.is_ssl();                    // true
req.get_proto_name();            // "https"
req.get_port();                  // (uint16_t) 443
req.get_port_str();              // "443"
req.get_host();                  // "www.google.com");
req.get_path();                  // "/search"
req.get_query_string();          // "?q=boost+beast&oq=boost+beast&aqs=chrome.0.69i59l3j69i60l3.2684j1j9&sourceid=chrome&ie=UTF-8");
req.get_query_value("q");        // "boost+beast"
req.get_query_value("oq");       // "boost+beast"
req.get_query_value("aqs");      // "chrome.0.69i59l3j69i60l3.2684j1j9"
req.get_query_value("sourceid"); // "chrome"
req.get_query_value("ie");       // "UTF-8"

// get array params
std::vector<std::string> arParams = req.get_query_array("array[]", true);

req.add_query({
    {"aaa", "vvv"},
    {"bbb", "vvv"}
});

// overwrite param
req.set_query({"param", "value1"})

req.add_header({"header-name", "header-value"});
req.add_headers(...as params...)
// overwrite header 
req.set_header({"user-agent", "Linkensphere 0.0.0"});

// set force ssl usage
req.use_ssl(true);
```

See more examples in [test](tests/HttpClientTest.cpp)

