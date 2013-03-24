<?php
/**
 * Live Help online chat and stats
 * 
 * @copyright Copyright (c) 2006 Live Help team;
 * @license LGPL+MIT License;
 */

if ( !defined('AREA') ) { die('Access denied'); }

// Scaning script directory and returning list of script files
function fn_get_scripts($path, $except = array())
{
	$handle = opendir($path);
	while (false !== ($file = readdir($handle))) {
		$file_name = explode('.', $file);
		if ($file == '.' || $file == '..' || in_array($file, $except) || count($file_name) != 2 || $file_name[1] != 'js') {
			continue;
		}
		$scripts[] = "$path/$file";
	}

	return $scripts;
}


function fn_live_help_init_secure_controllers($controllers)
{
	array_merge($controllers, array('visitor', 'chat' , 'operator'));
}

/**
 * Returns array of Live Help options from registry
 * @return type
 */
function fn_get_livehelp_options()
{
	$num_operators_online = db_get_field("SELECT COUNT(*) FROM ?:lh_operators WHERE ?i <= obsolete_time", TIME);
	Registry::set('addons.live_help.num_operators_online', $num_operators_online);

	return Registry::get('addons.live_help');
}

// Getting real ip of visitor
function fn_lh_get_ip($as_int = false)
{
	$ip = fn_get_ip($as_int);

	return $ip['host'];
}

// Adding event record to log table
// @param $visitor_data - 1) array('id' => ...) - passing visitor info by id; 2) array('ip' => ...) - passing visitor info by IP;
function fn_add_log_event($visitor_data, $event, $operator=0, $page_url="", $page_title="")
{

	if (array_key_exists('sess_id', $visitor_data)) {
		$visitor_id = db_get_field("SELECT visitor_id FROM ?:lh_visitors WHERE session_id = ?s", $visitor_data['sess_id']);
		if (!$visitor_id) {
			return;
		}
	} elseif (array_key_exists('id', $visitor_data)) {
		$visitor_id = $visitor_data['id'];
	}

	$operator_name = '';
	if ($operator) {
		$operator_data = db_get_row("SELECT name, login FROM ?:lh_operators WHERE operator_id = ?i", $operator);
		$operator_name = '';
		if ($operator_data) {
			$operator_name = "$operator_data[name] ($operator_data[login])";
		}
	}

	db_query('INSERT INTO ?:lh_visitors_log ?e', array(
		'visitor_id' => $visitor_id,
		'date' => array_key_exists('date', $visitor_data) ? $visitor_data['date'] : TIME,
		'event' => $event,
		'operator_id' => $operator,
		'operator_name' => $operator_name,
		'page_url' => $page_url,
		'page_title' => $page_title
	));
}

function cleanup_obsolete_visitors()
{
	if (Registry::get('settings.lh_visitor_data_last_clean') < TIME - Registry::get('livehelp_config.visitor_data_clean_period')) {
		fn_clean_visitors_data();

		$data = array (
			'value' => TIME,
		);
		CSettings::instance()->update_value('lh_visitor_data_last_clean', $data, 'live_help');
	}
}

// Removing obsolete visitor data from db
function fn_clean_visitors_data()
{
	$obsolete_time = TIME - Registry::get('addons.live_help.lh_visitor_data_keeping_time') * 24 * 60 * 60;

	$visitors = db_get_array("SELECT visitor_id AS id, ip FROM ?:lh_visitors WHERE start_time < ?i", $obsolete_time);

	// Getting list of ids and ips of deletable visitors
	$visitor_ids = array();
	$ips = array();
	foreach ($visitors as $visitor_data) {
		$visitor_ids[] = $visitor_data['id'];
		$ips[] = $visitor_data['ip'];
	}

	// Getting list of ids of chat sessions, created by visitors
	$sessions = db_get_array("SELECT session_id AS id FROM ?:lh_sessions WHERE start_time < ?i", $obsolete_time);
	$session_ids = array();
	foreach ($sessions as $row) {
		$session_ids[] = $row['id'];
	}

	// Executing queries
	if (!empty($visitor_ids)) {
		db_query("DELETE FROM ?:lh_visitors WHERE visitor_id IN (?n)", $visitor_ids);
		db_query("DELETE FROM ?:lh_visitors_log WHERE visitor_id IN (?n)", $visitor_ids);
		db_query("DELETE FROM ?:lh_visitors_notes WHERE visitor_id IN (?n)", $visitor_ids);
	}

	if (!empty($ips)) {
		db_query("DELETE FROM ?:lh_sessions WHERE ip IN (?n)", $ips);
	}

	if (!empty($session_ids)) {
		db_query("DELETE FROM ?:lh_messages WHERE (direction = ?s AND from_id IN (?n)) OR (direction = ?s AND to_id IN (?n))", LH_DIRECTION_VISITOR_OPERATOR, $session_ids, LH_DIRECTION_OPERATOR_VISITOR, $session_ids);
		db_query("DELETE FROM ?:lh_messages_map WHERE direction = ?s AND to_id IN (?n)", LH_DIRECTION_MAP_SESSIONS, $session_ids);
		db_query("DELETE FROM ?:lh_type_notify WHERE (direction = ?s AND self_id IN (?n)) OR (direction = ?s AND to_id IN (?n))", LH_DIRECTION_VISITOR_OPERATOR, $session_ids, LH_DIRECTION_OPERATOR_VISITOR, $session_ids);
	}
}


