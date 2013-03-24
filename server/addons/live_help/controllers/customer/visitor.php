<?php
/**
 * Live Help online chat and stats
 * 
 * @copyright Copyright (c) 2006 Live Help team;
 * @license LGPL+MIT License;
 */

if ( !defined('AREA') )	{ die('Access denied');	}

if ($_SERVER['REQUEST_METHOD'] == 'POST') {

	//
	// Post an offline message for operator
	//
	if ($mode == 'leave_message') {
		$message_data = fn_check_table_fields($_REQUEST['message_data'], 'lh_left_messages');
		$message_data['date'] = TIME;

		db_query("INSERT INTO ?:lh_left_messages ?e", $message_data);

		return array(CONTROLLER_STATUS_OK, "visitor.leave_message.sent");
	}
}

//
// Open Leave Message window
//
if ($mode == 'leave_message') {
	$view->display('addons/live_help/views/visitor/leave_message.tpl');
	exit;
}

//
// Open Chat window
//
if ($mode == 'open_chat')
{
//	$ip_num = fn_lh_get_ip(true);
	$sess_id = Session::get_id();

	if (isset($_REQUEST['user_name'])) {
		$data = array(
			'chat_name' => fn_unicode_to_utf8($_REQUEST['user_name'])
		);
//		db_query("UPDATE ?:lh_visitors SET ?u WHERE ip = ?s", $data, $ip_num);
		db_query("UPDATE ?:lh_visitors SET ?u WHERE session_id = ?s", $data, $sess_id);
	}

	// Operator invite visitor to chat session
	if ($action == 'force')
	{
//		$session_data = db_get_row("SELECT ?:lh_visitors.visitor_id, ?:lh_visitors.last_message, ?:lh_visitors.chat_name as user_name, ?:lh_operators.operator_id, ?:lh_operators.name as operator_name FROM ?:lh_visitors LEFT JOIN ?:lh_operators ON ?:lh_visitors.operator_id = ?:lh_operators.operator_id WHERE ?:lh_visitors.ip = ?s", $ip_num);
		$session_data = db_get_row("SELECT ?:lh_visitors.visitor_id, ?:lh_visitors.last_message, ?:lh_visitors.chat_name as user_name, ?:lh_operators.operator_id, ?:lh_operators.name as operator_name FROM ?:lh_visitors LEFT JOIN ?:lh_operators ON ?:lh_visitors.operator_id = ?:lh_operators.operator_id WHERE ?:lh_visitors.session_id = ?s", $sess_id);

		// set visitor 'chatting' state
		$data = array (
			'status' => LH_STATUS_CHATTING
		);
//		db_query("UPDATE ?:lh_visitors SET ?u WHERE ip = ?s", $data, $ip_num);
		db_query("UPDATE ?:lh_visitors SET ?u WHERE session_id = ?s", $data, $sess_id);

		// read invitation message(s)
		$condition = db_quote("message_id >= ?i AND ((to_id = ?i AND from_id = ?i AND direction = ?i) OR (from_id = ?i AND to_id = ?i AND direction = ?i))", $_REQUEST['mes_id'], $session_data['visitor_id'], $session_data['operator_id'], LH_DIRECTION_OPERATOR_VISITOR, $session_data['visitor_id'], $session_data['operator_id'], LH_DIRECTION_VISITOR_OPERATOR);
		$inv_mes = db_get_array("SELECT message_id, text FROM ?:lh_messages WHERE $condition");

		$inv_messages = array();
		foreach ($inv_mes as $k => $v) {
			$inv_messages[$k] = $v['text'];
		}

		$view->assign('invitation_messages', $inv_messages);
		$view->assign('session_data', $session_data);
	}
	else {
//		$visitor_chat_name = db_get_field("SELECT chat_name FROM ?:lh_visitors WHERE ?:lh_visitors.ip = ?s", $ip_num);
		$visitor_chat_name = db_get_field("SELECT chat_name FROM ?:lh_visitors WHERE ?:lh_visitors.session_id = ?s", $sess_id);
		$view->assign('visitor_chat_name', $visitor_chat_name);
	}

	$view->assign('request_interval', Registry::get('livehelp_config.chat_interval'));

	$view->display('addons/live_help/views/visitor/open_chat.tpl');
	exit;
}

