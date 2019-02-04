# httb - HyperText Transfer Beast
Lightweight C++ HTTP Client based on Boost.Beast

## Features
 * Any request type
 * Bundled SSL support (OpenSSL 1.1.1) with system root certs autoloading
 * Request builder
 * Follow redirects
 * Multipart body
 * File uploading
 * Parser for `x-www-form-urlencoded` `POST`/`PUT` body
 
 
## Examples:

#### `GET` request
```cpp
#include <iostream>
#include <httb/httb.h>

int main() {

    httb::request request("http://localhost:9000/simple-server.php/get");
    httb::client client;
    client.setEnableVerbose(true, std::cout);
    httb::response resp = client.execute(request);
    
    const std::string body = resp.getBody();
    if(!resp.isSuccess()) {
        std::cout << "Error response:" << std::endl;
        std::cout << body << std::endl;
    }
    
    return 0;
}
```

#### `POST` request
```cpp
#include <iostream>
#include <httb/httb.h>

int main() {

    httb::request req("http://localhost:9000/simple-server.php/post");
    req.setMethod(httb::request::method::post);
    req.setBody("aaa=1&bbb=2&ccc=3");
    req.setHeader({"content-type", "application/x-www-form-urlencoded"});
    
    httb::client client;
    client.setEnableVerbose(true);
    httb::response resp = client.execute(req);
    
    if(!resp.isSuccess()) {
        std::cout << "Error response:" << std::endl;
        std::cout << resp.getBody() << std::endl;
    }
    
    return 0;
}
```

#### `POST` file upload
```cpp
#include <iostream>
#include <httb/httb.h>

int main() {

    httb::request req("http://localhost:9000/simple-server.php/post");
    req.setMethod(httb::request::method::post);
    req.setHeader({"content-type", "application/x-www-form-urlencoded"});
    
    httb::body_multipart body;
    body.addEntry({"myfile", "text/plain", "myfile.txt", loadMyFileToString()});
    body.addEntry({"my_post_key", "post_value"});
    
    req.setBody(body);
    
    httb::client client;
    client.setEnableVerbose(true);
    httb::response resp = client.execute(req);
    
    if(!resp.isSuccess()) {
        std::cout << "Error response:" << std::endl;
        std::cout << resp.getBody() << std::endl;
    }
    
    return 0;
}
```

#### Fully customizable request
```cpp
const std::string src = "https://www.google.com/search?q=boost+beast&oq=boost+beast&aqs=chrome.0.69i59l3j69i60l3.2684j1j9&sourceid=chrome&ie=UTF-8";
httb::request req(src);

req.isSSL()              // true
req.getProto()           // "https"
req.getPort()            // (uint16_t) 443
req.getPortString()      // "443"
req.getHost()            // "www.google.com");
req.getPath()            // "/search"
req.getParamsString()    // "?q=boost+beast&oq=boost+beast&aqs=chrome.0.69i59l3j69i60l3.2684j1j9&sourceid=chrome&ie=UTF-8");
req.getParam("q")        // "boost+beast"
req.getParam("oq")       // "boost+beast");
req.getParam("aqs")      // "chrome.0.69i59l3j69i60l3.2684j1j9"
req.getParam("sourceid") // "chrome"
req.getParam("ie")       // "UTF-8"

req.addParam({"param", "value"});
req.addParam({"array[]", "val1"});
req.addParam({"array[]", "val2"});

// get array params
std::vector<std::string> arParams = req.getParamArray("array[]", true);

req.addParams({
    {"aaa", "vvv"},
    {"bbb", "vvv"}
});

// overwrite param
req.setParam({"param", "value1"})

req.addHeader({"header-name", "header-value"});
req.addHeaders(...as params...)
// overwrite header 
req.setHeader({"user-agent", "Linkensphere 0.0.0"});

// set force ssl usage
req.useSSL(true);
```

See more examples in [test](tests/HttpClientTest.cpp)

