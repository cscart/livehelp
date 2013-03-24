<?php
/**
 * Live Help online chat and stats
 * 
 * @copyright Copyright (c) 2006 Live Help team;
 * @license LGPL+MIT License;
 */

if ( !defined('AREA') )	{ die('Access denied');	}

fn_trusted_vars('request_chat');

//$ip_num = fn_lh_get_ip(true);
$sess_id = Session::get_id();

//
// Download transfered file
//
if ($mode == 'get_file') {
	$file_path = DIR_LH_UPLOADS . '/' . $_REQUEST['file_name'];
	$short_name = substr(basename($file_path), strpos(basename($file_path), '_') + 1);
	
	if (!file_exists($file_path)) {
		exit;
	}
	
	header('Content-length: ' . filesize($file_path));
	fn_get_file($file_path, $short_name, !empty($_REQUEST['encode_name']));
	exit;
}

//
// Getting transfered pic
//
if ($mode == 'get_pic') {
	$file_path = DIR_LH_PICTURES . '/' . $_REQUEST['file_name'];
	$short_name = substr(basename($file_path), strpos(basename($file_path), '_') + 1);
	
	if (!file_exists($file_path)) {
		exit;
	}
	
	header('Content-length: ' . filesize($file_path));
	fn_get_file($file_path, $short_name, !empty($_REQUEST['encode_name']));
	exit;
}

//
// Show transfered pic
//
if ($mode == 'show_pic' && !empty($_REQUEST['file_name'])) {
	Registry::get('smarty')->assign('file_name', $_REQUEST['file_name']);
	Registry::get('smarty')->display('addons/live_help/image.tpl');
	exit;
}

$xml_result = array();
//
// Send a request to chat
//
if ($mode == 'request_chat' && !empty($_REQUEST['user_name']) && !empty($_REQUEST['local_time'])) {
	// Checking and transforming params
//	$session_id = db_get_field("SELECT visitor_id FROM ?:lh_visitors WHERE ip = ?s", $ip_num);
	$session_id = db_get_field("SELECT visitor_id FROM ?:lh_visitors WHERE session_id = ?s", $sess_id);

	$data = array (
		'status' => LH_STATUS_WAITING,
		'obsolete_time_chat' => time() + Registry::get('livehelp_config.freshness_time_visitor'),
		'chat_name' => (isset($_REQUEST['user_name'])) ? $_REQUEST['user_name'] : 'Guest2'
	);
//	db_query("UPDATE ?:lh_visitors SET ?u WHERE ip = ?s", $data, $ip_num);
	db_query("UPDATE ?:lh_visitors SET ?u WHERE session_id = ?s", $data, $sess_id);

	$xml_result[] = "<session_data><session_id>$session_id</session_id></session_data>";
}

// Send the message
if ($mode == 'send_message' && !empty($_REQUEST['message_data'])) {
	$_data = fn_check_table_fields($_REQUEST['message_data'], 'lh_messages');
	$_data['text'] = str_replace('&lt;', '<', $_data['text']);//htmlspecialchars($_data['text']);
	$_data['date'] = TIME;
//	$_data['from_id'] = db_get_field("SELECT visitor_id FROM ?:lh_visitors WHERE ip = ?s", $ip_num);
	$_data['from_id'] = db_get_field("SELECT visitor_id FROM ?:lh_visitors WHERE session_id = ?s", $sess_id);
	db_query("INSERT INTO ?:lh_messages ?e", $_data);
}

// Typing notification
if (!empty($mode) && !empty($_REQUEST['$session_id']) && !empty($_REQUEST['operator_id'])) {
    $data = array (
        'flag' => $mode,
        'self_id' => $session_id,
        'to_id' => $_REQUEST['operator_id'],
        'direction' => LH_DIRECTION_VISITOR_OPERATOR,
    );
    db_query("REPLACE INTO ?:lh_type_notify ?e", $data);
}