//
// Accept or decline invitation by operator dialog
//
if ($mode == 'invite') {
//	$ip_num = fn_lh_get_ip(true);
	$sess_id = Session::get_id();

	if (isset($_REQUEST['check_invitation']) && $_REQUEST['check_invitation'] == 'Y' && !empty($_REQUEST['user_name']) && !empty($_REQUEST['operator_id'])) {
		// Accept invitation
		$user_name = fn_unicode_to_utf8($_REQUEST['user_name']);
		$data = array(
			'operator_id' => $_REQUEST['operator_id'],
			'status' => LH_STATUS_CHATTING,
			'obsolete_time_chat' => time() + Registry::get('livehelp_config.freshness_time_visitor'),
			'chat_name' => $user_name
		);
//		db_query("UPDATE ?:lh_visitors SET ?u WHERE ip = ?s", $data, $ip_num);
		db_query("UPDATE ?:lh_visitors SET ?u WHERE session_id = ?s", $data, $sess_id);

		fn_add_log_event(array('sess_id' => $sess_id), LH_LOG_ACCEPTED_INVITATION, $_REQUEST['operator_id']);
		fn_add_log_event(array('sess_id' => $sess_id), LH_LOG_OPERATOR_JOINED, $_REQUEST['operator_id']);
		fn_add_log_event(array('sess_id' => $sess_id), LH_LOG_CHAT_STARTED, $_REQUEST['operator_id']);

	} elseif ($_REQUEST['invitation'] == 'N') {
		// Decline invitation
//		$operator_id = db_get_field("SELECT operator_id FROM ?:lh_visitors WHERE ip = ?s", $ip_num);
		$operator_id = db_get_field("SELECT operator_id FROM ?:lh_visitors WHERE session_id = ?s", $sess_id);
		if ($operator_id > 0)
		{
			fn_add_log_event(array('ip' => $ip_num), LH_LOG_DECLINED_INVITATION, $operator_id);
		}

		$data = array(
			'operator_id' => 0,
			'status' => LH_STATUS_DECLINED
		);
	
//		db_query("UPDATE ?:lh_visitors SET ?u WHERE ip = ?s", $data, $ip_num);
		db_query("UPDATE ?:lh_visitors SET ?u WHERE session_id = ?s", $data, $sess_id);
	}
}