// Converting message text to representable view
function fn_convert_message(&$message, $replaces)
{
	if (strpos($message['message'], '<file>') !== false) {
		$file_name = explode('_', substr($message['message'], strlen('<file>'), -strlen('</file>')));
		$message['message'] = $file_name[1];
		$message['type'] = 'file';
	} elseif (strpos($message['message'], '<url>') !== false) {
		$href = substr($message['message'], strlen('<url>'), -strlen('</url>'));
		$message['message'] = $href;
		$message['type'] = 'url';
	} else {
		$message['message'] = str_replace(array_keys($replaces), array_values($replaces), $message['message']);
	}
}

// Prepares url for storing to db
function fn_prepare_url($href)
{
	if (!$href) {
		return $href;
	}

	$customer_index = Registry::get('config.customer_index');

	$href = htmlspecialchars($href);

	// replacing back-slashes by the slashes
	$href = str_replace("\\", '/', $href);
	// cutting 'index.php' at the end
	if (substr($href, -strlen($customer_index)) == $customer_index) {
		$href = substr($href, 0, -strlen($customer_index));
	}
	$offset = (!defined('HTTPS')) ? strlen('http://') : strlen('https://');
	// cutting double slashes
	$pos = strpos($href, '//', $offset);
	if ($pos !== false) {
		$href = substr_replace($href, '/', $pos, strlen('//'));
	}
	// cutting dot at the end
	if (substr($href, -1) == '.') {
		$href = substr($href, 0, -1);
	}
	// cutting slash at the end
	if (substr($href, -1) == '/') {
		$href = substr($href, 0, -1);
	}

	return $href;
}

// Puts array values into quotes and returns array
function fn_add_quotes_array_values($array)
{
	foreach ($array as $key => $value) {
		$array[$key] = "'$value'";
	}

	return $array;
}

//
// Generate navigation
//
function fn_live_help_generate_sections($section)
{
	Registry::set('navigation.dynamic.sections', array (
		'operators' => array (
			'title' => fn_get_lang_var('lh_operators'),
			'href' => 'operators.manage',
		),
		'chat_history' => array (
			'title' => fn_get_lang_var('lh_chat_history'),
			'href' => 'chat_history.manage',
		),
	));
	Registry::set('navigation.dynamic.active_section', $section);

	return true;
}

// Log function
function fn_log($type, $act, $data)
{
	// script from
	$type;
	// action
	$act;
	// data to flash
	$data;

	$from;
	if ($type == 1)
		$from = 'Operator.php';
	elseif ($type == 2)
		$from = 'Visitor.php';

	$f = fopen('log', 'a+');
	if ($type == 1)
		fwrite($f, "\n"."|***".date("H:i:s")."*******".$from."\n");
	else
		fwrite($f, "\n"."|---".date("H:i:s")."-------".$from."\n");
	fwrite($f, "| - ".$act." - "."\n"."|"."\n");
	fclose($f);
	file_put_contents('log', var_export($data, true), FILE_APPEND);
	$f = fopen('log', 'a+');
	fwrite($f, "\n"."\n"."\n"."|"."\n");
	fwrite($f, "----------------------------"."\n");
	fclose($f);

	// file_put_contents($filename, join('', $array)).
}

function fn_log_a($filename, $act, $data)
{
	// script from
	$type;
	// action
	$act;
	// data to flash
	$data;

	$f = fopen($filename, 'a+');
	fwrite($f, "\n"."|***".date("H:i:s")."*******"."\n");
	fwrite($f, "| - ".$act." - "."\n"."|"."\n");
	fclose($f);
	file_put_contents('log', var_export($data, true), FILE_APPEND);
	$f = fopen('log', 'a+');
	fwrite($f, "\n"."\n"."\n"."|"."\n");
	fwrite($f, "----------------------------"."\n");
	fclose($f);

	// file_put_contents($filename, join('', $array)).
}

function fn_live_help_fill_auth($auth, $user_data)
{
	if (empty($user_data['user_login']) && empty($user_data['user_email'])) {
		return false;
	}
	
	$assigned_name = db_get_field('SELECT assigned_name FROM ?:lh_visitors WHERE session_id = ?s', Session::get_id());
	$name = Registry::get('settings.General.use_email_as_login') == 'Y' ? $user_data['email'] : $user_data['user_login'];
	
	if (empty($assigned_name)) {
		db_query('UPDATE ?:lh_visitors SET assigned_name = ?s WHERE session_id = ?s', $name, Session::get_id());
	}
}

?>