// Scanning for new messages
if ($mode == 'scan_messages' && /*!empty($session_id) &&*/ isset($_REQUEST['operator_id'])) {


	// User updates his chat activity status
	$data = array (
		'obsolete_time_chat' => time() + Registry::get('livehelp_config.freshness_time_visitor')
	);

	$status = db_get_field("SELECT status FROM ?:lh_visitors WHERE session_id = ?s", $sess_id);
	if ($status != LH_STATUS_CHATTING)
	{
		$data["status"] = LH_STATUS_WAITING;
	}
	db_query("UPDATE ?:lh_visitors SET ?u WHERE session_id = ?s", $data, $sess_id);



	// Checking operator status

	$operator_id = db_get_field("SELECT operator_id FROM ?:lh_visitors WHERE session_id = ?s", $sess_id);
	if ($operator_id == '' || $operator_id < 1) {
		print_respond("");
	}

	$operator = db_get_row("SELECT name, obsolete_time FROM ?:lh_operators WHERE operator_id = ?i", $operator_id);
	if ($operator['obsolete_time'] < TIME) {
		$data = array (
			'operator_id' => '0',
			'status' => LH_STATUS_WAITING
		);
		db_query("UPDATE ?:lh_visitors SET ?u WHERE session_id = ?s", $data, $sess_id);

		print_respond('<op><id>-1</id><name>' . $operator['name'] . '</name></op>');
	}


	if ($operator_id != $_REQUEST['operator_id']) {
		$xml_result[] = '<op><id>' . $operator_id . '</id><name>' . $operator['name'] . '</name></op>';
	}


/*
//	$new_operator = db_get_field("SELECT operator_id FROM ?:lh_visitors WHERE ip = ?s", $ip_num);
	$new_operator = db_get_field("SELECT operator_id FROM ?:lh_visitors WHERE session_id = ?s", $sess_id);
	
	if ($new_operator != '') {
		// Checking validity of operator flag
		$is_valid = db_get_field("SELECT operator_id FROM ?:lh_operators WHERE operator_id = ?i AND obsolete_time >= ?i", $new_operator, TIME);
		
		if (!$new_operator || $is_valid) {
			if ($new_operator != $_REQUEST['operator_id']) {
				$operator_name = '';
				if ($new_operator) {
					$operator_name = db_get_field("SELECT name FROM ?:lh_operators WHERE operator_id = ?i", $new_operator);
				}
				$xml_result[] = "<op><id>$new_operator</id><name>$operator_name</name></op>";
				$operator_id = $new_operator;
			}
		}
	}
//*/	




	// Querying messages
//	$self = db_get_row("SELECT visitor_id, operator_id, last_message FROM ?:lh_visitors WHERE ip = ?s", $ip_num);
	$self = db_get_row("SELECT visitor_id, operator_id, last_message FROM ?:lh_visitors WHERE session_id = ?s", $sess_id);
	$condition = db_quote("message_id > ?i AND ((to_id = ?i AND from_id = ?i AND direction = ?i) OR (from_id = ?i AND to_id = ?i AND direction = ?i))", $self['last_message'], $self['visitor_id'], $self['operator_id'], LH_DIRECTION_OPERATOR_VISITOR, $self['visitor_id'], $self['operator_id'], LH_DIRECTION_VISITOR_OPERATOR);
	$result = db_get_array("SELECT message_id AS id, direction, from_id, to_id, text FROM ?:lh_messages WHERE $condition");

	// Flushing new messages
	$new_messages = false;
	if (!empty($result)) {
		$new_messages = true;
		$xml_result[] = '<messages>';

		foreach ($result as $row)
		{
			// skip messages visitor-operator
			if ($row['direction'] == LH_DIRECTION_VISITOR_OPERATOR) {
				continue;
			}

			if (preg_match("/^<file>.*<\/file>$/", $row['text'])) {
				$text = $row['text'];
			} else {
				$text = str_replace(array('<', '&'), array('+++lt;', '+++;'), $row['text']);
			}

			$xml_result[] = "<message><id>$row[id]</id><dir>$row[direction]</dir><text>$text</text></message>";
		}
		$xml_result[] = '</messages>';

		$data = array (
			'last_message' => $row['id'],
		);
		db_query("UPDATE ?:lh_visitors SET ?u WHERE visitor_id = ?i", $data, $self['visitor_id']);
	}

	// Checking typing notification
	if ($new_messages == false) {
		$flag = db_get_field("SELECT flag FROM ?:lh_type_notify WHERE self_id = ?i AND to_id = ?i AND direction = ?s AND flag IN (?n)", $_REQUEST['operator_id'], $self['visitor_id'], LH_DIRECTION_OPERATOR_VISITOR, array(LH_TYPING_ON, LH_TYPING_OFF));
		if (!empty($flag)) {
			if ($flag == LH_TYPING_ON) {
				$xml_result[] = '<type_on/>';
			} else {
				$xml_result[] = '<type_off/>';
			}

			// Reseting typing notify flag
			$data = array (
				'flag' => 0,
			);
			db_query("UPDATE ?:lh_type_notify SET ?u WHERE self_id = ?i AND to_id = ?i AND direction = ?s", $data, $_REQUEST['operator_id'], $session_id, LH_DIRECTION_OPERATOR_VISITOR);
		}
	}

	if (!empty($xml_result)) {
		print_respond(implode('', $xml_result));
		exit;
	}
}

function print_respond($text)
{
	echo '<?xml version="1.0" encoding="' . CHARSET . '"?><result>' . $text . '</result>';
	die;
}



exit;
?>