//
// Update visitor's data and return info
//
if ($mode == 'update') {

	$sess_id = Session::get_id();

//	$ip_num = fn_lh_get_ip(true);
//	$visitor_id = db_get_field("SELECT visitor_id FROM ?:lh_visitors WHERE ip = ?s", $ip_num);

//	$visitor_data = db_get_row("SELECT visitor_id, status, operator_id, last_message, chat_name FROM ?:lh_visitors WHERE ip = ?s", $ip_num);

	$visitor_data = db_get_row("SELECT visitor_id, status, operator_id, last_message, chat_name FROM ?:lh_visitors WHERE session_id = ?s", $sess_id);
	$visitor_id = 0;

	if (!empty($visitor_data)) {
		// Update visitor status
		if (in_array($visitor_data['status'], array(LH_STATUS_DECLINED, LH_STATUS_REQUEST_SENT/*, LH_STATUS_WAITING*/)) && $visitor_data['operator_id'] == 0) {
			if (!empty($_REQUEST['visitor_environment']['title'])) {
				$data['assigned_name'] = $_REQUEST['visitor_environment']['title'];
			}

			$data['operator_id'] = 0;
			$data['status'] = LH_STATUS_IDLE;
			db_query("UPDATE ?:lh_visitors SET ?u WHERE visitor_id = ?i", $data, $visitor_data['visitor_id']);
		}

		$visitor_id = $visitor_data['visitor_id'];
	} else {
		// Add new visitor
		$ip_num = fn_lh_get_ip(true);

		$data['session_id'] = $sess_id;
		$data['ip'] = $ip_num;
		// $data['assigned_name'] = 'anonymous (' . long2ip($ip_num) . ')'; //Leave empty for anonymous users
		$visitor_id = db_query("INSERT INTO ?:lh_visitors ?e", $data);
	}


	// store Requests history
//	$start_time = db_get_field("SELECT start_time FROM ?:lh_visitors WHERE ?:lh_visitors.ip = ?s", $ip_num);
	$start_time = db_get_field("SELECT start_time FROM ?:lh_visitors WHERE session_id = ?s", $sess_id);
	if ($start_time > 0)
	{
		$requests = db_get_array("SELECT r.timestamp, r.url, r.title FROM ?:stat_sessions AS s, ?:stat_requests AS r WHERE r.sess_id = s.sess_id AND s.session = ?s AND r.timestamp >= ?i", $sess_id, $start_time);

		if (!empty($requests))
		{
			foreach ($requests as $request)
			{
				fn_add_log_event(array('sess_id' => $sess_id, 'date' => $request['timestamp']), LH_LOG_PAGE_CHANGED, $visitor_data['operator_id'], $request['url'], $request['title']);
			}
		}
	}

	// *** Update activity timestamp ***
	$data = array (
		'start_time' => TIME,
		'obsolete_time' => TIME + Registry::get('livehelp_config.freshness_time_visitor')
	);
//	db_query("UPDATE ?:lh_visitors SET ?u WHERE ip = ?s", $data, $ip_num);
	db_query("UPDATE ?:lh_visitors SET ?u WHERE session_id = ?s", $data, $sess_id);

	// remove obsolete visotrs
	cleanup_obsolete_visitors();

	// *** check invitation ***
	if (!empty($visitor_data['operator_id']) && $visitor_data['status'] != LH_STATUS_CHATTING) {
//		$operator_data = db_get_row("SELECT ?:lh_visitors.operator_id, ?:lh_operators.name as operator_name FROM ?:lh_visitors LEFT JOIN ?:lh_operators ON ?:lh_operators.operator_id = ?:lh_visitors.operator_id WHERE ?:lh_visitors.ip = ?s AND ?:lh_visitors.status = ?i", $ip_num, LH_STATUS_REQUEST_SENT);
		$operator_data = db_get_row("SELECT ?:lh_visitors.operator_id, ?:lh_operators.name as operator_name FROM ?:lh_visitors LEFT JOIN ?:lh_operators ON ?:lh_operators.operator_id = ?:lh_visitors.operator_id WHERE ?:lh_visitors.session_id = ?s AND ?:lh_visitors.status = ?i", $sess_id, LH_STATUS_REQUEST_SENT);

		$condition = db_quote("message_id > ?i AND ((to_id = ?i AND from_id = ?i AND direction = ?i) OR (from_id = ?i AND to_id = ?i AND direction = ?i))", $visitor_data['last_message'], $visitor_data['visitor_id'], $visitor_data['operator_id'], LH_DIRECTION_OPERATOR_VISITOR, $visitor_data['visitor_id'], $visitor_data['operator_id'], LH_DIRECTION_VISITOR_OPERATOR);

		$invitation_messages = db_get_array("SELECT message_id, text FROM ?:lh_messages WHERE $condition");

		$inv = '';
		if (!empty($invitation_messages)) {
			foreach ($invitation_messages as $k => $inv_mes) {

				$text = $inv_mes['text'];

				// uploaded file
				if (preg_match("/^<file>.*<\/file>$/", $text)) {
					$text = preg_replace("/^<file>[^_]+_(.*)<\/file>$/", '$1', $text);
				}

				$inv .= "<inv_message_" . $k . ">$text</inv_message_" . $k . ">";
			}

			$data = array (
				'last_message' => $inv_mes['message_id']
			);
			$ert = $invitation_messages[0]['message_id'];
			$xml_result[] = "<invitation_messages><first_inv_message_id>$ert</first_inv_message_id>$inv</invitation_messages>";

//			db_query("UPDATE ?:lh_visitors SET ?u WHERE ip = ?s", $data, $ip_num);
			db_query("UPDATE ?:lh_visitors SET ?u WHERE session_id = ?s", $data, $sess_id);
		}

		if (isset($_REQUEST['check_invitation']) && $_REQUEST['check_invitation'] == 'Y') {
			if (!empty($operator_data)) {
				$visitor_name = (isset($_SESSION['visitor_chat_name'])) ? $_SESSION['visitor_chat_name'] : '';
				$xml_result[] = "<chat_data>".
						'<visitor_name>'.$visitor_data['chat_name'].'</visitor_name>'.
						'<operator_id>'.$operator_data['operator_id'].'</operator_id>'.
						'<operator_name>' . htmlspecialchars($operator_data['operator_name']) . '</operator_name>'.
						'</chat_data>';
			}
		}
	}

	// *** operators statistics ***
	$total = db_get_field("SELECT COUNT(*) FROM ?:lh_operators WHERE ?i <= obsolete_time", TIME);
	if (!empty($total)) {
		$busy = db_get_field("SELECT COUNT(*) FROM ?:lh_visitors WHERE ?i <= obsolete_time AND status = ?s GROUP BY operator_id", TIME, LH_STATUS_CHATTING);
	} else {
		$busy = 0;
	}

	$state = $total . ',' . ($total - $busy);
	$t_data = array (
		'chat_track' => '',
		'track' => $state,
		'avail' => 'Y',
		'user_type' => 'V',
		'user_id' => $visitor_id
	);
	db_query("DELETE FROM ?:lh_track WHERE user_id = ?i AND user_type = ?s", $visitor_id, 'V');
	db_query("REPLACE INTO ?:lh_track ?e ", $t_data);
	$xml_result[] = "<operators><total>$total</total><free>" . max(0, ($total - $busy)) . "</free></operators>";

	fn_lh_response($xml_result);
}

function fn_lh_response($result)
{
	if (!empty($result)) {
		echo "<?xml version=\"1.0\" encoding=\"" . CHARSET . "\"?><result>" . implode('', $result) . "</result>";
		exit;
	}
}

die;
?>