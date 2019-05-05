<?php
$html = "This is %s method response!";
$htmlWithGET = "This is %s method response! Input: %s";

$fhandle = fopen(getcwd()."/run.log", 'a+');
if(!$fhandle) {
    exit(255);
}

function arr_to_str($arr) {
    $out = "";
    foreach($arr AS $k=>$v) {
        if(is_array($v)) {
            $out .= "{$k}[" . arr_to_str($v) . "];";
        } else {
            $out .= "{$k}={$v};";
        }
        
    }

    return $out;

}

function getRequestHeaders() {
    $headers = array();
    foreach($_SERVER as $key => $value) {
        if (substr($key, 0, 5) <> 'HTTP_') {
            continue;
        }
        $header = str_replace(' ', '-', ucwords(str_replace('_', ' ', strtolower(substr($key, 5)))));
        $headers[$header] = "{$header}: {$value}";
    }
    return $headers;
}

try {

    $uri = str_replace($_SERVER['SCRIPT_NAME'], "", $_SERVER['REQUEST_URI']);

    $rm = $_SERVER['REQUEST_METHOD'];
    fwrite($fhandle, $rm."\n");
    fwrite($fhandle, print_r($_SERVER, true)."\n");

    fwrite($fhandle, implode("\n", getRequestHeaders())."\n");
    if($rm === 'GET') {
        if(sizeof($_GET) > 0) {
            echo sprintf($htmlWithGET, $rm, arr_to_str($_GET));
        } else {
            echo sprintf($html, $rm);
        }
        
        fwrite($fhandle, print_r($_GET, true)."\n");
    } else if($rm === 'POST') {
        echo sprintf($html, $rm);
        fwrite($fhandle, print_r($_POST, true)."\n");
    } else if($rm === 'PUT') {
        $vars = [];
        parse_str(file_get_contents("php://input"),$vars);
        echo sprintf($html, $rm);
        fwrite($fhandle, print_r($vars, true)."\n");
    } else if($rm === 'DELETE') {
        echo sprintf($html, $rm);
        fwrite($fhandle, print_r($_GET, true)."\n");
    } else if($rm === 'HEAD') {
        echo sprintf($html, $rm);
        fwrite($fhandle, print_r($_GET, true)."\n");
    }

    if($_SERVER["PATH_INFO"] === "/file") {
        fwrite($fhandle, "--php://input\n");
        fwrite($fhandle, print_r(file_get_contents('php://input'), true)."\n");
        fwrite($fhandle, "--\$_FILES\n");
        fwrite($fhandle, print_r($_FILES, true)."\n");
        fwrite($fhandle, "--\$_POST['somekey']\n");
        ob_start();
        var_dump($_POST);
        $var = ob_get_clean();
        fwrite($fhandle, $var);


    }

} finally {
    fclose($fhandle);
}


