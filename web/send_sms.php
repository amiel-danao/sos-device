<?php
// Update the path below to your autoload.php,
// see https://getcomposer.org/doc/01-basic-usage.md
if(empty($_GET) || $_SERVER['REQUEST_METHOD'] != "GET" || !isset($_GET['latlng'])){
	http_response_code(400);
	exit;
}

require_once __DIR__ . '/vendor/autoload.php';
use Twilio\Rest\Client;

$sid    = "AC0192c420b2e6ef350459803fd8a4ada9";
$token  = "d4d749263dedd958e3460317b3ba3659";
$twilio = new Client($sid, $token);
$sender = "+15074105833";
$latlng = $_GET['latlng'];

// $location = "www.google.com/maps/@" . $latlng . ",14z";
$location = "www.google.com/maps/place/" . $latlng;

$message = $twilio->messages
  ->create("+639954261220", // to
	array(
		"from" => $sender,
		"body" => "Please help me, this is an emergency sms. I'm located at $location"
	)
  );

echo "OK";
http_response_code(200);
