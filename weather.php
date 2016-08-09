
<?php

$hourly = get_data('http://api.wunderground.com/api/efa3f64211db75ba/hourly/q/20164.json');
$current = get_data('http://api.wunderground.com/api/efa3f64211db75ba/conditions/q/va/sterling.json');


date_default_timezone_set('America/New_York');
$hour = date('G');



$hourly_d = (json_decode($hourly, true));
$current_d = (json_decode($current, true));

$timechk;
$day = 0;


while($timechk !== $hour)
{
	$timechk = $hourly_d['hourly_forecast'][$day]['FCTTIME']['hour'];
	if ($timechk !== $hour)
	{
		$day++;
	}
	
}

$pop = intval($hourly_d['hourly_forecast'][$day]['pop']);
$temp = intval($current_d['current_observation']['temp_f']);
$hum = intval($current_d['current_observation']['relative_humidity']);
$bar = intval($current_d['current_observation']['pressure_mb']);
//$windspd = $current_d['current_observation']['wind_mph']; 


print "<$temp,$hum,$pop,$bar>";

/* gets the data from a URL */
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

