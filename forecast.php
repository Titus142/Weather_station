<?php

//function getWeather(){

$forecast = get_data('http://api.wunderground.com/api/efa3f64211db75ba/forecast/q/20164.json');


$forecast_d = json_decode($forecast, true);

$fcastDay = $forecast_d['forecast']['txt_forecast']['forecastday']['0']['fcttext'];
$fcastNight = $forecast_d['forecast']['txt_forecast']['forecastday']['1']['fcttext'];
$fcastTom = $forecast_d['forecast']['txt_forecast']['forecastday']['2']['fcttext'];




$file = fopen("fcastday.txt", "w");
fwrite($file, $fcastDay);
fclose($file);

$file = fopen("fcastnight.txt", "w");
fwrite($file, $fcastNight);
fclose($file);

$file = fopen("fcasttom.txt", "w");
fwrite($file, $fcastTom);
fclose($file);


function get_data($url) {
	$ch = curl_init();
	$timeout = 5;
	curl_setopt($ch, CURLOPT_URL, $url);
	curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
	curl_setopt($ch, CURLOPT_CONNECTTIMEOUT, $timeout);
	$data = curl_exec($ch);
	curl_close($ch);
	return $data;
}

?>
